# -*- coding: utf-8 -*-
"""
统一优化器入口 V2 - 修复版本

修复了原版本的以下问题：
1. 模块导入错误（bmps_spatio-temporal_nsga2.py文件名带连字符）
2. toolbox注册逻辑错误
3. 增强配置验证
4. 添加日志和错误处理

@author: SEIMS Team
@version: 2.0
"""

import os
import sys
import json
import logging
import importlib
from typing import Dict, List, Any, Optional, Union, Tuple

# 确保可以导入scenario_analysis模块
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.insert(0, current_dir)

from unified_config import UnifiedConfig
from function_registry import FUNCTION_REGISTRY


class UnifiedOptimizerV2:
    """
    统一优化器入口 V2 - 修复版本

    主要改进：
    1. 修复模块导入问题
    2. 正确的toolbox注册
    3. 完善的错误处理
    4. 结构化日志记录
    """

    def __init__(self, config: Union[dict, str, UnifiedConfig],
                 config_file: Optional[str] = None,
                 output_dir: Optional[str] = None,
                 log_level: str = 'INFO'):
        """
        初始化统一优化器

        Args:
            config: JSON配置字典、配置文件路径或UnifiedConfig对象
            config_file: 配置文件路径(可选)
            output_dir: 输出目录(可选)
            log_level: 日志级别
        """
        # 设置日志
        self._setup_logging(log_level)
        self.logger = logging.getLogger(__name__)

        # 解析配置
        try:
            if isinstance(config, UnifiedConfig):
                self.cfg = config
            elif isinstance(config, str):
                if os.path.isfile(config):
                    self.cfg = UnifiedConfig.from_json_file(config)
                    config_file = config
                else:
                    self.cfg = UnifiedConfig(json.loads(config))
            elif isinstance(config, dict):  # NEW: Handle dict input
                self.cfg = UnifiedConfig(config)
            else:
                self.cfg = config
        except Exception as e:
            self.logger.error(f"Failed to parse configuration: {e}")
            raise

        # 设置输出目录
        if output_dir:
            self.cfg.output_dir = output_dir
        elif config_file:
            self.cfg.output_dir = os.path.join(os.path.dirname(config_file), 'output')

        # 验证配置（区分 WARNING 提示和实际错误）
        all_messages = self.cfg.validate()
        warnings = [m for m in all_messages if m.startswith('[WARNING]')]
        errors = [m for m in all_messages if not m.startswith('[WARNING]')]
        for w in warnings:
            self.logger.warning(w)
        if errors:
            error_msg = "Configuration validation failed:\n" + "\n".join(f"  - {e}" for e in errors)
            self.logger.error(error_msg)
            raise ValueError(error_msg)

        self.logger.info(f"Configuration validated successfully")
        self.logger.info(f"Mode: {self.cfg.mode}, Spatial Unit: {self.cfg.spatial_unit}, "
                        f"Config Method: {self.cfg.config_method}")

        # 获取函数映射
        self.funcs = FUNCTION_REGISTRY.get_functions(
            self.cfg.mode,
            self.cfg.config_method
        )

        # 场景对象
        self.scenario = None

    def _setup_logging(self, log_level: str):
        """设置日志系统"""
        logging.basicConfig(
            level=getattr(logging, log_level.upper()),
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.StreamHandler(sys.stdout),
                logging.FileHandler('seims_optimizer.log', mode='a')
            ]
        )

    def _load_scenario_functions(self):
        """加载scenario模块的函数"""
        try:
            # NEW: Use relative import
            from spatialunits.scenario import (
                initialize_scenario,
                initialize_scenario_s_t,
                initialize_scenario_with_bmps_order,
                scenario_effectiveness,
                scenario_effectiveness_with_bmps_order
            )

            return {
                'initialize_scenario': initialize_scenario,
                'initialize_scenario_s_t': initialize_scenario_s_t,
                'initialize_scenario_with_bmps_order': initialize_scenario_with_bmps_order,
                'scenario_effectiveness': scenario_effectiveness,
                'scenario_effectiveness_with_bmps_order': scenario_effectiveness_with_bmps_order
            }
        except ImportError as e:
            self.logger.error(f"Failed to import scenario functions: {e}")
            raise

    def _load_userdef_functions(self):
        """加载userdef模块的函数"""
        try:
            # NEW: Use relative import
            from spatialunits.userdef import (
                crossover_slppos,
                crossover_updown,
                crossover_rdm,
                mutate_rule_s,
                mutate_rule_s_t,
                mutate_with_bmps_order,
                mutate_rdm
            )

            return {
                'crossover_slppos': crossover_slppos,
                'crossover_updown': crossover_updown,
                'crossover_rdm': crossover_rdm,
                'mutate_rule_s': mutate_rule_s,
                'mutate_rule_s_t': mutate_rule_s_t,
                'mutate_with_bmps_order': mutate_with_bmps_order,
                'mutate_rdm': mutate_rdm
            }
        except ImportError as e:
            self.logger.error(f"Failed to import userdef functions: {e}")
            raise

    def setup_toolbox(self, toolbox) -> None:
        """
        配置DEAP工具箱 - 修复版本

        修复：正确使用initIterateWithCfg和initRepeatWithCfg

        Args:
            toolbox: DEAP工具箱对象
        """
        # NEW: Use relative import
        from userdef import initIterateWithCfg, initRepeatWithCfg
        from deap import creator

        # 加载函数
        scenario_funcs = self._load_scenario_functions()
        userdef_funcs = self._load_userdef_functions()
        all_funcs = {**scenario_funcs, **userdef_funcs}

        # 获取函数名
        init_name = self.funcs['initializer']
        eval_name = self.funcs['evaluator']
        crossover_name = self.funcs.get('crossover_func', 'crossover_rdm')
        mutate_name = self.funcs.get('mutator', 'mutate_rdm')

        # 获取实际函数
        initializer_func = all_funcs.get(init_name)
        evaluator_func = all_funcs.get(eval_name)
        crossover_func = all_funcs.get(crossover_name)
        mutate_func = all_funcs.get(mutate_name)

        if not all([initializer_func, evaluator_func]):
            raise ValueError(f"Failed to load required functions: init={init_name}, eval={eval_name}")

        self.logger.info(f"Loaded functions: init={init_name}, eval={eval_name}, "
                        f"crossover={crossover_name}, mutate={mutate_name}")

        # NEW: Wrap evaluator with surrogate model if enabled
        if self.cfg.use_surrogate:
            evaluator_func = self._create_surrogate_evaluator(evaluator_func)
            self.logger.info("Surrogate model evaluation enabled")

        # 正确注册到toolbox
        toolbox.register('gene_values', initializer_func)
        toolbox.register('individual', initIterateWithCfg, creator.Individual, toolbox.gene_values)
        toolbox.register('population', initRepeatWithCfg, list, toolbox.individual)
        toolbox.register('evaluate', evaluator_func)

        # 注册交叉和变异
        if crossover_func:
            toolbox.register('mate', crossover_func)
        if mutate_func:
            toolbox.register('mutate', mutate_func)

        # 注册选择算子
        from deap import tools as deap_tools
        toolbox.register('select', deap_tools.selNSGA2)

        self.logger.info("Toolbox setup completed")

    def _create_surrogate_evaluator(self, original_evaluator):
        """创建代理模型评估函数包装器"""
        import pickle
        import numpy as np

        # 加载代理模型
        model_file = os.path.join(self.cfg.surrogate_model_dir, 'surrogate_model.pkl')
        self.logger.info(f"Loading surrogate model from {model_file}")

        with open(model_file, 'rb') as f:
            surrogate_model = pickle.load(f)

        def surrogate_evaluate(cf, ind):
            """使用代理模型评估个体"""
            # 提取基因值
            X = np.array(ind, dtype=float).reshape(1, -1)

            # 代理模型预测环境效益(泥沙)
            sed_pred = surrogate_model.predict(X)[0]

            # 计算经济成本（简化：以 BMP 数量作为代理成本）
            cost = float(np.sum(np.array(ind) > 0))

            # 设置个体属性
            ind.sed_sum = sed_pred
            ind.environment = sed_pred
            ind.economy = cost
            ind.fitness.values = [cost, sed_pred]

            # 时空模式所需的分期属性（代理模型无实际分期，用单期占位）
            ind.sed_per_period = [sed_pred]
            ind.net_costs_per_period = [cost]
            ind.costs_per_period = [cost]
            ind.incomes_per_period = [0.0]

            # 交互模式所需的偏好指标属性（代理模型简化估算）
            ind.env_on_invest = sed_pred / cost if cost > 0 else 0.0
            ind.abandon_possibility = 0.0
            ind.return_on_invest = 0.0
            ind.cost_variation = 0.0
            ind.bmp_type_count = {}

            # 设置时间属性(代理模型无实际运行时间)
            ind.io_time = 0.0
            ind.comp_time = 0.0
            ind.simu_time = 0.0
            ind.runtime = 0.0

            return ind

        return surrogate_evaluate

    def create_scenario(self):
        """创建场景对象"""
        # NEW: Use relative import
        from spatialunits.scenario import SUScenario, get_key_bmps
        from spatialunits.config import SASlpPosConfig, SAConnFieldConfig, SACommUnitConfig
        from configparser import ConfigParser

        self.logger.info("Creating scenario object...")

        try:
            # 转换为Legacy配置格式
            legacy_config = self.cfg.to_legacy_config()

            # 创建ConfigParser
            cf = ConfigParser()
            for section, values in legacy_config.items():
                cf.add_section(section)
                for key, value in values.items():
                    cf.set(section, key, str(value))

            # 根据空间单元类型创建配置对象
            if self.cfg.spatial_unit == 'SLPPOS':
                sa_cfg = SASlpPosConfig(cf)
            elif self.cfg.spatial_unit == 'CONNFIELD':
                sa_cfg = SAConnFieldConfig(cf)
            else:
                sa_cfg = SACommUnitConfig(cf)

            # 构建索引
            sa_cfg.construct_indexes_units_gene()

            # NEW: Inject surrogate model config
            if self.cfg.use_surrogate:
                sa_cfg.use_surrogate = True
                sa_cfg.surrogate_model_dir = self.cfg.surrogate_model_dir
                self.logger.info("Surrogate model mode enabled")

            # NEW: Inject fields that can't go through ConfigParser (complex nested structures)
            # These are needed by bmps_spatio-temporal_nsga2.py and interactive mode
            # key_bmps: extracted from reference scenarios or provided directly
            if self.cfg.prioritize_key_bmps:
                get_key_bmps(sa_cfg)
            else:
                sa_cfg.key_bmps = getattr(sa_cfg, 'key_bmps', {})

            # users: interactive mode user preferences (dict format expected by spatio-temporal code)
            if self.cfg.enable_interactive and self.cfg.users:
                users_dict = {}
                for user in self.cfg.users:
                    uid = user.get('user_id', f'user_{len(users_dict)}')
                    users_dict[uid] = {
                        'history_good': [],
                        'history_bad': [],
                        'history_bad_reasons': [],
                        'preference_param': user.get('preference_params', {})
                    }
                sa_cfg.users = users_dict
                self.logger.info(f"Interactive mode: {len(users_dict)} users configured")
            else:
                sa_cfg.users = {}

            # enable_interactive flag
            sa_cfg.enable_interactive = self.cfg.enable_interactive
            sa_cfg.interactive_interval = self.cfg.interactive_interval

            # enable_async flag for async interactive optimization
            if hasattr(self.cfg, 'enable_async'):
                sa_cfg.enable_async = self.cfg.enable_async
                sa_cfg.async_task_id = getattr(self.cfg, 'async_task_id', 'default_task')
                sa_cfg.async_signal_dir = getattr(self.cfg, 'async_signal_dir', '/data/config')
                sa_cfg.async_checkpoint_dir = getattr(self.cfg, 'async_checkpoint_dir', '/data/checkpoints')
                self.logger.info(f"Async interactive mode: enable_async={sa_cfg.enable_async}")

            # 创建场景
            self.scenario = SUScenario(sa_cfg)
            self.logger.info(f"Scenario created: {self.cfg.spatial_unit} with {sa_cfg.genes_num} genes")

            return self.scenario

        except Exception as e:
            self.logger.error(f"Failed to create scenario: {e}", exc_info=True)
            raise

    def run(self) -> Tuple:
        """
        执行优化

        Returns:
            (population, logbook)
        """
        self.logger.info("=" * 60)
        self.logger.info("SEIMS Unified Optimizer V2")
        self.logger.info("=" * 60)
        self.logger.info(f"Mode: {self.cfg.mode}")
        self.logger.info(f"Spatial Unit: {self.cfg.spatial_unit}")
        self.logger.info(f"Config Method: {self.cfg.config_method}")
        self.logger.info(f"Generations: {self.cfg.generations}")
        self.logger.info(f"Population Size: {self.cfg.population_size}")
        self.logger.info(f"Model Dir: {self.cfg.model_dir}")
        self.logger.info(f"Output Dir: {self.cfg.output_dir}")
        self.logger.info("=" * 60)

        try:
            # 根据模式执行
            if self.cfg.mode == 'spatial':
                return self._run_spatial()
            elif self.cfg.mode == 'temporal':
                return self._run_temporal()
            else:
                return self._run_spatio_temporal()
        except Exception as e:
            self.logger.error(f"Optimization failed: {e}", exc_info=True)
            raise

    def _run_spatial(self):
        """运行空间优化"""
        self.logger.info("Starting spatial optimization...")

        try:
            # NEW: Use relative import to avoid module path issues
            from spatialunits.main_nsga2 import main as spatial_main

            sce = self.create_scenario()
            result = spatial_main(sce)

            self.logger.info("Spatial optimization completed successfully")
            return result

        except Exception as e:
            self.logger.error(f"Spatial optimization failed: {e}", exc_info=True)
            raise

    def _run_temporal(self):
        """运行时间优化"""
        self.logger.info("Starting temporal optimization...")

        try:
            # NEW: Use relative import
            from spatialunits.bmps_order_nsga2 import main as temporal_main

            sce = self.create_scenario()

            # NEW: Temporal mode requires loading gene values from selected_scenario_file
            # This step was missing - without it, gene_values are all zeros
            # The selected_scenario_file contains the spatial BMP configuration from a prior spatial optimization
            if self.cfg.selected_scenario_file:
                selected_file = os.path.join(self.cfg.model_dir, self.cfg.selected_scenario_file)
                if os.path.isfile(selected_file):
                    sceid = -1
                    gvalues = []
                    with open(selected_file, 'r') as fp:
                        for line in fp.readlines():
                            items = line.split(':')
                            if items[0].strip() == 'Scenario ID':
                                sceid = int(items[1].strip())
                            elif items[0].strip() == 'Gene values':
                                gvalues = [float(v.strip()) for v in items[1].split(',')]
                    if sceid >= 0 and gvalues:
                        # OLD: MongoDB validation removed - temporal optimization does NOT need
                        # the scenario to exist in MongoDB's BMP_SCENARIOS collection.
                        # BMP parameter definitions (bmps_params) are loaded from the scenario DB
                        # during SUScenario.__init__() via read_bmp_parameters().
                        # The gene_values from the txt file are sufficient.
                        sce.set_unique_id(sceid)
                        sce.initialize(input_genes=gvalues)
                        self.logger.info(f"Loaded selected scenario {sceid} with {len(gvalues)} gene values")
                    else:
                        self.logger.warning(f"Failed to parse selected_scenario_file: {selected_file}")
                else:
                    self.logger.error(f"selected_scenario_file not found: {selected_file}")
                    raise FileNotFoundError(f"selected_scenario_file not found: {selected_file}")
            else:
                self.logger.warning("No selected_scenario_file specified for temporal mode")

            result = temporal_main(sce)

            self.logger.info("Temporal optimization completed successfully")
            return result

        except Exception as e:
            self.logger.error(f"Temporal optimization failed: {e}", exc_info=True)
            raise

    def _run_spatio_temporal(self):
        """运行时空优化 - 修复模块导入"""
        self.logger.info("Starting spatio-temporal optimization...")

        try:
            # 修复：使用importlib动态导入，避免文件名中的连字符问题
            # NEW: Use relative module path
            module_name = 'spatialunits.bmps_spatio-temporal_nsga2'
            st_module = importlib.import_module(module_name)
            st_main = getattr(st_module, 'main')

            sce = self.create_scenario()
            result = st_main(sce)

            self.logger.info("Spatio-temporal optimization completed successfully")
            return result

        except Exception as e:
            self.logger.error(f"Spatio-temporal optimization failed: {e}", exc_info=True)
            raise

    def get_config_summary(self) -> Dict:
        """获取配置摘要"""
        return {
            'mode': self.cfg.mode,
            'spatial_unit': self.cfg.spatial_unit,
            'config_method': self.cfg.config_method,
            'algorithm': {
                'type': self.cfg.algorithm,
                'generations': self.cfg.generations,
                'population_size': self.cfg.population_size
            },
            'temporal': {
                'enable_implementation_order': self.cfg.enable_implementation_order,
                'implementation_period': self.cfg.implementation_period
            } if self.cfg.mode != 'spatial' else None,
            'budget': {
                'enable_investment_quota': self.cfg.enable_investment_quota,
                'investment_each_period': self.cfg.investment_each_period
            },
            'model': {
                'model_dir': self.cfg.model_dir,
                'bin_dir': self.cfg.bin_dir
            },
            'output': {
                'output_dir': self.cfg.output_dir
            }
        }


# 便捷函数
def create_optimizer_v2(config: Union[dict, str, UnifiedConfig], **kwargs) -> UnifiedOptimizerV2:
    """
    创建优化器V2实例

    Args:
        config: 配置
        **kwargs: 其他参数

    Returns:
        UnifiedOptimizerV2对象
    """
    return UnifiedOptimizerV2(config, **kwargs)


def create_minimal_optimizer_v2(model_dir: str, mode: str = 'spatial', **kwargs) -> UnifiedOptimizerV2:
    """
    创建最小配置的优化器V2

    Args:
        model_dir: 模型目录
        mode: 优化模式
        **kwargs: 其他参数

    Returns:
        UnifiedOptimizerV2对象
    """
    config = UnifiedConfig.create_minimal(model_dir, mode, **kwargs)
    return UnifiedOptimizerV2(config)


# 主函数入口
def main():
    """命令行入口"""
    import argparse

    parser = argparse.ArgumentParser(description='SEIMS Unified Optimizer V2')
    parser.add_argument('--config', '-c', type=str, required=True,
                       help='Path to JSON configuration file')
    parser.add_argument('--output', '-o', type=str, default=None,
                       help='Output directory')
    parser.add_argument('--log-level', type=str, default='INFO',
                       choices=['DEBUG', 'INFO', 'WARNING', 'ERROR'],
                       help='Log level')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Verbose output')

    args = parser.parse_args()

    log_level = 'DEBUG' if args.verbose else args.log_level

    # 创建优化器
    try:
        optimizer = UnifiedOptimizerV2(args.config, output_dir=args.output, log_level=log_level)

        # 显示配置摘要
        if args.verbose:
            print("\nConfiguration Summary:")
            print(json.dumps(optimizer.get_config_summary(), indent=2))

        # 执行优化
        population, logbook = optimizer.run()

        print("\n" + "=" * 60)
        print("Optimization completed successfully!")
        print(f"Final population size: {len(population)}")
        print("=" * 60)

    except Exception as e:
        print(f"\nOptimization failed: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
