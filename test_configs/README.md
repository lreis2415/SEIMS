# SEIMS 情景优化标准配置使用说明

## 一、启动命令

所有配置均通过统一入口脚本 `run_unified_v2.py` 启动：

```bash
# 基本格式（在项目根目录 D:\EGC\SEIMS-dev 下执行）
python seims/scenario_analysis/run_unified_v2.py --config test_configs/<配置文件名>.json

# 验证配置但不实际运行（推荐首次验证用）
python seims/scenario_analysis/run_unified_v2.py --config test_configs/<配置文件名>.json --dry-run

# 覆盖输出目录（不修改配置文件，一次性指定）
python seims/scenario_analysis/run_unified_v2.py --config test_configs/<配置文件名>.json --output-dir /path/to/output
```

---

## 二、标准配置文件总览

### 非交互式（标准 NSGA-II）

| 配置文件 | 分辨率 | 优化模式 | 评估方法 | 代/种群 | 投资约束 |
|---|---|---|---|---|---|
| `30m_spatial_process.json` | 30m | 空间优化 | SEIMS过程模型 | 100代/60个 | 100万/期 |
| `30m_spatial_surrogate.json` | 30m | 空间优化 | 代理模型(XGBoost) | 300代/80个 | 100万/期 |
| `10m_spatial_process.json` | 10m | 空间优化 | SEIMS过程模型 | 100代/60个 | 90万/期 |
| `10m_spatial_surrogate.json` | 10m | 空间优化 | 代理模型(GBR) | 300代/80个 | 90万/期 |
| `10m_spatio_temporal_process.json` | 10m | 时空优化 | SEIMS过程模型 | 100代/60个 | 分期约束 |
| `10m_spatio_temporal_surrogate.json` | 10m | 时空优化 | 代理模型(GBR) | 300代/80个 | 分期约束 |

### 交互式（Interactive NSGA-II）

| 配置文件 | 分辨率 | 优化模式 | 评估方法 | 代/种群 | 用户数 | 交互间隔 |
|---|---|---|---|---|---|---|
| `30m_spatial_interactive_process.json` | 30m | 空间优化 | SEIMS过程模型 | 100代/60个 | 1 | 每10代 |
| `30m_spatial_interactive_surrogate.json` | 30m | 空间优化 | 代理模型(XGBoost) | 300代/80个 | 1 | 每30代 |
| `10m_spatial_interactive_process.json` | 10m | 空间优化 | SEIMS过程模型 | 100代/60个 | 1 | 每10代 |
| `10m_spatial_interactive_surrogate.json` | 10m | 空间优化 | 代理模型(GBR) | 300代/80个 | 1 | 每30代 |
| `10m_spatio_temporal_interactive_process.json` | 10m | 时空优化 | SEIMS过程模型 | 100代/60个 | 1 | 每10代 |
| `10m_spatio_temporal_interactive_surrogate.json` | 10m | 时空优化 | 代理模型(GBR) | 300代/80个 | 2 | 每30代 |

---

## 三、非交互式配置详细说明

### 3.1 `30m_spatial_process.json`
**用途**：30m分辨率小流域，仅优化BMP的空间布局（不考虑实施时序），使用SEIMS物理模型评估。

- **优化变量**：每个坡位单元选择哪种BMP（0=无 / 1-4=4种措施），时间统一为第1年实施
- **模型**：`demo_youwuzhen30m_longterm_model`（MongoDB: `demo_youwuzhen30m_Scenario`）
- **评估周期**：2013-01-01 ~ 2017-12-31
- **投资约束**：100万/期，浮动±20%
- **算法**：NSGA-II，100代，种群60，适合精度验证，运行较慢
- **输出目录**：`data/youwuzhen/demo_youwuzhen30m_longterm_model/`

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/30m_spatial_process.json
```

---

### 3.2 `30m_spatial_surrogate.json`
**用途**：30m分辨率，空间布局优化，使用代理模型（XGBoost，2000样本纯空间训练）加速评估。

- **优化变量**：同上（仅BMP类型，time=1）
- **代理模型**：`data/youwuzhen/surrogate_training_2000_optimized/models/surrogate_model.pkl`
  - 训练集：2000个样本，仅空间维度（type one-hot 编码）
  - R²≈0.967
- **算法**：NSGA-II，300代，种群80，比过程模型快约100倍
- **输出目录**：`data/youwuzhen/demo_youwuzhen30m_longterm_model/`

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/30m_spatial_surrogate.json
```

---

### 3.3 `10m_spatial_process.json`
**用途**：10m分辨率小流域（有雨区精细划分，105个坡位单元），仅优化BMP空间布局，使用SEIMS物理模型。

- **优化变量**：105个坡位单元各选BMP类型（0-4），时间固定为第1年
- **模型**：`ss_youwuzhen10m_longterm_model`（MongoDB: `ss_youwuzhen10m_Scenario`）
- **投资约束**：90万/期，浮动±20%
- **算法**：NSGA-II，100代，种群60

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatial_process.json
```

---

### 3.4 `10m_spatial_surrogate.json`
**用途**：10m分辨率，空间布局优化，使用代理模型（GBR，5000样本时空训练）加速评估。

- **代理模型**：`data/youwuzhen10m/surrogate_model_comparison/surrogate_model.pkl`
  - 模型类型：GradientBoostingRegressor
  - 训练集：5000个时空样本（type one-hot 525维 + time数值 105维 = 630维输入）
  - 目标y：sediment（年均产沙量 kg/yr）
  - 测试集R²=0.9817，MAPE=0.42%
  - **注意**：该模型训练于时空样本，用于纯空间优化时存在轻微系统偏差（约0.45M kg，1%），
    后续可补充纯空间样本重训以提高精度
- **算法**：NSGA-II，300代，种群80

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatial_surrogate.json
```

---

### 3.5 `10m_spatio_temporal_process.json`
**用途**：10m分辨率，同时优化BMP的空间布局和实施时序，使用SEIMS物理模型，适合最终精确结果。

- **优化变量**：105个坡位单元各选BMP类型（0-4）和实施时期（0-5，对应年份1-5）
- **时间设置**：实施周期5年，每年可调整（change_frequency=1），BMP效益随时间变化
- **投资约束（分期）**：
  - 前3年（years_first_period=3）：每年25万
  - 后2年：每年15万（第4年）和10万（第5年）
  - 浮动±20%
- **算法**：NSGA-II，100代，种群60（运算量较大，建议在服务器运行）

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatio_temporal_process.json
```

---

### 3.6 `10m_spatio_temporal_surrogate.json`
**用途**：10m分辨率，时空联合优化，使用代理模型（GBR，5000样本）加速，是推荐的主要运行方案。

- **代理模型**：同 `10m_spatial_surrogate.json`（同一个模型，支持时空两维）
- **时间设置**：同 `10m_spatio_temporal_process.json`
- **投资约束**：同 `10m_spatio_temporal_process.json`（[25,15,10]万/年，前3年/后2年）
- **算法**：NSGA-II，300代，种群80
- **推荐场景**：这是精度最高且效率最优的组合，是时空优化的首选配置

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatio_temporal_surrogate.json
```

---

## 四、交互式配置详细说明

交互式优化在标准 NSGA-II 基础上，每隔固定代数暂停，让决策者从 Pareto 前沿中选择"满意/不满意"方案，算法据此动态调整偏好引导后续搜索。

### 4.1 `30m_spatial_interactive_process.json`
**用途**：30m空间优化 + 人机交互，使用SEIMS物理模型。

- **用户数**：1个决策者
- **交互时机**：每10代暂停一次（共约10次交互）
- **偏好指标**：economy（<80万）、environment（>5）、env_on_invest（>0.08）、return_on_invest（>1.2）

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/30m_spatial_interactive_process.json
```

---

### 4.2 `30m_spatial_interactive_surrogate.json`
**用途**：30m空间优化 + 人机交互，使用XGBoost代理模型（推荐）。

- **用户数**：1个决策者
- **交互时机**：每30代暂停一次（300代共约10次交互）
- **代理模型**：`surrogate_training_2000_optimized/models/surrogate_model.pkl`

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/30m_spatial_interactive_surrogate.json
```

---

### 4.3 `10m_spatial_interactive_process.json`
**用途**：10m空间优化（105个坡位单元）+ 人机交互，使用SEIMS物理模型。

- **用户数**：1个决策者
- **交互时机**：每10代暂停一次
- **偏好指标**：economy（<80万）、environment（>5）、env_on_invest（>0.08）、return_on_invest（>1.2）

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatial_interactive_process.json
```

---

### 4.4 `10m_spatial_interactive_surrogate.json`
**用途**：10m空间优化 + 人机交互，使用GBR代理模型（推荐）。

- **用户数**：1个决策者
- **交互时机**：每30代暂停一次（300代共约10次交互）
- **代理模型**：`surrogate_model_comparison/surrogate_model.pkl`

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatial_interactive_surrogate.json
```

---

### 4.5 `10m_spatio_temporal_interactive_process.json`
**用途**：10m时空联合优化 + 人机交互，使用SEIMS物理模型，包含全套6个偏好指标。

- **用户数**：1个决策者
- **交互时机**：每10代暂停一次
- **偏好指标（全套）**：
  - `economy`：总投资 < 150万（5年总NPV，浮动容差±50万）
  - `environment`：产沙削减 > 5（容差±2）
  - `env_on_invest`：投资效益比 > 0.08（容差±0.02）
  - `return_on_invest`：收益投入比 > 1.2（容差±0.3）
  - `abandon_possibility`：放弃率 < 15%（容差±5%，时空模式专属）
  - `cost_variation`：年际费用波动 < 20万（容差±10万，时空模式专属）

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatio_temporal_interactive_process.json
```

---

### 4.6 `10m_spatio_temporal_interactive_surrogate.json`
**用途**：10m时空联合优化 + 人机交互，使用GBR代理模型。**2个决策者示例**（多决策者场景）。

- **用户数**：2个决策者，不同关注侧重
  - user1：关注总投资（<150万）、环境效益（>5）、投资效益比、放弃率
  - user2：关注总投资（<120万）、环境效益（>8）、收益投入比、年际费用波动
- **交互时机**：每30代暂停一次（300代共约10次）
- **多用户偏好融合**：采用加权平均合并多用户偏好评分，引导 Pareto 前沿向多方共同满意的区域收敛

```bash
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatio_temporal_interactive_surrogate.json
```

---

## 五、其他配置文件说明

| 文件 | 说明 |
|---|---|
| `surrogate_300x80_constrained.json` | 30m空间优化，代理模型，有约束，历史验证版 |
| `surrogate_300x80_unconstrained.json` | 30m空间优化，代理模型，无约束，历史验证版 |
| `test_surrogate_2000_5gen.json` | 仅用于快速功能测试（5代） |
| `test_surrogate_quick.json` | 仅用于快速功能测试 |

---

## 六、关键参数说明

### 6.1 优化模式（`mode`）

| 值 | 含义 |
|---|---|
| `spatial` | 纯空间优化：每个坡位单元选择BMP类型，时间固定为第1期同时实施 |
| `temporal` | 纯时序优化：固定BMP类型（从已有情景文件读取），优化各BMP的实施时期 |
| `spatio_temporal` | 时空联合优化：同时优化BMP类型和实施时期，是最完整的优化方案 |

### 6.2 评估模型（`surrogate.use_surrogate`）

| 值 | 含义 | 速度 | 精度 |
|---|---|---|---|
| `false` | 使用SEIMS物理过程模型 | 慢（~100s/次） | 高（真实模拟） |
| `true` | 使用预训练的机器学习代理模型 | 快（<1ms/次） | 高（R²>0.98） |

### 6.3 代理模型文件路径

| 分辨率 | 路径 | 模型类型 | 训练样本 |
|---|---|---|---|
| 30m | `data/youwuzhen/surrogate_training_2000_optimized/models/` | XGBoost | 2000，纯空间 |
| 10m | `data/youwuzhen10m/surrogate_model_comparison/` | GradientBoosting | 5000，时空 |

> 代理模型文件名固定为 `surrogate_model.pkl`（框架硬编码要求）

### 6.4 投资约束（`budget`）

```json
{
  "enable_investment_quota": true,     // 是否启用投资配额约束
  "investment_each_period": [100],     // 每期投资上限（万元）；单值=各期相同；多值=分期不同
  "investment_float_range": 0.2,       // 浮动幅度（0.2表示±20%）
  "years_first_period": 3              // 前N年使用第一个预算值（用于分期配置）
}
```

### 6.5 交互式优化（`interactive`）

```json
{
  "interactive": {
    "enable": true,                    // 是否开启交互式优化
    "interval_generations": 30,        // 每隔多少代暂停一次交互（过程模型推荐10，代理模型推荐30）
    "users": [
      {
        "user_id": "user1",            // 决策者标识（任意字符串）
        "preference_params": {
          // 每个指标的格式：[目标值h, 方向, 容差半径r]
          // 方向 "less"    → 越小越好（满足 value ≤ h 时得满分）
          // 方向 "greater" → 越大越好（满足 value ≥ h 时得满分）
          // 容差半径r：偏离目标 r 时满意度降为 0.5（余弦形满意度函数）
          "economy":           [150,  "less",    50],
          "environment":       [5,    "greater",  2],
          "env_on_invest":     [0.08, "greater", 0.02],
          "return_on_invest":  [1.2,  "greater",  0.3],
          "abandon_possibility":[0.15, "less",   0.05],
          "cost_variation":    [20,   "less",    10]
        }
      }
    ]
  }
}
```

### 6.6 可用偏好指标（`preference_params` 键名）

| 指标名 | 含义 | 可用模式 | 建议方向 |
|---|---|---|---|
| `economy` | 总投资成本（万元） | 所有模式 | `less` |
| `environment` | 产沙削减量 | 所有模式 | `greater` |
| `env_on_invest` | 环境效益/投资比（效益率） | 所有模式 | `greater` |
| `return_on_invest` | 折现收益/折现成本比（ROI） | 所有模式 | `greater` |
| `abandon_possibility` | BMP放弃率（多期实施中被放弃的比例） | 仅时间/时空模式 | `less` |
| `cost_variation` | 年际净成本波动（万元） | 仅时间/时空模式 | `less` |

> **注意**：在空间模式下指定 `abandon_possibility` 或 `cost_variation` 时，系统会输出 `[WARNING]` 提示
> 该指标在单期模式下无实际意义，但不会报错，将被静默跳过。

---

## 七、注意事项

1. **运行前确保 MongoDB 已启动**：所有配置均需要 `127.0.0.1:27017` 的 MongoDB 服务
2. **bin_dir 路径**：确保 `D:/EGC/SEIMS-dev/build/bin` 下存在 `seims_omp.exe`（使用 SEIMS 过程模型时必需）
3. **代理模型切换**：修改 `surrogate.use_surrogate` 为 `true/false` 即可在过程模型和代理模型之间切换
4. **分辨率选择**：
   - 30m 分辨率更快，适合初步分析
   - 10m 分辨率更精细，适合最终分析
5. **算法参数调整**：`GenerationsNum` 和 `PopulationSize` 可根据计算资源按比例调整，不建议低于 50代×40个
6. **交互式优化使用建议**：
   - 首次运行建议先用代理模型版（`*_surrogate.json`）熟悉偏好设置
   - preference_params 中的目标值需根据试运行结果中 Pareto 前沿的实际范围进行调整
   - 多用户场景下，系统会对各用户的偏好评分取加权平均后再引导选择

---

*最后更新：2026-04-16*
