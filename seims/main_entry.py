import sys
import os
import json
# 1. 定义 SEIMS 的根目录路径
seims_root_path = r"D:\EGC\SEIMS-dev\seims"

# 2. 将其加入系统路径，这样 Python 就能找到 'scenario_analysis' 模块了
if seims_root_path not in sys.path:
    sys.path.insert(0, seims_root_path)
# -------------------------------------------------------------------------
# 1. 环境路径设置 (确保能导入 app 包)
# 假设 main_entry.py 位于项目根目录，或者你需要根据实际位置调整路径
# -------------------------------------------------------------------------
'''current_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(current_dir, "../../.."))  # 根据你的实际目录层级调整
if project_root not in sys.path:
    sys.path.insert(0, project_root)'''

try:
    from optimization_framework.utils.factory import OptimizationFactory
    from optimization_framework.utils.visualizer import OptimizationVisualizer
except ImportError as e:
    print(f"❌ Import Error: {e}")
    sys.exit(1)
# -------------------------------------------------------------------------
# 2. Agent Payload (模拟前端/Agent传来的 JSON)
# -------------------------------------------------------------------------
AGENT_PAYLOAD = {
    "optimization_problem": {
        # 定义优化目标
        "objectives": [
            # evaluator 字段必须对应 Factory 中 adapter_map 的 key

            {"name": "economic_cost", "type": "min", "evaluator": "bmp_net_cost_model"},
            {"name": "env_benefit", "type": "max", "evaluator": "seims"}
        ],
        # 定义决策变量
        "decision_variable": {
            "spatial_discretization": "SLPPOS",  # 空间单元类型
            "BMP_value": {
                "BMP_type": [1, 2, 3, 4],  # 可选 BMP 类型
                # 如果 BMP_time 为空列表，则为非时空优化；如果有值，则开启时空优化
                #"BMP_time": [2013, 2014, 2015, 2016, 2017]
            }
        },
        # 定义约束
        "constraints": [
            {
                "name": "total_budget",
                "type": "Budget_limitation",
                "evaluator": "bmp_net_cost_model",  # 关联哪个评估器的结果
                "budget": 50  # 预算阈值
            }
        ],
        # 空间约束 (可选)
        "spatial_constraints": {
            "topology": False
        }
    },
    # 配置评估器参数 (对应 Legacy Config 需要的信息)
    "evaluator": {
        "seims": {
            "spatial_scope": "Youwuzhen",
            "time_span": "2013-2017",
            # 请修改为你本地的实际路径
            "MODEL_DIR": r"D:\EGC\SEIMS-dev\data\youwuzhen\demo_youwuzhen30m_longterm_model",
            "BIN_DIR": r"D:\EGC\SEIMS-dev\build\bin",
            "HOSTNAME": "127.0.0.1",
            "PORT": "27017",
            "db_name": "demo_youwuzhen30m_longterm_model"
        }
    },
    # 配置求解器
    "solver": {
        "Type": "NSGA2",
        "GenerationsNum": 5,  # 迭代代数 (测试用少量)
        "PopulationSize": 20,  # 种群大小 (测试用少量)
        "CrossoverRate": 0.8,
        "MutateRate": 0.1,
        "Interactive": False  # 是否开启交互模式
    }
}


# -------------------------------------------------------------------------
# 3. Main Entry
# -------------------------------------------------------------------------
def main():
    print("🚀 [System] Initializing Optimization Framework...")
    print(f"📂 Model Dir: {AGENT_PAYLOAD['evaluator']['seims']['MODEL_DIR']}")

    # --- 核心步骤 1: 工厂创建 ---
    # 这一步会自动完成：Config桥接、Legacy配置加载、Problem构建、Solver选择
    try:
        problem, solver = OptimizationFactory.create(AGENT_PAYLOAD)
    except Exception as e:
        print(f"❌ Factory Creation Failed: {e}")
        import traceback
        traceback.print_exc()
        return

    # 打印一些调试信息
    print("-" * 40)
    print(f"✅ Problem Initialized: {problem.name}")
    print(f"   - Decision Variables (Genes): {problem.n_vars}")
    print(f"   - Objectives: {len(problem.evaluators)} {problem.weights}")
    print(f"   - Constraints: {len(problem.constraints)}")
    print(f"✅ Solver Initialized: {solver.__class__.__name__}")
    print(f"   - Mode: {'Temporal (S-T)' if solver.is_temporal else 'Spatial Only'}")
    print("-" * 40)
    # 2. 初始化可视化器
    # 结果输出到当前目录下的 results 文件夹
    output_dir = os.path.join(os.path.dirname(__file__), "results")
    visualizer = OptimizationVisualizer(output_dir)

    print(f"📊 Visualizer initialized. Results will be saved to: {output_dir}")

    # 3. 定义回调函数 (每一代结束后被调用)
    def on_generation(pop, gen):
        # A. 绘制帕累托前沿图
        visualizer.plot_pareto_front(pop, gen)

        # B. 更新 Hypervolume 数据
        # 参考点设置：Env=0 (最小值), Cost=1000万 (最大预算的相反数)
        # 注意：DEAP 计算 HV 时使用 fitness 值 (Env, -Cost)
        # 所以参考点应该是 (0.0, -10000000.0) 这种“比所有个体都差”的点
        ref_point = (0.0, -200000000.0)
        visualizer.update_hypervolume(pop, reference_point=ref_point)

        print(f"   📸 Gen {gen} visualized.")
    # 4. 运行求解 (传入回调)
    print("\n🏃 running solver...")
    try:
        # 注意：我们在 nsga2_solver.py 中新增了 callback 参数
        population = solver.solve(problem, on_generation_callback=on_generation)
    except KeyboardInterrupt:
        print("\n⚠️ User interrupted optimization.")
        return
    except Exception as e:
        print(f"\n❌ Optimization Failed: {e}")
        import traceback
        traceback.print_exc()
        return
    # 5. 结束后绘制 HV 曲线
    visualizer.plot_hypervolume_curve()

    print("\n🏆 Optimization Finished!")

    # 简单的帕累托前沿提取 (非支配排序的第一层)
    # DEAP 的 tools.sortNondominated 返回的是不同层级的列表
    from deap import tools
    pareto_fronts = tools.sortNondominated(population, len(population), first_front_only=True)

    if pareto_fronts:
        best_solutions = pareto_fronts[0]
        print(f"   Found {len(best_solutions)} non-dominated solutions (Pareto Front).")
        print("\n   [Top 5 Solutions (Cost vs Env)]:")

        # 假设 Obj 0 是 Cost (Min), Obj 1 是 Env (Max)
        # 注意：DEAP 内部 fitness 是加权的，打印时可能需要根据权重还原原始值含义
        # 这里直接打印 fitness.values (加权后的值) 或者根据 adapter 逻辑理解

        for i, ind in enumerate(best_solutions[:5]):
            # 获取具体的评估值 (在 Problem.evaluate 中存入的 obj_0, obj_1)
            # 如果没有存属性，直接看 fitness
            cost = getattr(ind, 'obj_0', 'N/A')
            env = getattr(ind, 'obj_1', 'N/A')

            # 格式化一下
            cost_str = f"{cost:.2f}" if isinstance(cost, (float, int)) else str(cost)
            env_str = f"{env:.4f}" if isinstance(env, (float, int)) else str(env)

            print(f"   #{i + 1}: Cost={cost_str}, Env_Benefit={env_str} | ID={getattr(ind, 'id', -1)}")
    else:
        print("   No valid solutions found.")


if __name__ == "__main__":
    main()
