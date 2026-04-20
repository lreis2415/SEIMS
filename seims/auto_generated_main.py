import sys
import os

# 1. 路径注入
sys.path.append(r"D:\EGC\SEIMS-dev\seims")

from optimization_framework.utils.factory import OptimizationFactory
from optimization_framework.utils.visualizer import OptimizationVisualizer

# 2. 自定义约束代码 (由 LLM 生成)
constraint_code_1 = """
def validate(individual, context, mode):
    if str(mode) != 'ConstraintMode.PRE_EVALUATION': return True
    print("🚀 slope adaption constraint...")
    return True
"""

# 3. 配置定义
AGENT_PAYLOAD = {
    "optimization_problem": {
        "objectives": [

            {
                "name": "eco",
                "type": "min",
                "indicator": "net cost",
                "evaluator": "BMP_net_cost_model"
            },{
                "name": "env",
                "type": "max",
                "indicator": "sediment reduction rate",
                "evaluator": "SEIMS"
            }
        ],
        "decision_variable": {
            "spatial_discretization": "SLPPOS",
            "BMP_value": {
                "BMP_type": [1, 3, 4]

            }
        },
        "constraints": [
            {
                "name": "cost_constraint",
                "type": "Resource_Limitation",
                "budget": 70,#budget参数目前是必须的，同时factory大小写识别有问题
                "evaluator": "BMP_net_cost_model"
            },
            {
                "name": "slope_adaption",
                "type": "Custom_Python",
                "code": constraint_code_1
            }
        ]
    },
    "evaluator": {
        "seims": {
            "MODEL_DIR": r"D:\EGC\SEIMS-dev\data\youwuzhen\demo_youwuzhen30m_longterm_model",
            "BIN_DIR": r"D:\EGC\SEIMS-dev\build\bin",
            "spatial_scope": "Youwuzhen",
            "time_span": "2013-2017",
            "spatial_resolution": 30
        }
    },
    "solver": {
        "Type": "NSGA2",
        "GenerationsNum": 2,
        "PopulationSize": 4
    }
}

def main():
    print("🚀 Starting Optimization...")
    # 标准工厂调用
    problem, solver = OptimizationFactory.create(AGENT_PAYLOAD)

    # 建立可视化
    out_dir = os.path.join(os.path.dirname(__file__), "results")
    vis = OptimizationVisualizer(out_dir)

    def on_step(pop, gen):
        vis.plot_pareto_front(pop, gen)
        ref_point = (0.0, -200000000.0)
        vis.update_hypervolume(pop, reference_point=ref_point)

    # 执行求解
    solver.solve(problem, on_generation_callback=on_step)
    vis.plot_hypervolume_curve()

if __name__ == "__main__":
    main()
