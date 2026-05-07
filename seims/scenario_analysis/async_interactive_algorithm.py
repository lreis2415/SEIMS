# -*- coding: utf-8 -*-
"""
异步交互式算法模块

扩展交互式NSGA-II算法，支持异步交互模式。

与同步模式区别：
- 到达交互世代时，保存 checkpoint 并写入 WAIT_INTERACTION signal 文件
- 抛出 AsyncInteractionPending 异常，进程退出（exit code 42）
- 外部系统：用 generate_continue.py 读取 checkpoint + 用户分类 → 写入 CONTINUE JSON
- 用 --resume 重启优化器，从 checkpoint 断点继续，应用新偏好参数

@author: SEIMS Team
@version: 2.1
@date: 2026-04-21
"""

import os
import sys
import json
import time
import pickle
import random
import logging
from typing import Dict, List, Any, Optional, Tuple, Callable

# 确保 scenario_analysis 目录在 sys.path 中
_this_dir = os.path.dirname(os.path.abspath(__file__))
if _this_dir not in sys.path:
    sys.path.insert(0, _this_dir)

# 导入基类和工具函数
from interactive_algorithm import InteractiveAlgorithm
from deap_tool import (
    selNSGA2_prefer,
    interactive_selection,
    update_preference_params,
    merge_multiuser_prefs
)

# 退出码：优化器正常暂停等待用户交互（非错误）
ASYNC_INTERACTION_EXIT_CODE = 42


class AsyncInteractionPending(Exception):
    """
    异步交互暂停信号。

    当优化到达交互节点，保存 checkpoint 和 WAIT_INTERACTION 信号后抛出此异常。
    调用方（NSGA2 main / unified_optimizer_v2）捕获后以 exit code 42 退出进程。
    外部系统完成用户交互后，用 generate_continue.py 生成 CONTINUE JSON，
    再以 --resume 重启优化器从断点继续。

    Attributes:
        generation: 触发交互的代数
        task_id: 任务标识
        checkpoint_file: 保存的 checkpoint 路径
        signal_file: 写入的 WAIT_INTERACTION 信号路径
    """
    def __init__(self, generation: int, task_id: str,
                 checkpoint_file: str = '', signal_file: str = ''):
        super().__init__(
            f"Async interaction pending at generation {generation} "
            f"(task={task_id}). "
            f"Run generate_continue.py then restart with --resume."
        )
        self.generation = generation
        self.task_id = task_id
        self.checkpoint_file = checkpoint_file
        self.signal_file = signal_file


class AsyncInteractiveAlgorithm(InteractiveAlgorithm):
    """
    异步交互式算法封装类

    继承自 InteractiveAlgorithm，扩展异步交互功能。
    当 enable_async=True 时，run_interaction() 会：
    1. 保存 checkpoint 到文件
    2. 写入 WAIT_INTERACTION signal 文件
    3. 写入 SOLUTIONS 文件供前端展示
    4. 抛出 AsyncInteractionPending（进程以 exit code 42 退出）
    续跑时（--resume 模式）：
    5. 读取 CONTINUE JSON，应用新偏好参数，从断点继续
    """

    def __init__(self,
                 enable_interactive: bool = False,
                 interactive_interval: int = 10,
                 users: Optional[Dict] = None,
                 logger: Optional[logging.Logger] = None,
                 enable_async: bool = False,
                 task_id: str = 'default_task',
                 signal_dir: str = '/tmp/signals',
                 checkpoint_dir: str = '/tmp/checkpoints',
                 preference_fusion_strategy: str = 'merge_preferences',
                 timeout_seconds: int = 3600):
        """
        初始化异步交互式算法

        Args:
            enable_interactive: 是否启用交互式算法
            interactive_interval: 交互间隔代数
            users: 用户配置字典
            logger: 日志记录器
            enable_async: 是否启用异步模式
            task_id: 任务标识符（用于signal文件命名）
            signal_dir: signal文件目录
            checkpoint_dir: checkpoint文件目录
            timeout_seconds: 等待用户提交的超时时间
        """
        super().__init__(
            enable_interactive=enable_interactive,
            interactive_interval=interactive_interval,
            users=users,
            preference_fusion_strategy=preference_fusion_strategy,
            logger=logger
        )

        self.enable_async = enable_async
        self.task_id = task_id
        self.signal_dir = signal_dir
        self.checkpoint_dir = checkpoint_dir
        self.timeout_seconds = timeout_seconds

        # 当前偏好参数（用于checkpoint保存）
        self.current_prefs = {}

        # 创建必要的目录
        os.makedirs(self.signal_dir, exist_ok=True)
        os.makedirs(self.checkpoint_dir, exist_ok=True)

        if self.enable_async:
            self.logger.info(f"AsyncInteractiveAlgorithm initialized: task_id={task_id}, signal_dir={signal_dir}")

    def _get_signal_path(self, gen: int) -> Tuple[str, str, str]:
        """获取signal文件路径"""
        wait_file = os.path.join(self.signal_dir, f"WAIT_INTERACTION_{self.task_id}_gen{gen}.signal")
        continue_file = os.path.join(self.signal_dir, f"CONTINUE_{self.task_id}_gen{gen}.json")
        solutions_file = os.path.join(self.signal_dir, f"SOLUTIONS_{self.task_id}_gen{gen}.json")
        return wait_file, continue_file, solutions_file

    def _get_checkpoint_path(self, gen: int) -> str:
        """获取checkpoint文件路径"""
        return os.path.join(self.checkpoint_dir, f"checkpoint_{self.task_id}_gen{gen}.pkl")

    def _build_population_summary(self, pop: List) -> Dict:
        """构建种群摘要信息"""
        pop_ids = [ind.id for ind in pop]

        # 构建方案列表（用于前端展示）
        display_solutions = []
        for ind in pop[:30]:  # 只取前30个方案
            try:
                sol = {
                    'id': ind.id,
                    'genes': [float(x) for x in ind],
                    'economy': getattr(ind, 'economy', ind.fitness.values[0] if ind.fitness.valid else None),
                    'environment': getattr(ind, 'environment', ind.fitness.values[1] if ind.fitness.valid else None),
                }
                display_solutions.append(sol)
            except:
                display_solutions.append({'id': ind.id, 'genes': [float(x) for x in ind]})

        # 获取目标范围
        try:
            costs = [ind.fitness.values[0] for ind in pop if ind.fitness.valid]
            envs = [ind.fitness.values[1] for ind in pop if ind.fitness.valid]
            obj_range = {
                'cost': [min(costs), max(costs)] if costs else [0, 1],
                'env': [min(envs), max(envs)] if envs else [0, 1]
            }
        except:
            obj_range = {'cost': [0, 1], 'env': [0, 1]}

        return {
            'size': len(pop),
            'pop_ids': pop_ids,
            'objectives_range': obj_range
        }

    def _write_wait_signal(self, pop: List, gen: int) -> str:
        """写入 WAIT_INTERACTION signal 文件"""
        wait_file, continue_file, solutions_file = self._get_signal_path(gen)
        checkpoint_file = self._get_checkpoint_path(gen)

        # 构建signal数据
        signal_data = {
            'type': 'WAIT_INTERACTION',
            'task_id': self.task_id,
            'generation': gen,
            'interactive_interval': self.interactive_interval,
            'population_summary': self._build_population_summary(pop),
            'timestamp': time.strftime('%Y-%m-%dT%H:%M:%S')
        }

        with open(wait_file, 'w') as f:
            json.dump(signal_data, f, indent=2)

        # 写入方案文件供前端展示
        solutions_data = {
            'generation': gen,
            'task_id': self.task_id,
            'solutions': self._build_population_summary(pop)['pop_ids'],
            'total_count': len(pop),
            'timestamp': time.strftime('%Y-%m-%dT%H:%M:%S')
        }
        with open(solutions_file, 'w') as f:
            json.dump(solutions_data, f, indent=2)

        self.logger.info(f"[Async] Signal written: {wait_file}")
        self.logger.info(f"[Async] Solutions written: {solutions_file}")

        return wait_file

    def _save_checkpoint(self, pop: List, gen: int) -> str:
        """保存checkpoint文件"""
        checkpoint_file = self._get_checkpoint_path(gen)

        data = {
            'population': pop,
            'generation': gen,
            'random_state': random.getstate(),
            'preference_params': self.current_prefs,
            'preference_fusion_strategy': self.preference_fusion_strategy,
            'interactive_interval': self.interactive_interval,
            'users': self.users,
            'timestamp': time.strftime('%Y-%m-%dT%H:%M:%S')
        }

        try:
            with open(checkpoint_file, 'wb') as f:
                pickle.dump(data, f)
            self.logger.info(f"[Async] Checkpoint saved: {checkpoint_file}")
            return checkpoint_file
        except Exception as e:
            self.logger.error(f"[Async] Failed to save checkpoint: {e}")
            return None

    def _load_checkpoint(self, gen: int) -> Optional[Dict]:
        """加载checkpoint文件"""
        checkpoint_file = self._get_checkpoint_path(gen)

        if not os.path.exists(checkpoint_file):
            return None

        try:
            with open(checkpoint_file, 'rb') as f:
                data = pickle.load(f)
            self.logger.info(f"[Async] Checkpoint loaded: {checkpoint_file}, gen={data.get('generation')}")
            return data
        except Exception as e:
            self.logger.error(f"[Async] Failed to load checkpoint: {e}")
            return None

    def _wait_for_continue_signal(self, gen: int) -> Optional[Dict]:
        """等待 CONTINUE signal 文件"""
        wait_file, continue_file, solutions_file = self._get_signal_path(gen)

        start_time = time.time()
        self.logger.info(f"[Async] Waiting for CONTINUE signal: {continue_file}")

        while time.time() - start_time < self.timeout_seconds:
            if os.path.exists(continue_file):
                try:
                    with open(continue_file, 'r') as f:
                        continue_data = json.load(f)

                    # 清理signal文件
                    for f in [wait_file, continue_file, solutions_file]:
                        if os.path.exists(f):
                            os.remove(f)

                    self.logger.info(f"[Async] Received CONTINUE signal at generation {gen}")
                    return continue_data

                except Exception as e:
                    self.logger.error(f"[Async] Error reading continue file: {e}")
                    time.sleep(2)
                    continue

            time.sleep(2)  # 每2秒轮询一次

        self.logger.warning(f"[Async] Timeout waiting for CONTINUE at generation {gen}")
        return None

    def _update_preferences_from_continue(self, continue_data: Dict) -> bool:
        """从 CONTINUE signal 更新偏好参数。

        支持两种 users_preferences 格式：
          - 扁平格式（旧）: [{"economy": [80, "less", 20], ...}, ...]
          - 带 user_id 格式（generate_continue.py 输出）:
            [{"user_id": "user1", "preference_params": {"economy": [...], ...}}, ...]
        """
        if not continue_data:
            return False

        raw_prefs = continue_data.get('users_preferences', [])
        merge_method = continue_data.get('merge_method', self.preference_fusion_strategy)

        if not raw_prefs:
            return False

        # 标准化：提取纯 preference_param 列表（用于 merge_multiuser_prefs）
        pref_list = []
        for item in raw_prefs:
            if 'preference_params' in item:
                # generate_continue.py 格式：{user_id, preference_params}
                params = item['preference_params']
            else:
                # 扁平格式：直接是 {metric: [h, dir, r]}
                params = item

            # JSON 反序列化后 list → tuple
            normalized = {}
            for metric, val in params.items():
                if metric == 'bmp_rules':
                    normalized[metric] = val
                elif isinstance(val, (list, tuple)) and len(val) == 3:
                    normalized[metric] = tuple(val)
                else:
                    normalized[metric] = val
            pref_list.append(normalized)

            # 同步更新 self.users 中对应用户的 preference_param
            user_id = item.get('user_id')
            if user_id and user_id in self.users:
                self.users[user_id]['preference_param'] = normalized

        if pref_list:
            self.user_prefs_list = pref_list
            self.current_prefs = merge_multiuser_prefs(pref_list)
            self.merged_prefs = self.current_prefs
            if merge_method in ('merge_preferences', 'select_then_merge'):
                self.preference_fusion_strategy = merge_method
            self.logger.info(
                f"[Async] Preferences updated: {len(pref_list)} users, method={merge_method}"
            )
            return True

        return False

    def _restore_random_state(self, random_state):
        """恢复随机状态"""
        if random_state:
            random.setstate(random_state)

    def apply_continue(self, continue_data: Dict) -> bool:
        """
        续跑入口：读取 CONTINUE JSON 数据，更新偏好参数。

        由外部（unified_optimizer_v2 --resume 模式）在重启后调用。

        Args:
            continue_data: CONTINUE JSON 内容（由 generate_continue.py 生成）

        Returns:
            是否成功更新偏好
        """
        return self._update_preferences_from_continue(continue_data)

    def run_async_interaction(self, pop: List, gen: int) -> bool:
        """
        执行异步交互流程。

        保存 checkpoint + 写入 WAIT_INTERACTION 信号后，
        抛出 AsyncInteractionPending 异常，进程将以 exit code 42 退出。

        外部流程：
          1. 用 generate_continue.py 生成 CONTINUE JSON
          2. 用 --resume 重启优化器，从断点继续

        Args:
            pop: 当前种群
            gen: 当前代数

        Raises:
            AsyncInteractionPending: 触发异步暂停
        """
        if not self.should_interact(gen):
            return False

        self.logger.info(f"{'=' * 60}")
        self.logger.info(f"[Async] Interactive session at generation {gen}")
        self.logger.info(f"{'=' * 60}")

        # 1. 保存 checkpoint
        checkpoint_file = self._save_checkpoint(pop, gen)

        # 2. 写入 WAIT_INTERACTION signal
        signal_file = self._write_wait_signal(pop, gen)

        self.logger.info(f"[Async] Checkpoint: {checkpoint_file}")
        self.logger.info(f"[Async] Signal:     {signal_file}")
        self.logger.info(
            f"[Async] Run generate_continue.py then restart with --resume to continue."
        )

        # 3. 抛出异常通知调用方退出
        raise AsyncInteractionPending(
            generation=gen,
            task_id=self.task_id,
            checkpoint_file=checkpoint_file or '',
            signal_file=signal_file or ''
        )

    def run_interaction(self, pop: List, generation: int) -> bool:
        """
        执行交互流程（重写基类方法）

        根据 enable_async 决定使用同步或异步模式。

        Args:
            pop: 当前种群
            generation: 当前代数

        Returns:
            是否成功执行交互
        """
        if not self.should_interact(generation):
            return False

        if self.enable_async:
            return self.run_async_interaction(pop, generation)
        else:
            # 同步模式调用父类方法
            return super().run_interaction(pop, generation)

    def load_from_checkpoint(self, gen: int) -> Optional[List]:
        """
        从checkpoint加载种群

        Args:
            gen: 要加载的代数

        Returns:
            种群列表，如果不存在则返回None
        """
        data = self._load_checkpoint(gen)
        if data:
            # 恢复用户状态
            if 'users' in data:
                self.users = data['users']
            if 'preference_params' in data:
                self.current_prefs = data['preference_params']
                self.merged_prefs = self.current_prefs
            if 'preference_fusion_strategy' in data:
                self.preference_fusion_strategy = data['preference_fusion_strategy']
            if 'interactive_interval' in data:
                self.interactive_interval = data['interactive_interval']

            # 恢复随机状态
            if 'random_state' in data:
                self._restore_random_state(data['random_state'])

            return data.get('population')
        return None

    def get_latest_checkpoint_gen(self) -> Optional[int]:
        """获取最新的checkpoint代数"""
        if not os.path.exists(self.checkpoint_dir):
            return None

        gens = []
        prefix = f"checkpoint_{self.task_id}_gen"
        for f in os.listdir(self.checkpoint_dir):
            if f.startswith(prefix) and f.endswith('.pkl'):
                try:
                    gen = int(f.replace(prefix, '').replace('.pkl', ''))
                    gens.append(gen)
                except:
                    pass

        return max(gens) if gens else None

    def has_checkpoint(self, gen: int) -> bool:
        """检查是否存在指定世代的checkpoint"""
        return os.path.exists(self._get_checkpoint_path(gen))

    def clear_signals(self, gen: int):
        """清除指定世代的所有signal和checkpoint文件"""
        # 清除signal文件
        wait_file, continue_file, solutions_file = self._get_signal_path(gen)
        for f in [wait_file, continue_file, solutions_file]:
            if os.path.exists(f):
                os.remove(f)

        # 清除checkpoint文件
        checkpoint_file = self._get_checkpoint_path(gen)
        if os.path.exists(checkpoint_file):
            os.remove(checkpoint_file)

        self.logger.info(f"[Async] Cleared all files for generation {gen}")


# 便捷函数
def create_async_interactive_algorithm(config, logger=None) -> AsyncInteractiveAlgorithm:
    """
    从配置对象创建异步交互式算法实例

    Args:
        config: 配置对象
        logger: 日志记录器

    Returns:
        AsyncInteractiveAlgorithm实例
    """
    enable_interactive = getattr(config, 'enable_interactive', False)
    interactive_interval = getattr(config, 'interactive_interval', 30)
    users = getattr(config, 'users', {})
    enable_async = getattr(config, 'enable_async', False)
    task_id = getattr(config, 'task_id', 'default_task')
    signal_dir = getattr(config, 'signal_dir', '/tmp/signals')
    checkpoint_dir = getattr(config, 'checkpoint_dir', '/tmp/checkpoints')
    preference_fusion_strategy = getattr(config, 'preference_fusion_strategy', 'merge_preferences')

    return AsyncInteractiveAlgorithm(
        enable_interactive=enable_interactive,
        interactive_interval=interactive_interval,
        users=users,
        logger=logger,
        enable_async=enable_async,
        task_id=task_id,
        signal_dir=signal_dir,
        checkpoint_dir=checkpoint_dir,
        preference_fusion_strategy=preference_fusion_strategy
    )


def create_async_interactive_algorithm_from_dict(config_dict: Dict, logger=None) -> AsyncInteractiveAlgorithm:
    """
    从配置字典创建异步交互式算法实例

    Args:
        config_dict: 配置字典
        logger: 日志记录器

    Returns:
        AsyncInteractiveAlgorithm实例
    """
    enable_interactive = config_dict.get('enable_interactive', False)
    interactive_interval = config_dict.get('interactive_interval', 30)
    users = config_dict.get('users', {})
    enable_async = config_dict.get('enable_async', False)
    task_id = config_dict.get('task_id', 'default_task')
    signal_dir = config_dict.get('signal_dir', '/tmp/signals')
    checkpoint_dir = config_dict.get('checkpoint_dir', '/tmp/checkpoints')
    preference_fusion_strategy = config_dict.get('preference_fusion_strategy', 'merge_preferences')

    return AsyncInteractiveAlgorithm(
        enable_interactive=enable_interactive,
        interactive_interval=interactive_interval,
        users=users,
        logger=logger,
        enable_async=enable_async,
        task_id=task_id,
        signal_dir=signal_dir,
        checkpoint_dir=checkpoint_dir,
        preference_fusion_strategy=preference_fusion_strategy
    )
