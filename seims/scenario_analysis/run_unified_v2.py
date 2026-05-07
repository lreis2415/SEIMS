# -*- coding: utf-8 -*-
"""
SEIMS情景优化统一入口脚本 V2 - 增强版本

改进：
1. 更完善的命令行参数解析
2. 增强的错误处理
3. 配置文件验证
4. 进度显示

用法:
    python run_unified_v2.py --config config.json
    python run_unified_v2.py --mode spatial --model-dir /data/youwuzhen/...
    python run_unified_v2.py --preset quick_scan --model-dir /data/youwuzhen/...

@author: SEIMS Team
@version: 2.0
"""

import os
import sys
import json
import argparse
from typing import Dict, Optional

# 添加当前目录到路径
current_dir = os.path.dirname(os.path.abspath(__file__))
if current_dir not in sys.path:
    sys.path.insert(0, current_dir)

# NEW: 添加seims根目录到路径，以便导入utility等模块
seims_root = os.path.abspath(os.path.join(current_dir, '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)


def load_config_from_args(args) -> Dict:
    """从命令行参数构建配置"""
    config = {
        'mode': args.mode,
        'model': {
            'model_dir': args.model_dir,
            'bin_dir': args.bin_dir or '/opt/seims/bin'
        },
        'algorithm': {
            'GenerationsNum': args.generations,
            'PopulationSize': args.population_size,
            'CrossoverRate': args.crossover_rate,
            'MutateRate': args.mutate_rate
        }
    }

    # 空间配置
    if args.spatial_unit:
        config['spatial'] = {
            'unit': args.spatial_unit,
            'config_method': args.config_method or 'HILLSLP'
        }

    # 时间配置
    if args.mode in ['temporal', 'spatio_temporal']:
        config['temporal'] = {
            'implementation_period': args.implementation_period,
            'change_frequency': args.change_frequency,
            'runtime_years': args.runtime_years
        }
        if args.mode == 'temporal' and args.scenario_file:
            config['temporal']['selected_scenario_file'] = args.scenario_file

    # 预算约束
    if args.budget:
        config['budget'] = {
            'enable_investment_quota': True,
            'investment_each_period': args.budget
        }

    return config


def load_config_from_preset(preset_name: str, model_dir: str) -> Dict:
    """从预设加载配置"""
    presets = {
        'quick_scan': {
            'mode': 'spatial',
            'algorithm': {'GenerationsNum': 50, 'PopulationSize': 40},
            'budget': {'enable_investment_quota': False}
        },
        'spatial_constrained': {
            'mode': 'spatial',
            'algorithm': {'GenerationsNum': 100, 'PopulationSize': 60},
            'budget': {'enable_investment_quota': True, 'investment_each_period': [100]}
        },
        'temporal': {
            'mode': 'temporal',
            'algorithm': {'GenerationsNum': 200, 'PopulationSize': 60},
            'budget': {'enable_investment_quota': True, 'investment_each_period': [[50,30,20], [10,10,10]]}
        },
        'spatio_temporal': {
            'mode': 'spatio_temporal',
            'algorithm': {'GenerationsNum': 300, 'PopulationSize': 80},
            'temporal': {
                'enable_implementation_order': True,
                'implementation_period': 5,
                'change_frequency': 1
            },
            'budget': {'enable_investment_quota': True, 'investment_each_period': [100]}
        },
        'spatio_temporal_constrained': {
            'mode': 'spatio_temporal',
            'algorithm': {'GenerationsNum': 300, 'PopulationSize': 80},
            'temporal': {
                'enable_implementation_order': True,
                'implementation_period': 5,
                'change_frequency': 1
            },
            'budget': {
                'enable_investment_quota': True,
                'investment_each_period': [60, 50, 110],
                'investment_float_range': 0.2
            }
        },
        'interactive': {
            'mode': 'spatio_temporal',
            'algorithm': {'GenerationsNum': 500, 'PopulationSize': 80},
            'temporal': {
                'enable_implementation_order': True,
                'implementation_period': 5
            },
            'budget': {
                'enable_investment_quota': True,
                'investment_each_period': [100]
            },
            'interactive': {
                'enable': True,
                'interval_generations': 10,
                'users': [
                    {
                        'user_id': 'user1',
                        'preference_params': {
                            'economy': [60, 'less', 5],
                            'environment': [10, 'greater', 2]
                        }
                    }
                ]
            }
        }
    }

    if preset_name not in presets:
        raise ValueError(f"Unknown preset: {preset_name}. Available: {list(presets.keys())}")

    config = presets[preset_name].copy()
    config['model'] = {
        'model_dir': model_dir
    }

    return config


def validate_config_file(config_path: str) -> bool:
    """验证配置文件"""
    if not os.path.exists(config_path):
        print(f"Error: Configuration file not found: {config_path}")
        return False

    try:
        with open(config_path, 'r', encoding='utf-8') as f:
            config = json.load(f)

        # 基本验证
        if 'mode' not in config:
            print("Error: 'mode' is required in configuration")
            return False

        if 'model' not in config or 'model_dir' not in config['model']:
            print("Error: 'model.model_dir' is required in configuration")
            return False

        return True

    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON format: {e}")
        return False
    except Exception as e:
        print(f"Error: Failed to validate configuration: {e}")
        return False


def main():
    parser = argparse.ArgumentParser(
        description='SEIMS情景优化统一入口 V2',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 从配置文件运行
  python run_unified_v2.py --config config.json

  # 使用预设
  python run_unified_v2.py --preset quick_scan -m /data/youwuzhen/demo_youwuzhen30m_longterm_model

  # 使用命令行参数
  python run_unified_v2.py --mode spatial --generations 100 -m /data/youwuzhen/...

可用预设:
  quick_scan                  - 快速扫描 (50代, 40个体)
  spatial_constrained         - 空间优化+约束 (100代, 60个体)
  temporal                    - 时间优化 (200代, 60个体)
  spatio_temporal             - 时空优化 (300代, 80个体)
  spatio_temporal_constrained - 时空优化+约束 (300代, 80个体)
  interactive                 - 交互式优化 (500代, 80个体)
        """
    )

    # 配置来源
    config_group = parser.add_mutually_exclusive_group(required=True)
    config_group.add_argument('--config', '-c', type=str,
                             help='配置文件路径 (JSON格式)')
    config_group.add_argument('--preset', '-p', type=str,
                             choices=['quick_scan', 'spatial_constrained', 'temporal',
                                     'spatio_temporal', 'spatio_temporal_constrained', 'interactive'],
                             help='使用预设配置')
    config_group.add_argument('--mode', type=str,
                             choices=['spatial', 'temporal', 'spatio_temporal'],
                             help='优化模式 (与其他参数组合使用)')

    # 模型配置
    parser.add_argument('--model-dir', '-m', type=str,
                       help='模型目录路径')
    parser.add_argument('--bin-dir', type=str,
                       help='SEIMS二进制文件目录 (默认: /opt/seims/bin)')
    parser.add_argument('--resume', type=str, default=None, metavar='CONTINUE_JSON',
                       help='从异步交互断点续跑，指定 CONTINUE JSON 路径 '
                            '(由 generate_continue.py 生成)')

    # 算法参数
    # NEW: Remove default values to avoid overriding config file
    # OLD: parser.add_argument('--generations', '-g', type=int, default=100,
    parser.add_argument('--generations', '-g', type=int, default=None,
                       help='迭代代数 (默认: 100)')
    # OLD: parser.add_argument('--population-size', type=int, default=60,
    parser.add_argument('--population-size', type=int, default=None,
                       help='种群大小 (默认: 60)')
    parser.add_argument('--crossover-rate', type=float, default=0.8,
                       help='交叉率 (默认: 0.8)')
    parser.add_argument('--mutate-rate', type=float, default=0.1,
                       help='变异率 (默认: 0.1)')

    # 空间配置
    parser.add_argument('--spatial-unit', type=str,
                       choices=['HRU', 'EXPLICITHRU', 'CONNFIELD', 'SLPPOS'],
                       help='空间单元类型')
    parser.add_argument('--config-method', type=str,
                       choices=['RAND', 'SUIT', 'UPDOWN', 'HILLSLP'],
                       help='BMP配置方法')

    # 时间配置
    parser.add_argument('--implementation-period', type=int, default=5,
                       help='实施周期 (年) (默认: 5)')
    parser.add_argument('--change-frequency', type=int, default=1,
                       help='变化频率 (年) (默认: 1)')
    parser.add_argument('--runtime-years', type=int, default=5,
                       help='运行年数 (默认: 5)')
    parser.add_argument('--scenario-file', type=str,
                       help='选定的情景文件 (时间优化模式必需)')

    # 预算约束
    parser.add_argument('--budget', type=float, nargs='+',
                       help='预算约束 (可以是单个值或多个值)')

    # 输出配置
    parser.add_argument('--output-dir', '-o', type=str,
                       help='输出目录')

    # 其他选项
    parser.add_argument('--log-level', type=str, default='INFO',
                       choices=['DEBUG', 'INFO', 'WARNING', 'ERROR'],
                       help='日志级别 (默认: INFO)')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='详细输出')
    parser.add_argument('--dry-run', action='store_true',
                       help='仅验证配置，不执行优化')

    args = parser.parse_args()

    # 设置日志级别
    log_level = 'DEBUG' if args.verbose else args.log_level

    # 构建配置
    if args.config:
        # 从文件加载
        if not validate_config_file(args.config):
            sys.exit(1)

        if args.verbose:
            print(f"Loading config from: {args.config}")

        with open(args.config, 'r', encoding='utf-8') as f:
            config = json.load(f)

        # 允许命令行参数覆盖
        if args.model_dir:
            config.setdefault('model', {})['model_dir'] = args.model_dir
        if args.generations:
            config.setdefault('algorithm', {})['GenerationsNum'] = args.generations
        if args.output_dir:
            config.setdefault('output', {})['output_dir'] = args.output_dir

    elif args.preset:
        # 从预设加载
        if not args.model_dir:
            print("Error: --model-dir is required when using --preset")
            sys.exit(1)

        if args.verbose:
            print(f"Loading preset: {args.preset}")

        config = load_config_from_preset(args.preset, args.model_dir)

    else:
        # 从命令行参数构建
        if not args.model_dir:
            print("Error: --model-dir is required when using --mode")
            sys.exit(1)

        if args.verbose:
            print(f"Building config from arguments")

        config = load_config_from_args(args)

    # Dry run模式
    if args.dry_run:
        print("\n" + "=" * 60)
        print("DRY RUN MODE - Configuration Preview")
        print("=" * 60)
        print(json.dumps(config, indent=2, ensure_ascii=False))
        print("=" * 60)
        print("\nConfiguration is valid. Use without --dry-run to execute.")
        sys.exit(0)

    # 创建优化器
    from unified_optimizer_v2 import UnifiedOptimizerV2

    try:
        optimizer = UnifiedOptimizerV2(
            config,
            output_dir=args.output_dir,
            log_level=log_level
        )

        # --resume：注入 CONTINUE JSON 路径
        if args.resume:
            if not os.path.isfile(args.resume):
                print(f"Error: CONTINUE JSON not found: {args.resume}")
                sys.exit(1)
            optimizer.cfg.async_continue_file = args.resume
            print(f"Resume mode: CONTINUE JSON = {args.resume}")

        if args.verbose:
            print("\nConfiguration Summary:")
            print(json.dumps(optimizer.get_config_summary(), indent=2, ensure_ascii=False))

        # 执行优化
        print(f"\nStarting optimization...")
        population, logbook = optimizer.run()

        print(f"\n" + "=" * 60)
        print("Optimization completed successfully!")
        print(f"Final population size: {len(population)}")
        print(f"Output directory: {optimizer.cfg.output_dir}")
        print("=" * 60)

    except KeyboardInterrupt:
        print("\n\nOptimization interrupted by user.")
        sys.exit(130)
    except SystemExit:
        raise  # 透传 sys.exit()（包括 exit code 42 异步暂停）
    except Exception as e:
        print(f"\nError: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
