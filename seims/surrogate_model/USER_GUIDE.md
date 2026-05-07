# 代理模型训练框架 - 用户使用指南

## 一、用户需要准备的数据与参数

### 1.1 必需的前置数据（与现有SEIMS优化保持一致）

用户需要已经完成以下SEIMS预处理步骤：

#### ✅ 流域基础数据（通过preprocessing完成）
- DEM、土地利用、土壤类型等栅格数据
- 气象数据（降雨、温度等时间序列）
- MongoDB数据库中已导入的流域模型数据
- 空间单元划分结果（SLPPOS/LANDUSE等）

#### ✅ BMP配置数据（存储在MongoDB中）
- BMP类型定义（TERRACING、GRASS_WATERWAY等）
- 每个BMP的适用空间单元（SLPPOS/LANDUSE）
- BMP的成本参数（COST）
- BMP的效益参数（INCOME，可选）
- BMP的有效性等级（EFFECTIVENESS，可选）

**数据位置示例**：
```
MongoDB数据库: model_youwuzhen30m_longterm_scenario
  - Collection: BMP_SCENARIOS
    - 文档示例:
      {
        "SUBSCENARIO": 1,
        "NAME": "TERRACING",
        "SLPPOS": [1, 2, 3],  # 适用于坡位1,2,3
        "LANDUSE": [33],       # 适用于土地利用类型33
        "COST": 2400,
        "EFFECTIVENESS": 2
      }
```

### 1.2 用户需要指定的配置参数

用户只需创建一个JSON配置文件，参数与现有优化系统保持一致：

```json
{
  "project": {
    "name": "youwuzhen_surrogate",
    "work_dir": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training"
  },

  "seims": {
    "model_dir": "D:/EGC/SEIMS-dev/data/youwuzhen/demo_youwuzhen30m_longterm_model",
    "bin_dir": "D:/EGC/SEIMS-dev/build/bin",
    "model_name": "model_youwuzhen30m_longterm",
    "host": "127.0.0.1",
    "port": 27017,
    "scenario_db": "model_youwuzhen30m_longterm_scenario",
    "nthread": 4,
    "lyrmtd": 0
  },

  "spatial": {
    "unit": "SLPPOS",
    "config_method": "HILLSLP"
  },

  "sampling": {
    "n_samples": 500,
    "seed": 12345
  },

  "evaluation": {
    "eval_stime": "2014-01-01 00:00:00",
    "eval_etime": "2015-12-31 23:59:59",
    "output_vars": ["SED", "Q", "TN"]
  },

  "training": {
    "test_size": 0.2,
    "epochs": 200,
    "target_r2": 0.85
  }
}
```

#### 参数说明（与现有系统对应关系）

| 参数组 | 参数名 | 说明 | 对应现有系统 |
|--------|--------|------|--------------|
| **seims** | model_dir | 模型目录 | 同`test_spatial_quick.json`的`model.model_dir` |
| | bin_dir | 可执行文件目录 | 同`model.bin_dir` |
| | model_name | 模型名称 | 同`model.model_name` |
| | host/port | MongoDB地址 | 同`model.host/port` |
| | scenario_db | 场景数据库名 | 同`model.scenario_db` |
| | nthread | 线程数 | SEIMS运行参数 |
| **spatial** | unit | 空间单元类型 | 同`spatial.unit`（SLPPOS/LANDUSE） |
| | config_method | 配置方法 | 同`spatial.config_method`（HILLSLP/UPDOWN） |
| **sampling** | n_samples | 采样数量 | **新增**，建议500-1000 |
| | seed | 随机种子 | **新增**，保证可重复性 |
| **evaluation** | eval_stime | 评估开始时间 | 同`evaluation.eval_stime` |
| | eval_etime | 评估结束时间 | 同`evaluation.eval_etime` |
| | output_vars | 输出变量 | **新增**，如["SED","Q","TN"] |
| **training** | test_size | 测试集比例 | **新增**，默认0.2 |
| | epochs | 训练轮数 | **新增**，默认200 |
| | target_r2 | 目标R²精度 | **新增**，默认0.85 |

---

## 二、训练流程与中间检查点

### 2.1 完整训练流程

```
用户准备配置文件 (surrogate_config.json)
         ↓
运行训练命令: python main.py --config surrogate_config.json
         ↓
[阶段1] 拉丁超立方采样 (1分钟)
         ↓ 检查点1: samples/lhs_samples.csv
[阶段2] 生成BMP方案 (5分钟)
         ↓ 检查点2: samples/scenario_configs/*.txt
[阶段3] 批量SEIMS模拟 (2-8小时，取决于样本数)
         ↓ 检查点3: simulation_results/progress.log
         ↓ 检查点4: simulation_results/collected_data.csv
[阶段4] 数据预处理 (1分钟)
         ↓ 检查点5: training/train_data.csv, test_data.csv
[阶段5] 神经网络训练 (10-30分钟)
         ↓ 检查点6: training/training_history.csv
         ↓ 检查点7: training/plots/*.png
[阶段6] 模型评估与保存 (1分钟)
         ↓ 最终输出: models/surrogate_model.h5
```

### 2.2 中间结果检查点详解

#### 检查点1: `work_dir/samples/lhs_samples.csv`
**生成时机**: 采样完成后
**内容**: n_samples × gene_num 的采样矩阵
**用户检查**:
```csv
sample_id,gene_0,gene_1,gene_2,...,gene_N
0,0,1,0,...,1
1,1,0,1,...,0
...
```
- 检查样本数是否正确（n_samples行）
- 检查基因数是否正确（gene_num列）
- 检查取值范围（0/1或其他指定范围）

#### 检查点2: `work_dir/samples/scenario_configs/`
**生成时机**: 方案生成完成后
**内容**: n_samples个Scenario配置文件
**用户检查**:
```
scenario_configs/
├── scenario_0001.txt
├── scenario_0002.txt
└── ...
```
- 随机打开几个文件，检查BMP配置是否合理
- 确认空间单元分配正确

#### 检查点3: `work_dir/simulation_results/progress.log`
**生成时机**: 模拟过程中实时更新
**内容**: 模拟进度和错误信息
**用户检查**:
```
[2026-03-26 10:30:15] 开始批量模拟，共500个样本，4进程并行
[2026-03-26 10:31:20] [1/500] scenario_0001 完成，耗时65秒
[2026-03-26 10:32:18] [2/500] scenario_0002 完成，耗时58秒
[2026-03-26 10:33:05] [3/500] scenario_0003 失败，错误: ...
...
[2026-03-26 14:25:30] 模拟完成，成功495个，失败5个
```
- 监控模拟进度
- 检查失败率（<5%可接受）
- 估算剩余时间

#### 检查点4: `work_dir/simulation_results/collected_data.csv`
**生成时机**: 所有模拟完成后
**内容**: 训练数据集（方案→响应）
**用户检查**:
```csv
scenario_id,gene_0,gene_1,...,gene_N,SED,Q,TN
scenario_0001,0,1,...,1,12345.6,234.5,1.23
scenario_0002,1,0,...,0,23456.7,345.6,2.34
...
```
- 检查数据完整性（成功样本数）
- 检查响应值范围是否合理
- 识别异常值（极大或极小）

#### 检查点5: `work_dir/training/train_data.csv` & `test_data.csv`
**生成时机**: 数据预处理完成后
**内容**: 划分后的训练集和测试集
**用户检查**:
```
training/
├── train_data.csv  (80%样本)
├── test_data.csv   (20%样本)
└── data_statistics.txt  (统计信息)
```
- 检查划分比例
- 查看`data_statistics.txt`了解数据分布

#### 检查点6: `work_dir/training/training_history.csv`
**生成时机**: 训练过程中实时更新
**内容**: 每轮训练的损失值
**用户检查**:
```csv
epoch,loss,val_loss,lr
1,0.523,0.612,0.001
2,0.412,0.534,0.001
...
50,0.045,0.052,0.0001
```
- 检查loss是否持续下降
- 检查val_loss是否过拟合（val_loss上升而loss下降）
- 确认早停是否触发

#### 检查点7: `work_dir/training/plots/`
**生成时机**: 训练完成后
**内容**: 可视化图表
**用户检查**:
```
plots/
├── training_curves.png      (loss曲线)
├── predictions_SED.png      (SED预测vs真实)
├── predictions_Q.png        (Q预测vs真实)
├── predictions_TN.png       (TN预测vs真实)
└── residuals.png            (残差分布)
```
- 查看训练曲线是否收敛
- 查看预测散点图是否接近y=x直线
- 查看残差是否随机分布

---

## 三、最终代理模型的生成与保存

### 3.1 输出文件结构

训练成功后，在`work_dir/models/`目录下生成以下文件：

```
models/
├── surrogate_model.h5           # Keras模型文件（核心）
├── data_scaler.pkl              # 数据标准化器
├── model_metadata.json          # 模型元数据
└── evaluation_report.txt        # 评估报告
```

### 3.2 文件说明

#### `surrogate_model.h5`
- **格式**: Keras HDF5格式
- **大小**: 通常1-10MB
- **内容**: 训练好的神经网络权重
- **加载方式**: `tensorflow.keras.models.load_model()`

#### `data_scaler.pkl`
- **格式**: Python pickle格式
- **内容**: StandardScaler对象（用于输入数据标准化）
- **加载方式**: `joblib.load()`

#### `model_metadata.json`
- **格式**: JSON
- **内容**: 模型配置和性能指标
```json
{
  "model_type": "multi_output_ann",
  "input_dim": 105,
  "output_dim": 3,
  "output_vars": ["SED", "Q", "TN"],
  "n_samples": 500,
  "training_date": "2026-03-26",
  "performance": {
    "SED": {"r2": 0.92, "rmse": 1234.5, "mae": 987.6},
    "Q": {"r2": 0.89, "rmse": 56.7, "mae": 43.2},
    "TN": {"r2": 0.87, "rmse": 0.12, "mae": 0.09}
  },
  "gene_num": 105,
  "spatial_unit": "SLPPOS",
  "config_method": "HILLSLP"
}
```

#### `evaluation_report.txt`
- **格式**: 纯文本
- **内容**: 详细的评估报告
```
========================================
代理模型评估报告
========================================
训练日期: 2026-03-26 15:30:45
样本数量: 500 (训练集400, 测试集100)
输入维度: 105 (基因数)
输出维度: 3 (SED, Q, TN)

----------------------------------------
测试集性能指标
----------------------------------------
SED (泥沙):
  R² = 0.9234
  RMSE = 1234.56 kg
  MAE = 987.65 kg
  MAPE = 8.23%

Q (径流):
  R² = 0.8912
  RMSE = 56.78 m³
  MAE = 43.21 m³
  MAPE = 6.45%

TN (总氮):
  R² = 0.8756
  RMSE = 0.123 kg
  MAE = 0.098 kg
  MAPE = 7.89%

----------------------------------------
模型结构
----------------------------------------
Input(105) → Dense(128) → Dense(64) → Dense(32) → Output(3)
总参数量: 18,563
训练轮数: 87 (早停)
训练时长: 23分钟

----------------------------------------
结论
----------------------------------------
✓ 所有输出变量R² > 0.85，满足精度要求
✓ 模型可用于优化加速
```

---

## 四、代理模型嵌入优化器的方式

### 4.1 方式一：修改配置文件（推荐）

在现有的优化配置JSON中添加`surrogate_model`字段：

```json
{
  "mode": "spatial",
  "model": {
    "model_dir": "D:/EGC/SEIMS-dev/data/youwuzhen/demo_youwuzhen30m_longterm_model",
    ...
  },
  "surrogate_model": {
    "enabled": true,
    "model_path": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training/models/surrogate_model.h5",
    "scaler_path": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training/models/data_scaler.pkl",
    "metadata_path": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training/models/model_metadata.json"
  },
  "algorithm": {
    "GenerationsNum": 100,
    "PopulationSize": 60
  }
}
```

### 4.2 方式二：命令行参数

```bash
python run_unified_v2.py \
  --config test_spatial.json \
  --use-surrogate \
  --surrogate-model "path/to/surrogate_model.h5"
```

### 4.3 代码集成（自动完成，用户无需修改）

框架会在`seims/scenario_analysis/spatialunits/scenario.py`中自动集成：

```python
# 原有代码（调用SEIMS模拟）
def calculate_environment(self):
    if self.cfg.use_surrogate:
        # 使用代理模型预测
        from seims.surrogate_model.predict import SurrogatePredictor
        predictor = SurrogatePredictor(
            self.cfg.surrogate_model_path,
            self.cfg.surrogate_scaler_path,
            self.cfg.surrogate_metadata_path
        )
        sed, q, tn = predictor.predict(self.gene_values)
        return sed  # 或其他目标变量
    else:
        # 调用SEIMS模拟（原有逻辑）
        return self.run_seims_simulation()
```

### 4.4 使用效果对比

| 项目 | SEIMS物理模型 | 代理模型 | 加速比 |
|------|---------------|----------|--------|
| 单次评估时间 | ~60秒 | ~0.05秒 | 1200× |
| 100代×60个体 | 100小时 | 5分钟 | 1200× |
| 精度 | 100%（真实值） | R²>0.85 | - |
| 适用场景 | 最终验证 | 快速优化 | - |

---

## 五、成熟方法与库的使用

### 5.1 采样方法

**使用库**: `pyDOE2` 或 `scipy.stats.qmc`

```python
# 方案1: pyDOE2 (更简单)
from pyDOE2 import lhs
samples = lhs(n_vars, samples=n_samples, criterion='maximin')

# 方案2: scipy (Python 3.8+)
from scipy.stats import qmc
sampler = qmc.LatinHypercube(d=n_vars)
samples = sampler.random(n=n_samples)
```

**无需自己编写LHS算法**，直接调用成熟库。

### 5.2 神经网络模型

**使用库**: `TensorFlow/Keras` (推荐) 或 `PyTorch`

```python
# Keras实现（极简）
from tensorflow import keras

model = keras.Sequential([
    keras.layers.Dense(128, activation='relu', input_dim=input_dim),
    keras.layers.BatchNormalization(),
    keras.layers.Dropout(0.2),
    keras.layers.Dense(64, activation='relu'),
    keras.layers.BatchNormalization(),
    keras.layers.Dropout(0.2),
    keras.layers.Dense(32, activation='relu'),
    keras.layers.Dense(output_dim)  # 多输出回归
])

model.compile(optimizer='adam', loss='mse', metrics=['mae'])
model.fit(X_train, y_train, validation_data=(X_val, y_val),
          epochs=200, callbacks=[early_stopping, reduce_lr])
```

**无需自己实现反向传播、优化器等**，Keras自动处理。

### 5.3 数据预处理

**使用库**: `scikit-learn`

```python
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split

# 数据划分
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# 标准化
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)
```

**无需自己实现标准化算法**，sklearn提供。

### 5.4 模型评估

**使用库**: `scikit-learn`

```python
from sklearn.metrics import r2_score, mean_squared_error, mean_absolute_error

r2 = r2_score(y_true, y_pred)
rmse = mean_squared_error(y_true, y_pred, squared=False)
mae = mean_absolute_error(y_true, y_pred)
```

**无需自己实现评估指标**，sklearn提供。

### 5.5 可视化

**使用库**: `matplotlib` + `seaborn`

```python
import matplotlib.pyplot as plt
import seaborn as sns

# 训练曲线
plt.plot(history.history['loss'], label='train')
plt.plot(history.history['val_loss'], label='val')
plt.legend()
plt.savefig('training_curves.png')

# 预测散点图
sns.scatterplot(x=y_true, y=y_pred)
plt.plot([y_true.min(), y_true.max()], [y_true.min(), y_true.max()], 'r--')
plt.savefig('predictions.png')
```

**无需自己实现绘图逻辑**，matplotlib/seaborn提供。

### 5.6 模型保存与加载

**使用库**: `tensorflow.keras` + `joblib`

```python
# 保存
model.save('surrogate_model.h5')
joblib.dump(scaler, 'data_scaler.pkl')

# 加载
from tensorflow.keras.models import load_model
import joblib

model = load_model('surrogate_model.h5')
scaler = joblib.load('data_scaler.pkl')
```

**无需自己实现序列化**，Keras和joblib提供。

---

## 六、完整使用示例

### 步骤1: 准备配置文件

创建 `surrogate_config.json`:
```json
{
  "project": {
    "name": "youwuzhen_surrogate",
    "work_dir": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training"
  },
  "seims": {
    "model_dir": "D:/EGC/SEIMS-dev/data/youwuzhen/demo_youwuzhen30m_longterm_model",
    "bin_dir": "D:/EGC/SEIMS-dev/build/bin",
    "model_name": "model_youwuzhen30m_longterm",
    "host": "127.0.0.1",
    "port": 27017,
    "scenario_db": "model_youwuzhen30m_longterm_scenario"
  },
  "spatial": {
    "unit": "SLPPOS",
    "config_method": "HILLSLP"
  },
  "sampling": {
    "n_samples": 500
  },
  "evaluation": {
    "eval_stime": "2014-01-01 00:00:00",
    "eval_etime": "2015-12-31 23:59:59",
    "output_vars": ["SED"]
  },
  "training": {
    "epochs": 200,
    "target_r2": 0.85
  }
}
```

### 步骤2: 运行训练

```bash
cd D:\EGC\SEIMS-dev\seims\surrogate_model
python main.py --config surrogate_config.json
```

### 步骤3: 监控进度

```bash
# 查看采样结果
cat work_dir/samples/lhs_samples.csv

# 监控模拟进度
tail -f work_dir/simulation_results/progress.log

# 查看训练曲线
# 打开 work_dir/training/plots/training_curves.png
```

### 步骤4: 检查模型质量

```bash
# 查看评估报告
cat work_dir/models/evaluation_report.txt

# 如果R² < 0.85，调整配置重新训练：
# - 增加n_samples到1000
# - 增加epochs到300
# - 调整网络结构（在代码中修改hidden_layers）
```

### 步骤5: 使用代理模型优化

修改优化配置 `test_spatial_with_surrogate.json`:
```json
{
  "mode": "spatial",
  "surrogate_model": {
    "enabled": true,
    "model_path": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training/models/surrogate_model.h5",
    "scaler_path": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training/models/data_scaler.pkl",
    "metadata_path": "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training/models/model_metadata.json"
  },
  "algorithm": {
    "GenerationsNum": 100,
    "PopulationSize": 60
  }
}
```

运行优化：
```bash
cd D:\EGC\SEIMS-dev\seims\scenario_analysis
python run_unified_v2.py --config test_spatial_with_surrogate.json
```

---

## 七、常见问题

### Q1: 采样数量如何确定？
**A**: 建议基因数×5到×10倍。例如105个基因，建议500-1000样本。

### Q2: 模拟失败率过高怎么办？
**A**: 检查BMP配置是否合理，某些组合可能导致SEIMS崩溃。可在采样时添加约束。

### Q3: 训练时间过长怎么办？
**A**:
- 减少epochs（如100）
- 使用GPU加速（需安装tensorflow-gpu）
- 简化网络结构

### Q4: 模型精度不达标怎么办？
**A**:
- 增加采样数量
- 延长训练时间
- 使用集成模型（Random Forest + ANN）
- 检查数据质量（是否有异常值）

### Q5: 代理模型能否用于其他流域？
**A**: 不能直接迁移。需要为新流域重新训练，但可以使用迁移学习加速训练。

---

## 八、总结

本框架通过以下方式简化用户操作：

✅ **配置驱动**: 用户只需准备JSON配置文件，无需修改代码
✅ **参数一致**: 与现有SEIMS优化系统参数保持一致，学习成本低
✅ **成熟库**: 全部使用成熟的第三方库（pyDOE2、Keras、sklearn），无需自己编写算法
✅ **中间检查**: 每个阶段生成可检查的中间结果，便于调试
✅ **自动集成**: 代理模型自动嵌入优化器，用户只需修改配置文件
✅ **详细报告**: 生成完整的评估报告和可视化图表，便于质量评估

**核心优势**: 用户无需了解机器学习细节，只需准备与现有优化系统相同的数据和参数，即可训练出高精度代理模型，实现1000倍以上的优化加速。
