# -*- coding: utf-8 -*-
"""
统一配置解析器

支持从JSON配置文件加载配置，解析并验证所有参数。
根据优化模式自动推断和填充默认值。

@author: SEIMS Team
"""

import json
import os
from typing import Dict, List, Any, Union, Optional


class UnifiedConfig:
    """
    统一配置类 - 支持所有三种优化模式

    Attributes:
        mode: 优化模式 (spatial/temporal/spatio_temporal)
        spatial_unit: 空间单元类型
        config_method: BMP配置方法
    """

    # 优化模式枚举
    MODE_SPATIAL = "spatial"
    MODE_TEMPORAL = "temporal"
    MODE_SPATIO_TEMPORAL = "spatio_temporal"

    # 支持的空间单元
    SPATIAL_UNITS = ['HRU', 'EXPLICITHRU', 'CONNFIELD', 'SLPPOS']

    # 空间单元支持的配置方法映射
    UNIT_METHOD_MAP = {
        'HRU': ['RAND', 'SUIT'],
        'EXPLICITHRU': ['RAND', 'SUIT'],
        'CONNFIELD': ['RAND', 'SUIT', 'UPDOWN'],
        'SLPPOS': ['RAND', 'SUIT', 'UPDOWN', 'HILLSLP']
    }

    # 默认内置案例
    DEFAULT_MODEL_DIR = "/data/youwuzhen/demo_youwuzhen30m_longterm_model"
    DEFAULT_BIN_DIR = "/opt/seims/bin"

    def __init__(self, config_data: Union[Dict, str]):
        """
        初始化统一配置

        Args:
            config_data: JSON字典或JSON文件路径
        """
        if isinstance(config_data, str):
            with open(config_data, 'r', encoding='utf-8') as f:
                self.config = json.load(f)
        else:
            self.config = config_data

        # 解析各部分配置
        self._parse()

    def _parse(self):
        """解析配置"""
        # 1. 优化模式
        self.mode = self.config.get('mode', self.MODE_SPATIAL)

        # 2. 算法参数
        algo = self.config.get('algorithm', {})
        self.algorithm = algo.get('type', 'NSGA2')
        self.generations = algo.get('GenerationsNum', 100)
        self.population_size = algo.get('PopulationSize', 60)
        self.crossover_rate = algo.get('CrossoverRate', 0.8)
        self.mutate_rate = algo.get('MutateRate', 0.1)
        self.max_mutate_perc = algo.get('MaxMutatePerc', 0.2)
        self.select_rate = algo.get('SelectRate', 0.8)

        # 3. 空间配置
        spatial = self.config.get('spatial', {})
        self.spatial_unit = spatial.get('unit', 'SLPPOS')
        self.config_method = spatial.get('config_method', 'HILLSLP')
        self.boundary_adaptive = spatial.get('boundary_adaptive', False)

        # 4. 时间配置
        temporal = self.config.get('temporal', {})
        # 根据mode推断时间配置
        if self.mode == self.MODE_SPATIAL:
            self.enable_implementation_order = False
            self.implementation_period = 1
            self.change_frequency = 1
            self.effectiveness_changeable = False
            # runtime_years will be calculated from eval time range later
            self.selected_scenario_file = ''
        elif self.mode == self.MODE_TEMPORAL:
            self.enable_implementation_order = True
            self.implementation_period = temporal.get('implementation_period', 5)
            self.change_frequency = temporal.get('change_frequency', 1)
            self.effectiveness_changeable = temporal.get('effectiveness_changeable', True)
            # runtime_years will be calculated from eval time range later
            self.selected_scenario_file = temporal.get('selected_scenario_file', '')
        else:  # spatio_temporal
            self.enable_implementation_order = temporal.get('enable_implementation_order', True)
            self.implementation_period = temporal.get('implementation_period', 5)
            self.change_frequency = temporal.get('change_frequency', 1)
            self.effectiveness_changeable = temporal.get('effectiveness_changeable', True)
            # runtime_years will be calculated from eval time range later
            self.selected_scenario_file = temporal.get('selected_scenario_file', '')

        # 计算change_times
        self.change_times = int(self.implementation_period / self.change_frequency)

        # 5. 预算约束
        budget = self.config.get('budget', {})
        self.enable_investment_quota = budget.get('enable_investment_quota', False)
        self.investment_each_period = budget.get('investment_each_period', [100])
        self.investment_float_range = budget.get('investment_float_range', 0.2)
        self.investment_aver_constrain = budget.get('investment_aver_constrain', False)
        self.discount_rate = budget.get('discount_rate', 0.1)
        self.years_first_period = budget.get('years_first_period', self.implementation_period)
        # NEW: Multi-stage support (2026-03-19)
        # stage_years: list of years per stage, e.g. [3, 4, 3] for 3 stages
        # If not provided, falls back to 2-stage using years_first_period
        self.stage_years = budget.get('stage_years', [])

        # 6. 关键BMP
        key_bmp = self.config.get('key_bmp', {})
        self.prioritize_key_bmps = key_bmp.get('enable_prioritize', False)
        self.key_bmps = key_bmp.get('key_bmps', {})
        self.reference_scenarios = key_bmp.get('reference_scenarios', [])

        # 7. 交互配置
        interactive = self.config.get('interactive', {})
        self.enable_interactive = interactive.get('enable', False)
        self.interactive_interval = interactive.get('interval_generations', 10)
        self.users = interactive.get('users', [])
        self.preference_fusion_strategy = interactive.get(
            'preference_fusion_strategy',
            interactive.get('fusion_strategy', 'merge_preferences')
        )
        # 异步交互配置
        self.enable_async = interactive.get('enable_async', False)
        self.async_task_id = interactive.get('task_id', 'default_task')
        self.async_signal_dir = interactive.get('signal_dir', '/data/config')
        self.async_checkpoint_dir = interactive.get('checkpoint_dir', '/data/checkpoints')

        # 8. 继续运行
        resume = self.config.get('resume', {})
        self.resume_enable = resume.get('enable', False)
        self.pareto_fronts_file = resume.get('pareto_fronts_file', '')
        self.generation_selected = resume.get('generation_selected', -1)

        # 9. 评估参数
        evaluation = self.config.get('evaluation', {})
        self.output_id = evaluation.get('output_id', 'SED_OL')
        self.env_file = evaluation.get('env_file', 'SED_OL_SUM.tif')
        self.base_env = evaluation.get('base_env', -9999)
        # FIXED: Support both old and new parameter names for backward compatibility
        # OLD: Used 'time_start' and 'time_end' which don't match config.py expectations
        # NEW: Support both 'eval_stime'/'eval_etime' (preferred) and 'time_start'/'time_end' (fallback)
        self.time_start = evaluation.get('eval_stime', evaluation.get('time_start', '2013-01-01 00:00:00'))
        self.time_end = evaluation.get('eval_etime', evaluation.get('time_end', '2017-12-31 23:59:59'))
        self.worst_economy = evaluation.get('worst_economy', 300.0)
        self.worst_environment = evaluation.get('worst_environment', 0.0)

        # NEW: Calculate runtime_years from eval time range (2026-03-19)
        # OLD: runtime_years was read from config, but should be auto-calculated
        from datetime import datetime
        eval_start = datetime.strptime(self.time_start, '%Y-%m-%d %H:%M:%S')
        eval_end = datetime.strptime(self.time_end, '%Y-%m-%d %H:%M:%S')
        runtime_days = (eval_end - eval_start).days
        self.runtime_years = int(runtime_days / 365) + (1 if runtime_days % 365 > 0 else 0)  # Round up

        # 10. 模型参数
        model = self.config.get('model', {})
        self.model_dir = model.get('model_dir', self.DEFAULT_MODEL_DIR)
        self.bin_dir = model.get('bin_dir', self.DEFAULT_BIN_DIR)
        self.hostname = model.get('hostname', model.get('host', '127.0.0.1'))
        self.port = model.get('port', 27017)
        self.db_name = model.get('db_name', model.get('model_name', 'seims'))
        self.scenario_db = model.get('scenario_db', self.db_name + '_scenario')
        self.sim_time_start = model.get('sim_time_start', '2011-01-01 00:00:00')
        self.sim_time_end = model.get('sim_time_end', '2017-12-31 23:59:59')
        self.num_threads = model.get('num_threads', 16)

        # 11. 输出配置
        output = self.config.get('output', {})
        self.export_scenario_txt = output.get('export_scenario_txt', True)
        self.export_scenario_tif = output.get('export_scenario_tif', False)
        self.output_dir = output.get('output_dir', './output')

        # 12. BMP配置
        bmps = self.config.get('bmps', {})
        self.bmp_collection = bmps.get('collection', 'AREAL_STRUCT_MANAGEMENT')
        self.bmp_subscenario = bmps.get('subscenario', [1, 2, 3, 4])
        self.bmp_id = bmps.get('id', 17)
        self.retain_collection = bmps.get('retain_collection', 'PLANT_MANAGEMENT')
        self.retain_subscenario = bmps.get('retain_subscenario', 0)
        self.retain_id = bmps.get('retain_id', 12)
        # NEW: Add DISTRIBUTION and LOCATION for retained BMP
        self.retain_distribution = bmps.get('retain_distribution', 'RASTER|LANDUSE')
        self.retain_location = bmps.get('retain_location', '33')

        # 13. Surrogate model configuration
        surrogate = self.config.get('surrogate', {})
        self.use_surrogate = surrogate.get('use_surrogate', False)
        self.surrogate_model_dir = surrogate.get('surrogate_model_dir', '')
        # NEW (2026-03-30): Enable comparison mode - run both surrogate and SEIMS
        self.surrogate_comparison = surrogate.get('surrogate_comparison', False)

    def validate(self) -> List[str]:
        """
        验证配置合法性

        Returns:
            错误信息列表
        """
        errors = []

        # 1. 模式验证
        if self.mode not in [self.MODE_SPATIAL, self.MODE_TEMPORAL, self.MODE_SPATIO_TEMPORAL]:
            errors.append(f"Invalid mode: {self.mode}. Must be one of: spatial, temporal, spatio_temporal")

        # 2. 空间单元验证
        if self.spatial_unit not in self.SPATIAL_UNITS:
            errors.append(f"Invalid spatial_unit: {self.spatial_unit}. Must be one of: {self.SPATIAL_UNITS}")

        # 3. 配置方法验证
        valid_methods = self.UNIT_METHOD_MAP.get(self.spatial_unit, [])
        if self.config_method not in valid_methods:
            errors.append(f"config_method '{self.config_method}' not supported for {self.spatial_unit}. "
                         f"Supported methods: {valid_methods}")

        # 4. 时间优化模式验证
        if self.mode == self.MODE_TEMPORAL:
            if not self.selected_scenario_file:
                errors.append("temporal mode requires selected_scenario_file in temporal section")

        # NEW: Validate implementation_period <= runtime_years (2026-03-19)
        if self.mode != self.MODE_SPATIAL:
            if self.implementation_period > self.runtime_years:
                errors.append(f"implementation_period ({self.implementation_period}) cannot exceed "
                             f"runtime_years ({self.runtime_years})")

        # 5. 预算约束验证
        if self.enable_investment_quota:
            if not self.investment_each_period:
                errors.append("enable_investment_quota requires investment_each_period")
            # NEW: Multi-stage validation (2026-03-19)
            if self.mode != self.MODE_SPATIAL and self.enable_implementation_order:
                # Multi-stage format: [stage1_budget, stage2_budget, ...] (no total)
                # stage_years: [years_stage1, years_stage2, ...]
                if self.stage_years:
                    num_stages = len(self.stage_years)
                    if len(self.investment_each_period) != num_stages:
                        errors.append(f"investment_each_period length ({len(self.investment_each_period)}) "
                                     f"should be {num_stages} (same as stage_years) when stage_years is provided")
                    # Validate stage_years sum equals implementation_period
                    total_years = sum(self.stage_years)
                    if total_years != self.implementation_period:
                        errors.append(f"stage_years sum ({total_years}) should equal "
                                     f"implementation_period ({self.implementation_period})")
                else:
                    # Accept any non-empty list: [total] or [s1, s2, ...] or legacy [s1, s2, total]
                    pass
            # OLD: Original validation for spatial mode
            # 验证投资期数与implementation_period匹配
            # if self.mode != self.MODE_SPATIAL:
            #     inv_periods = len(self.investment_each_period)
            #     if isinstance(self.investment_each_period[0], list):
            #         # 二维数组格式: [[capex], [opex]]
            #         inv_periods = len(self.investment_each_period[0])
            #     if inv_periods != self.implementation_period:
            #         errors.append(f"investment_each_period length ({inv_periods}) should match "
            #                      f"implementation_period ({self.implementation_period})")

        # 6. 关键BMP验证
        if self.prioritize_key_bmps:
            if not self.key_bmps and not self.reference_scenarios:
                errors.append("prioritize_key_bmps=true requires either key_bmps or reference_scenarios")

        # 7. 交互式验证
        if self.enable_interactive:
            if not self.users:
                errors.append("enable_interactive=true requires users")
            # 各模式可用的 preference indicator 说明：
            # - economy, environment: 所有模式均可用（直接来自评估结果）
            # - env_on_invest: 所有模式均可用（= environment/economy，任何评估后均可算）
            # - return_on_invest: 所有模式均可用
            #     空间模式：单期 NPV 比，由 capex/opex/income 计算
            #     时间/时空模式：多期折现收益/成本比
            # - abandon_possibility: 仅时间/时空模式有意义（需要多期实施顺序数据）
            # - cost_variation: 仅时间/时空模式有意义（需要多期净成本数据，单期方差=0）
            # - bmp_rules: 所有模式均可用（基于基因值）
            TEMPORAL_ONLY_INDICATORS = {'abandon_possibility', 'cost_variation'}
            for i, user in enumerate(self.users):
                if 'preference_params' not in user:
                    errors.append(f"User {i} requires preference_params")
                    continue
                uid = user.get('user_id', f'user_{i}')
                for indicator in user['preference_params']:
                    if indicator == 'bmp_rules':
                        continue
                    if self.mode == self.MODE_SPATIAL and indicator in TEMPORAL_ONLY_INDICATORS:
                        # 空间模式无多期数据，abandon_possibility/cost_variation 无实际意义
                        errors.append(
                            f"[WARNING] User '{uid}': indicator '{indicator}' has no meaningful "
                            f"value in spatial mode (requires multi-period data). "
                            f"It will evaluate to 0 and be silently skipped."
                        )
        # 注意：WARNING 类条目仅为提示，不阻止运行

        # 8. 继续运行验证
        if self.resume_enable:
            if not self.pareto_fronts_file:
                errors.append("resume.enable=true requires pareto_fronts_file")
            if self.generation_selected < 0:
                errors.append("resume.enable=true requires generation_selected")

        # 9. 模型目录验证
        if not self.model_dir:
            errors.append("model.model_dir is required")

        return errors

    def to_legacy_config(self) -> Dict:
        """
        转换为Legacy格式配置字典 (用于兼容旧代码)

        Returns:
            Legacy格式配置字典
        """
        # 确定NSGA2配置section名称
        # FIXED: Distinguish temporal and spatio_temporal modes
        if self.mode == 'temporal':
            # Temporal mode: optimize BMP implementation order only
            if self.enable_investment_quota:
                nsga2_section = f"SA_NSGA2_TEMPORAL_CONSTRAINED_{self.spatial_unit}_{self.config_method}"
            else:
                nsga2_section = f"SA_NSGA2_TEMPORAL_{self.spatial_unit}_{self.config_method}"
        elif self.mode == 'spatio_temporal':
            # Spatio-temporal mode: optimize both spatial configuration and temporal order
            if self.enable_investment_quota:
                nsga2_section = f"SA_NSGA2_S_T_CONSTRAINED_{self.spatial_unit}_{self.config_method}"
            else:
                nsga2_section = f"SA_NSGA2_S_T_{self.spatial_unit}_{self.config_method}"
        else:  # spatial mode
            if self.enable_investment_quota:
                nsga2_section = f"SA_NSGA2_SPATIAL_CONSTRAINED_{self.spatial_unit}_{self.config_method}"
            else:
                nsga2_section = f"SA_NSGA2_SPATIAL_{self.spatial_unit}_{self.config_method}"

        return {
            'SEIMS_Model': {
                'HOSTNAME': self.hostname,
                'PORT': str(self.port),
                'VERSION': 'OMP',
                'NTHREAD': str(self.num_threads),
                'FDIRMTD': '0',
                'LYRMTD': '1',
                'SCENARIO_ID': '0',
                'Sim_Time_start': self.sim_time_start,
                'Sim_Time_end': self.sim_time_end,
                'MODEL_DIR': self.model_dir,
                'BIN_DIR': self.bin_dir,
                'DB_NAME': self.db_name,
                'SCENARIODB': self.scenario_db,
            },
            'Scenario_Common': {
                'optimization_mode': self.mode,  # NEW: Add mode to distinguish temporal/spatio_temporal
                'eval_time_start': self.time_start,
                'eval_time_end': self.time_end,
                'worst_economy': str(self.worst_economy),
                'worst_environment': str(self.worst_environment),
                'runtime_years': str(self.runtime_years),
                'enable_implementation_order': str(self.enable_implementation_order).lower(),
                'implementation_period': str(self.implementation_period),
                'effectiveness_changeable': str(self.effectiveness_changeable).lower(),
                'change_frequency': str(self.change_frequency),
                'enable_investment_quota': str(self.enable_investment_quota).lower(),
                'investment_each_period': str(self.investment_each_period),
                'investment_float_range': str(self.investment_float_range),
                'years_first_period': str(self.years_first_period),
                # NEW: Multi-stage support (2026-03-19)
                'stage_years': json.dumps(self.stage_years) if self.stage_years else '',
                'discount_rate': str(self.discount_rate),
                'investment_aver_constrain': str(self.investment_aver_constrain).lower(),
                'export_scenario_txt': str(self.export_scenario_txt).lower(),
                'export_scenario_tif': str(self.export_scenario_tif).lower(),  # FIXED: was self.export_scenario_txt
                'prioritize_key_bmps': str(self.prioritize_key_bmps).lower(),
                'pareto_front_scenarios': str(self.reference_scenarios),
                'selected_scenario_file': self.selected_scenario_file,
                # NEW: Add interactive mode fields for legacy compatibility
                'enable_interactive': str(self.enable_interactive).lower(),
                'interactive_interval': str(self.interactive_interval),
                'preference_fusion_strategy': str(self.preference_fusion_strategy),
                # NEW: Add async interactive mode fields (2026-04-20)
                'enable_async': str(self.enable_async).lower(),
                'async_task_id': str(self.async_task_id),
                'async_signal_dir': str(self.async_signal_dir),
                'async_checkpoint_dir': str(self.async_checkpoint_dir),
                # NEW (2026-03-30): Add surrogate model configuration
                'use_surrogate': str(self.use_surrogate).lower(),
                'surrogate_model_dir': self.surrogate_model_dir,
                'surrogate_comparison': str(self.surrogate_comparison).lower(),
            },
            'BMPs': self._generate_bmps_config(),
            'NSGA2': {
                'GenerationsNum': str(self.generations),
                'PopulationSize': str(self.population_size),
                'CrossoverRate': str(self.crossover_rate),
                'MaxMutatePerc': str(self.max_mutate_perc),
                'MutateRate': str(self.mutate_rate),
                'SelectRate': str(self.select_rate),
                'inputpopulation': str(self.resume_enable).lower(),
                'paretofrontsfile': self.pareto_fronts_file,
                'generationselected': str(self.generation_selected),
            }
        }

    def _generate_bmps_config(self) -> Dict:
        """生成BMP配置"""
        # UNITJSON映射
        unit_json_map = {
            'SLPPOS': 'slppos_3cls_units_updown.json',
            'CONNFIELD': 'connected_field_units_updown_15.json',
            'HRU': 'hru_units.json',
            'EXPLICITHRU': 'explicit_hru_units.json'
        }

        # TAG_NAME映射
        tag_name_map = {
            'SLPPOS': {"1": "summit", "4": "backslope", "16": "valley"}
        }

        gfs_name_map = {
            'SLPPOS': {"1": "rdgInf", "4": "bksInf", "16": "vlyInf"}
        }

        cfg_units = {
            self.spatial_unit: {
                'DISTRIBUTION': f'RASTER|{self.spatial_unit}_UNITS',
                'UNITJSON': unit_json_map.get(self.spatial_unit, 'units.json')
            }
        }

        if self.spatial_unit == 'SLPPOS':
            cfg_units[self.spatial_unit].update({
                # NEW: Don't use json.dumps here - the outer json.dumps(cfg_units) will handle it
                # OLD: 'SLPPOS_TAG_NAME': json.dumps(tag_name_map['SLPPOS']),
                # OLD: 'SLPPOS_GFS_NAME': json.dumps(gfs_name_map['SLPPOS'])
                'SLPPOS_TAG_NAME': tag_name_map['SLPPOS'],
                'SLPPOS_GFS_NAME': gfs_name_map['SLPPOS']
            })

        return {
            'bmps_info': json.dumps({
                str(self.bmp_id): {
                    'COLLECTION': self.bmp_collection,
                    'SUBSCENARIO': self.bmp_subscenario
                }
            }),
            'bmps_retain': json.dumps({
                str(self.retain_id): {
                    'COLLECTION': self.retain_collection,
                    'SUBSCENARIO': self.retain_subscenario,
                    # NEW: Add DISTRIBUTION and LOCATION for retained BMP
                    'DISTRIBUTION': self.retain_distribution,
                    'LOCATION': self.retain_location
                }
            }),
            'eval_info': json.dumps({
                'OUTPUTID': self.output_id,
                'ENVEVAL': self.env_file,
                'BASE_ENV': self.base_env
            }),
            'bmps_cfg_units': json.dumps(cfg_units),
            'bmps_cfg_method': self.config_method
        }

    @staticmethod
    def from_json_file(json_path: str) -> 'UnifiedConfig':
        """从JSON文件加载配置"""
        return UnifiedConfig(json_path)

    @staticmethod
    def create_minimal(model_dir: str, mode: str = 'spatial', **kwargs) -> 'UnifiedConfig':
        """
        创建最小配置

        Args:
            model_dir: 模型目录路径
            mode: 优化模式
            **kwargs: 其他覆盖参数

        Returns:
            UnifiedConfig对象
        """
        config = {
            'mode': mode,
            'model': {
                'model_dir': model_dir
            }
        }
        # 合并其他参数
        for key, value in kwargs.items():
            # 将嵌套参数如 "budget.enable_investment_quota" 转换
            parts = key.split('.')
            current = config
            for part in parts[:-1]:
                if part not in current:
                    current[part] = {}
                current = current[part]
            current[parts[-1]] = value

        return UnifiedConfig(config)

    def __repr__(self):
        return (f"UnifiedConfig(mode={self.mode}, spatial_unit={self.spatial_unit}, "
                f"config_method={self.config_method}, generations={self.generations}, "
                f"population_size={self.population_size})")
