# -*- coding: utf-8 -*-
"""
函数注册表

根据优化模式自动路由到对应的函数。

@author: SEIMS Team
"""

from typing import Dict, Any, Optional, Callable


class FunctionRegistry:
    """
    函数注册表 - 根据优化模式自动路由

    使用示例:
        registry = FunctionRegistry()
        funcs = registry.get_functions('spatio_temporal', 'HILLSLP')
        print(funcs['initializer'])  # 'initialize_scenario_s_t'
    """

    def __init__(self):
        self._registry = {
            'spatial': {
                'main_module': 'scenario_analysis.spatialunits.main_nsga2',
                'initializer': 'initialize_scenario',
                'evaluator': 'scenario_effectiveness',
                'baseline': 'run_base_scenario',
                'crossover': {
                    'HILLSLP': 'crossover_slppos',
                    'UPDOWN': 'crossover_updown',
                    'RAND': 'crossover_rdm',
                    'SUIT': 'crossover_rdm'
                },
                'mutator': 'mutate_rule_s'
            },
            'temporal': {
                'main_module': 'scenario_analysis.spatialunits.bmps_order_nsga2',
                'initializer': 'initialize_scenario_with_bmps_order',
                'evaluator': 'scenario_effectiveness_with_bmps_order',
                'baseline': 'run_benchmark_scenario',
                'crossover': {
                    'HILLSLP': 'crossover_slppos',
                    'UPDOWN': 'crossover_updown',
                    'RAND': 'crossover_rdm',
                    'SUIT': 'crossover_rdm'
                },
                'mutator': 'mutate_with_bmps_order'
            },
            'spatio_temporal': {
                'main_module': 'scenario_analysis.spatialunits.bmps_spatio_temporal_nsga2',
                'initializer': 'initialize_scenario_s_t',
                'evaluator': 'scenario_effectiveness_with_bmps_order',
                'baseline': 'run_base_scenario',
                'crossover': {
                    'HILLSLP': 'crossover_slppos',
                    'UPDOWN': 'crossover_updown',
                    'RAND': 'crossover_rdm',
                    'SUIT': 'crossover_rdm'
                },
                'mutator': 'mutate_rule_s_t'
            }
        }

        # 函数缓存
        self._func_cache = {}

    def get_functions(self, mode: str, config_method: Optional[str] = None) -> Dict[str, Any]:
        """
        获取指定模式对应的函数名

        Args:
            mode: 优化模式 (spatial/temporal/spatio_temporal)
            config_method: 配置方法 (HILLSLP/UPDOWN/RAND/SUIT)

        Returns:
            函数名映射字典

        Raises:
            ValueError: 未知的模式
        """
        if mode not in self._registry:
            raise ValueError(f"Unknown mode: {mode}. Must be one of: {list(self._registry.keys())}")

        funcs = self._registry[mode].copy()

        # 如果提供了config_method，解析对应的交叉变异函数
        if config_method and 'crossover' in funcs:
            crossover_funcs = funcs['crossover']
            funcs['crossover_func'] = crossover_funcs.get(config_method, 'crossover_rdm')

        return funcs

    def get_main_module(self, mode: str) -> str:
        """获取指定模式的主模块路径"""
        return self._registry.get(mode, {}).get('main_module', '')

    def get_initializer(self, mode: str) -> str:
        """获取指定模式的初始化函数名"""
        return self._registry.get(mode, {}).get('initializer', '')

    def get_evaluator(self, mode: str) -> str:
        """获取指定模式的评估函数名"""
        return self._registry.get(mode, {}).get('evaluator', '')

    def get_mutator(self, mode: str) -> str:
        """获取指定模式的变异函数名"""
        return self._registry.get(mode, {}).get('mutator', '')

    def get_crossover(self, mode: str, config_method: str) -> str:
        """获取指定模式和配置方法的交叉函数名"""
        crossovers = self._registry.get(mode, {}).get('crossover', {})
        return crossovers.get(config_method, 'crossover_rdm')

    def register(self, mode: str, func_type: str, func_name: str, func: Optional[Callable] = None):
        """
        注册自定义函数

        Args:
            mode: 优化模式
            func_type: 函数类型 (initializer/evaluator/mutator/crossover)
            func_name: 函数名或键名
            func: 实际函数对象(可选)
        """
        if mode not in self._registry:
            self._registry[mode] = {}

        if func_type == 'crossover':
            if 'crossover' not in self._registry[mode]:
                self._registry[mode]['crossover'] = {}
            self._registry[mode]['crossover'][func_name] = func_name
        else:
            self._registry[mode][func_type] = func_name

        # 如果提供了函数对象，也缓存起来
        if func:
            cache_key = f"{mode}_{func_type}_{func_name}"
            self._func_cache[cache_key] = func

    def load_function(self, mode: str, func_type: str, config_method: Optional[str] = None) -> Callable:
        """
        动态加载函数

        Args:
            mode: 优化模式
            func_type: 函数类型
            config_method: 配置方法(用于交叉函数)

        Returns:
            实际的函数对象
        """
        # 尝试从缓存获取
        if config_method:
            cache_key = f"{mode}_{func_type}_{config_method}"
            if cache_key in self._func_cache:
                return self._func_cache[cache_key]

        cache_key = f"{mode}_{func_type}"
        if cache_key in self._func_cache:
            return self._func_cache[cache_key]

        # 获取函数名
        if func_type == 'crossover':
            func_name = self.get_crossover(mode, config_method or 'RAND')
        elif func_type == 'main':
            func_name = 'main'
        else:
            func_name = self._registry.get(mode, {}).get(func_type, '')

        if not func_name:
            raise ValueError(f"Unknown function type: {func_type} for mode: {mode}")

        # 动态导入
        func = self._import_function(mode, func_type, func_name)

        # 缓存
        self._func_cache[cache_key] = func

        return func

    def _import_function(self, mode: str, func_type: str, func_name: str) -> Callable:
        """动态导入函数"""
        # 获取模块路径
        if func_type == 'main':
            module_path = self.get_main_module(mode)
        else:
            # 需要根据函数名推断模块
            module_path = self._get_module_for_function(mode, func_type)

        # 动态导入
        import importlib
        module = importlib.import_module(module_path)

        if not hasattr(module, func_name):
            raise AttributeError(f"Module {module_path} has no function: {func_name}")

        return getattr(module, func_name)

    def _get_module_for_function(self, mode: str, func_type: str, func_name: str = None) -> str:
        """根据函数类型推断模块路径"""
        # scenario模块中的函数
        scenario_funcs = ['initialize_scenario', 'initialize_scenario_s_t',
                         'initialize_scenario_with_bmps_order',
                         'scenario_effectiveness', 'scenario_effectiveness_with_bmps_order']

        if func_name and func_name in scenario_funcs:
            return 'scenario_analysis.spatialunits.scenario'

        # userdef模块中的函数
        userdef_funcs = ['crossover_slppos', 'crossover_updown', 'crossover_rdm',
                        'mutate_rule_s', 'mutate_rule_s_t', 'mutate_with_bmps_order', 'mutate_rdm']

        if func_name and func_name in userdef_funcs:
            return 'scenario_analysis.spatialunits.userdef'

        # 默认返回主模块
        return self.get_main_module(mode)


# 全局注册表实例
FUNCTION_REGISTRY = FunctionRegistry()


# 便捷函数
def get_functions_for_mode(mode: str, config_method: Optional[str] = None) -> Dict[str, Any]:
    """获取指定模式对应的函数名"""
    return FUNCTION_REGISTRY.get_functions(mode, config_method)


def get_initializer_name(mode: str) -> str:
    """获取指定模式的初始化函数名"""
    return FUNCTION_REGISTRY.get_initializer(mode)


def get_evaluator_name(mode: str) -> str:
    """获取指定模式的评估函数名"""
    return FUNCTION_REGISTRY.get_evaluator(mode)


def get_mutator_name(mode: str) -> str:
    """获取指定模式的变异函数名"""
    return FUNCTION_REGISTRY.get_mutator(mode)


def get_crossover_name(mode: str, config_method: str) -> str:
    """获取指定模式的交叉函数名"""
    return FUNCTION_REGISTRY.get_crossover(mode, config_method)
