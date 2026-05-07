# -*- coding: utf-8 -*-
"""
交互式算法模块 - 独立封装

将交互式NSGA-II算法封装为独立模块，可在任何优化模式中使用。

核心功能:
1. 用户偏好管理
2. 交互式选择算子
3. 多用户偏好合并
4. 交互流程控制

@author: SEIMS Team
@version: 1.0
@date: 2026-03-19
"""

import os
import sys
import logging
from typing import Dict, List, Any, Optional, Tuple, Callable

# 确保 scenario_analysis 目录在 sys.path 中，使 deap_tool 可正确导入
_this_dir = os.path.dirname(os.path.abspath(__file__))
if _this_dir not in sys.path:
    sys.path.insert(0, _this_dir)

# 导入DEAP工具函数
from deap_tool import (
    selNSGA2_prefer,
    selNSGA2_multiuser_prefer,
    interactive_selection,
    update_preference_params,
    merge_multiuser_prefs
)


class InteractiveAlgorithm:
    """
    交互式算法封装类

    提供统一的交互式NSGA-II算法接口，可在spatial/temporal/spatio_temporal模式中使用。
    """

    def __init__(self,
                 enable_interactive: bool = False,
                 interactive_interval: int = 10,
                 users: Optional[Dict] = None,
                 preference_fusion_strategy: str = 'merge_preferences',
                 logger: Optional[logging.Logger] = None):
        """
        初始化交互式算法

        Args:
            enable_interactive: 是否启用交互式算法
            interactive_interval: 交互间隔代数
            users: 用户配置字典 {user_id: {history_good, history_bad, history_bad_reasons, preference_param}}
            logger: 日志记录器
        """
        self.enable_interactive = enable_interactive
        self.interactive_interval = interactive_interval
        self.users = users if users else {}
        self.preference_fusion_strategy = preference_fusion_strategy or 'merge_preferences'
        self.logger = logger if logger else logging.getLogger(__name__)

        # 合并后的偏好参数
        self.merged_prefs = []
        self.user_prefs_list = []

        # 初始化
        if self.enable_interactive and self.users:
            self._initialize_users()
            self.logger.info(f"Interactive algorithm initialized with {len(self.users)} users")
        else:
            self.logger.info("Interactive algorithm disabled")

    def _initialize_users(self):
        """初始化用户配置，确保所有必需字段存在"""
        for user_id, user in self.users.items():
            if 'history_good' not in user:
                user['history_good'] = []
            if 'history_bad' not in user:
                user['history_bad'] = []
            if 'history_bad_reasons' not in user:
                user['history_bad_reasons'] = []
            if 'preference_param' not in user:
                user['preference_param'] = {}

        # 初始合并偏好
        self._update_merged_preferences()

    def _update_merged_preferences(self):
        """更新合并后的多用户偏好参数"""
        if not self.enable_interactive or not self.users:
            self.merged_prefs = []
            self.user_prefs_list = []
            return

        user_prefs_list = []
        for user_id, user in self.users.items():
            user_prefs_list.append(user['preference_param'])

        self.user_prefs_list = user_prefs_list
        if user_prefs_list:
            self.merged_prefs = merge_multiuser_prefs(user_prefs_list)
            self.logger.debug(f"Merged preferences updated: {len(self.merged_prefs)} metrics")
        else:
            self.merged_prefs = []
            self.user_prefs_list = []

    def should_interact(self, generation: int) -> bool:
        """
        判断当前代是否需要进行交互

        Args:
            generation: 当前代数

        Returns:
            是否需要交互
        """
        if not self.enable_interactive:
            return False

        if not self.users:
            return False

        # 第0代不交互，之后每隔interactive_interval代交互一次
        if generation == 0:
            return False

        return (generation % self.interactive_interval) == 0

    def get_selection_function(self) -> Callable:
        """
        获取选择函数

        Returns:
            选择函数（selNSGA2_prefer或标准selNSGA2）
        """
        if self.enable_interactive and self.preference_fusion_strategy == 'select_then_merge' and self.user_prefs_list:
            return toolbox.select_multiuser_prefer(
                population, k, user_preference_params=self.user_prefs_list
            )
        elif self.enable_interactive and self.merged_prefs:
            # 返回带偏好的选择函数
            def select_with_preference(pop, k):
                return selNSGA2_prefer(pop, k, preference_params=self.merged_prefs)
            return select_with_preference
        else:
            # 返回标准NSGA-II选择（需要外部提供）
            return None  # 调用方应使用toolbox.select

    def run_interaction(self, population: List, generation: int) -> bool:
        """
        执行交互流程

        Args:
            population: 当前种群
            generation: 当前代数

        Returns:
            是否成功执行交互
        """
        if not self.should_interact(generation):
            return False

        self.logger.info(f"=" * 60)
        self.logger.info(f"Interactive session at generation {generation}")
        self.logger.info(f"=" * 60)

        # 对每个用户执行交互
        for user_id, user in self.users.items():
            self.logger.info(f"User {user_id} interaction:")

            # 执行交互选择（传入 preference_param 以动态显示用户关注的指标）
            good_pop, bad_pop, bad_reasons, next_interval = interactive_selection(
                population,
                user['history_good'],
                user['history_bad'],
                user['history_bad_reasons'],
                user_id,
                preference_param=user.get('preference_param', None)
            )
            # 根据用户输入更新下次交互代数
            if next_interval > 0:
                self.interactive_interval = next_interval

            # 更新历史记录
            user['history_good'] = good_pop
            user['history_bad'] = bad_pop
            user['history_bad_reasons'] = bad_reasons

            # 更新偏好参数
            updated_params = update_preference_params(
                user['history_good'],
                user['history_bad'],
                user['preference_param'],
                user['history_bad_reasons']
            )
            user['preference_param'] = updated_params

            self.logger.info(f"User {user_id}: {len(good_pop)} good, {len(bad_pop)} bad scenarios")

        # 重新合并偏好
        self._update_merged_preferences()

        self.logger.info(f"Interactive session completed")
        self.logger.info(f"=" * 60)

        return True

    def register_to_toolbox(self, toolbox):
        """
        将交互式选择函数注册到DEAP toolbox

        Args:
            toolbox: DEAP toolbox对象
        """
        if self.enable_interactive:
            toolbox.register('select_prefer', selNSGA2_prefer)
            toolbox.register('select_multiuser_prefer', selNSGA2_multiuser_prefer)
            self.logger.info("Registered select_prefer to toolbox")

    def select_population(self, toolbox, population: List, k: int) -> List:
        """
        选择种群（自动判断使用标准或偏好选择）

        Args:
            toolbox: DEAP toolbox对象
            population: 当前种群
            k: 选择个体数

        Returns:
            选择后的种群
        """
        if self.enable_interactive and self.merged_prefs:
            # 使用偏好选择
            return toolbox.select_prefer(population, k, preference_params=self.merged_prefs)
        else:
            # 使用标准选择
            return toolbox.select(population, k)

    def get_config_summary(self) -> Dict:
        """
        获取配置摘要

        Returns:
            配置摘要字典
        """
        return {
            'enable_interactive': self.enable_interactive,
            'interactive_interval': self.interactive_interval,
            'num_users': len(self.users),
            'has_merged_prefs': len(self.merged_prefs) > 0,
            'preference_fusion_strategy': self.preference_fusion_strategy
        }

    def __repr__(self):
        return (f"InteractiveAlgorithm(enable={self.enable_interactive}, "
                f"interval={self.interactive_interval}, users={len(self.users)})")


# 便捷函数
def create_interactive_algorithm(config, logger=None) -> InteractiveAlgorithm:
    """
    从配置对象创建交互式算法实例

    Args:
        config: 配置对象（需要有enable_interactive, interactive_interval, users属性）
        logger: 日志记录器

    Returns:
        InteractiveAlgorithm实例
    """
    enable_interactive = getattr(config, 'enable_interactive', False)
    interactive_interval = getattr(config, 'interactive_interval', 10)
    users = getattr(config, 'users', {})
    preference_fusion_strategy = getattr(config, 'preference_fusion_strategy', 'merge_preferences')

    return InteractiveAlgorithm(
        enable_interactive=enable_interactive,
        interactive_interval=interactive_interval,
        users=users,
        preference_fusion_strategy=preference_fusion_strategy,
        logger=logger
    )


def create_interactive_algorithm_from_dict(config_dict: Dict, logger=None) -> InteractiveAlgorithm:
    """
    从配置字典创建交互式算法实例

    Args:
        config_dict: 配置字典
        logger: 日志记录器

    Returns:
        InteractiveAlgorithm实例
    """
    enable_interactive = config_dict.get('enable_interactive', False)
    interactive_interval = config_dict.get('interactive_interval', 10)
    users = config_dict.get('users', {})
    preference_fusion_strategy = config_dict.get('preference_fusion_strategy', 'merge_preferences')

    return InteractiveAlgorithm(
        enable_interactive=enable_interactive,
        interactive_interval=interactive_interval,
        users=users,
        preference_fusion_strategy=preference_fusion_strategy,
        logger=logger
    )
