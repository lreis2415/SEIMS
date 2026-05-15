# 工作进展记录

## 日期：2026-05-13

## 任务：对比并合并 wwj/validation-2026-05-07 分支的合理修改

### 1. 背景与目标

用户要求对比当前 `revise-fxy` 分支与远程仓库 `go-bananas-wwj/SEIMS` 的 `validation-2026-05-07` 分支的差异，将对方修改中合理的部分合并到本地，最终目的是改善 **Andrews Forest 模型在 storm 模式下的建模效果**。

### 2. 对比分析

- 对方分支共有约 100+ 次提交，时间跨度 2025-04 至 2026-05
- 修改集中在：
  - C++ 核心模块的水文过程改进（IKW_OL、IKW_CH、IKW_IF、SUR_SGA、PERCO_DARCY、GW_RSVR、SNO_SP）
  - MongoDB 驱动兼容性修复（mongoc_cursor_more 死条件）
  - Andrews Forest 暴雨模型参数调优（NSE 最高达到 0.7329）
  - 大量实验性调试代码和参数扫描脚本

### 3. 合并策略

**合并的修改**（33 个文件）：

| 类别 | 文件 | 说明 |
|------|------|------|
| **MongoDB 兼容性** | `DataCenterMongoDB.cpp`, `Scenario.cpp`, `InputStation.cpp`, `NotRegularMeasurement.cpp`, `RegularMeasurement.cpp`, `clsReach.cpp` | 将 `mongoc_cursor_more` 替换为 `mongoc_cursor_next`，修复 mongo-c-driver 1.30+ 下参数完全不加载的致命 bug；修复 `bson_destroy` use-after-free；修复内存分配 `delete[]` → `free` 不匹配 |
| **坡面汇流** | `IKW_OL.cpp/.h/api.cpp` | 修复 `m_qs` 数组未初始化的 bug；添加 `OL_SPEED_FACTOR` 可调参数；增加坡度下限 0.001 rad 防止除零 |
| **河道汇流** | `IKW_CH.cpp/.h/api.cpp` | 添加子时间步机制 `m_substeps=2`，显著改善峰现时间精度；修复 `m_qgDeep` 硬编码为 0；增加坡度下限；修复 `m_streamLink` 类型 |
| **壤中流** | `IKW_IF.cpp/.h/api.cpp` | 添加 moisture-scaled bypass 机制；新增 `fast_ratio`、`anisotropy`、`macroporeFactor` 三个可调参数；修复水量守恒 |
| **入渗** | `SUR_SGA.cpp/.h/api.cpp` | 修复 rain-on-snow 逻辑错误（`hWater = m_netPcp[i] + m_sd[i]`）；添加深层渗漏机制；新增 `INFIL_FACTOR`、`DEEP_PERC_RATE` 参数 |
| **渗漏** | `PERCO_DARCY.cpp/.h/api.cpp` | 添加 `poreIdx < 1e-6`  guard 防止 NaN/Inf；新增 `FC_ADJUST` 参数；修复析构函数空指针判断错误 |
| **地下水** | `GW_RSVR.cpp` | 修复 `m_storageMax` 初始化 bug；添加 GWMAX 超出强制释放逻辑；修复 `SetValue(GW0)` 不更新已分配 `m_storage` 的问题 |
| **融雪** | `SNO_SP.cpp/.h/api.cpp` | 支持 sub-daily 时间步长的 `dtFactor` 和 `lagEff` 修正 |
| **框架** | `invoke.cpp`, `main.cpp`, `ModelMain.cpp`, `DataCenter.cpp` | 修复路径末尾分隔符导致模型名解析错误；跳过未知命令行参数避免死循环 |
| **构建** | `FindLLVM.cmake`, `linklib.h` | CMake 4.x `VERSION_LESS` 空变量保护；TauDEM 循环条件 bug 修复 |
| **演示配置** | `demo_config.py` | 添加 AndrewsForest 支持 |
| **常量定义** | `text.h` | 新增 `VAR_FC_ADJUST`, `VAR_FAST_RATIO`, `VAR_ANISOTROPY` 等常量 |

**未合并的内容**：
- 大量实验性调试代码（已清理）
- Docker 配置、参数扫描脚本、实验报告
- 各轮次挑战的 param.cali 备份
- Andrews Forest 土壤数据修复（该修改在 MongoDB 中，未在 git 中）

### 4. 编译验证

- 使用 Homebrew LLVM Clang 成功编译 C++ 核心
- `seims_omp` 和 `seims_mpi` 均编译通过
- 所有模块动态库（`.dylib`）生成成功

### 5. 模型运行与评估

- Andrews Forest storm 模型成功运行完成（1020 个时间步，约 19 秒）
- 为支持新模块参数，向 MongoDB `andrews_forest_model.PARAMETERS` 插入了 7 个新参数
- 创建了 `FILE_OUT_SPEC` 集合以支持输出配置
- 将 `config.fig` 中的河道汇流模块从 `CH_DW` 切换为 `IKW_CH`（与代码改进匹配）

**当前运行结果**：
- NSE = -3.24
- PBIAS = -94.2%
- PeakError = -92.9%
- TTP_diff = -7.25h

**结果分析**：
模拟流量远低于实测值（均值 0.30 vs 4.65 m³/s）。主要原因是本地 MongoDB 中的 Andrews Forest **土壤数据存在问题**——wwj 分支的提交记录明确提到 "替换错误的中国 SOILLOOKUP，重建 FIELDCAP/POROSITY/SOILDEPTH 等栅格"。这些数据库层面的修复不在 git 仓库中，因此仅靠代码合并无法复现对方的 NSE=0.7329。

### 6. 下一步建议

1. **数据库修复**：需要按照 wwj 分支的方法，重新导入 Andrews Forest 的正确土壤属性数据（SOILLOOKUP、土壤栅格等）
2. **参数调优**：在数据库修复后，使用新引入的参数（`OL_SPEED_FACTOR`、`FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR`、`INFIL_FACTOR` 等）进行系统调参
3. **河道模块验证**：当前已切换为 `IKW_CH`，需确认其 sub-timestep 机制在目标流域的适用性

### 7. 代码提交

- 分支：`merge-wwj-validation`（基于 `revise-fxy`）
- 提交：`ac8ff175` — Merge reasonable improvements from wwj/validation-2026-05-07
