# 代理模型数据流详细说明

## 1. MongoDB存储位置问题

### 训练样本的存储
**回答：不在同一位置，训练样本不存储到MongoDB**

- **训练阶段**：生成的BMP场景样本**临时**写入MongoDB，仅用于SEIMS模拟
  - 数据库：`scenario_db`（如 `model_youwuzhen30m_longterm_scenario`）
  - 集合：`BMP_SCENARIOS`
  - 每个样本有唯一的`SCENARIO_ID`
  - **模拟完成后可以删除**，因为基因值已保存在CSV文件中

- **正式优化阶段**：优化器生成的场景也写入同一个MongoDB
  - 使用相同的数据库和集合
  - 但`SCENARIO_ID`不同（优化器会分配新的ID）
  - 优化过程中的场景会持久保存

### 关键区别
```
训练阶段：
  samples.csv (基因值) → MongoDB (临时) → SEIMS模拟 → collected_data.csv (结果)
                          ↓
                    模拟后可删除

优化阶段：
  优化器生成基因值 → MongoDB (持久) → SEIMS模拟 → 评估适应度
                      ↓
                  保存最优解
```

## 2. 训练数据格式

### 输入：BMP方案数组
一个样本 = 一个基因值数组，例如72个空间单元：
```
[0, 0, 0, 0, 0, 4, 0, 2, 0, 1, ..., 0, 0]
 ↑  ↑  ↑  ↑  ↑  ↑
单元1 单元2 ... 单元72
```

每个值代表该空间单元上实施的BMP类型ID：
- `0` = 不实施BMP
- `1,2,3,4` = 不同的BMP子情景（如不同的植被缓冲带宽度）

### 输出：流域响应值

**一个BMP方案 → 一个或多个流域响应值**

当前实现支持多个输出变量（多输出神经网络）：
```python
output_vars = ['SED', 'Q', 'TN']  # 可配置
```

每个样本对应的输出：
```
样本1: [SED=1234.5, Q=5678.9, TN=12.3]
样本2: [SED=1100.2, Q=5500.1, TN=11.8]
...
```

## 3. 输出值的含义和计算方法

### 3.1 SED（泥沙）的计算

**读取的文件**：`SED_OL_SUM.tif` 或时间序列txt文件

**计算逻辑**（见scenario.py第850-867行）：

```python
# 1. 读取模拟输出文件
rfile = modelout_dir + '/' + 'SED_OL_SUM.tif'  # 或 .txt

# 2. 如果是栅格文件
if rfile.endswith('.tif'):
    rr = RasterUtilClass.read_raster(rfile)
    sed_sum = rr.get_sum() / eval_timerange  # 年均泥沙总量（吨/年）

# 3. 如果是时间序列文件
elif rfile.endswith('.txt'):
    sed_sum = read_simulation_from_txt(modelout_dir, ['SED'],
                                       OutletID, eval_stime, eval_etime)

# 4. 判断是绝对值还是削减率
if BASE_ENV < 0:  # 基准场景（无BMP）
    environment = sed_sum  # 绝对值（吨/年）
    sed_sum = sed_sum
else:  # BMP场景
    environment = (BASE_ENV - sed_sum) / BASE_ENV  # 削减率（0-1）
    sed_sum = sed_sum  # 保存绝对值
```

### 3.2 代理模型训练使用的值

**关键：代理模型使用的是绝对值（sed_sum），不是削减率**

原因：
1. **绝对值更稳定**：削减率依赖BASE_ENV，不同基准场景会导致不一致
2. **可以后续计算削减率**：有了绝对值，可以用任何基准场景计算削减率
3. **适用于多种优化目标**：可以最小化绝对值，也可以最大化削减率

```python
# batch_runner.py 第39-44行
results = {'sample_id': sample_id}
for var in output_vars:
    if var == 'SED':
        results['SED'] = scenario.sed_sum  # 使用绝对值
    elif var == 'Q':
        results['Q'] = scenario.runoff_sum  # 径流总量
    elif var == 'TN':
        results['TN'] = scenario.tn_sum  # 总氮总量
```

### 3.3 输出文件位置

SEIMS模拟输出目录：
```
model_dir/OUTPUT/
├── SED_OL_SUM.tif          # 泥沙累积总量栅格
├── Q_OL.txt                # 径流时间序列
├── SED_OL.txt              # 泥沙时间序列
└── TN_OL.txt               # 总氮时间序列
```

配置文件中指定：
```json
"evaluation": {
    "output_vars": ["SED", "Q", "TN"],
    "eval_stime": "2012-02-27 00:00:00",
    "eval_etime": "2012-04-30 23:59:59"
}
```

对应INI文件：
```ini
[BMPs]
Eval_info = {"OUTPUTID": "SED_OL", "ENVEVAL": "SED_OL_SUM.tif", "BASE_ENV": -9999}
```

## 4. 并行模拟实现

### 4.1 并行策略

使用Python `multiprocessing.Pool` 实现进程级并行：

```python
# batch_runner.py 第84-89行
if n_jobs > 1:
    with Pool(processes=n_jobs) as pool:
        results = pool.map(run_single_scenario, args_list)
else:
    results = [run_single_scenario(args) for args in args_list]
```

### 4.2 并行流程

```
主进程
  ├─ 准备100个样本的参数列表
  ├─ 创建进程池（n_jobs=4）
  └─ 分发任务
      ├─ 子进程1: 样本0-24  ─┐
      ├─ 子进程2: 样本25-49  ├─ 并行执行
      ├─ 子进程3: 样本50-74  │
      └─ 子进程4: 样本75-99 ─┘
           ↓
      收集所有结果
```

### 4.3 单个样本的执行流程

```python
def run_single_scenario(args):
    sample_id, gene_values, config_ini, output_vars = args

    # 1. 创建场景对象
    scenario = SUScenario(sa_config)
    scenario.initialize(input_genes=gene_values)
    scenario.ID = sample_id

    # 2. 写入MongoDB
    scenario.export_scenario_to_mongodb()

    # 3. 运行SEIMS模拟
    scenario.execute_seims_model()

    # 4. 读取结果文件
    results = {
        'sample_id': sample_id,
        'SED': scenario.sed_sum,      # 从OUTPUT/SED_OL_SUM.tif读取
        'Q': scenario.runoff_sum,     # 从OUTPUT/Q_OL.txt读取
        'TN': scenario.tn_sum         # 从OUTPUT/TN_OL.txt读取
    }

    return results
```

### 4.4 并行性能

假设：
- 单个SEIMS模拟耗时：60秒
- 并行进程数：4

串行执行100个样本：
```
100 × 60秒 = 6000秒 ≈ 100分钟
```

并行执行100个样本：
```
(100 / 4) × 60秒 = 1500秒 ≈ 25分钟
```

**加速比：4倍**

### 4.5 并行限制

1. **MongoDB连接**：每个进程独立连接MongoDB
2. **磁盘I/O**：多个进程同时写入可能有竞争
3. **内存占用**：每个进程加载完整的SEIMS模型
4. **CPU核心数**：建议 `n_jobs = CPU核心数 - 1`

推荐配置：
```json
"simulation": {
    "parallel_jobs": 4  // 8核CPU建议4-6
}
```

## 5. 完整数据流示意图

```
[1] 场景采样
    samples.csv
    ├─ sample_0: [0,0,4,2,1,...]  (72 genes)
    ├─ sample_1: [0,3,0,4,2,...]
    └─ sample_99: [4,0,2,0,3,...]

[2] 批量模拟（并行）
    ├─ 进程1 ─┬─ sample_0 → MongoDB → SEIMS → SED=1234.5
    ├─ 进程2 ─┤  sample_1 → MongoDB → SEIMS → SED=1100.2
    ├─ 进程3 ─┤  sample_2 → MongoDB → SEIMS → SED=1350.8
    └─ 进程4 ─┘  sample_3 → MongoDB → SEIMS → SED=1280.3

[3] 收集结果
    collected_data.csv
    ├─ sample_id, SED, Q, TN, gene_0, gene_1, ..., gene_71
    ├─ 0, 1234.5, 5678.9, 12.3, 0, 0, 4, 2, 1, ...
    ├─ 1, 1100.2, 5500.1, 11.8, 0, 3, 0, 4, 2, ...
    └─ 99, 1280.3, 5600.5, 12.0, 4, 0, 2, 0, 3, ...

[4] 训练神经网络
    输入X: gene_0, gene_1, ..., gene_71  (72维)
    输出y: SED, Q, TN                    (3维)

    模型: X → [128] → [64] → [32] → y

[5] 保存模型
    surrogate_model.h5
    scaler_X.pkl  (输入标准化)
    scaler_y.pkl  (输出标准化)
    model_metadata.json

[6] 优化器集成
    from surrogate_model.predict import SurrogatePredictor
    predictor = SurrogatePredictor(model_file, scaler_X, scaler_y)

    # 替代SEIMS模拟
    gene_values = [0, 0, 4, 2, 1, ...]
    result = predictor.predict(gene_values)
    # result = {'SED': 1234.5, 'Q': 5678.9, 'TN': 12.3}

    # 加速：0.001秒 vs 60秒 = 60000倍
```

## 6. 关键代码位置

| 功能 | 文件 | 行号 | 说明 |
|------|------|------|------|
| 泥沙计算 | scenario.py | 850-867 | sed_sum绝对值计算 |
| 削减率计算 | scenario.py | 866 | environment削减率 |
| 并行执行 | batch_runner.py | 84-89 | multiprocessing.Pool |
| 结果收集 | batch_runner.py | 39-44 | 读取scenario属性 |
| MongoDB写入 | scenario.py | ~600 | export_scenario_to_mongodb |
| SEIMS执行 | scenario.py | ~700 | execute_seims_model |

## 7. 常见问题

**Q1: 为什么不直接用削减率训练？**
A: 削减率依赖BASE_ENV基准值，不同研究可能用不同基准。用绝对值更通用。

**Q2: 并行会不会导致MongoDB冲突？**
A: 不会。每个场景有唯一SCENARIO_ID，MongoDB支持并发写入。

**Q3: 如果某个样本模拟失败怎么办？**
A: batch_runner会捕获异常，返回None，最后过滤掉失败样本。

**Q4: 训练样本需要多少个？**
A: 建议100-500个。太少欠拟合，太多训练时间长。可以先用100个测试。

**Q5: 代理模型精度如何？**
A: 目标R²>0.85。如果达不到，增加样本数或调整网络结构。
