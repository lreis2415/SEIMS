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

---

## 日期：2026-05-13（续）— 土壤数据修复专项

### 背景

用户决定**不合并** wwj 的代码修改，仅使用其土壤修复思路。当前分支已回退到 `revise-fxy`（不合并 wwj 内容）。目标是修复 Andrews Forest storm 模型的土壤数据，使 FIELDCAP 等关键土壤属性恢复正常。

### 问题诊断

1. **SOILLOOKUP 集合被污染**：MongoDB 中 `SOILLOOKUP` 集合包含中国土壤编码（101~111, 231xxxxx），与 Andrews Forest 的 SOILTYPE 栅格（1-34）不匹配
2. **GridFS 土壤栅格全 0**：FIELDCAP、WILTINGPOINT 等栅格在 GridFS 中全为 0
3. **根本原因定位**：`data/AndrewsForest/data_prepare/lookup/soil_properties_lookup.csv` 中的 `SOL_FC` 和 `SOL_WP` 列本身就是 **全 0**。预处理脚本 `sp_soil.py` 的条件 `elif not self.FIELDCAP` 检查列表是否为空；当 CSV 提供 `[0, 0, 0, ...]` 时，列表非空，因此不会触发基于 SAND/CLAY/OM 的 SWAT 经验公式自动计算。0 被当作有效值保留下来，最终通过 C++ `mask_rasterio` 工具写入 GridFS 栅格。

### 修复措施

**措施 1：修复 MongoDB GridFS（立即生效）**

- 创建 `fix_soil_gridfs_v2.py` 脚本，复用 `seims/preprocess/sp_soil.py` 中的 `SoilProperty` 类
- 读取 CSV 时**不设置** FIELDCAP/WILTINGPOINT，让 `check_data_validation()` 自动根据土壤质地计算
- 脚本成功为全部 6 个子流域（136,812 个栅格单元）重新生成土壤属性栅格

**验证结果**：

| 属性 | Layer0 Min | Layer0 Max | Layer0 Mean | 状态 |
|------|-----------|-----------|------------|------|
| FIELDCAP | 0.3517 | 0.9034 | 0.8571 | ✅ 正常 |
| POROSITY | 0.6980 | 0.9250 | 0.9181 | ✅ 正常 |
| WILTINGPOINT | 0.0991 | 0.5370 | 0.3719 | ✅ 正常 |
| CONDUCTIVITY | 180.0 | 1440.0 | 1401.7 | ✅ 正常 |

**措施 2：修复源 CSV（为完整预处理做准备）**

- 修改 `soil_properties_lookup.csv`，将所有 `SOL_FC` 和 `SOL_WP` 列的 0 值替换为 `-9999`（`DEFAULT_NODATA`）
- 共修复 68 处零值条目
- 这样今晚重新运行完整预处理时，`sp_soil.py` 会检测到 `DEFAULT_NODATA` 并自动触发计算

### 新增/修改文件

| 文件 | 说明 |
|------|------|
| `fix_soil_gridfs_v2.py` | 改进版土壤栅格修复脚本，使用 SoilProperty 自动计算 FC/WP |
| `data/AndrewsForest/data_prepare/lookup/soil_properties_lookup.csv` | SOL_FC/SOL_WP 0 → -9999 |

### 待办事项（用户计划今晚执行）

1. **完整预处理**：重新运行 `demo_preprocess.py`，从干净的源数据重新生成 SOILLOOKUP 集合和所有 GridFS 栅格
2. **模型测试**：使用修复后的土壤数据运行 storm 模型，评估 NSE/PBIAS/PeakError
3. **参数调优**：若土壤修复后模拟流量仍然偏低，可尝试调整 `OL_SPEED_FACTOR`、`FAST_RATIO`、`ANISOTROPY` 等参数

---

## 日期：2026-05-31 — 工作目录只读盘点

### 工作内容

按用户要求读取 `/Users/flora/code/seims` 工作目录，建立当前仓库结构、关键入口文件、构建方式、数据目录和工作区状态的整体认知。

### 执行过程

- 使用 `rg --files`、`find`、`ls`、`git status` 等命令扫描顶层目录、源码目录、文档目录、数据目录和配置文件。
- 阅读了 `README.md`、`seims/README.md`、根目录 `CMakeLists.txt`、`seims/run_seims.py` 以及本文件最近的工作记录。
- 统计了非构建产物范围内的文件类型和数量，用于判断项目规模和主要文件构成。

### 决策依据

- 本次任务是“先读取整个工作目录”，因此只做只读扫描和关键文件抽样阅读，不运行模型、不编译、不修改业务代码。
- `build/` 和 `build-openmp-test/` 为已有构建产物目录，扫描时作为背景信息记录，源码理解以 `seims/`、`cmake/`、`data/`、`doc/`、`docker/`、`knowledge/` 为主。

### 最终结果

- 确认项目为 CMake 管理的 C++/Python/MongoDB 流域水文模型框架。
- 当前 Git 分支为 `revise-fxy`，本地领先远端 2 个提交；工作区存在用户/历史任务留下的已修改和未跟踪文件。
- 未改动源码、配置、数据或数据库；仅按项目约定追加本次工作日志。

### 遇到的问题与处理

- 工作目录包含大量栅格、构建产物和数据文件，完整逐字读取成本高且无必要；采用结构扫描、关键入口阅读和文件统计方式完成整体盘点。

---

## 日期：2026-05-31 — 安装 grill-me skill

### 工作内容

按用户要求为 Codex 下载并安装名为 `grill-me` 的 skill。

### 执行过程

- 读取系统内置 `skill-installer` skill 的安装说明。
- 尝试从 OpenAI 官方 skills curated/experimental 路径查找 `grill-me`，未找到对应路径。
- 定位到 `mattpocock/skills` 仓库中的 `skills/productivity/grill-me` 路径，并通过安装脚本完成安装。
- 验证安装目录 `/Users/flora/.codex/skills/grill-me` 中存在 `SKILL.md`。

### 决策依据

- 用户只提供 skill 名称，没有提供仓库地址；优先尝试官方来源，失败后使用公开可定位且名称匹配的 GitHub skill 来源。
- 安装使用系统提供的 `install-skill-from-github.py` 脚本，避免手工复制文件。

### 最终结果

- `grill-me` 已安装到 `/Users/flora/.codex/skills/grill-me`。
- 新 skill 需要重启 Codex 后才会被会话加载和识别。

### 遇到的问题与处理

- 官方 skills 列表接口返回 HTTP 403，改为直接探测具体 GitHub 路径。
- 官方仓库中未找到 `grill-me`，最终使用 `mattpocock/skills` 仓库的 `skills/productivity/grill-me` 路径安装成功。

---

## 日期：2026-05-31 — AndrewsForest storm 输出缺失 Q.txt 排查

### 工作内容

排查用户修复 Andrews Forest 土壤数据库后，运行 storm 模型时 `/Users/flora/code/seims/data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--` 下没有生成 `Q.txt` 的原因。

### 执行过程

- 检查 storm 模型目录、输出目录、`file.out`、`config.fig` 和旧运行日志。
- 查询 MongoDB `andrews_forest_model.FILE_OUT_SPEC` 与当前 `storm/file.out` 的差异。
- 使用当前配置直接运行 `build/bin/seims_omp` 复现问题，并将标准输出重定向后确认真实退出码。
- 使用 `lldb` 捕获段错误位置，定位到 `PrintInfoItem::Flush(...)` 输出阶段。

### 关键发现

- 当前 `storm/file.out` 请求的是 `QSUBBASIN`、`QS`、`QI`、`SBQG`、`SBGS`、`SOLST`。
- MongoDB 中 `FILE_OUT_SPEC` 仍是旧配置：`QOVERLAND -> QS.txt`、`QSOIL -> QI.txt`、`QSUBBASIN -> Q.txt`、`SBQG`、`SBGS`、`SOL_ST`。
- 运行时 C++ 优先读取 MongoDB 的 `FILE_OUT_SPEC`，因此实际输出项不是当前 `file.out`。
- 模型计算阶段可以跑到 1020 个时间步，但输出阶段退出码为 `139`，即 segmentation fault。
- 崩溃发生在 `PrintInfoItem::Flush(...)`，先写出 `QOVERLAND` 对应的 `2_QS.txt`，随后因二维坡面输出按 outlet/text 输出处理产生越界，程序在写 `QSUBBASIN -> Q.txt` 前已崩溃。

### 结论

`Q.txt` 没有生成，不是土壤数据库修复导致，也不是 `CH_DW` 不支持 `QSUBBASIN`；根因是 MongoDB `FILE_OUT_SPEC` 与当前 `storm/file.out` 不一致，且旧的 `QOVERLAND/QSOIL` 输出配置触发输出阶段段错误。

### 建议处理

- 刷新 MongoDB 中 `FILE_OUT_SPEC`，使其与当前 `storm/file.out` 一致。
- 或临时删除/禁用 `FILE_OUT_SPEC` 中的 `QOVERLAND`、`QSOIL`、`SOL_ST`，保留 `QSUBBASIN`、`QS`、`QI`、`SBQG`、`SBGS`、`SOLST`。
- 重新运行前建议清理旧输出目录，避免把旧的 `2_QS.txt` 和旧日志误认为新结果。

### 后续修复与验证

- 按当前 `data/AndrewsForest/andrews_forest_model/storm/file.out` 重建 MongoDB `andrews_forest_model.FILE_OUT_SPEC` 中 `SUB_MODEL=storm`、`TASK=SingleRun` 的 6 条记录。
- 删除了旧输出目录 `data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--` 后重新运行 `seims_omp`。
- 模型运行退出码为 `0`，输出阶段正常完成。
- 新输出目录已生成 `Q.txt`、`QS.txt`、`QI.txt`、`SBQG.txt`、`SBGS.txt`，以及 `SOLST_SUM_*.tif`、`SOLST_AVE_*.tif`。

### 本次修改范围

- 仅修改 MongoDB 中的 `FILE_OUT_SPEC` 运行输出配置。
- 未修改土壤数据库、源代码、模型参数、`config.fig` 或 `file.out`。

---

## 日期：2026-05-31 — 提取 AndrewsForest 流域出口观测流量

### 工作内容

按用户要求，从 AndrewsForest `observed` 文件夹中将流域出口的观测流量单独提取出来。

### 执行过程

- 读取 `data/AndrewsForest/data_prepare/observed/SiteInfo.csv`，确认 `isOutlet=1` 的出口流量站点为 `StationID=1`、`Name=GSLOOK`、`Type=Q`、单位为 `m3/s`。
- 从 `data/AndrewsForest/data_prepare/observed/observed_Q_storm.csv` 中筛选 `StationID=1` 的全部流量记录。
- 生成独立文件 `data/AndrewsForest/data_prepare/observed/observed_Q_storm_outlet.csv`。

### 最终结果

- 新文件共 32706 行，包括 2 行注释、1 行表头和 32703 条出口流量记录。
- 文件保留原始字段结构：`StationID,DATETIME,Type,VALUE`。

### 修改范围

- 新增 `data/AndrewsForest/data_prepare/observed/observed_Q_storm_outlet.csv`。
- 未修改原始 `SiteInfo.csv`、`observed_Q_storm.csv` 或其他模型配置。

---

## 日期：2026-05-31 — 土壤修复后模拟结果变化不大初步分析

### 工作内容

针对用户反馈“修改土壤数据后，模拟结果变化不大”，对 AndrewsForest storm 最新输出和出口观测流量进行快速对比，并查看主要径流分量。

### 执行过程

- 使用 `OUTPUT_D8_DOWNUP--/Q.txt` 与 `observed_Q_storm_outlet.csv` 按 300 秒时间步对齐。
- 计算观测与模拟的均值、峰值、NSE、PBIAS、RMSE。
- 检查 `QS.txt`、`QI.txt`、`SBQG.txt` 三个主要分量的均值、峰值和非零时段数量。
- 确认 MongoDB 中存在修复后的土壤空间栅格，如 `FIELDCAP`、`WILTINGPOINT`、`POROSITY`、`CONDUCTIVITY` 等。

### 对比结果

- 匹配时间步：1020 个，时间范围 `2015-02-05 07:00:00` 至 `2015-02-08 19:55:00`。
- 观测流量均值约 `5.19 m3/s`，峰值约 `10.45 m3/s`。
- 模拟流量均值约 `0.31 m3/s`，峰值约 `0.96 m3/s`。
- NSE 约 `-3.22`，PBIAS 约 `-93.95%`，RMSE 约 `5.59 m3/s`。
- `QS` 均值约 `0.069 m3/s`，峰值约 `0.639 m3/s`。
- `QI` 均值约 `0.053 m3/s`，峰值约 `0.416 m3/s`。
- `SBQG` 均值约 `0.070 m3/s`，变化很小，主要表现为稳定基流。

### 初步结论

土壤栅格修复后模型可以正常运行并输出结果，但当前 storm 事件的快速产流仍然明显不足。模拟流量偏低约一个数量级，说明问题不只是 `FIELDCAP/WILTINGPOINT` 是否为 0；更可能还涉及降雨输入量级、Green-Ampt 入渗参数、初始土壤含水状态、洼地蓄水/坡面汇流参数、地下水和河道汇流参数等。

### 后续建议

- 暂时不要继续盲目调土壤表，应先做事件水量平衡诊断。
- 建议临时增加 `NEPR`、`EXCP`、`INFIL`、`DPST`、`PERCO`、`QS`、`QI`、`SBQG` 等输出，确认降雨最终进入了入渗、土壤蓄水、地下水还是快速径流。
- 同时核对 storm 时段降雨输入总量和单位，判断是否存在降雨强度偏小或时间范围不匹配。

---

## 日期：2026-05-31 — 对比 wwj validation 分支中可能改善 AndrewsForest 流量偏低的修改

### 工作内容

按用户提供的 `go-bananas-wwj/SEIMS` 的 `validation-2026-05-07` 分支，比较其与当前本地分支的差异，筛选哪些修改可能改善 AndrewsForest storm 模拟流量过低、土壤修复后结果变化不大的问题。

### 执行过程

- 使用本地远程引用 `wwj/validation-2026-05-07` 与当前 `HEAD` 比较。
- 先查看全量差异，确认差异很大，包含大量实验输出、文档、脚本和调试工具，不适合整体合并。
- 重点检查水文过程模块与数据库参数读取相关差异：`SUR_SGA`、`IKW_IF`、`PERCO_DARCY`、`GW_RSVR`、`IKW_OL`、`IKW_CH`、`DataCenterMongoDB.cpp` 和参数常量定义。
- 结合当前最新模拟结果中 `QS/QI/SBQG` 均明显偏低的现象，按机制相关性排序。

### 关键发现

- `SUR_SGA` 将 `MOIST_IN` 初始土壤含水量从相对 `FieldCap` 初始化改为相对 `Porosity` 初始化，这会显著减少初始土壤亏缺，是最可能改变当前“降雨大量被土壤吸收、快速产流不足”的代码差异。
- `IKW_IF` 新增 `FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR`，并加入优先流、大孔隙流和更快的水平壤中流；当前 `QI` 极低，因此该改动与问题高度相关。
- `DataCenterMongoDB.cpp` 移除了多处 `mongoc_cursor_more(cursor) && mongoc_cursor_next(...)` 的读取模式。wwj 提交记录说明该问题可能导致 `PARAMETERS_SPEC` 参数完全不加载；若参数未真正加载，后续参数调整不会生效。
- `PERCO_DARCY` 添加 `FC_ADJUST` 并修复释放指针条件错误，但当前问题主要是快速产流不足，它更影响土壤水向地下水分配，优先级低于 `SUR_SGA` 和 `IKW_IF`。
- `GW_RSVR` 修复 `GW0` 对已分配 `m_storage` 不生效，以及 `GWMAX` 截断后的水量守恒；它主要影响基流，目前 `SBQG` 约为稳定小值，对洪峰改善有限。
- `IKW_OL` 新增 `OL_SPEED_FACTOR` 和坡面流相关调整，可影响 `QS` 与峰现时间，但当前水量偏低主要发生在产流/壤中流阶段。
- `IKW_CH` 有大量子时间步和调试性改动，且当前 `config.fig` 使用的是 `CH_DW` 而非 `IKW_CH`，因此不应作为第一步移植对象。

### 结论

wwj 分支中最值得优先借鉴的是三个方向：先保证 `PARAMETERS_SPEC` 等 MongoDB 配置确实被 C++ 读取；再移植或验证 `SUR_SGA` 的 `MOIST_IN` 相对孔隙度初始化；然后考虑 `IKW_IF` 的优先流/各向异性/大孔隙流参数。全量合并 wwj 分支风险较高，因为其中混有大量实验文件、调试日志、路径差异和非必要工具。

### 后续建议

- 不建议直接 merge 整个 wwj 分支。
- 建议先做最小补丁：修复 MongoDB cursor 读取逻辑，确认参数加载日志或数据库参数生效。
- 第二步再移植 `SUR_SGA` 的初始含水量计算修复，并重跑 storm 对比 NSE/PBIAS。
- 第三步再小范围移植 `IKW_IF` 的 `FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR`，配套在 `PARAMETERS_SPEC` 中加入 `SUB_MODEL=storm`、`TASK=SingleRun` 的参数记录。

---

## 日期：2026-05-31 — 实施第一项：修复 MongoDB cursor 读取逻辑

### 工作内容

按用户要求先处理 wwj 分支差异中的第一项，即修复 C++ 端 MongoDB cursor 遍历逻辑，避免 `mongoc_cursor_more(cursor)` 导致 `FILE_IN`、`FILE_OUT`、`FILE_OUT_SPEC`、`PARAMETERS`、`PARAMETERS_SPEC` 等集合读取不到记录。

### 执行过程

- 修改 `seims/src/seims_main/base/data/DataCenterMongoDB.cpp`：
  - 将 5 处 `while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, ...))` 改为 `while (mongoc_cursor_next(cursor, ...))`。
  - 覆盖函数包括 `GetFileInStringVector()`、`GetInitialFileOutMap()`、`GetSelectedFileOutVector()`、`ReadParametersInDB()`、`ReadCalibrateParametersInDB()`。
  - 顺手补充 `GetSelectedFileOutVector()` 和 `ReadCalibrateParametersInDB()` 正常返回路径中的 `bson_destroy(...)` 与 `mongoc_cursor_destroy(...)`，避免资源泄漏。
- 构建时遇到已知 CMake 4.x / LLVM 查找兼容问题：
  - 修改 `seims/src/ccgl/cmake/FindLLVM.cmake`，仅当 `LLVM_FIND_VERSION` 存在时才比较 LLVM 版本。
- 重新构建 `seims_omp`，并将新二进制同步到实际 demo 配置使用的 `build/bin/seims_omp`。

### 验证结果

- `cmake --build build --target seims_omp -j4` 成功。
- `cmake --install build` 成功。
- `build/bin/seims_omp` 已更新到 2026-05-31 22:40。
- 使用更新后的 `build/bin/seims_omp` 运行 AndrewsForest storm：
  - 命令使用 `-wp data/AndrewsForest/andrews_forest_model -cfg storm -task SingleRun -mode 1 -lyr 1 -fdir 0`。
  - 模型正常完成，`[SIMU][ALL]` 用时约 20.137 秒。
  - `Q.txt`、`QI.txt` 等输出文件更新到 2026-05-31 22:42。

### 重要发现

- MongoDB 中当前 `PARAMETERS_SPEC` 有 14 条记录，但记录为 `SUB_MODEL=_BASE_`、`TASK=SingleRun`。
- 本次实际运行使用 `-cfg storm`，因此这些参数记录不会被 `ReadCalibrateParametersInDB()` 的 `SUB_MODEL=storm` 查询匹配到。
- 当前运行后的流量指标与前一次基本一致：NSE 约 `-3.2248`，PBIAS 约 `-93.955%`，模拟均值约 `0.314 m3/s`，峰值约 `0.962 m3/s`。

### 结论

第一项代码修复已完成并验证可运行。它解决的是“cursor 遍历可能读不到数据库记录”的基础问题，但当前结果未变化，主要是因为现有校准参数记录的 `SUB_MODEL` 与运行配置 `storm` 不匹配。下一步若要让这些参数生效，需要把 `PARAMETERS_SPEC` 中相关记录补成或改成 `SUB_MODEL=storm`、`TASK=SingleRun`，或者运行时改用能匹配 `_BASE_` 的配置目录。

---

## 日期：2026-05-31 — 处理“cursor 修复后模拟值仍未变化”的原因

### 工作内容

用户反馈第一项修改后模拟值没有变化。继续核查 MongoDB 中校准参数记录与当前运行配置的匹配关系，并验证参数真正生效后的模型响应。

### 执行过程

- 查询 `andrews_forest_model.PARAMETERS_SPEC`：
  - 原有 14 条记录均为 `SUB_MODEL=_BASE_`、`TASK=SingleRun`。
  - 当前模型运行使用 `-cfg storm`，C++ 查询条件为 `SUB_MODEL=storm`、`TASK=SingleRun`，因此原有记录不会被读取。
- 将 `_BASE_/SingleRun` 下的 14 条参数复制/补写到 `storm/SingleRun`。
- 同时补充 `CHANGE=VC` 和 `VALUE=IMPACT`，使 `MOIST_IN`、`CONDUCTIVITY`、`MANNING`、`CH_N`、`GW0`、`KG`、`KI` 等已有参数按指定值覆盖，而不是沿用原始 `PARAMETERS` 表中的 `RC/AC` 变化方式。
- 使用更新后的 `build/bin/seims_omp` 重新运行 AndrewsForest storm，并重新计算流量指标。

### 验证结果

- 参数匹配后，模拟结果确实发生变化，说明 cursor 修复和 `PARAMETERS_SPEC` 读取路径已经生效。
- 新结果：
  - 模拟均值约 `0.0815 m3/s`，峰值约 `0.381 m3/s`。
  - NSE 约 `-3.5387`。
  - PBIAS 约 `-98.43%`。
  - `QS` 均值约 `0.0391 m3/s`，峰值约 `0.381 m3/s`。
  - `QI` 均值约 `0.0204 m3/s`，峰值约 `0.262 m3/s`。
  - `SBQG` 均值约 `0.0118 m3/s`。
- 相比前一次结果，流量进一步变小，说明仅让现有参数生效并不能改善当前流量偏低问题。

### 重要发现

运行日志显示以下 wwj 分支参数在当前代码中仍“不支持”：

- `ANISOTROPY`
- `DEEP_PERC_RATE`
- `FAST_RATIO`
- `FC_ADJUST`
- `INFIL_FACTOR`
- `MACROPORE_FACTOR`
- `OL_SPEED_FACTOR`

这些参数正是 wwj 分支中用于改善 AndrewsForest 快速产流、入渗、壤中流和坡面汇流的关键新增参数。当前代码没有对应的 `PARAMETERS` 基础记录和模块 `SetValue()` 支持，因此即使写入 `PARAMETERS_SPEC` 也会被 C++ 忽略。

### 结论

“第一项”现在已验证完成：数据库读取和参数匹配都已经打通，模型也确实会响应已有参数。但这组参数单独生效后结果变差，说明 wwj 分支中真正改善问题的不是单纯参数表，而是后续 `SUR_SGA`、`IKW_IF`、`PERCO_DARCY`、`IKW_OL` 等模块新增参数与过程代码。下一步应优先移植 `SUR_SGA` 的 `MOIST_IN` 按孔隙度初始化修复，再考虑 `IKW_IF` 的优先流/各向异性/大孔隙流改动。

---

## 日期：2026-06-01 — 梳理 wwj 分支 SUR_SGA 模块修改

### 工作内容

按用户要求，专门比较 wwj `validation-2026-05-07` 分支中 `SUR_SGA` 模块相对当前分支的修改，并分析每项修改的目的。

### 执行过程

- 对比 `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.cpp`、`StormGreenAmpt.h`、`api.cpp`。
- 查看相关提交记录，重点关注 `INFIL_FACTOR`、`DEEP_PERC_RATE`、rain-on-snow、初始土壤含水量和诊断日志相关提交。
- 将修改分为物理过程修改、参数接口修改、输入输出元数据修改和调试诊断修改。

### 主要结论

- 最关键修改是将初始土壤含水量从 `MOIST_IN * FieldCap` 改为 `MOIST_IN * Porosity`，用于减少初始土壤亏缺过大导致降雨几乎全部入渗的问题。
- `INFIL_FACTOR` 用于缩放 Green-Ampt 计算出的潜在入渗量，使模型可以通过参数控制更多水转为超渗降雨/地表产流。
- rain-on-snow 情况下不再将液态降雨完全置零，而是保留 `m_netPcp + m_sd`，避免温度处于雪雨边界时过度抑制有效降雨。
- `DEEP_PERC_RATE` 从底层土壤移除部分水量到深层地下水，是实验性水量再分配机制；对短历时洪峰未必是第一优先。
- wwj 还加入了 `/tmp/sga_diag.csv`、`/tmp/sga_infilfactor.txt` 和 `printf` 调试输出，这些有助于诊断，但不建议原样合并到正式代码。

### 后续建议

- 若要移植，应优先移植 `MOIST_IN * Porosity` 初始化和 rain-on-snow 修复。
- `INFIL_FACTOR` 可以作为可选参数移植，但需要同步 `api.cpp` 元数据和 MongoDB `PARAMETERS`/`PARAMETERS_SPEC` 记录。
- `DEEP_PERC_RATE` 和 `/tmp` 诊断输出应谨慎处理，最好先不纳入正式补丁，或改成受日志开关控制。

---

## 日期：2026-06-01 — AndrewsForest storm 水量平衡诊断

### 工作内容

按用户要求，对 AndrewsForest storm 模拟进行水量平衡诊断，检查本场降雨在 `INFIL`、`EXCP`、`DPST`、`PERCO`、`QS`、`QI`、`SBQG` 等环节中的去向，并生成本地诊断文档。

### 执行过程

- 查询 MongoDB `FILE_OUT`，确认 `NEPR`、`INFIL`、`EXCP`、`DPST`、`PERCO`、`QS`、`QI`、`SBQG`、`QSUBBASIN` 均可输出。
- 在 `andrews_forest_model.FILE_OUT_SPEC` 的 `SUB_MODEL=storm`、`TASK=SingleRun` 下补充 5 个过程输出项：`NEPR`、`INFIL`、`EXCP`、`DPST`、`PERCO`。
- 清理旧输出目录后，使用当前 `build/bin/seims_omp` 重新运行 AndrewsForest storm。
- 用 GDAL 读取新生成的 `*_SUM.tif` 栅格，按有效栅格数和栅格面积换算为全流域等效水深和总体积。
- 读取 `Q.txt`、`QS.txt`、`QI.txt`、`SBQG.txt`，按 300 秒时间步积分为事件体积和等效水深。
- 生成诊断文档：`data/AndrewsForest/andrews_forest_model/storm/water_balance_diagnosis.md`。

### 诊断结果

- 有效流域面积约 `61.585 km2`。
- `NEPR_SUM` 净雨约 `61.260 mm`，总体积约 `3,772,652 m3`。
- `INFIL_SUM` 入渗约 `61.400 mm`，总体积约 `3,781,330 m3`，量级上说明净雨几乎全部进入土壤。
- `EXCP_SUM` 为 `0 mm`。
- `DPST_SUM` 为 `0 mm`。
- `PERCO_SUM` 为 `0 mm`。
- 出口模拟 `Q.txt` 事件体积约 `24,935 m3`，等效水深约 `0.405 mm`，仅占净雨约 `0.661%`。
- `QS` 等效水深约 `0.195 mm`，`QI` 约 `0.101 mm`，`SBQG` 约 `0.059 mm`。
- 观测出口流量事件体积约 `1,587,963 m3`，等效水深约 `25.785 mm`；模拟体积仅为观测体积约 `1.57%`。

### 结论

本场雨在当前模型中几乎全部被 `SUR_SGA` 之后的 `INFIL/SOLST` 吸收并滞留在土壤剖面。`EXCP=0` 表明几乎没有形成超渗地表产流源；`PERCO=0` 表明事件内也没有形成地下水补给；`QS/QI/SBQG` 释放的水量很小。因此当前流量偏低的直接原因是：降雨没有被转化为足够的快速坡面流或壤中流，而是主要进入并留在土壤储水中。

### 注意事项

- 当前运行日志仍提示 wwj 新参数 `FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR`、`INFIL_FACTOR`、`FC_ADJUST`、`OL_SPEED_FACTOR`、`DEEP_PERC_RATE` 不被当前代码支持，因此这些参数没有参与本次诊断。
- `DPST` 是状态量；本次为 0，可以直接判断没有洼地蓄水。如果未来非零，`DPST_SUM` 不能简单理解为最终蓄水量。

---

## 日期：2026-06-01 — 解释净雨为何几乎全部转化为入渗

### 工作内容

按用户要求查看当前代码逻辑，解释为什么 AndrewsForest storm 中 `NEPR` 净雨几乎全部进入 `INFIL`，而 `EXCP`、`DPST`、`PERCO` 为 0。

### 代码逻辑发现

- 当前 `config.fig` 顺序为 `PI_MCS -> SUR_SGA -> DEP_FS -> IKW_OL -> IKW_IF -> PERCO_DARCY -> GW_RSVR -> CH_DW`。
- `SUR_SGA` 在初始化时将每层初始土壤水设为 `MOIST_IN * FieldCap`，即 `m_soilWtrSto[i][j] = m_initSoilWtrStoRatio[i] * m_soilFC[i][j]`。
- 每个时间步中，`SUR_SGA` 将可用水量 `hWater` 设为 `m_netPcp + m_sd + snowMelt`，然后计算 Green-Ampt 潜在入渗。
- 入渗容量 `infilCap` 不是只看表层，而是累加所有土层到孔隙度的亏缺：`sum((m_soilPor - m_soilWtrSto) * m_soilDepth)`。
- 实际入渗先取 `min(潜在入渗, infilCap)`；如果该值大于本步可用水 `hWater`，再被截断为 `hWater`。
- 最终 `m_exsPcp = hWater - m_infil`。因此只要潜在入渗能力和全剖面亏缺都大于本步净雨，`EXCP` 必然为 0。
- `DEP_FS` 只接收 `EXCP`，当 `EXCP=0` 时，`DPST` 和坡面水 `SURU` 也不会增加。
- `IKW_OL` 只有在 `SURU` 或上游坡面流存在时才产生 `QS`，否则直接返回 0。

### 结论

当前代码把整个土壤剖面到孔隙度的亏缺都作为入渗容量，并且当入渗能力超过本步净雨时把净雨全部吃掉。由于初始土壤水按 `FieldCap` 初始化偏低，土壤亏缺很大，导致本场 storm 中 `NEPR` 几乎全部转成 `INFIL/SOLST`，没有形成 `EXCP -> DPST/SURU -> QS` 的地表径流链条。

---

## 日期：2026-06-01 — 核查 MOIST_IN 语义和初始土壤水分调节方式

### 工作内容

按用户要求，核查 `MOIST_IN` 到底表示相对田间持水量还是相对饱和度，并寻找是否存在可调节“初始土壤水分深度分布”的模块参数。

### 代码证据

- `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.h` 中 `m_initSoilWtrStoRatio` 的注释明确写为：初始土壤水储量相对于田间持水量 `FC-WP` 的比例。
- 当前 `SUR_SGA` 初始化公式为 `m_soilWtrSto[i][j] = m_initSoilWtrStoRatio[i] * m_soilFC[i][j]`，即 `MOIST_IN * FieldCap`。
- 长期模块 `SUR_MR` 中也使用同样语义：`m_initSoilWtrStoRatio * m_soilFC`，注释同样说明相对于田间持水量。
- 预处理函数 `SoilUtilClass.initial_soil_moisture()` 的 docstring 写明是 “soil moisture fraction of field capacity”，并将 TWI 映射到约 `0.6-1.0` 的相对田间持水量范围。
- `model_param_ini.csv` 中 `Moist_in` 的范围为 `0.8-1.0`，虽然单位写成 `m3/m3` 不够严谨，但结合代码和预处理逻辑，应按比例参数理解，而不是绝对体积含水量。

### 关于“深度衰减”参数

- 在 wwj 历史提交中确实出现过 `SUR_SGA` 深层土壤初始含水量深度衰减实验，公式类似：表层 `100%`、第二层 `85%`、第三层 `70%`，最低 `30%`。
- 该做法是写死在 `StormGreenAmpt.cpp` 里的 `depthFactor`，不是 MongoDB 或 `PARAMETERS_SPEC` 中的可调参数。
- 随后的提交 `64181833` 明确写着“撤销失败的深层土壤深度衰减修改，恢复原始初始化逻辑”，说明该实验在 wwj 分支历史中也被判定为失败并撤销。

### 结论和建议

- 从项目自身语义看，`MOIST_IN` 应理解为相对田间持水量，而不是相对孔隙度/饱和度。
- 直接把初始化改成 `MOIST_IN * Porosity` 可以人为增大初始土壤含水量、减少亏缺，但等价于重定义 `MOIST_IN` 语义；作为临时校准实验可以测试，作为正式物理逻辑需要谨慎。
- 更保守的调节方式是先把 storm 当前 `PARAMETERS_SPEC` 中的 `MOIST_IN=0.58` 调高到 `1.0` 或恢复预处理默认范围，表示初始水分达到田间持水量。
- 如果 `MOIST_IN=1.0` 后仍然没有足够 `EXCP/QS`，问题更可能在 `SUR_SGA` 将全剖面到孔隙度的亏缺全部作为入渗容量，而不是 `MOIST_IN` 本身；后续可考虑限制活跃入渗深度、加入受控 `INFIL_FACTOR`，或重构 Green-Ampt 湿润锋逻辑。

---

## 日期：2026-06-01 — 查询 param.cali 写入数据库入口

### 工作内容

按用户要求，确认修改 `storm/param.cali` 后需要运行哪个 Python 入口才能写入 MongoDB。

### 代码发现

- 写入 `param.cali` 的实际函数是 `MainSEIMS.ImportCalibratedParameters()`。
- 该函数位于 `seims/run_seims.py`，读取路径由 `ModelCfgUtils(model_dir, cfg_name)` 决定；对于 AndrewsForest storm 即读取 `data/AndrewsForest/andrews_forest_model/storm/param.cali`。
- 写入目标表是 MongoDB 的 `andrews_forest_model.PARAMETERS_SPEC`，筛选键为 `SUB_MODEL=storm`、`TASK=SingleRun`。
- `seims/run_seims.py` 的命令行主函数只执行模型，不自动导入 `param.cali`。
- `seims/test/demo_runmodel.py` 会调用 `ImportModelIOConfiguration()` 和 `ImportCalibratedParameters()`，但当前 demo 列表默认只包含 youwuzhen，不适合作为 AndrewsForest 的直接入口。

### 建议命令

使用一个短 Python 入口直接调用 `MainSEIMS.ImportCalibratedParameters()`，即可只更新 `PARAMETERS_SPEC` 而不重跑完整预处理。

---

## 日期：2026-06-01 — 新增 AndrewsForest storm 参数导入脚本

### 工作内容

按用户要求，将导入 `storm/param.cali` 到 MongoDB 的短命令整理为可重复调用的 Python 脚本。

### 执行过程

- 新增脚本 `import_andrews_storm_params.py`。
- 脚本默认读取 `data/AndrewsForest/andrews_forest_model/storm/param.cali`。
- 脚本默认写入数据库 `andrews_forest_model`、配置名 `storm`、任务名 `SingleRun`。
- 支持通过 `--host`、`--port`、`--bin-dir`、`--model-dir`、`--db-name`、`--cfg-name`、`--task-name` 覆盖默认值。
- 使用 `/Users/flora/miniconda3/envs/pyseims/bin/python import_andrews_storm_params.py --help` 验证脚本参数解析和依赖导入正常。

### 使用方式

修改 `storm/param.cali` 后运行：

```bash
conda activate pyseims
cd /Users/flora/code/seims
python import_andrews_storm_params.py
```

---

## 日期：2026-06-01 — 查询 SEIMS 主程序运行命令

### 工作内容

按用户要求确认运行 SEIMS 主程序的命令格式，尤其是 AndrewsForest storm 当前模型的直接运行命令。

### 结果

- Python 封装入口 `seims/run_seims.py` 会拼接 C++ 主程序命令，但它只执行模型，不自动重新导入 `param.cali`。
- C++ 主程序参数名来自 `seims/src/seims_main/base/module_setting/invoke.cpp`，核心参数包括 `-wp`、`-cfg`、`-thread`、`-mode`、`-lyr`、`-fdir`、`-host`、`-port`、`-sce`、`-cali`。
- AndrewsForest storm 当前可直接使用 `build/bin/seims_omp` 运行，必要时先导出 macOS 动态库路径。

---

## 日期：2026-06-01 — MOIST_IN=0.99 后结果变化判断

### 工作内容

记录用户反馈：将 `MOIST_IN` 调整为 `0.99` 后，部分时段洪峰值有所增加，但整体模拟曲线趋势和大部分时段变化不大。

### 初步判断

- `MOIST_IN` 确实影响了初始土壤亏缺，因此局部峰值有所响应。
- 但整体趋势没有明显变化，说明当前低流量问题并不主要由初始土壤水分一个参数控制。
- 更可能的主控瓶颈仍是 `SUR_SGA` 中入渗容量按全土壤剖面亏缺计算，导致净雨优先进入土壤储水；以及 `IKW_IF`、`PERCO_DARCY`、`GW_RSVR` 等快速释放通道参数/逻辑尚未充分生效。

### 下一步建议

- 先重新做一次 `MOIST_IN=0.99` 条件下的水量平衡诊断，对比 `NEPR/INFIL/EXCP/PERCO/QS/QI/SBQG` 是否发生实质转移。
- 若 `EXCP` 仍接近 0，则继续调 `MOIST_IN` 意义有限，应优先处理 `SUR_SGA` 入渗容量或加入受控 `INFIL_FACTOR`。
- 若 `EXCP` 增加但 `QS` 不增加，则应检查 `DEP_FS` 和 `IKW_OL` 的地表水传递。
- 若 `QI/SBQG` 仍很小，则应检查 `IKW_IF`、`PERCO_DARCY`、`GW_RSVR` 的释放参数和水量连通。

---

## 日期：2026-06-01 — MOIST_IN=0.99 水量平衡诊断

### 工作内容

按用户要求，执行前述水量平衡诊断计划，检查当前 `MOIST_IN=0.99` 条件下净雨在 `INFIL`、`EXCP`、`DPST`、`PERCO`、`QS`、`QI`、`SBQG` 等环节中的分配。

### 执行过程

- 确认 `storm/param.cali` 当前只有 `MOIST_IN,0.99` 为有效参数，其余参数均已注释。
- 运行 `python import_andrews_storm_params.py`，将当前 `param.cali` 重新写入 MongoDB `PARAMETERS_SPEC`，本次写入 1 条参数。
- 发现直接重跑会向既有 `Q.txt` 等文本输出追加记录，因此先清理 `OUTPUT_D8_DOWNUP--`，再重新运行 `seims_omp`。
- 读取新生成的 `NEPR_SUM.tif`、`INFIL_SUM.tif`、`EXCP_SUM.tif`、`DPST_SUM.tif`、`PERCO_SUM.tif` 和 `Q/QS/QI/SBQG.txt`，按有效面积与 300 秒时间步积分。
- 生成诊断文档：`data/AndrewsForest/andrews_forest_model/storm/water_balance_diagnosis_moist099.md`。

### 诊断结果

- 有效流域面积约 `61.585 km2`。
- `NEPR` 为 `61.260 mm`，`INFIL` 为 `62.411 mm`，说明净雨仍几乎全部进入入渗/土壤链条。
- `EXCP=0`，`DPST=0`，说明仍没有形成超渗地表产流源。
- `PERCO=0.063 mm`，只占 `NEPR` 约 `0.10%`，地下水补给仍很小。
- 出口 `Q` 事件体积约 `96,181 m3`，等效水深约 `1.562 mm`，约为观测体积的 `6.06%`。
- `QS=0.339 mm`，`QI=0.274 mm`，`SBQG=0.346 mm`，三者均有所增加但总量仍远低于观测出口径流 `25.785 mm`。
- 拟合指标：`NSE=-3.2244`，`PBIAS=-93.94%`，`RMSE=5.5877 m3/s`，`R2=0.0102`。

### 结论

`MOIST_IN=0.99` 让部分分量和出口总量增加，但没有改变主控水量路径：`EXCP` 和 `DPST` 仍为 0，`PERCO` 仍很小，水仍主要被 `SUR_SGA` 入渗并滞留在土壤系统中。因此下一步不应继续单独调 `MOIST_IN`，而应优先处理 `SUR_SGA` 的入渗容量/超渗产生机制，或引入受控 `INFIL_FACTOR` 做实验。

---

## 日期：2026-06-01 — 梳理 INFIL_FACTOR 的加入方式和作用

### 工作内容

按用户要求，查看 wwj 分支中 `INFIL_FACTOR` 如何加入、用在何处，以及它试图改善的问题。

### 代码发现

- `SUR_SGA/api.cpp` 中新增 `INFIL_FACTOR` 元数据，类型为 `DT_Single`，来源为 `ParameterDB`，说明它应作为 MongoDB 参数读取。
- `StormGreenAmpt.h` 中新增成员变量 `m_infilFactor`，注释为入渗削减因子，默认值为 `1.0`。
- `StormGreenAmpt.cpp` 构造函数将 `m_infilFactor` 初始化为 `1.0`，保证不配置该参数时行为接近原逻辑。
- `StormGreenAmpt.cpp` 新增 `SetValue(const char*, FLTPT)`，当参数名匹配 `INFIL_FACTOR` 时，把数据库传入值赋给 `m_infilFactor`。
- 实际使用位置在 Green-Ampt 每步潜在入渗计算后：先计算 `rawInfil = min(infilRate * dt * 1000, infilCap)`，再执行 `rawInfil *= m_infilFactor`，最后 `m_infil = min(hWater, rawInfil)`。

### 作用机制

- 当前原始逻辑中，只要潜在入渗能力大于本步可用水 `hWater`，实际入渗就会被截断为 `hWater`，导致 `EXCP = hWater - m_infil = 0`。
- `INFIL_FACTOR < 1` 后，即使原始潜在入渗大于 `hWater`，乘以系数后的 `rawInfil` 也可能小于 `hWater`，从而产生 `EXCP`。
- 因此它直接作用于 `SUR_SGA` 的入渗-超渗分配，用来把一部分原本全部进入 `INFIL/SOLST` 的净雨转成 `EXCP`，再交给 `DEP_FS -> IKW_OL` 形成坡面流。

### 判断

`INFIL_FACTOR` 是一个经验性校准因子，不是 Green-Ampt 方程本身的物理参数。它适合用于诊断和校准当前“净雨几乎全部入渗、EXCP/DPST 为 0、快速产流不足”的问题，但如果作为长期正式修复，最好进一步解释为有效入渗面积、土壤结皮/疏水性、降雨强度空间异质性或湿润锋有效深度等机制。

---

## 日期：2026-06-01 — 加入 INFIL_FACTOR 并执行 SUR_SGA 入渗诊断实验

### 工作内容

按用户要求，在当前代码中加入 `INFIL_FACTOR` 做诊断实验，判断 AndrewsForest storm 流量偏低是否主要由于 `SUR_SGA` 入渗过强。

### 代码与数据库改动

- 修改 `seims/src/seims_main/modules/hydrology/SUR_SGA/api.cpp`，新增 `INFIL_FACTOR` 参数元数据，类型为 `DT_Single`。
- 修改 `StormGreenAmpt.h/.cpp`，新增 `m_infilFactor`，默认值为 `1.0`，并新增 `SetValue(const char*, FLTPT)` 接收数据库参数。
- 修改 `SUR_SGA` 入渗计算：先计算 `rawInfil = min(infilRate * dt * 1000, infilCap)`，再乘以 `m_infilFactor`。
- 修改 `seims/preprocess/database/model_param_ini.csv`，新增基础参数 `INFIL_FACTOR`。
- 在 MongoDB `andrews_forest_model.PARAMETERS` 中补入 `INFIL_FACTOR`，基础记录采用 `VALUE=1`、`CHANGE=RC`、`IMPACT=1`。
- 在 `storm/param.cali` 中加入实验参数，最终保留为 `INFIL_FACTOR,0.005,VC`。

### 构建与运行

- 构建 `SUR_SGA` 目标并安装。
- 将新 `libSUR_SGA.dylib` 同步到 `build/lib/libSUR_SGA.dylib`，将新 `seims_omp` 同步到 `build/bin/seims_omp`。
- 每次实验前运行 `python import_andrews_storm_params.py`，并清理 `OUTPUT_D8_DOWNUP--`，避免文本输出追加污染积分。

### 实验结果

- `INFIL_FACTOR=0.05` 时结果与基准几乎一致，说明削减到 5% 仍不足以突破当前潜在入渗能力。
- `INFIL_FACTOR=0.01` 时开始产生 `DPST`，但出口 `Q` 基本不变。
- `INFIL_FACTOR=0.005` 时 `DPST` 增至约 `19.508 mm`，`QS` 增至约 `0.777 mm`，出口 `Q` 增至约 `1.791 mm`，峰值增至约 `1.724 m3/s`，但模拟体积仍只有观测的约 `6.95%`。
- `INFIL_FACTOR=0` 的极端诊断中，出口 `Q` 达约 `61.255 mm`，峰值约 `57.46 m3/s`，明显过量产流，证明入渗闸门一旦关闭，模型可以快速产生大量地表径流。

### 诊断结论

当前流量偏低可以确认与 `SUR_SGA` 入渗过强高度相关。`SUR_SGA` 的潜在入渗能力相对本场降雨过大，必须削减到约 `1%` 以下才开始出现地表水响应；完全关闭入渗则会严重过量产流。因此下一步应围绕 `SUR_SGA` 入渗容量/湿润锋有效深度做更物理的修正，同时检查 `DEP_FS/IKW_OL` 对新增地表水的传递效率。

### 输出文档

- 诊断文档已生成：`data/AndrewsForest/andrews_forest_model/storm/water_balance_diagnosis_infil_factor.md`。

---

## 日期：2026-06-02 — 检索 SUR_SGA 合理修改方向的论文和资料

### 工作内容

按用户要求，检索 Green-Ampt、Mein-Larson、SWAT Green-Ampt、层状土入渗以及 Andrews Forest 水文过程相关资料，判断 `SUR_SGA` 应如何修改更合理。

### 资料要点

- Green-Ampt/Mein-Larson 类方法的核心是用湿润锋累计入渗 `F` 和含水量亏缺 `Delta theta` 控制入渗能力，而不是把整个土壤剖面到孔隙度的亏缺一次性作为入渗容量。
- SWAT Green-Ampt 文档也按 `K_e`、湿润锋吸力和有效含水量亏缺计算入渗率，并强调时间步内降雨强度和产流判断。
- HEC-HMS 和 GSSHA 文档中的 Green-Ampt/层状土入渗资料都强调湿润锋推进、层间参数变化和 ponding 条件，而不是全剖面快速填充。
- Andrews Forest/陡坡森林流域资料显示，风暴径流常由饱和区扩展、浅层地下/壤中流、优先流等机制控制，单纯 Horton 型超渗产流可能不足以解释过程。

### 初步建议

- `INFIL_FACTOR` 可保留为诊断/率定参数，但不宜作为最终物理修复。
- `SUR_SGA` 更合理的修改方向是限制入渗容量到表层或湿润锋有效深度，并按 Green-Ampt 累计入渗推进湿润锋。
- 对层状土，应在湿润锋到达层界后再切换/组合下一层参数，而不是从一开始使用全剖面孔隙亏缺。
- Andrews Forest storm 后续还应考虑饱和超渗/可变源区和快速壤中流机制。

---

## 日期：2026-06-02 — 解释限制湿润锋是否会阻止地下渗漏

### 工作内容

回应用户关于“若限制 `SUR_SGA` 入渗容量到表层或湿润锋有效深度，水是否就无法渗漏到地下”的疑问。

### 判断

- 合理的修改不是禁止水向深层渗漏，而是把“本时间步地表可接收多少水”和“土壤水随后如何向下层再分配/渗漏”分开。
- 当前 `SUR_SGA` 的问题是把全剖面孔隙亏缺都当作瞬时地表入渗容量，使深层空隙在同一个时间步内参与吃掉降雨。
- 更合理的 Green-Ampt 逻辑应让水先进入湿润锋范围内的表层土壤，随着累计入渗增加，湿润锋逐步向下推进；湿润锋达到下层后，水才进入更深土层。
- 地下渗漏应由 `PERCO_DARCY` 或土壤水再分配逻辑处理：当某层水分超过田间持水量/排水阈值时，水再向下层或地下水补给转移。

### 后续实现提醒

- 若只做一个固定 `ACTIVE_DEPTH` 上限且不允许湿润锋推进，也不做层间再分配，确实会人为阻断深层渗漏。
- 推荐实现是“限制瞬时入渗容量 + 湿润锋推进 + 层间排水/再分配”，而不是永久限制入渗深度。
## 2026-06-02 SUR_SGA 有效入渗深度修改与模型复跑

- 按前述思路修改 `SUR_SGA`：新增 `INFIL_FACTOR` 和 `ACTIVE_DEPTH_MAX` 两个可选参数，并将入渗容量从“全土壤剖面孔隙亏缺”改为“事件有效入渗深度内的孔隙亏缺 - 事件累计入渗量”。
- 修正 `SUR_SGA` 元数据中 `SOILDEPTH` 的单位为 mm；预处理代码中该字段注释和实际栅格均为从地表到土层底部的深度，单位 mm。
- 为避免影响其他模型，C++ 中 `ACTIVE_DEPTH_MAX` 默认值设为 0，未配置时保持旧的全剖面容量逻辑；AndrewsForest storm 的 `param.cali` 中显式设置 `ACTIVE_DEPTH_MAX=150,VC`。
- 重新编译 `SUR_SGA`，安装后发现 `seims_omp` 实际加载 `build/lib/libSUR_SGA.dylib`，而 `cmake --install` 更新的是 `build/install/lib/libSUR_SGA.dylib`；已手动同步新库到 `build/lib` 后重新运行。
- 结果：原先 `MOIST_IN=0.99` 时出口总径流深约 1.563 mm，仅为观测 25.785 mm 的 6.1%；`ACTIVE_DEPTH_MAX=100` 时总径流深增至 31.327 mm、峰值 57.140 m3/s，过强；`ACTIVE_DEPTH_MAX=150` 时总径流深 10.880 mm、峰值 22.097 m3/s、NSE 从约 -3.224 改善到 -1.495。
- 结论：问题确实与 SUR_SGA 入渗容量过强有关，新逻辑能显著改变水量分配；但单独限制入渗会使地表快流峰值偏尖，下一步应联合诊断/校准地表汇流、洼地蓄水和壤中流相关参数。
- 详细诊断记录写入 `data/AndrewsForest/andrews_forest_model/storm/water_balance_diagnosis_active_depth.md`。

## 2026-06-02 MANNING=0.06 地表糙率实验

- 在 AndrewsForest storm 的 `param.cali` 中启用 `MANNING,0.06,VC`，保持 `MOIST_IN=0.99`、`INFIL_FACTOR=1.0`、`ACTIVE_DEPTH_MAX=150` 不变，并重新导入 MongoDB 后运行模型。
- 运行输出目录为 `data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--`；此前 `ACTIVE_DEPTH_MAX=150`、未启用 MANNING 的结果已留档为 `OUTPUT_D8_DOWNUP--.active_depth_150_before_manning006`。
- 结果：`INFIL_SUM` 均值约 51.609 mm，`Q_sim` 总径流深 11.298 mm，约为观测 25.785 mm 的 43.8%；峰值为 45.319 m3/s，发生在 2015-02-07 11:15；NSE=-4.740，PBIAS=-56.19%。
- 对比未启用 MANNING 的 `ACTIVE_DEPTH_MAX=150` 结果：总水量仅从 10.880 mm 小幅增加到 11.298 mm，但峰值从 22.097 m3/s 升至 45.319 m3/s，峰值时间从 13:40 提前到 11:15，模拟效果变差。
- 结论：在当前 IKW_OL 数值实现下，直接把 `MANNING` 统一设为 0.06 没有产生预期的削峰滞后作用，反而导致更尖更早的地表流峰值。下一步不建议继续盲目增大 MANNING；应优先检查 IKW_OL 的糙率/运动波公式响应，或转向增强 IKW_IF 快速壤中流以把一部分地表快流转为较平缓的侧向流。

## 2026-06-02 取消 MANNING 设置并检查 IKW_OL 运动波实现

- 按用户要求取消 AndrewsForest storm `param.cali` 中的 `MANNING` 设置，重新导入 MongoDB，确认 `PARAMETERS_SPEC` 中不再包含 `MANNING`。
- 将上一轮 `MANNING=0.06` 输出留档为 `OUTPUT_D8_DOWNUP--.active_depth_150_manning006`，并重跑模型恢复当前 `OUTPUT_D8_DOWNUP--` 到无 MANNING 设置的结果。
- 当前输出参数为 `ACTIVE_DEPTH_MAX=150,VC`、`INFIL_FACTOR=1,VC`、`MOIST_IN=0.99,RC`；`Q.txt` 峰值恢复为 22.097 m3/s，发生在 2015-02-07 13:40；总径流深 10.880 mm，NSE=-1.495，PBIAS=-57.81%。
- 检查 `IKW_OL` 实现发现几个优先关注点：
  - 局部 Manning 关系的形式基本符合宽浅流道运动波推导，`A = alpha * Q^beta`，`beta=0.6`，`alpha=(n/sqrt(S)*P^(2/3))^beta`，从该局部公式看增大 Manning 应降低局部出流。
  - 坡度初始化存在潜在错误：当下游 DEM 高于当前格元时先设 `MIN_SLOPE`，但随后又用负的 `deltaZ / horizontalDist` 覆盖，后续 `sqrt(sin(atan(s0)))` 可能得到非法值或使 `alpha=0`。
  - `if (m_streamLink[id] >= 0 && m_flowWidth[id] <= 0)` 将 `m_flowWidth[id]` 指针与 0 比较，疑似应为 `m_flowWidth[id][1] <= 0`；wwj 分支中也改成了 `[1]`。
  - 对多流向单元，代码把当前单元初始水量按 `flowOutFrac` 分配，但对上游总流量先求和再按当前单元出流比例重分配，可能与上游各流向真实连接比例不完全一致，易影响洪峰集中性。
  - `GetNewQ` 牛顿迭代最多 10 次且未检查是否收敛；在大坡度、极小水深或 Manning 统一替换后可能产生非直觉响应。
- 初步结论：`MANNING=0.06` 反常增峰不应解释为“糙率物理上会加快流速”，而更像当前 IKW_OL 的坡度/流宽/多流向分配/数值迭代实现存在问题。下一步建议先修 IKW_OL 的明显实现问题并做小范围对照实验，再继续校准 Manning。

## 2026-06-02 IKW_OL 明显实现问题修复与复跑

- 按用户要求先修复 IKW_OL 中几个明确问题，并分别提交：
  - `f08a5ece Fix IKW_OL flow width guard`：将 `m_flowWidth[id] <= 0` 指针比较修为 `m_flowWidth[id][1] <= 0`。
  - `71d24021 Clamp IKW_OL downslope gradient`：反坡/平坡不再被负的 `deltaZ / horizontalDist` 覆盖，统一夹到 `MIN_SLOPE`。
  - `94b5320e Harden IKW_OL Newton solve`：将局部牛顿迭代上限提高到 50 次，并增加相对残差判断和负值保护。
- 编译验证：每次修复后均运行 `cmake --build build --target IKW_OL -j4`，均编译通过；仅有既有 `data_raster.hpp` 常量转换警告。
- 模型验证 1：保持当前无 `MANNING` 设置，参数为 `ACTIVE_DEPTH_MAX=150,VC`、`INFIL_FACTOR=1,VC`、`MOIST_IN=0.99,RC`，复跑 storm 模型。结果与修复前一致：`Q_sim=10.880 mm`，观测 `25.785 mm`，峰值 `22.097 m3/s`（2015-02-07 13:40），NSE=-1.495，PBIAS=-57.81%。
- 模型验证 2：临时在 MongoDB 中加入 `MANNING=0.06,VC` 复跑，结果仍为 `Q_sim=11.298 mm`、峰值 `45.319 m3/s`（2015-02-07 11:15）、NSE=-4.740、PBIAS=-56.19%，与修复前 MANNING 实验一致，说明这三个明显 bug 不是 `MANNING=0.06` 反常增峰的直接原因。
- 运行文件整理：无 MANNING 且 IKW_OL 修复后的输出恢复为当前 `OUTPUT_D8_DOWNUP--`；`MANNING=0.06` 对照输出留档为 `OUTPUT_D8_DOWNUP--.ikw_ol_fixes_manning006`；旧基线输出留档为 `OUTPUT_D8_DOWNUP--.active_depth_150_before_ikw_ol_fixes`。
- 当前判断：IKW_OL 明显 bug 已修，但对本事件出口流量没有提升。下一步若继续追踪 Manning 反常响应，需深入检查 IKW_OL 的水量分配算法，尤其是多出流格元中上游 `qUp` 先求和再按当前格元 `flowOutFrac` 重分配的做法，以及 `qLast`、`surplus` 和剩余水量更新的离散形式。

## 2026-06-03 IKW_OL 深层水量分配逻辑与 wwj 分支对比

- 按用户要求检查 `IKW_OL` 更深层的水量分配逻辑，并对比 `wwj/validation-2026-05-07` 分支。
- 当前 AndrewsForest storm 配置为 `FDIRMTD = 0`，即 D8 单流向；因此多出流格元的 `flowOutFrac` 分配问题在当前实验中大概率退化为单一出流，不太可能是本轮 D8 结果不变或 Manning 反常响应的直接根因。
- 代码层面发现：`IKW_OL` 先汇总所有非河道上游格元实际流入 `qUp`，再按当前格元的 `flowOutFrac` 分到各出流分支；同时把当前格元初始水量、入渗剩余容量也按分支分别进入独立运动波求解。这种做法对 D8 基本等价于单通道计算，但对 MFD/Dinf 会把“一个格元的控制体存储”和“多个独立分支平面”混在一起，物理解释不够清晰。
- `FLOWIN_FRACTION` 在元数据和输入检查中是必需参数，但在 `OverlandFlow()` 中实际没有使用。由于代码已经通过上游 `m_flowOutIdx` 找到真正流向当前格元的那一支 `m_qs[upstream][j]`，直接求和并不一定错；但“要求输入却不用”说明多流向水量分配设计不完整。
- 入渗剩余容量处理也有尺度不一致风险：分支运动波里的 `surplus` 用 `flowWidth` 换算成 m2/s 后再乘 `flowOutFrac`，但最终二次入渗 `potentialInfilVol` 用整格 `cellArea` 计算。这在多出流条件下可能影响水量和时序。
- wwj 分支对 `IKW_OL` 的主体修改包括：增加 `OL_SPEED_FACTOR` 并用 `effective_n = m_n / OL_SPEED_FACTOR` 调整有效曼宁系数；对异常 Manning 值设 0.05 兜底；将坡度计算中的 `sin` 参数用 `Max(..., 0.001)` 防止非法值；修正 `m_flowWidth[id] <= 0` 指针比较；禁用若干调试输出；把 `RadianSlope` 输出类型从 1D 改为 2D。
- wwj 分支没有修改上述深层分配框架：上游流量求和后按当前格元 `flowOutFrac` 重新分配、`FLOWIN_FRACTION` 未使用、分支独立运动波求解这些逻辑仍然保留。因此不建议把 wwj 的 IKW_OL 当作物理分配修复直接合并。
- 当前建议：先在 `IKW_OL` 加质量守恒诊断输出，逐格记录 `initialVolume + qUp*dt`、`sum(qOut*dt)`、`m_sr` 剩余水量、`reInfilVol` 和残差；若当前 D8 残差正常，则下一步应重点检查 `SUR_SGA`、`DEP_FS`、`IKW_IF`/浅层侧向流与河道耦合，而不是继续优先改多流向分配。

## 2026-06-03 IKW_OL 增加 OL_SPEED_FACTOR 并做糙率响应实验

- 按用户要求在当前分支加入 wwj 类似的 `OL_SPEED_FACTOR` 参数，用于诊断原始 Manning 是否存在异常导致地表汇流响应错误。
- 修改内容：
  - `IKW_OL` 新增成员 `m_speedFactor`，默认值为 1.0；
  - 元数据新增单值参数 `OL_SPEED_FACTOR`；
  - `OverlandFlow()` 中使用 `effectiveN = m_n / OL_SPEED_FACTOR` 计算运动波系数；
  - 对无效 Manning 值增加兜底：`effectiveN <= 0` 或 `> 10` 时使用 0.05；
  - `model_param_ini.csv` 新增 `OL_SPEED_FACTOR` 初始参数定义；
  - AndrewsForest storm 的 `param.cali` 启用 `OL_SPEED_FACTOR,0.5,VC`。
- 编译：运行 `cmake --build build --target IKW_OL -j4` 通过，仅有既有 `data_raster.hpp` 常量转换警告；并将新 `libIKW_OL.dylib` 同步到 `build/lib`。
- MongoDB：运行 `import_andrews_storm_params.py` 导入 `param.cali` 后，发现 `PARAMETERS` 初始表缺少 `OL_SPEED_FACTOR`，导致 C++ 初始化警告“不支持该参数”并报 `OL_SPEED_FACTOR must be specified`；已在 MongoDB `PARAMETERS` 集合补入该参数初始定义。
- 运行：清空当前 `OUTPUT_D8_DOWNUP--` 后重新运行 storm 模型，日志写入 `data/AndrewsForest/andrews_forest_model/storm/seims_ol_speed_factor_0p5.log`，模型正常结束。
- 结果对比：
  - 无 `OL_SPEED_FACTOR` 基线：总径流深 10.8797905 mm，峰值 22.09733391 m3/s，峰现 2015-02-07 13:40，NSE=-1.494581，PBIAS=-57.805779%；
  - `OL_SPEED_FACTOR=0.5`：总径流深 10.42618899 mm，峰值 13.01329613 m3/s，峰现 2015-02-07 14:50，NSE=-0.727824，PBIAS=-59.564945%。
- 进一步检查原始 Manning 栅格 `workspace/spatial_raster/MANNING.tif`：有效格元 68406 个，最小 0.15，中位数 0.4，均值约 0.3955，95 分位数 0.4，最大 0.4；其中 66930 个格元为 0.4。
- 判断：此前统一设置 `MANNING=0.06,VC` 并不是增大地表糙率，而是把绝大多数森林格元从 0.4 降到 0.06，等效为大幅加快地表流，因此峰值变大、提前是合理响应；不是 IKW_OL 对 Manning 的物理方向完全错误。`OL_SPEED_FACTOR=0.5` 在原始 Manning 栅格基础上加倍有效糙率，确实削峰并滞后峰现，但总水量仍偏低，后续仍需处理产流/壤中流/地下水分配问题。

## 2026-06-03 撤销 OL_SPEED_FACTOR 并改用 MANNING 参数调节

- 按用户要求撤销 `OL_SPEED_FACTOR` 参数，不再保留该诊断钩子，改用 SEIMS 原有的 `MANNING` 参数控制地表糙率。
- 已从 `IKW_OL` 的 C++ 代码、模块元数据和 `model_param_ini.csv` 中移除 `OL_SPEED_FACTOR`；重新编译 `IKW_OL` 并同步 `build/lib/libIKW_OL.dylib`。编译通过，仅有既有 `data_raster.hpp` 常量转换警告。
- 已从 MongoDB 的 `PARAMETERS` 和 `PARAMETERS_SPEC` 集合删除前一轮临时加入的 `OL_SPEED_FACTOR` 记录。
- `param.cali` 中启用 `MANNING,2,RC`，表示保留原始 Manning 空间分布并整体乘 2；这比 `MANNING,0.06,VC` 更合理，因为 AndrewsForest 原始 Manning 栅格主值为 0.4。
- 重新导入 `param.cali` 后确认 storm/SingeRun 的校准参数为 `ACTIVE_DEPTH_MAX=150,VC`、`INFIL_FACTOR=1,VC`、`MOIST_IN=0.99,RC`、`MANNING=2,RC`，不再包含 `OL_SPEED_FACTOR`。
- 清空当前 `OUTPUT_D8_DOWNUP--` 后重跑模型，日志写入 `data/AndrewsForest/andrews_forest_model/storm/seims_manning_rc2.log`，模型正常结束。
- 结果：
  - 无 Manning 调节基线：总径流深 10.879791 mm，峰值 22.09733391 m3/s，峰现 2015-02-07 13:40，NSE=-1.494581，PBIAS=-57.806%；
  - 当前 `MANNING,2,RC`：总径流深 10.426189 mm，峰值 13.01329613 m3/s，峰现 2015-02-07 14:50，NSE=-0.727824，PBIAS=-59.565%。
- 峰形对比：
  - 基线上升段 10%-90% 约 2.417 h，上升率约 7.315 m3/s/h；当前约 3.250 h，上升率约 3.203 m3/s/h；
  - 基线下降段 90%-10% 约 15.667 h，下降率约 1.128 m3/s/h；当前约 25.500 h，下降率约 0.408 m3/s/h。
- 判断：通过原生 `MANNING` 参数按比例增大糙率，确实能降低峰值、推迟峰现，并减慢洪峰上升和下降速度；效果与前一轮 `OL_SPEED_FACTOR=0.5` 完全一致，说明后续无需保留 `OL_SPEED_FACTOR`。不过总径流深仍只有约 10.43 mm，而观测为 25.79 mm，水量偏低问题仍需通过产流/壤中流/地下水分配继续处理。

## 2026-06-03 新增雨量-流量对比绘图脚本

- 按用户要求新增 `plot_storm_q_pcp.py`，用于自动把流量模拟值、流量实测值和降雨实测值画在同一张图中。
- 默认输入：
  - 模拟流量：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/Q.txt`；
  - 出口实测流量：`data/AndrewsForest/data_prepare/observed/observed_Q_storm_outlet.csv`；
  - 5 分钟降雨：`data/AndrewsForest/data_prepare/climate/pcp_Intensity_5min.csv`。
- 图形布局：上方子图为降雨柱状图，下方子图为实测流量与模拟流量折线图，共用时间轴；默认使用所有降雨站点平均值，也可通过 `--pcp-station` 指定单站列。
- 脚本支持 `--sim-q`、`--obs-q`、`--pcp`、`--output`、`--pcp-station`、`--start`、`--end`、`--dpi` 参数，便于切换不同模型输出目录或时间范围。
- 已运行 `python -m py_compile plot_storm_q_pcp.py` 通过语法检查，并在 `pyseims` 环境下运行 `python plot_storm_q_pcp.py` 成功生成图片：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison.png`。

## 2026-06-03 基流偏低诊断与 GW0 调整实验

- 按用户指出的“2015-02-07 前流量上升太少、基流太少”问题，检查当前模型的基流相关模块逻辑。
- 当前 storm 模块链中，稳定基流主要由 `GW_RSVR` 产生，输出为 `SBQG`；`GW_RSVR` 使用 `GW0` 初始化地下水库储量，使用 `Kg` 作为退水系数，公式近似为 `outFlowDepth = storage * (1 - exp(-Kg * dt_days))`。
- 当前 `MANNING,2,RC` 但未调 `GW0` 的结果中，2015-02-05 07:00 至 2015-02-07 00:00：
  - 模拟总流量均值 0.310 m3/s，实测均值 2.766 m3/s；
  - `SBQG` 均值仅 0.070 m3/s，明显缺少约 2 m3/s 的稳定基流背景。
- 判断：初始土壤水分 `MOIST_IN` 主要影响事件内入渗、壤中流和后续渗漏，不能直接提供事件开始前稳定的 2 m3/s 基流；要抬升雨前基流，优先应调 `GW0` 和必要时调 `Kg`。
- 结合 wwj 分支：
  - wwj 对 `GW_RSVR` 加了 `GWMAX` 超限转基流和 `GW0` 已分配后更新储量的修复，并禁用调试输出；
  - wwj 对 `IKW_IF` 增加 `FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR`，用于增强快速壤中流/优先流；
  - wwj 对 `PERCO_DARCY` 增加 `FC_ADJUST`，可降低或提高进入渗漏的水分阈值。
- 先做参数实验而不改 C++：
  - `GW0,28,RC`：`SBQG` 提高到约 1.95 m3/s，但总流量被抬得过高，雨前均值达 7.42 m3/s，整场 PBIAS=+77.57%，NSE=-1.640，不合适。
  - `GW0,10,RC`：雨前模拟总流量均值 2.680 m3/s，接近实测 2.766 m3/s；`SBQG` 均值约 0.698 m3/s；整场总径流深 22.211 mm，观测 25.785 mm，PBIAS=-13.86%，NSE=0.493。
- 当前 `param.cali` 已设置 `GW0,10,RC`，并保留 `MANNING,2,RC`、`MOIST_IN,0.99,RC`、`INFIL_FACTOR,1,VC`、`ACTIVE_DEPTH_MAX,150,VC`。
- 当前输出目录为 `data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--`；`GW0,28,RC` 输出留档为 `OUTPUT_D8_DOWNUP--.gw0_rc28`，调 `GW0` 前输出留档为 `OUTPUT_D8_DOWNUP--.manning_rc2_before_gw0`。
- 更新对比图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_gw0_rc10.png`。
- 结论：基流偏低可以通过 `GW0` 有效调整，当前 `GW0,10,RC` 明显改善雨前基流和整体水量；但峰值仍偏高到 15.391 m3/s，下一步应结合 `MANNING`、`ACTIVE_DEPTH_MAX`、`DEP_FS` 或 wwj 的 `IKW_IF` 快速壤中流机制继续调整峰形。

## 2026-06-03 MANNING 与 ACTIVE_DEPTH_MAX 压峰实验

- 按用户要求，在 `GW0,10,RC` 的基础上分别尝试略增 `MANNING` 和略增 `ACTIVE_DEPTH_MAX`，目标是降低峰值并减缓峰值上升/下降速度。
- 基线为 `GW0,10,RC`、`MANNING,2,RC`、`ACTIVE_DEPTH_MAX,150,VC`：
  - 总径流深 22.211 mm，观测 25.785 mm；
  - NSE=0.493，PBIAS=-13.86%；
  - 峰值 15.391 m3/s，峰现 2015-02-07 14:50；
  - 2015-02-07 前模拟均值 2.680 m3/s，实测均值 2.766 m3/s。
- 实验 1：`MANNING,3,RC`、`ACTIVE_DEPTH_MAX,150,VC`：
  - 总径流深 21.759 mm；
  - NSE=0.680，PBIAS=-15.61%；
  - 峰值降到 11.713 m3/s，峰现推迟到 2015-02-07 17:00；
  - 2015-02-07 前基流均值仍为 2.680 m3/s，说明主要影响洪峰汇流，不破坏雨前基流。
- 实验 2：`MANNING,2,RC`、`ACTIVE_DEPTH_MAX,200,VC`：
  - 总径流深降到 13.356 mm；
  - NSE=-0.854，PBIAS=-48.20%；
  - 峰值过度削减为 3.334 m3/s，峰现提前到 2015-02-07 07:45；
  - 判断为入渗容量增大过强，产流不足，不适合作为当前方向。
- 当前采用实验 1 结果：`param.cali` 已恢复为 `ACTIVE_DEPTH_MAX,150,VC`、`MANNING,3,RC`、`GW0,10,RC`，MongoDB `PARAMETERS_SPEC` 已同步。
- 输出留档：
  - 基线：`OUTPUT_D8_DOWNUP--.gw0_rc10_manning_rc2_active150`；
  - `MANNING,3,RC`：`OUTPUT_D8_DOWNUP--.gw0_rc10_manning_rc3_active150`；
  - `ACTIVE_DEPTH_MAX,200,VC`：`OUTPUT_D8_DOWNUP--.gw0_rc10_manning_rc2_active200`。
- 当前 `OUTPUT_D8_DOWNUP--` 已恢复为 `MANNING,3,RC` 的最佳结果，并生成对比图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_manning_rc3.png`。

## 2026-06-03 绘图脚本增加 NSE 和 PBIAS 标注

- 按用户要求修改 `plot_storm_q_pcp.py`，在雨量-流量对比图上显示 NSE 和 PBIAS。
- 新增 `calc_stats()`：基于模拟流量和实测流量的时间交集计算 NSE、PBIAS 和样本数，避免非重叠时段影响指标。
- 在下方流量子图左上角添加白底文本框，显示 `NSE`、`PBIAS` 和 `n`。
- 清理脚本中未使用的 `csv` 和 `datetime` import。
- 已运行 `python -m py_compile plot_storm_q_pcp.py` 通过语法检查，并生成新图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_with_stats.png`。

## 2026-06-03 wwj 的 IKW_IF 快速壤中流机制与峰前波动分析

- 按用户问题，检查 wwj 分支对 `IKW_IF` 的快速壤中流机制，并结合当前输出分量分析 2015-02-07 峰前低流量阶段曲线波动大的原因。
- wwj 对 `IKW_IF` 的核心改动：
  - 新增 `FAST_RATIO`：把上游进入土层的一部分水作为快速旁路流直接传递，土壤越接近田间持水量，旁路比例越大；
  - 新增 `ANISOTROPY`：把水平导水能力放大，用于模拟火山灰土/森林坡地中横向导水能力强于垂向导水能力；
  - 新增 `MACROPORE_FACTOR`：额外增加大孔隙/优先流通道，并允许这部分水从低于田间持水量的范围内快速排出；
  - 将超过孔隙度的回归流按所有土层累加后统一加入地表径流，避免逐层覆盖；
  - 在低于田间持水量时，原代码完全不产生壤中流，wwj 改为仍允许一部分快速旁路流通过。
- 当前代码的 `IKW_IF` 机制仍有明显阈值特征：土壤含水量低于田间持水量时 `m_subSurfQ=0`，一旦超过田间持水量就按可用水和导水率出流。这种“开/关”逻辑容易造成峰前小雨阶段的脉冲波动。
- 当前 `MANNING,3,RC`、`GW0,10,RC`、`ACTIVE_DEPTH_MAX=150` 结果中，2015-02-07 00:00-10:00：
  - 实测流量均值约 4.912 m3/s，模拟均值约 2.812 m3/s；
  - `SBQG` 均值约 0.694 m3/s，几乎不波动；
  - `QS` 均值约 1.772 m3/s，`QI` 均值约 1.041 m3/s；
  - 每 5 分钟平均绝对变化量：实测约 0.025 m3/s，模拟总流量约 0.077 m3/s，而 `QS` 和 `QI` 分别约 0.407 和 0.415 m3/s。
- 判断：峰前低流量阶段的“波动大”主要不是地下水基流引起的，而是地表快流和当前阈值型壤中流在小雨/间歇降雨下产生脉冲。实测更像是稳定基流加平滑的坡地壤中流/优先流响应。
- 是否与壤中流机制有关：有关。当前 `IKW_IF` 的壤中流太弱且阈值太硬，导致一部分应表现为平滑坡地出流的水，要么被推迟，要么变成地表/壤中流脉冲。wwj 的快速壤中流机制正是试图把一部分水从地表快流转到更连续的坡地侧向流/优先流。
- 调整建议：
  - 不建议继续只靠 `GW0` 抬升，因为当前雨前基流已经接近实测，继续抬会整体偏高；
  - 不建议大幅增大 `ACTIVE_DEPTH_MAX`，前一轮 200 mm 已经使产流过弱；
  - 可以先小幅试 `IKW_IF` 方向：引入 `ANISOTROPY` 和较温和的 `MACROPORE_FACTOR`，谨慎使用 `FAST_RATIO`，避免把壤中流又变成更快的脉冲；
  - 若要更物理地减小波动，后续可考虑给 `IKW_IF` 增加一阶储水/滞后系数，让壤中流从“即时出流”变成带蓄泄的侧向流。

## 2026-06-03 保守引入 wwj 的 IKW_IF 快速壤中流机制并测试

- 按用户要求，将 wwj 分支的 `IKW_IF` 快速壤中流机制保守接入当前代码：
  - 在 `text.h` 中新增 `FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR` 参数名；
  - 在 `IKW_IF/api.cpp` 中注册三个参数，使模块运行时可从 MongoDB/`param.cali` 读取；
  - 在 `InterFlow_IKW` 中新增三个成员变量，默认值设为 `FAST_RATIO=0`、`ANISOTROPY=1`、`MACROPORE_FACTOR=1`，因此默认不改变原模型行为；
  - 在土壤侧向流计算中加入可调快速旁路流、水平各向异性导水增强和大孔隙流；
  - 关闭 `IKW_IF` 内部调试打印；
  - 修复 `plot_storm_q_pcp.py` 读取多轮追加输出时的统计问题：同一时间重复时保留最后一轮结果。
- 编译与数据库处理：
  - 运行 `cmake --build build --target IKW_IF -j4` 编译通过；
  - 将新编译的 `libIKW_IF.dylib` 同步到 `build/lib/`，供常用的 `build/bin/seims_omp` 加载；
  - MongoDB `PARAMETERS` 主表补入 `FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR` 三条默认参数记录；
  - 注意：新参数必须在 `param.cali` 中显式使用 `VC`，否则会继承主表 `AC` 解释，导致 `ANISOTROPY,1` 被当作基准值加 1，而不是绝对值 1。
- 实验 1：`FAST_RATIO=0.08`、`ANISOTROPY=4`、`MACROPORE_FACTOR=1.5`。
  - 结果明显变坏，峰值约 29.17 m3/s，NSE 约 -0.91，PBIAS 约 +14.96%；
  - 说明即使较小的快速旁路和大孔隙流，也会把水过快送到河道，放大峰前波动和洪峰；
  - 该结果已作为坏实验留档。
- 实验 2：关闭快速旁路和大孔隙，仅保留侧向导水增强，`FAST_RATIO=0`、`ANISOTROPY=4`、`MACROPORE_FACTOR=1`。
  - 结果仍明显变坏，峰值约 27.73 m3/s，NSE 约 -1.81，PBIAS 约 +34.54%；
  - `QS` 仍是峰值放大的主因，说明增强 `IKW_IF` 后改变了坡面/壤中流时序，反而推动更多水以快流形式到达出口。
- 恢复当前可用结果：
  - 将 `param.cali` 改为真正中性值：`FAST_RATIO,0,VC`、`ANISOTROPY,1,VC`、`MACROPORE_FACTOR,1,VC`；
  - 清理并重跑当前输出，`Q.txt/QS.txt/QI.txt/SBQG.txt` 已恢复为单轮干净结果；
  - 中性值结果回到原基线：NSE=0.6799，PBIAS=-15.61%，模拟峰值 11.713 m3/s，峰现 2015-02-07 17:00。
- 输出留档：
  - 坏实验文本：`Q_ikw_if_aniso4.txt`、`QS_ikw_if_aniso4.txt`、`QI_ikw_if_aniso4.txt`、`SBQG_ikw_if_aniso4.txt`；
  - 坏实验图：`q_pcp_comparison_ikw_if_aniso4.png`；
  - 当前中性回跑图：`q_pcp_comparison_neutral_vc_after_ikw_if.png`。
- 结论：
  - wwj 的 `IKW_IF` 快速/增强壤中流机制已经成功接入并可通过参数控制；
  - 但在 AndrewsForest 当前参数组合下，开启该机制不是改善方向，会显著放大峰值和峰前波动；
  - 下一步更合理的方向不是继续增加“快速壤中流”，而是给 `IKW_IF` 或坡面汇流增加滞后/蓄泄机制，或者继续从 `SUR_SGA/DEP_FS/IKW_OL` 的快流生成与汇流平滑性入手。

## 2026-06-03 小幅 ANISOTROPY 与增大 MANNING 组合实验

- 按用户提出的思路，测试“轻微增加壤中流侧向导水能力，同时增大坡面曼宁系数压慢地表快流”是否能降低峰值并让曲线更缓。
- 实验参数：
  - `FAST_RATIO,0,VC`：关闭快速旁路流；
  - `ANISOTROPY,1.2,VC`：仅比中性值 1 略增侧向导水能力；
  - `MACROPORE_FACTOR,1,VC`：关闭大孔隙增强；
  - `MANNING,3.5,RC`：在此前较优 `MANNING,3,RC` 基础上继续略增坡面阻力；
  - 其他保持当前设置：`GW0,10,RC`、`ACTIVE_DEPTH_MAX,150,VC`、`MOIST_IN,0.99`。
- 运行方式：
  - 先备份当前中性基线输出为 `Q_neutral_before_aniso12_manning35.txt`、`QS_neutral_before_aniso12_manning35.txt`、`QI_neutral_before_aniso12_manning35.txt`、`SBQG_neutral_before_aniso12_manning35.txt`；
  - 清理当前 `Q/QS/QI/SBQG.txt` 后重新运行模型，日志保存为 `seims_ikw_if_aniso12_manning35.log`；
  - 结果另存为 `Q_aniso12_manning35.txt`、`QS_aniso12_manning35.txt`、`QI_aniso12_manning35.txt`、`SBQG_aniso12_manning35.txt`；
  - 生成图：`q_pcp_comparison_aniso12_manning35.png`。
- 结果：
  - NSE=0.7005，略优于此前中性基线的 NSE=0.6799；
  - PBIAS=-15.77%，与此前基线 -15.61% 基本相同；
  - 模拟峰值从 11.713 m3/s 降到 10.967 m3/s，更接近实测峰值 10.449 m3/s；
  - 峰现仍为 2015-02-07 17:00，仍晚于实测峰现 2015-02-07 15:00；
  - 2015-02-07 00:00-10:00 的模拟均值约 2.818 m3/s，实测均值约 4.912 m3/s，峰前上升不足问题仍存在。
- 判断：
  - 这个组合方向对“压峰”有效，且没有像 `ANISOTROPY=4` 那样造成峰值暴涨；
  - 但它主要是通过更大的 `MANNING` 压慢/降低地表快流峰值，`ANISOTROPY=1.2` 只带来很小的壤中流变化；
  - 该方向不能解决 2015-02-07 10:00 前模拟流量偏低的问题，下一步若要改善峰前抬升，应考虑基流/慢壤中流滞后蓄泄、降雨时序或更平滑的坡地出流机制。

## 2026-06-03 MANNING=3.5 单独控制实验

- 用户指出上一轮 `ANISOTROPY=1.2 + MANNING=3.5` 没有控制变量，因此补做单独 `MANNING=3.5` 实验。
- 实验参数：
  - `MANNING,3.5,RC`；
  - `FAST_RATIO,0,VC`；
  - `ANISOTROPY,1,VC`；
  - `MACROPORE_FACTOR,1,VC`；
  - 其他保持不变。
- 结果：
  - NSE=0.6949，PBIAS=-16.49%；
  - 模拟峰值 10.715 m3/s，实测峰值 10.449 m3/s；
  - 峰现为 2015-02-07 18:25，比 `MANNING=3` 和组合实验更晚；
  - 2015-02-07 00:00-10:00 模拟均值约 2.809 m3/s，实测均值约 4.912 m3/s，峰前偏低仍存在。
- 与组合实验对比：
  - `MANNING=3.5, ANISOTROPY=1`：NSE=0.6949，峰值 10.715 m3/s；
  - `MANNING=3.5, ANISOTROPY=1.2`：NSE=0.7005，峰值 10.967 m3/s；
  - 因此上一轮改善主要来自 `MANNING=3.5`，`ANISOTROPY=1.2` 只带来很小的 NSE 提升，同时略微抬高峰值并使峰现提前回 17:00。
- 输出留档：
  - 控制实验图：`q_pcp_comparison_manning35_aniso1_control.png`；
  - 控制实验文本：`Q_manning35_aniso1_control.txt`、`QS_manning35_aniso1_control.txt`、`QI_manning35_aniso1_control.txt`、`SBQG_manning35_aniso1_control.txt`。
- 结论：
  - 单独增大 `MANNING` 已经能取得主要压峰效果；
  - `ANISOTROPY=1.2` 的额外贡献很小，不足以说明增强侧向导水是当前主要改善机制；
  - 当前如果只从指标看，组合实验 NSE 略高；如果从物理解释和控制变量稳健性看，应优先保留 `MANNING=3.5, ANISOTROPY=1` 作为更干净的压峰方案。

## 2026-06-03 扩展 storm 模拟时段至 2015-02-04 至 2015-02-16

- 按用户要求，将 AndrewsForest storm 模拟时段改为：
  - `STARTTIME|2015-02-04 06:00:00`；
  - `ENDTIME|2015-02-16 12:00:00`。
- 修改文件：
  - `data/AndrewsForest/andrews_forest_model/storm/file.in`；
  - `data/AndrewsForest/andrews_forest_model/storm/file.out`，同步将 `QSUBBASIN/QS/QI/SBQG/SBGS/SOLST` 输出时段改为相同长时段。
- 运行设置：
  - 当前参数为 `MANNING,3.5,RC`、`FAST_RATIO,0,VC`、`ANISOTROPY,1,VC`、`MACROPORE_FACTOR,1,VC`、`GW0,10,RC`、`ACTIVE_DEPTH_MAX,150,VC`；
  - 重新导入 `file.in/file.out` 和 `param.cali` 到 MongoDB；
  - 清理旧 `Q/QS/QI/SBQG/SBGS.txt` 后运行 `seims_omp`；
  - 日志保存为 `seims_long_20150204_20150216.log`。
- 输出检查：
  - 当前 `Q.txt` 从 `2015-02-04 06:00:00` 输出到 `2015-02-16 11:55:00`，共 3528 个 5 分钟步长；末步为 11:55 是因为输出间隔 300 秒，到 12:00 正好为结束边界。
  - 长时段结果另存为 `Q_long_20150204_20150216.txt`、`QS_long_20150204_20150216.txt`、`QI_long_20150204_20150216.txt`、`SBQG_long_20150204_20150216.txt`。
  - 长时段图：`q_pcp_comparison_long_20150204_20150216.png`。
- 与实测重叠全时段统计：
  - 重叠时段：`2015-02-04 06:00:00` 至 `2015-02-16 11:55:00`；
  - NSE=-13.1645，PBIAS=+18.24%；
  - 实测峰值 10.449 m3/s，发生在 `2015-02-07 15:00:00`；
  - 模拟峰值 54.311 m3/s，发生在 `2015-02-10 01:30:00`。
- 原短窗口表现：
  - 在 `2015-02-05 07:00` 至 `2015-02-08 19:55` 原窗口内，模拟均值 4.320 m3/s，实测均值 5.189 m3/s；
  - 模拟峰值约 10.701 m3/s，短窗口表现仍接近控制实验结果。
- 新暴露的问题：
  - 2015-02-09 至 2015-02-10 期间，降雨文件显示平均降雨累计很大，`2015-02-09 00:00` 至 `2015-02-10 12:00` 平均降雨约 577.92；
  - 模拟在 `2015-02-10 01:30` 出现 54.311 m3/s 巨峰，而实测同期峰值仅约 8.38 m3/s；
  - 该巨峰几乎完全来自 `QS` 地表径流，峰时 `QS=54.311 m3/s`、`QI≈0.00005 m3/s`，说明长时段后暴雨事件的地表产流/坡面汇流严重过强。
- 判断：
  - 扩展时段后，模型不再只面对 2 月 7 日单场事件，而是暴露了 2 月 10 日事件的明显高估；
  - 当前问题主因不是 `IKW_IF` 壤中流，而是后续大雨条件下 `SUR_SGA/DEP_FS/IKW_OL` 快速地表径流响应过强，或降雨输入单位/强度需要进一步核查；
  - 下一步应优先检查 2 月 9-10 日降雨输入单位、站点平均方法、`SUR_SGA` 入渗限制和 `DEP_FS/IKW_OL` 地表水排空逻辑。

## 2026-06-03 2 月 9-10 日降雨输入单位与地表快流逻辑检查

- 按用户要求，检查 `2015-02-09` 至 `2015-02-10` 降雨输入单位/强度，以及 `SUR_SGA`、`DEP_FS`、`IKW_OL` 的地表快流逻辑。
- 生成本地诊断文档：
  - `data/AndrewsForest/andrews_forest_model/storm/feb09_10_fast_runoff_diagnosis.md`。
- 降雨输入检查：
  - `data/AndrewsForest/data_prepare/climate/pcp_Intensity_5min.csv` 文件头为 `#UTCTIME` 和 `#TIMESTEP 300`；
  - `data/AndrewsForest/data_prepare/climate/Variables.csv` 中 `P` 的单位写为 `mm`；
  - MongoDB `andrews_forest_HydroClimate.DATA_VALUES` 中 `TYPE=P, TIMESTEP=300` 的 `VALUE` 与 CSV 原值一致，没有额外单位换算；
  - `2015-02-09 00:00` 至 `2015-02-10 12:00` 每站累计值为 577.92，最大单步值 10.92，发生在 `2015-02-09 22:55`；
  - `2015-02-10 00:00` 至 `2015-02-10 03:00` 每站累计值为 73.56，最大单步值 10.8，发生在 `2015-02-10 00:30`。
- 输出响应检查：
  - `2015-02-09 00:00` 至 `2015-02-10 12:00`，模拟 `Q` 峰值 54.310848 m3/s，发生在 `2015-02-10 01:30`；
  - 同时刻 `QS` 峰值 54.310795 m3/s，`QI` 峰值仅 0.035510 m3/s，峰时 `QI` 接近 0；
  - `2015-02-10 00:00` 至 `03:00`，模拟 `Q/QS` 均值约 52.92 m3/s，而实测均值约 7.86 m3/s；
  - 因此 2 月 10 日异常峰值几乎完全由 `QS` 地表快流贡献。
- 代码逻辑判断：
  - `SUR_SGA` 将 `VAR_NEPR` 作为当前时间步水深使用，核心链路为 `hWater = m_netPcp + m_sd + snowMelt`，入渗后剩余水量进入 `m_exsPcp`；
  - `SUR_SGA` 不把降雨强度从 `mm/h` 自动换算成 5 分钟雨深，因此如果输入值实际是 mm/h，会被当成 mm/5min 使用，相当于放大约 12 倍；
  - `DEP_FS` 将 `m_exsPcp` 分配到洼地蓄水和 `m_sr`，当前实现用 `totalWater = m_exsPcp[i]`，没有把已有 `m_sd[i]` 纳入本步总水量，洼地蓄水连续性偏弱；
  - `IKW_OL` 从 `m_sr` 计算地表水深并用隐式运动波汇流，且对出流体积有 `allocatedVolume / dt` 上限约束，不应凭空创造水，主要是在快速输送 `DEP_FS` 传来的地表水。
- 结论：
  - 当前最强证据指向降雨输入含义不一致：文件名为 `pcp_Intensity_5min.csv`，数值量级像强度，但元数据和模型都按每 5 分钟雨深 `mm` 使用；
  - 如果将 577.92 解释为 mm/h 强度积分并换成 5 分钟雨深，累计约为 48.16 mm，更符合该事件的实测流量量级；
  - 下一步建议先做一个严格控制实验：备份降雨文件，将 P 值除以 12 后重新导入 MongoDB 并重新运行，再判断是否仍需修改 `DEP_FS` 或继续调 `MANNING/ACTIVE_DEPTH_MAX`。

### 补充：与 2 月 7 日降雨强度对比

- 用户指出 `2015-02-10` 附近最大单步降雨约 10，与 `2015-02-07 06:00` 附近量级接近，因此重新比较两段事件。
- 结果：
  - `2015-02-07 06:00` 至 `09:00`：累计降雨 136.20，最大单步 12.96，非零降雨步数 34，模拟 `Q` 峰值 3.298 m3/s，`QS` 峰值 2.730 m3/s，`QI` 均值 1.346 m3/s；
  - `2015-02-07 00:00` 至 `12:00`：累计降雨 389.40，最大单步 12.96，模拟 `Q` 峰值 4.717 m3/s，`QS` 峰值 4.535 m3/s，`QI` 均值 0.917 m3/s；
  - 原短事件窗口 `2015-02-05 07:00` 至 `2015-02-08 20:00`：累计降雨 815.40，模拟 `Q` 峰值 10.701 m3/s，接近实测峰值 10.449 m3/s；
  - `2015-02-09 00:00` 至 `2015-02-10 12:00`：累计降雨 577.92，最大单步 10.92，模拟 `Q/QS` 峰值 54.311 m3/s，`QI` 均值仅 0.0017 m3/s。
- 判断：
  - 单步最大降雨确实不能单独解释 2 月 10 日异常峰；
  - 更关键的差异是长时段运行中，2 月 7 日事件之后 `SUR_SGA` 的活动层入渗容量被前期累计入渗消耗，导致 2 月 9-10 日降雨几乎不再进入 `QI/INFIL`，而是主要变为 `EXCP -> DEP_FS -> m_sr -> IKW_OL -> QS`；
  - 因此下一步除了继续核查降雨单位，还必须检查 `m_accumuDepth` 是否按整个长时段持续累积、是否应该按事件/退水过程恢复入渗容量，以及 `DEP_FS`/`IKW_OL` 对前期地表水和洼地蓄水的处理是否过于直接。

## 2026-06-03 SUR_SGA 中 m_accumuDepth 累积/恢复机制检查

- 按用户要求，专门检查 `SUR_SGA` 中 `m_accumuDepth` 是否在整个长模拟期持续累积，以及是否存在恢复或重置机制。
- 检查文件：
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.cpp`；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.h`；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/api.cpp`；
  - `seims/src/seims_main/modules/hydrology/IKW_OL/ImplicitKinematicWave.cpp`；
  - `seims/src/seims_main/modules/hydrology/IKW_OL/api.cpp`。
- 代码发现：
  - `m_accumuDepth` 在 `StormGreenAmpt::Execute()` 首次运行、`m_capillarySuction == nullptr` 时通过 `Initialize1DArray(m_nCells, m_accumuDepth, 0.f)` 初始化；
  - 正常入渗时，`SUR_SGA` 通过 `m_accumuDepth[i] += m_infil[i]` 累加；
  - 无雨时，`SUR_SGA` 只计算 `m_infilCapacitySurplus[i] = Min(infilRate * dt * 1000.f, infilCap)`，没有减少或恢复 `m_accumuDepth`；
  - `CalculateActiveInfilCap()` 使用 `activeStorage - m_accumuDepth[cell]` 计算剩余活动层入渗容量，因此 `m_accumuDepth` 会直接扣减后续事件可用入渗容量；
  - `IKW_OL` 读取 `SUR_SGA` 输出的 `AccumuInfil`，在地表水再入渗时还会继续执行 `m_accumuDepth[id] += reInfil`；
  - 全项目搜索没有发现任何 `m_accumuDepth -=`、按无雨时长衰减、按事件边界重置、按土壤排水恢复的机制。
- 判断：
  - 当前实现中，`m_accumuDepth` 在一次模型运行内基本是单调不减状态，直到模型结束或模块重新初始化；
  - 在短事件窗口内，这相当于事件尺度 Green-Ampt 累积入渗，尚可解释；
  - 在当前扩展到 `2015-02-04 06:00` 至 `2015-02-16 12:00` 的长 storm 模拟中，它会把 2 月 7 日等前期事件的入渗持续带到 2 月 9-10 日，导致活动层剩余入渗容量被耗尽，后续降雨更容易直接转为 `EXCP/QS`；
  - 这与 2 月 10 日 `Q` 几乎完全等于 `QS`、`QI` 接近 0 的现象一致。
- 需要注意：
  - `StormGreenAmpt.h` 中注释写 `m_accumuDepth` 为 cumulative infiltration depth `(m)`，但实际代码中它以 `mm` 存储，使用时在 `m_accumuDepth / 1000.f` 转为 m；
  - `git fetch wwj validation-2026-05-07` 因网络等待被手动终止，本次结论基于当前工作区源码和已有全项目搜索。

## 2026-06-03 SUR_SGA m_accumuDepth 物理修正方案

- 用户询问应如何修改才能改善结果并符合物理机理逻辑。
- 进一步检查 `PERCO_DARCY`：
  - storm 配置顺序为 `SUR_SGA -> DEP_FS -> IKW_OL -> IKW_IF -> PERCO_DARCY -> GW_RSVR -> CH_DW`；
  - `PERCO_DARCY` 读取 `SUR_SGA` 输出的 `SOL_ST`，并在每步中将超过田间持水量的水向下层/地下水排泄；
  - 因此真实土壤水状态 `m_soilWtrSto` 是动态变化的，但当前 `CalculateActiveInfilCap()` 仍使用初始含水量亏缺并减去单调累积的 `m_accumuDepth`，没有利用 `PERCO_DARCY` 后恢复出来的孔隙空间。
- 推荐修改方向：
  - 第一优先级：把 `CalculateActiveInfilCap()` 改为基于当前动态土壤水亏缺，即用 `m_soilPor - m_soilWtrSto` 计算活动层可入渗容量，而不是用 `m_soilPor - MOIST_IN * FC - m_accumuDepth`；
  - 这样当前土壤经 `PERCO_DARCY` 排水后，下一步/下一场雨的活动层入渗容量会自然恢复，更符合连续多事件 storm 模拟；
  - 第二优先级：将 `m_accumuDepth` 明确作为“当前降雨事件的 Green-Ampt 累积入渗量”，在无雨且无地表水持续一定时间后按参数逐步衰减或重置；
  - 可以增加可校准参数如 `GA_ACC_RECOVERY_RATE`（mm/h）或 `GA_DRY_RESET_HOURS`（h），用于控制湿润锋恢复速度，避免长模拟期内 `m_accumuDepth` 永久占用后续事件入渗能力。
- 判断：
  - 不建议继续单纯增大 `ACTIVE_DEPTH_MAX`，因为它只是扩大容量上限，不能解决“土壤已经排水但 `m_accumuDepth` 不恢复”的状态变量断裂；
  - 也不建议直接每步清零 `m_accumuDepth`，因为会破坏同一场雨内部 Green-Ampt 累积入渗导致入渗能力递减的机理；
  - 推荐先做最小物理修正：动态亏缺容量 + 保留事件内 `m_accumuDepth`；如果 2 月 10 日仍过强，再加入干期衰减/重置参数。

## 2026-06-03 SUR_SGA 动态入渗容量与干期恢复实现

- 按用户要求，基于上一节方案修改 `SUR_SGA` 并重新运行 AndrewsForest storm 长时段模型。
- 修改文件：
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.h`；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.cpp`；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/api.cpp`；
  - `seims/preprocess/database/model_param_ini.csv`；
  - `data/AndrewsForest/andrews_forest_model/storm/param.cali`。
- 代码修改：
  - 新增可选参数 `GA_ACC_RECOVERY_RATE`，单位 `mm/h`，表示无雨时 Green-Ampt 事件累计入渗量 `m_accumuDepth` 的恢复/衰减速率；
  - `m_accumuDepth` 注释单位从 `(m)` 改为 `(mm)`，与实际代码存储一致；
  - 当 `hWater <= 0` 且 `GA_ACC_RECOVERY_RATE > 0` 时，按 `rate * dt / 3600` 逐步减少 `m_accumuDepth`；
  - `CalculateActiveInfilCap()` 最终采用组合约束：`min(当前动态土壤亏缺, 初始活动层事件容量 - 已恢复后的 m_accumuDepth)`；
  - 这样保留同一场雨内部 Green-Ampt 累积入渗效应，同时允许长时段多事件间有一定恢复。
- 参数库修正：
  - 发现仅把 `GA_ACC_RECOVERY_RATE` 放入 `param.cali/PARAMETERS_SPEC` 会被 C++ 警告为 `not supported`，因为基础 `PARAMETERS` 表缺少该参数；
  - 已在 `model_param_ini.csv` 增加基础定义，并直接向当前 MongoDB `andrews_forest_model.PARAMETERS` upsert 该参数；
  - 后续日志确认 `GA_ACC_RECOVERY_RATE` 不再出现不支持警告。
- 编译/运行：
  - `cmake --build build --target SUR_SGA -j4` 成功，仅有既有 `data_raster.hpp` 数值转换 warning；
  - 将新库同步到 `build/lib/libSUR_SGA.dylib` 和 `build/install/lib/libSUR_SGA.dylib`；
  - 尝试 `cmake --build build --target install -j4` 时被 TauDEM `streamnet.cpp` 既有编译错误阻断，和本次 `SUR_SGA` 修改无关；
  - 使用 `build/bin/seims_omp` 运行长时段 storm 模型，日志分别保存为：
    - `seims_dynamic_infilcap.log`；
    - `seims_dynamic_infilcap_ad50.log`；
    - `seims_dynamic_infilcap_ad50_recovery2.log`；
    - `seims_infilcap_combined_recovery2.log`；
    - `seims_infilcap_combined_recovery0p2.log`；
    - `seims_infilcap_combined_recovery1p0.log`；
    - `seims_infilcap_combined_recovery0p5.log`。
- 实验结果摘要：
  - 原长时段结果：全时段 NSE=-13.1645，PBIAS=+18.24%，2 月 10 日 `Q` 峰值 54.31 m3/s；
  - 单用动态亏缺容量或 `GA_ACC_RECOVERY_RATE=2.0`：2 月 10 日峰值被压到约 3.22 m3/s，但 2 月 7 日原事件也被压低到约 3.32 m3/s，整体明显偏低；
  - `GA_ACC_RECOVERY_RATE=0.2`：恢复太弱，2 月 10 日峰值仍约 53.15 m3/s，接近旧异常峰；
  - `GA_ACC_RECOVERY_RATE=1.0`：恢复太强，长时段峰值约 3.36 m3/s，整体偏低；
  - 当前保留 `GA_ACC_RECOVERY_RATE=0.5`：2 月 10 日峰值从 54.31 降至 30.19 m3/s，全时段 NSE 从 -13.16 改善到 -2.8449，PBIAS 从 +18.24% 变为 -19.20%，但峰值仍偏高且 2 月 7 日被低估。
- 当前 `GA_ACC_RECOVERY_RATE=0.5` 详细指标：
  - 全时段 `2015-02-04 06:00` 至 `2015-02-16 11:55`：NSE=-2.8449，PBIAS=-19.20%，模拟峰值 30.19 m3/s；
  - 2 月 7 日旧事件窗口：NSE=-0.7960，PBIAS=-47.54%，模拟峰值 3.32 m3/s，实测峰值 10.45 m3/s；
  - 2 月 9-10 日事件窗口：NSE=-64.6681，PBIAS=+66.37%，模拟峰值 30.19 m3/s，实测峰值 8.38 m3/s；
  - 2 月 10 日 00:00-03:00：模拟峰值 29.35 m3/s，旧结果峰值 54.31 m3/s。
- 生成图：
  - `data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_dynamic_infilcap_recovery0p5.png`。
- 判断：
  - 新机制可以证明 `m_accumuDepth` 恢复会显著影响 2 月 10 日异常地表快流，但单一线性恢复速率非常敏感；
  - 当前 `0.5 mm/h` 不是最终最优参数，只是保留了一个中间折中结果；
  - 后续更合理的方向可能是：恢复机制只在连续无雨且无地表水/洼地蓄水超过一定时长后启动，而不是每个无雨 5 分钟步立即恢复；或将恢复速率与 `PERCO_DARCY` 的实际排水量挂钩。

## 2026-06-03 SUR_SGA 干期延迟恢复机制实现与测试

- 用户指出 2 月 7 日峰值被压没，解释后按用户要求继续修改恢复机制。
- 代码修改：
  - 在 `SUR_SGA` 中新增每栅格状态 `m_dryDuration`，单位秒，用于记录连续干期长度；
  - 新增可选参数 `GA_ACC_RECOVERY_DELAY`，单位小时，含义为连续干期达到该阈值后才允许 `m_accumuDepth` 按 `GA_ACC_RECOVERY_RATE` 恢复；
  - 干期判据为当前步净雨、融雪、洼地蓄水和地表水均小于阈值；
  - 只要任一项重新有水，就将对应栅格 `m_dryDuration` 清零；
  - 这样短暂 5 分钟无雨不会立即恢复 Green-Ampt 累积入渗量。
- 文件修改：
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.h`；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.cpp`；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/api.cpp`；
  - `seims/preprocess/database/model_param_ini.csv`；
  - `data/AndrewsForest/andrews_forest_model/storm/param.cali`。
- 参数库：
  - 已向 `seims/preprocess/database/model_param_ini.csv` 增加 `GA_ACC_RECOVERY_DELAY`；
  - 已向当前 MongoDB `andrews_forest_model.PARAMETERS` upsert `GA_ACC_RECOVERY_DELAY` 基础参数；
  - `param.cali` 中当前保留：
    - `GA_ACC_RECOVERY_RATE,0.5,VC`；
    - `GA_ACC_RECOVERY_DELAY,0.5,VC`。
- 编译/运行：
  - `cmake --build build --target SUR_SGA -j4` 成功，仅有既有 `data_raster.hpp` warning；
  - 新 `libSUR_SGA.dylib` 已同步到 `build/lib/` 和 `build/install/lib/`；
  - 已重新运行 AndrewsForest storm 长时段模型。
- delay 参数测试结果：
  - `delay=6 h`：2 月 7 日峰值基本保住，模拟峰值 10.535 m3/s；但 2 月 10 日峰值仍 54.254 m3/s，几乎回到旧异常峰；
  - `delay=2 h`：2 月 7 日峰值 8.883 m3/s，仍可接受一些；但 2 月 10 日峰值 53.805 m3/s，仍几乎未改善；
  - `delay=0.5 h`：2 月 10 日峰值降到 48.054 m3/s，只是小幅改善；2 月 7 日峰值被压到 4.834 m3/s，明显低估。
- 当前 `delay=0.5 h` 详细指标：
  - 全时段：NSE=-9.5589，PBIAS=+4.08%，模拟峰值 48.054 m3/s；
  - 2 月 7 日旧事件窗口：NSE=-0.1086，PBIAS=-37.12%，模拟峰值 4.834 m3/s；
  - 2 月 9-10 日事件窗口：NSE=-242.9369，PBIAS=+182.60%，模拟峰值 48.054 m3/s；
  - 2 月 10 日 00:00-03:00：模拟峰值 48.054 m3/s，仍远高于实测约 8.21 m3/s。
- 当前图：
  - `data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_recovery0p5_delay0p5.png`。
- 判断：
  - “连续干期延迟恢复”机制能解释为什么 2 月 7 日会被压没，并能在 `delay=6h/2h` 下保护 2 月 7 日峰值；
  - 但它无法同时显著修复 2 月 10 日异常峰，说明 2 月 9-10 日问题不只是事件间 `m_accumuDepth` 恢复不足；
  - 继续只调 `GA_ACC_RECOVERY_RATE/DELAY` 会在“2 月 7 日保峰”和“2 月 10 日压峰”之间拉扯，很难两者兼顾；
  - 下一步更合理的方向应检查 2 月 10 日事件的降雨体积/时序、`DEP_FS` 地表水累积逻辑、以及 `IKW_OL` 地表水排空速度；或者将恢复量与 `PERCO_DARCY` 的实际排水量/土壤水变化耦合，而不是只按无雨时长恢复。

## 2026-06-03 DEP_FS/IKW_OL 诊断与 SUR_SGA 状态约束实验

- 按用户要求继续执行下一步建议，重点检查 2 月 9-10 日异常高峰是否来自 `DEP_FS`/`IKW_OL` 地表快流逻辑。
- 诊断过程：
  - 查询 MongoDB 参数与 GridFS 栅格，确认 `Depression` 洼地容量在 2 号子流域均值约 `0.784 mm`、中位数约 `0.134 mm`，`MANNING` 多数为 `0.4`；
  - 解析 `seims_infilcap_recovery0p5_delay0p5.log` 中 `DEP_FS` 的 TRACE，发现 2 月 10 日前后跟踪格子的 `SURU/Sr` 局部最高约 `229 mm`，而 `DPST` 均值仅约 `0.67 mm`；
  - 分析 `IKW_OL` 代码，确认当前实现有体积上限检查，未发现明显凭空放大水量的数值逻辑；高峰主要来自前序模块持续交给地表水池的水量。
- 参数实验 1：`Depression,3,RC`
  - 在 `param.cali` 中临时加入 `Depression,3,RC`，导入 MongoDB 后重跑；
  - 输出备份：
    - 改前结果：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--.before_depression_rc3_20260603_154307`；
    - 洼地容量实验结果：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--.depression_rc3_20260603_154809`；
  - 结果：全时段 NSE 从 `-9.5589` 仅改善到 `-9.3922`，2 月 10 日峰值从 `48.054` 仅降到 `47.473 m3/s`；
  - 判断：几毫米级洼地容量无法解释/修复上百毫米量级的地表水池累积，单独调 `Depression` 不是主方向；已将该项在 `param.cali` 中重新注释。
- 代码实验：将 `SUR_SGA::CalculateActiveInfilCap()` 中的 `m_accumuDepth` 与当前活动层土壤水状态耦合。
  - 新增逻辑：根据 `eventStorage - dynamicStorage` 估计当前活动层内仍保留的事件水量，若 `m_accumuDepth` 高于该值，则将其下调；
  - 物理含义：当 `PERCO_DARCY` 或层间水分转移已经让活动层恢复孔隙亏缺时，Green-Ampt 事件累计入渗记忆不应永久锁死后续入渗容量；
  - 修改文件：`seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.cpp`；
  - 编译：`cmake --build build --target SUR_SGA -j4` 成功，仅有既有 `data_raster.hpp` warning；
  - 已将新 `libSUR_SGA.dylib` 同步到 `build/lib/` 和 `build/install/lib/`。
- 状态约束实验结果：
  - `ACTIVE_DEPTH_MAX=150, INFIL_FACTOR=1.0`：全时段 NSE `-1.0798`、PBIAS `-47.05%`；2 月 10 日峰值从 `48.054` 压到 `3.222 m3/s`，但 2 月 7 日峰值也只有 `3.321 m3/s`；
  - `INFIL_FACTOR=0.7`：结果与 `1.0` 完全一致，说明当前不是入渗速率限制，而是可用水量/容量限制；
  - `ACTIVE_DEPTH_MAX=100`：结果与 `150` 完全一致，说明当前状态约束下活动深度不是有效调参旋钮。
- 当前保留状态：
  - `param.cali` 当前为 `INFIL_FACTOR=1.0`、`ACTIVE_DEPTH_MAX=100`、`GA_ACC_RECOVERY_RATE=0.5`、`GA_ACC_RECOVERY_DELAY=0.5`、`MANNING=3.5`、`GW0=10` 等；
  - 当前输出目录 `data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--` 对应 `SUR_SGA` 状态约束 + `ACTIVE_DEPTH_MAX=100`；
  - 当前图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_accumu_state_clamp_active100.png`。
- 判断与下一步：
  - 新状态约束强烈证明异常峰与 `SUR_SGA` 累计入渗记忆/地表水再入渗链条有关；
  - 但全约束过强，会把快响应压得过低，导致 PBIAS 明显偏负；
  - 下一步不建议继续调 `INFIL_FACTOR` 或 `ACTIVE_DEPTH_MAX`，而应给状态约束强度本身增加可校准系数，例如在“旧的长期累计记忆”和“完全按当前土壤水状态恢复”之间做加权，从而寻找兼顾 2 月 7 日保峰与 2 月 10 日压峰的中间解。

## 2026-06-03 wwj 分支对照后的下一步修改判断

- 用户指出当前状态约束结果图很差，并要求判断修改是否有效、下一步如何参考 wwj 分支修改。
- 执行：
  - 运行 `git fetch wwj validation-2026-05-07` 更新远程分支；
  - 检查 `wwj/validation-2026-05-07` 中 `SUR_SGA`、`IKW_IF`、`IKW_OL` 以及 AndrewsForest storm 配置相关文件；
  - 进一步查看 wwj 关键提交，包括 `c70b26fc`、`514c2533`、`a4ca1524`、`81144516`、`3022b6f0` 等。
- 对当前修改的判断：
  - 当前“状态约束”不是最终可用方案；它把 2 月 10 日异常峰从约 `48 m3/s` 压到约 `3.2 m3/s`，说明定位有效；
  - 但它把所有事件快响应都压得太低，PBIAS 约 `-47%`，因此只能作为诊断，不应作为最终水文方案直接保留。
- wwj 分支可参考点：
  - `SUR_SGA`：wwj 曾尝试按当前多土层孔隙亏缺计算入渗容量，并引入 `INFIL_FACTOR`、`ACTIVE_DEPTH_MAX`、`DEEP_PERC_RATE` 等参数；关键思想是让入渗容量受当前土壤水状态约束，而不是让 `m_accumuDepth` 在长 storm 期永久锁死；
  - `MOIST_IN` 初始化：wwj 最终版本中将 `MOIST_IN` 解释为相对孔隙度（`MOIST_IN * POROSITY`），这会让 `MOIST_IN=0.99` 代表接近饱和，比当前 `MOIST_IN * FIELDCAP` 更湿，更容易产生快响应；
  - `IKW_IF`：wwj 通过 `FAST_RATIO`、`ANISOTROPY`、`MACROPORE_FACTOR` 引入快速壤中流/优先流机制，相关提交记录中有 `NSE=0.46~0.47` 的中间好结果；
  - `IKW_OL`：wwj 添加过 `OL_SPEED_FACTOR`，但此前本地实验已决定不保留该新参数，优先通过已有 `MANNING` 控制地表汇流速度。
- 下一步建议：
  - 不再继续调 `INFIL_FACTOR` 或 `ACTIVE_DEPTH_MAX`，因为本地实验中二者在当前状态约束下完全无效；
  - 第一优先：把当前“全状态夹紧”改为可调强度的状态恢复，例如新增 `GA_STATE_RECOVERY_FACTOR`，`0` 表示旧逻辑、`1` 表示当前全夹紧，先测试 `0.25/0.5/0.75`；
  - 第二优先：参考 wwj，将 `MOIST_IN` 初始化方式作为可配置选项，测试 `MOIST_IN` 是否应相对孔隙度而非田间持水量；
  - 第三优先：在找到合理 SUR_SGA 中间解后，再启用保守的 wwj 快速壤中流参数，例如 `FAST_RATIO=0.1~0.3`、`ANISOTROPY=4~10`、`MACROPORE_FACTOR=1~3`，用壤中流补足基流和峰前上升，而不是把水全部放进 `QS`。

## 2026-06-03 SUR_SGA 状态恢复强度参数实验

- 按上一节计划，将原来的“全状态夹紧”改为可调强度方案，新增 `GA_STATE_RECOVERY_FACTOR`：
  - `0` 表示完全使用旧的 Green-Ampt 事件累计入渗记忆；
  - `1` 表示完全按当前活动层土壤水状态恢复入渗记忆；
  - 中间值用于在“2 月 7 日保峰”和“2 月 10 日压峰”之间寻找折中。
- 修改文件：
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.h`：新增 `m_stateRecoveryFactor`；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/StormGreenAmpt.cpp`：在 `SetValue()` 中读取 `GA_STATE_RECOVERY_FACTOR`，并在 `CalculateActiveInfilCap()` 中按当前土壤水亏缺和事件累计记忆计算有效入渗容量；
  - `seims/src/seims_main/modules/hydrology/SUR_SGA/api.cpp`：增加可选参数元数据；
  - `seims/preprocess/database/model_param_ini.csv`：增加参数库记录；
  - `data/AndrewsForest/andrews_forest_model/storm/param.cali`：当前设为 `GA_STATE_RECOVERY_FACTOR,0.45,VC`，并恢复 `ACTIVE_DEPTH_MAX,150,VC`。
- 实现细节修正：
  - 第一次实现中曾直接修改 `m_accumuDepth[cell]`，导致 `GA_STATE_RECOVERY_FACTOR=0.5` 在多步运行后接近全夹紧效果；
  - 已改为只计算 `effectiveAccumuDepth`，不覆盖 `m_accumuDepth` 原始状态，从而让该参数真正表现为每步入渗容量计算中的可调权重。
- 数据库/编译：
  - 已向 MongoDB `andrews_forest_model.PARAMETERS` upsert `GA_STATE_RECOVERY_FACTOR`；
  - `cmake --build build --target SUR_SGA -j4` 成功，仅有既有 `data_raster.hpp` warning；
  - 新 `libSUR_SGA.dylib` 已同步到 `build/lib/` 和 `build/install/lib/`。
- 试验结果：
  - `GA_STATE_RECOVERY_FACTOR=0.25`：全时段 NSE `-2.3042`、PBIAS `-22.12%`，2 月 10 日峰值 `27.936 m3/s`，仍明显过高；
  - `GA_STATE_RECOVERY_FACTOR=0.50`：全时段 NSE `-0.7814`、PBIAS `-43.71%`，2 月 10 日峰值 `4.551 m3/s`，压峰过强；
  - `GA_STATE_RECOVERY_FACTOR=0.45`：全时段 NSE `-0.5551`、PBIAS `-40.22%`，2 月 10 日峰值 `6.974 m3/s`，是当前三组中整体最好；
  - 但三组对 2 月 7 日事件均改善不足，模拟峰值仍约 `3.321 m3/s`，实测峰值约 `10.449 m3/s`。
- 当前结果：
  - 输出目录：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--`；
  - 当前图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_state_factor0p45_effective.png`；
  - 当前 `param.cali` 关键参数：`MOIST_IN=0.99`、`INFIL_FACTOR=1.0`、`ACTIVE_DEPTH_MAX=150`、`GA_ACC_RECOVERY_RATE=0.5`、`GA_ACC_RECOVERY_DELAY=0.5`、`GA_STATE_RECOVERY_FACTOR=0.45`、`MANNING=3.5`、`GW0=10`、`FAST_RATIO=0`、`ANISOTROPY=1`、`MACROPORE_FACTOR=1`。
- 判断：
  - 新参数有效证明 2 月 9-10 日异常峰可以通过 SUR_SGA 入渗记忆状态约束控制；
  - 当前最好结果仍整体偏低，特别是 2 月 7 日主峰没有起来，说明问题已不只是第二场事件入渗记忆过强；
  - 下一步更应参考 wwj 的 `MOIST_IN * POROSITY` 初始化逻辑，或以保守参数启用快速壤中流机制，补足峰前上升和基流，而不是继续单独调 `ACTIVE_DEPTH_MAX` 或 `INFIL_FACTOR`。

## 2026-06-03 基于 wwj 高 NSE 分支的下一步计划

- 用户指出 wwj 分支已经能模拟出较好结果，要求结合该分支内容判断下一步。
- 已重新检查 `wwj/validation-2026-05-07`：
  - wwj 最佳验证报告 `challenges/BEST_SINGLE_RUN_VERIFIED/evaluation_report.md` 给出 NSE `0.7329`、PBIAS `-4.42%`、峰值误差 `-22.83%`；
  - 最佳参数组合包括 `MOIST_IN=0.58`、`CONDUCTIVITY=15`、`FAST_RATIO=1.00`、`ANISOTROPY=32`、`MACROPORE_FACTOR=21`、`CH_N=0.85`、`MANNING=0.03`、`GW0=0.5`、`KG=0.10`；
  - wwj 的关键代码机制不是单一调参，而是 `SUR_SGA` 中将 `MOIST_IN` 解释为相对孔隙度、全土层孔隙亏缺控制入渗、底层深渗漏，以及 `IKW_IF` 中的快速旁路流/各向异性/大孔隙壤中流。
- 与当前本地状态对比：
  - 本地 `IKW_IF` 已经具备快速壤中流主体机制，但 `param.cali` 中 `FAST_RATIO=0`、`ANISOTROPY=1`、`MACROPORE_FACTOR=1`，相当于关闭了该机制；
  - 本地 `SUR_SGA` 当前仍保留基于田间持水量的初始含水量解释，并新增了 `GA_STATE_RECOVERY_FACTOR` 状态恢复强度参数；
  - 当前最主要问题是 2 月 7 日主峰和峰前上升不足，而 wwj 的高 NSE 方案正是通过更湿初始土壤和快速壤中流补足快响应。
- 下一步计划：
  - 第一步：新增一个可选参数控制 `MOIST_IN` 初始化基准，例如 `MOIST_IN_BASE=FC/POROSITY`，默认保持现有逻辑，实验时切换到 wwj 的 `MOIST_IN * POROSITY`，避免直接破坏现有行为；
  - 第二步：暂时不要继续加大 `GA_STATE_RECOVERY_FACTOR`，先用 wwj 参数思想重跑一组：`MOIST_IN=0.58`、`CONDUCTIVITY=15`、`FAST_RATIO=1.0`、`ANISOTROPY=32`、`MACROPORE_FACTOR=21`、`CH_N=0.85`、`MANNING=0.03`、`GW0=0.5`、`KG=0.1`，确认当前数据/数据库能否复现 wwj 高 NSE 形态；
  - 第三步：做消融实验，按“只切换 MOIST_IN 基准”“只开启 IKW_IF 快速壤中流”“两者同时开启”“加入 CH_N/MANNING 参数”四组比较，判断贡献来自哪个机制；
  - 第四步：若 wwj 全组合可复现，再把当前新增的 `GA_STATE_RECOVERY_FACTOR` 降低或关闭，避免它过度压制 2 月 7 日峰值；
  - 第五步：把有效组合整理成可回溯备份和图表，更新 `progress.md`，必要时再提交 git。

## 2026-06-03 另一个 agent 修改审查

- 用户说明另一个 agent 已做修改，要求先判断是否合理。本次仅审查，不改动源码。
- 主要发现：
  - `seims/run_seims.py` 中把 `param.cali` 第二列同时写入 `VALUE` 与 `IMPACT`，并在缺少第三列时默认 `CHANGE=VC`，方向基本合理；C++ `ParamInfo::GetAdjustedValue()` 对 `VC` 实际读取 `IMPACT` 作为替换值，因此这能修复两列 `param.cali` 参数不生效的问题；
  - 但同类逻辑没有同步到 `seims/preprocess/db_import_model_parameters.py`，后续若使用该独立导入程序，仍可能只有 `IMPACT`、没有默认 `VC`；
  - `config.fig` 将河道模块从 `IKW_CH` 改成 `CH_DW`，这偏离 wwj 最佳报告中明确的模块组合；同时日志出现 `Load parameter: CH_N failed! We will ignore it!`，说明 wwj 的 `CH_N=0.85` 并没有以预期方式参与当前运行；
  - `param.cali` 写成 wwj-like，但遗漏了 wwj 最佳组合中的 `CONDUCTIVITY=15`、`KG=0.10`、`CH_N=0.85`，同时保留了当前诊断参数 `GA_STATE_RECOVERY_FACTOR=0.45`，因此不是干净复现 wwj；
  - `DEP_FS` 将洼地容量硬截断到 `20 mm`，作为防异常值保护可以理解，但不是从 wwj 高 NSE 方案来的机制，建议仅作为数据质量防护保留，不能作为调参依据；
  - `IKW_IF` 快速壤中流机制方向接近 wwj，但当前实现中 `fastInflow` 加入 `m_subSurfQ` 和 `m_qi`，却没有加入 `total_h_vol`，因此 `QI` 输出深度可能低估，不适合直接用 `QI` 做水量分解；
  - 最新 `Q.txt` 中出现第二个 `Subbasin: 2` 块，说明输出目录未清空或同一结果被追加，评估前必须清理输出目录；
  - 最新当前输出窗口 `2015-02-05 07:00:00` 至 `2015-02-08 20:00:00` 的指标很差：NSE `-3.5959`、PBIAS `-99.25%`、模拟峰值仅 `0.491 m3/s`，远低于实测峰值 `10.449 m3/s`；
  - 新运行日志中曾出现 `GreenAmpt` 输出 `inf/nan` 并传播到 `DEP_FS`，说明当前组合存在数值污染风险。
- 审查结论：
  - 可以保留并完善“参数导入修复”；
  - 不建议接受当前 `config.fig` 切换到 `CH_DW`、当前 wwj-like `param.cali`、以及未验证的 `IKW_IF` 输出水量统计作为最终方案；
  - 下一步应恢复到 wwj 报告一致的模块组合 `IKW_CH`，清理输出目录，使用干净的 wwj 参数集做基准复现，然后再逐项消融，而不是将 CH_DW、GA 状态恢复参数和 wwj 参数混合测试。

## 2026-06-03 IKW_CH 与 CH_DW、不同流向数据支持核对

- 用户询问此前修改是否都在 `IKW_CH` 上、当前 `config.fig` 是否使用 `CH_DW`、两个河道模块差异以及 `IKW_CH` 是否支持多流向栅格。
- 核对结论：
  - 当前工作树 `data/AndrewsForest/andrews_forest_model/storm/config.fig` 启用的是 `CH_DW`，`IKW_CH` 被注释；
  - 此前本地主要代码修改集中在 `SUR_SGA`、`IKW_IF`、少量 `DEP_FS`/参数导入逻辑，并没有在 `IKW_CH` 中实现改动；
  - `IKW_CH` 和 `CH_DW` 差异较大：`IKW_CH` 是隐式运动波河道汇流，主要输入 `RadianSlope`、坡面/壤中流/地下水等；`CH_DW` 是扩散波河道汇流，额外使用 `DEM`、`SLOPE`、`CHS0_PERC`，并输出 `QS/QI/QG` 分量；
  - 两个河道模块都读取 `FLOWIN_INDEX`、`FLOWOUT_INDEX` 和 `FLOW_DIR`，运行时可随 `-fdir 0/1/2` 切换到 `_D8/_DINF/_MFDMD` 后缀数据；
  - 但两个河道模块都没有声明或使用 `FLOWIN_FRACTION/FLOWOUT_FRACTION`，因此不能认为它们完整支持 Dinf/MFDmd 的比例分流；
  - `IKW_CH` 中确实遍历 `m_flowOutIdx[curCell][0]` 的多个下游索引来整理河道栅格拓扑，但实际河道流量计算仍按 reach 内一维 cell 序列与上游 reach 汇流推进，没有按多流向比例拆分河道流量；
  - 真正显式使用 `FLOWIN_FRACTION/FLOWOUT_FRACTION` 的是坡面 `IKW_OL`，因此不同流向数据对坡面快流影响更直接，河道模块主要通过不同的 `FLOWIN_INDEX/FLOWOUT_INDEX/ROUTING_LAYERS` 和 reach 拓扑间接受影响。
- 后续建议：
  - 若目标是对比 D8/Dinf/MFDmd，总体可以通过 `-fdir 0/1/2` 运行，因为 AndrewsForest workspace 已存在大量 `_D8/_DINF/_MFDMD` layering 文件；
  - 但对比前应固定河道模块，建议先用 wwj 报告一致的 `IKW_CH` 做基准；
  - 解释结果时需要说明：坡面 `IKW_OL` 使用多流向比例，而 `IKW_CH/CH_DW` 河道模块不按比例分流，河道侧不是完整 MFD 水量分配实现。

## 2026-06-04 AndrewsForest 物理机理化计划执行：可重复运行基础修复

- 用户要求按“接近 wwj 结果但尽量避免无物理含义旋钮”的计划正式执行，并要求有价值修改提交 git、记录 `progress.md`。
- 本阶段先处理运行可重复性，不改变水文机理：
  - 修复 `seims/run_seims.py` 中 `param.cali` 导入逻辑，兼容 `NAME,VALUE,CHANGE`、缺省 `CHANGE`、以及 wwj 使用的 `NAME,CHANGE,VALUE,MAX,MIN` 格式；
  - `VC` 参数同时写入 `VALUE` 与 `IMPACT`，`RC/AC` 只写入 `IMPACT`，避免相对/绝对变化被错误当作默认值覆盖；
  - 同步修复 `seims/preprocess/db_import_model_parameters.py`，防止不同导入入口行为不一致；
  - 修复 `ResetOutputsPeriod()` 后未释放 MongoDB 连接的问题，并让 `ImportModelIOConfiguration()` 写入当前 `TASK`；
  - 增强 `plot_storm_q_pcp.py`，默认拒绝重复时间戳的 `Q.txt`，并输出/标注 NSE、PBIAS、峰值误差和峰现时间误差；
  - 新增 `run_andrews_storm.py`，统一执行“导入配置与参数 → 更新运行/输出时段 → 清理 OUTPUT → 运行模型 → 绘图评价”的流程。
- 自测：
  - `python -m py_compile seims/run_seims.py seims/preprocess/db_import_model_parameters.py plot_storm_q_pcp.py run_andrews_storm.py import_andrews_storm_params.py` 通过；
  - `conda run -n pyseims` 参数解析自测通过：`VC`、`RC`、wwj tab 风格和 header 行均按预期解析。
- 这一步的意义：
  - 后续每次实验都能从干净输出目录开始，避免 `Q.txt` 追加重复块；
  - 参数调整语义可追踪，尤其适合继续做 `MANNING/CH_N/ACTIVE_DEPTH_MAX/ANISOTROPY` 等消融实验。

## 2026-06-04 AndrewsForest wwj 可比结构基线恢复

- 本阶段目标是先恢复与 wwj 高 NSE 报告一致的结构基线，用于判断差异来自代码逻辑、配置还是参数数据库，而不是直接进入无约束调参。
- 配置与接口修改：
  - `storm/config.fig` 当前使用 `SUR_SGA, IKW_OL, IKW_IF, PERCO_DARCY, GW_RSVR, IKW_CH`，保留 `CH_DW` 注释，后续多流向对比再单独处理；
  - `storm/param.cali` 改为 wwj verified-best 结构参数集，并有意不加入 `OL_SPEED_FACTOR`，地表汇流速度仍通过已有 `MANNING` 解释；
  - `storm/file.out` 去掉 `QS/QI`，因为这两个输出属于当前 `CH_DW` 路径，不属于 `IKW_CH` 的输出接口；
  - `run_andrews_storm.py` 增加运行 C++ 主程序所需的 `DYLD_LIBRARY_PATH` 注入，避免提前污染 Python/conda 进程。
- 代码兼容性修复：
  - 修复 `IKW_CH` 元数据，把 `RadianSlope` 与 `QOverland` 改为 2D 模块输入，匹配 `IKW_OL` 实际输出；
  - 修复 `IKW_OL` 的 `RadianSlope` 输出声明，从 1D 改为 2D；
  - 修复 `IKW_CH` 把 `QCH` 错声明为输入的问题，并恢复 `QSUBBASIN` 输出读取；
  - `IKW_CH` 增加内部子步长、读取坡面径流/壤中流/地下水侧向输入，并兼容 `SBQG` 数组长度。
- 编译与运行：
  - `IKW_CH` 与 `IKW_OL` 模块单独编译通过，并已同步到 `build/lib` 与 `build/install/lib`；
  - `cmake --build build --target install` 仍会被无关 TauDEM `streamnet` 源码的 macOS 编译错误阻断，因此本次仅同步已改模块；
  - 使用 `run_andrews_storm.py --figure-name q_pcp_comparison_wwj_structural_baseline.png` 可完成运行和绘图。
- 当前基线结果：
  - 输出图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_wwj_structural_baseline.png`；
  - NSE `-3.6437`，PBIAS `-100.00%`，模拟峰值 `0.000 m3/s`，实测峰值 `10.449 m3/s`；
  - 结果说明：结构已跑通，但水文结果完全不可用。
- 关键诊断：
  - 运行日志中 `SUR_SGA` 出现 `Infiltration=-inf` 与 `Excess_Pcp=inf`，并传播到 `DEP_FS`；
  - `param.cali` 中 `CH_N`、`FC_ADJUST` 当前仍被数据库报告为 unsupported/ignored，说明这些参数还没有按当前数据库参数表生效；
  - 下一步优先修复 `SUR_SGA` 的非有限入渗值保护，并加入可解释的 `MOIST_IN` 初始化基准切换，再重新跑同一基线。

## 2026-06-04 AndrewsForest SUR_SGA 数值保护与 IKW_CH 河道源头修复

- 本阶段继续执行物理机理化计划，目标是先解决 wwj 结构基线下 `Q.txt` 全零和 `SUR_SGA` 非有限值传播问题。
- `SUR_SGA` 修改内容：
  - 去掉默认开启的调试输出，避免运行日志被逐栅格入渗打印淹没；
  - 对降雨、雪融水、洼蓄水、土壤孔隙度、含水量、导水率、毛管吸力、入渗率和入渗容量加入 `finite/nonnegative` 防护，避免 `inf/-inf/nan` 继续传播到 `DEP_FS` 与汇流模块；
  - 新增可选参数 `MOIST_IN_REF`：`0` 表示 `MOIST_IN` 相对田间持水量初始化，`1` 表示相对孔隙度初始化。本次最佳组合采用 `MOIST_IN_REF=0`，因为它更接近原模型 `MOIST_IN * FC` 语义，且水量体积明显更合理；
  - 新增 `GA_ACC_RECOVERY_RATE`、`GA_ACC_RECOVERY_DELAY`、`GA_STATE_RECOVERY_FACTOR` 三个可选接口，为后续按降雨间歇期恢复 Green-Ampt 累计入渗记忆做实验；当前最佳组合保持默认 `0`，避免恢复项过度压制 2 月 7 日峰值；
  - 入渗只填充活动湿润锋深度内的土层孔隙亏缺，并把土壤含水量限制在 `0-porosity` 范围内。
- `IKW_CH` 关键修复：
  - 发现 `m_sourceCellIds` 原来使用未初始化的 `m_idToIndex[reachId]`，导致所有源头河道单元几乎都写到错误索引，`ChannelFlow()` 实际没有执行，最终 `QSUBBASIN/Q.txt` 全零；
  - 修复为按真实 reach id 记录源头单元：`m_sourceCellIds[reachId] = i`；
  - 修复河道 BFS 中用起点 `iCell` 判断 reach 归属的问题，改为用当前弹出的 `curCell` 判断；
  - 对空 reach 单元列表增加保护，避免除以 0 或写入未定义输出。
- 消融诊断结果：
  - 修复前 wwj 结构基线：NSE `-3.6437`，PBIAS `-100.00%`，模拟峰值 `0.000 m3/s`；
  - `MOIST_IN_REF=1` 时径流体积明显偏大，短窗口 PBIAS 超过 `160%`；
  - `MOIST_IN_REF=0, MANNING=0.3, GW0=0.5`：NSE `-4.0973`，PBIAS `15.05%`，峰值 `27.033 m3/s`；
  - `MOIST_IN_REF=0, MANNING=6.0, GW0=0.5`：NSE `0.1188`，PBIAS `-7.72%`，峰值 `14.436 m3/s`；
  - 当前最佳短窗口组合 `MOIST_IN_REF=0, MANNING=8.0, GW0=30`：NSE `0.2984`，PBIAS `-0.36%`，峰值误差 `30.52%`，峰现时间偏晚 `7.75 h`。
- 当前输出图：
  - `data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_commit_sga_ikwch_best.png`
- 结论与问题：
  - 这一步有效解决了 `Q.txt` 全零和水量体积严重偏差问题，模型已经能产生连续非零流量；
  - 但结果还远没有达到 wwj 的较好模拟：峰值仍偏高、峰现偏晚，2 月 7 日前稳定基流仍低于实测；
  - 分量诊断显示当前 `QS` 地表快流占主导，`QG` 有贡献，`QI` 壤中流几乎为 0；下一步应集中检查 `IKW_IF` 的输入维度、土层含水量单位、侧向导水率公式和出流写入逻辑，而不是继续单纯加大 `MANNING`。

## 2026-06-04 AndrewsForest IKW_IF 与 MOIST_IN 导入残留诊断

- 在继续检查 `IKW_IF` 时发现一个数据库可重复性问题：
  - `param.cali` 当前写的是 `MOIST_IN,0.58,VC`；
  - 但 MongoDB `PARAMETERS_SPEC` 中残留了旧的 `_BASE_/SingleRun` 记录 `MOIST_IN=0.99`；
  - 旧记录会干扰当前运行判断，使此前 `MOIST_IN=0.58` 文件实际可能仍带有旧的偏湿初始状态。
- 本阶段修复：
  - `seims/run_seims.py` 与 `seims/preprocess/db_import_model_parameters.py` 在导入 `param.cali` 前，先解析本次文件中的参数名；
  - 除删除当前 `SUB_MODEL/TASK` 的旧记录外，也删除同一 `TASK` 下这些参数名的旧记录，避免 `_BASE_` 残留遮蔽当前子模型；
  - `run_andrews_storm.py` 的可选参数初始化改为修正已有默认参数元数据，而不是只在不存在时插入，避免 `FAST_RATIO/ANISOTROPY/MACROPORE_FACTOR` 这类旧空记录保留错误 `MODULE/TYPE`。
- `IKW_IF` 诊断修改与结论：
  - 增加环境变量触发的轻量诊断：设置 `SEIMS_IKW_IF_DIAG=1` 时输出 `IKW_IF_diag.csv`，正常运行不输出；
  - 参考 wwj 的快速壤中流思想，但把阈值改为“事件新增水”：`IKW_IF` 读取 `Moist_in` 与 `MOIST_IN_REF`，用 `MOIST_IN * Fieldcap/Porosity` 作为事件前水分阈值，优先流和 macropore 只排出高于该阈值的水；
  - 这样避免了 wwj 中 macropore 可排到 `0.2*FC` 的过强假设，也避免从土壤中抽走事件前稳定水；
  - 诊断显示修复后 `IKW_IF` 确实产生了非零壤中流：`all_qi_sum` 最大约 `0.80 m3/s`，但进入河道的 `river_qi_sum` 最大仅约 `0.0065 m3/s`，相对出口峰值仍可忽略；
  - 尝试 `IF_SUBSTEPS=5` 作为数值传播实验，运行明显变慢，`river_qi_sum` 与出口指标几乎不变，因此没有保留在当前 `param.cali`。
- 干净数据库后的当前正式结果：
  - 图件：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_current_clean.png`；
  - NSE `0.2670`，PBIAS `-3.98%`，模拟峰值 `13.576 m3/s`，峰值误差 `29.92%`；
  - 峰现时间仍偏晚 `8.00 h`，说明当前主要问题不是 `IKW_IF` 公式本身，而是壤中流难以有效进入河道/或地表与地下水汇流时序仍不对。
- 下一步建议：
  - 暂不把 `IF_SUBSTEPS` 作为调参方向；
  - 优先检查 `IKW_CH` 对河道侧向输入的空间映射、子流域/河道栅格 ID 对齐，以及是否需要像 wwj 一样在河道侧保留更合理的子步长/侧向入流累积；
  - 同时需要复查 `Fieldcap` 值偏高的土壤数据库来源：当前顶层 `Fieldcap` 平均约 `0.86`、`Porosity` 平均约 `0.92`，这会让“超过 FC 才侧向流”的机制天然很难触发。

## 2026-06-04 AndrewsForest 长窗口基流与河道初始状态诊断

- 用户要求使用 `2015-02-04 06:00:00` 至 `2015-02-16 12:00:00` 长窗口继续实验。
- 首先发现 `run_andrews_storm.py` 虽然会把长窗口写入 MongoDB `FILE_IN`，但 C++ 主程序没有带 `-filein_mongo 1`，实际仍读取本地 `storm/file.in` 的短窗口：
  - 错误输出范围：`2015-02-05 07:00:00` 至 `2015-02-08 19:55:00`；
  - 修复后 C++ 命令包含 `-filein_mongo 1`，真实长窗口输出范围为 `2015-02-04 06:00:00` 至 `2015-02-16 11:55:00`，共 `3528` 个 5 分钟步长。
- 真实长窗口基线（`GW0=30, MANNING=8.0, CH_N=0.85`，输出备份 `storm/OUTPUT_D8_DOWNUP--.long_baseline_20260604_174350`）：
  - 完整期 NSE `-0.4105`，PBIAS `-25.75%`，模拟峰值 `13.493 m3/s`，实测峰值 `10.449 m3/s`，峰值误差 `29.13%`，峰现偏晚 `8.0 h`；
  - 起始出口流量几乎为 `0`，实测约 `2.690 m3/s`；
  - `SBQG` 起始约 `0.4208 m3/s`、末期约 `0.1237 m3/s`，明显不足以维持实测约 `2-3 m3/s` 的稳定基流；
  - `IKW_IF_diag.csv` 显示 `river_qi_sum` 最大仅约 `0.0065 m3/s`，说明当前壤中流进入河道的量级仍很小。
- 负向实验记录：
  - `IKW_IF` 强化“已连通快壤中流”实验让 `river_qi_sum` 从约 `0.0065` 增至约 `0.0366 m3/s`，但完整期 NSE 仅从 `-0.4105` 到 `-0.4069`，峰值还略高，因此不保留；
  - 强行把 `MANNING=0.08, CH_N=0.25` 的糙率实验会导致峰值爆到 `35.814 m3/s`，完整期 NSE `-7.2195`，说明不能用这种方向削峰；
  - 单独增大地下水初始库容：`GW0=180` 可把 `SBQG` 起始抬到约 `2.525 m3/s`，但完整期 PBIAS `+37.74%`、峰值误差 `65.17%`；`GW0=90` 完整期 PBIAS 接近 `-0.42%`，但旧窗口 NSE 降到 `0.106`、峰值误差 `43.6%`，说明只靠增大地下水库容会把风暴峰一起抬高。
- 本阶段保留的代码修复：
  - `IKW_CH` 在第一次执行时，如果 `SBQG` 已存在，则用当前地下水基流初始化河道 `q/h`，避免 storm 模式以“干河道”起算；
  - 初始化时按上游 reach 出流和本 reach 地下水基流沿河道长度线性累积，使用已有 Manning/坡度关系估算水深，不新增纯幅度参数。
- 修复效果：
  - 仅加入河道初始基流、仍用 `GW0=30`：起始出口流量由几乎 `0` 提高到 `2.136 m3/s`，完整期 NSE 从 `-0.4105` 提高到 `-0.3432`，PBIAS 从 `-25.75%` 到 `-23.99%`；
  - 但后续基流仍衰减过快，2 月 7 日前 PBIAS 仍为 `-55.73%`，说明还需要适度提高地下水初始库容。
- 当前临时长窗口基线：
  - 采用 `GW0=50` 配合 `IKW_CH` 初始基流修复；
  - 图件：`storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_long_ch_init_gw0_50.png`；
  - 完整期 NSE `-0.2059`，PBIAS `-14.91%`，峰值误差 `33.94%`，峰现偏晚 `7.92 h`；
  - 旧窗口 NSE `0.365`、PBIAS `9.07%`，略好于原长窗口基线对应的旧窗口 NSE `0.355`；
  - 2 月 9-10 日峰值仍偏高：模拟峰值 `13.428 m3/s`，实测 `8.382 m3/s`，峰现偏晚约 `9.17 h`。
- 判断：
  - `IKW_CH` 初始基流修复符合物理机理，解决了 storm 模式有地下水输入却以干河道起算的问题，建议保留；
  - `GW0=50` 是当前长窗口下更合理的临时参数值，能把完整期 PBIAS 拉到目标范围内，但还不是最终校准结果；
  - 下一步不应继续增大 `GW0`，应转向削减 2 月 9-10 日地表快流峰值和改善峰现时序，优先检查 `SUR_SGA/DEP_FS/IKW_OL/IKW_CH` 的快流分配和河道传播，而不是引入 `OL_SPEED_FACTOR` 这类纯速度旋钮。

## 2026-06-04 AndrewsForest IKW_CH 河道源项分解诊断

- 为判断 2 月 9-10 日异常峰值来源，给 `IKW_CH` 增加可选诊断开关 `SEIMS_IKW_CH_DIAG=1`：
  - 默认关闭，不影响正常运行；
  - 打开时在输出目录写 `IKW_CH_diag.csv`；
  - 每个时间步记录河道侧向输入的降雨、地表径流 `QS`、壤中流 `QI`、地下水 `QG` 的体积与等效平均流量；
  - 诊断按子步长体积累加再除以原始时间步，避免子步重复计数。
- 使用当前临时长窗口基线 `GW0=50` 重新运行，水文指标与上一轮一致，说明诊断没有扰动计算：
  - NSE `-0.2059`，PBIAS `-14.91%`，峰值误差 `33.94%`，峰现偏晚 `7.92 h`。
- 河道侧向输入分解：
  - 完整期：`QS` 占 `73.79%`，`QG` 占 `25.74%`，`QI` 仅 `0.028%`，河道面降雨约 `0.44%`；
  - 2 月 7 日事件：`QS` 占 `86.77%`，`QG` 占 `12.83%`，`QI` 仅 `0.015%`；
  - 2 月 9-10 日事件：`QS` 占 `86.40%`，`QG` 占 `13.01%`，`QI` 仅 `0.016%`。
- 峰值时刻源项：
  - 2 月 7 日模拟峰值出现在 `2015-02-07 22:55:00`，`Q=13.995 m3/s`，当时河道侧向输入约 `13.469 m3/s`，其中 `QS=12.237 m3/s`、`QG=1.230 m3/s`、`QI=0.00145 m3/s`；
  - 2 月 9-10 日模拟峰值出现在 `2015-02-10 13:55:00`，`Q=13.428 m3/s`，当时河道侧向输入约 `12.788 m3/s`，其中 `QS=11.841 m3/s`、`QG=0.946 m3/s`、`QI=0.00123 m3/s`。
- 结论：
  - 当前峰值偏高主要由 `IKW_OL` 传入河道的 `QS` 地表快流控制；
  - `QI` 几乎不参与出口洪峰，说明即使前面加入了保守版优先流/各向异性机制，壤中流仍没有成为有效的缓释通道；
  - 下一步若要同时压峰并保持水量，应优先测试更物理的“减少地表超渗并转入土壤/缓释路径”的方案，例如 `ACTIVE_DEPTH_MAX`、`SUR_SGA` 入渗容量恢复/湿润锋状态，以及 `IKW_IF` 从非河道坡面向河道连通的传播效率；单纯继续调 `GW0` 或河道糙率不会解决 `QS` 主导的问题。

## 2026-06-04 AndrewsForest ACTIVE_DEPTH_MAX=200 负向实验

- 基于 `IKW_CH_diag.csv` 已确认洪峰由 `QS` 主导后，测试 `ACTIVE_DEPTH_MAX=200`，意图是扩大 Green-Ampt 活动湿润锋深度、减少超渗地表快流。
- 结果显示该方向过强：
  - 完整期 NSE `-1.7467`，PBIAS `-57.60%`；
  - 模拟峰值仅 `7.394 m3/s`，相对 `10.449 m3/s` 实测峰值偏低 `-29.23%`；
  - 2 月 7 日主峰几乎消失，该窗口 PBIAS `-82.01%`；
  - 2 月 9-10 日峰值虽然降到 `7.394 m3/s`，但整体水量仍偏低，峰现仍偏晚约 `12.83 h`。
- 源项诊断：
  - 完整期 `QS` 占比从当前基线的 `73.79%` 降到 `45.85%`，`QG` 占比升到 `52.65%`；
  - 2 月 7 日事件 `QS` 几乎被完全消除，说明 `ACTIVE_DEPTH_MAX=200` 不是轻微削峰，而是过度吸收入渗。
- 决策：
  - 不保留 `ACTIVE_DEPTH_MAX=200`；
  - 已恢复 `param.cali` 为当前基线 `GW0=50`，并重新运行长窗口；
  - 当前主输出目录 `storm/OUTPUT_D8_DOWNUP--` 已恢复为 `GW0=50` 基线，图件为 `q_pcp_comparison_long_current_gw0_50.png`，指标为 NSE `-0.2059`、PBIAS `-14.91%`、峰值误差 `33.94%`。

## 2026-06-04 参考 wwj 分支差异后的下一步判断

- 重新比较当前分支与 `wwj/validation-2026-05-07` 的水文模块差异，重点查看 `SUR_SGA`、`IKW_IF`、`IKW_OL`、`PERCO_DARCY`、`GW_RSVR`、`IKW_CH` 与 `DEP_FS`。
- 已经吸收或替代的内容：
  - 当前已使用 `IKW_CH` 而不是 `CH_DW`，并修复了当前分支中河道源头和初始基流问题；
  - 当前 `IKW_IF` 已保留 wwj 的“各向异性/优先流”思想，但采用更保守的事件水阈值，避免像 wwj 那样排到 `0.2*FC`；
  - 当前没有保留 `OL_SPEED_FACTOR`，仍坚持通过物理糙率 `MANNING/CH_N` 与水量分配解释。
- wwj 中不建议直接照抄的内容：
  - `OL_SPEED_FACTOR` 属于纯速度旋钮，缺少明确物理含义；
  - `IKW_IF` 中 `MACROPORE_FACTOR` 可把土壤水排到 `0.2*FC`，对 H.J. Andrews 的优先流现象有启发，但过强，容易成为无约束调参；
  - `SUR_SGA` 使用全土层孔隙亏缺作为入渗容量，当前测试 `ACTIVE_DEPTH_MAX=200` 已证明类似方向会过度削弱 2 月 7 日主峰；
  - `IKW_CH` 中按 `m_qg[i+1]` 取地下水输入更像索引权宜修复，当前按真实 `reachIndex` 映射更稳。
- wwj 中仍值得吸收的物理/数值修复：
  - `GW_RSVR` 的 `GWMAX` 溢出转为基流，属于水量守恒修复，可去掉 debug 后吸收；
  - `GW_RSVR::Get1DData()` 设置输出长度，属于接口稳健性修复；
  - `PERCO_DARCY` 的析构条件修复和 `FC_ADJUST` 接口值得吸收；当前 `param.cali` 写了 `FC_ADJUST=1.0`，但当前代码并未真正使用；
  - `PERCO_DARCY` 可作为“已入渗水如何进入地下水”的物理调节点，比继续调 `GW0` 更适合解释后期基流不足；
  - `IKW_IF` 需要一个介于当前保守阈值和 wwj 极强 macropore 之间的机制，让部分事件水从 `QS` 转到 `QI`，目标是把事件期 `QI` 从约 `0.016%` 提高到可见但不过强的比例。
- 当前诊断对下一步的约束：
  - 2 月 7 与 2 月 9-10 事件中，河道侧向输入约 `86%` 来自 `QS`；
  - 因此下一步不能主要靠 `GW0` 或 `CH_N/MANNING`，而应减少 `QS` 过强，同时避免像 `ACTIVE_DEPTH_MAX=200` 那样让水量消失；
  - 最优先的方向是“温和增加入渗/渗漏，并把一部分事件水转为可到达河道的 `QI`”，每一步都用 `IKW_CH_diag.csv` 检查 `QS/QI/QG` 分量是否按预期变化。
- 建议执行顺序：
  1. 先吸收安全 bugfix：`GW_RSVR` 的 `GWMAX` 守恒与输出长度、`PERCO_DARCY` 的析构修复和 `FC_ADJUST` 可选参数接口；
  2. 在当前长窗口基线下测试 `FC_ADJUST=0.95/0.90`，看能否增加 `QG`/尾水而不抬高峰值；
  3. 测试比 `ACTIVE_DEPTH_MAX=200` 更温和的活动深度或动态湿润锋方案，例如 `160/175 mm` 或随累计入渗逐步扩展，而不是一次使用全剖面；
  4. 放松 `IKW_IF` 的优先流启动阈值，但不采用 wwj 的 `0.2*FC` 下限；目标是事件期 `QI` 进入河道占比达到若干百分点，观察是否能降低 `QS` 峰并让退水更缓；
  5. 若上述水量分配已经合理，再小幅调整 `MANNING/CH_N` 做时序和峰宽校正。

## 2026-06-04 AndrewsForest PERCO_DARCY 与 GW_RSVR 安全修复实验

- 按上一阶段计划，先吸收 wwj 分支中属于物理边界/接口稳健性的修复，而不引入 `OL_SPEED_FACTOR` 或更强的纯幅度旋钮。
- 本阶段代码修改：
  - `GW_RSVR` 初始化 `m_storageMax=-1`，避免未初始化的 `GWMAX` 上限参与判断；
  - 当地下水库容超过 `GWMAX` 时，将超出部分转成当步基流出流，保持水量守恒；
  - `GW_RSVR::Get1DData()` 对 `SBQG/SBGS` 显式返回数组长度，增强模块接口稳定性；
  - 关闭 `GW_RSVR/PERCO_DARCY` 中遗留的逐步 TRACE 调试输出，避免长窗口日志被无关诊断污染；
  - 修复 `PERCO_DARCY` 析构条件，避免 `m_recharge` 已分配时不释放；
  - `PERCO_DARCY` 新增可选参数 `FC_ADJUST`，其物理含义是渗漏启动阈值的田间持水量调节因子：`effective_fc = clamp(FieldCapacity * FC_ADJUST, 0, Porosity)`；
  - `PERCO_DARCY` 对孔隙度、土层厚度和 pore index 增加基础数值保护，避免除零、负阈值或高于孔隙度的渗漏阈值导致负渗漏。
- 为保证 `FC_ADJUST` 真正写入数据库，`run_andrews_storm.py` 的可选参数初始化列表加入 `FC_ADJUST`，默认值仍为 `1.0`。
- 编译与同步：
  - 已执行 `cmake --build build --target GW_RSVR PERCO_DARCY -j4`；
  - 已将 `libGW_RSVR.dylib` 与 `libPERCO_DARCY.dylib` 同步到 `build/lib/` 与 `build/install/lib/`；
  - 编译只出现既有 SDK/override 警告，没有新增编译错误。
- 长窗口基线恢复：
  - 时间：`2015-02-04 06:00:00` 至 `2015-02-16 12:00:00`；
  - 当前 `param.cali` 保持 `FC_ADJUST=1.0`；
  - 当前输出图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_long_safefix_fc1_restored.png`；
  - 指标：NSE `-0.2059`，PBIAS `-14.91%`，模拟峰值 `13.995 m3/s`，实测峰值 `10.449 m3/s`，峰值误差 `33.94%`，峰现偏晚 `7.92 h`；
  - 与上一阶段 `GW0=50` 基线一致，说明安全修复没有扰动当前结果。
- `FC_ADJUST` 控制变量实验：
  - `FC_ADJUST=0.95`：NSE `-0.2060`，PBIAS `-14.92%`，峰值误差 `33.93%`，峰现偏晚 `7.92 h`；
  - `FC_ADJUST=0.90`：NSE `-0.2060`，PBIAS `-14.92%`，峰值误差 `33.93%`，峰现偏晚 `7.92 h`；
  - 相对 `FC_ADJUST=1.0`，`Q.txt` 最大差异仅约 `0.0017 m3/s`，基本不可见；
  - 河道源项占比也没有朝预期改善：完整期 `QS` 仍约 `74.1%`，`QG` 约 `25.9%`，`QI` 反而从约 `0.028%` 降到 `0.008-0.013%`。
- 结论：
  - `GW_RSVR/PERCO_DARCY` 的安全修复建议保留并提交，因为它们提升了水量守恒和接口稳定性；
  - `FC_ADJUST` 可作为以后解释深层渗漏阈值的参数接口保留，但本轮 `0.95/0.90` 对 Andrews 长窗口几乎无效，不作为当前调参方向；
  - 当前问题仍是 `QS` 地表快流主导洪峰，而 `QI` 没有成为有效缓释路径。下一步应进入建议顺序的第 3/4 步：测试更温和的活动湿润锋深度或动态湿润锋方案，并进一步放松 `IKW_IF` 的事件水连通机制，而不是继续调 `FC_ADJUST`。

## 2026-06-04 AndrewsForest 活动深度复核与 IKW_IF 连通性修复

- 执行上一阶段第 3/4 步时，先复核了当前 MongoDB 参数状态：
  - `PARAMETERS` 中 `ACTIVE_DEPTH_MAX` 当前为 `0`，而不是早期记录中的 `150`；
  - 当前 `SOILDEPTH` 为 9 层累积深度，最后一层为 `1400 mm`，因此 `ACTIVE_DEPTH_MAX=0` 在当前 C++ 逻辑下表示使用全剖面活动深度；
  - 这说明 `160/175/500 mm` 并不是围绕 `150 mm` 的小扰动，而是相对当前全剖面 `1400 mm` 的强约束。
- 固定活动深度实验：
  - `ACTIVE_DEPTH_MAX=160`：完整期 NSE `-1.5755`，PBIAS `-47.43%`，峰值误差 `18.33%`，峰现错误 `72.50 h`；
  - 源项显示 2 月 7 日事件 `QS` 从基线约 `87.46%` 降到约 `6.95%`，主峰被过度吸收入渗/地下水路径，不能保留；
  - `ACTIVE_DEPTH_MAX=500`：完整期 NSE `-3.1398`，PBIAS `-76.18%`，模拟最大流量仅 `3.558 m3/s`，`QS` 几乎为 0，也不能保留；
  - 结论：在当前代码状态下，固定活动深度不是温和削峰手段，继续测试 `175 mm` 没有意义。
- `IKW_IF` 连通性修复：
  - 保留已有 `FAST_RATIO/ANISOTROPY/MACROPORE_FACTOR` 参数，不新增纯幅度旋钮；
  - 将优先流/快流路径的连通性尺度从 `initial -> porosity` 改为 `initial -> field capacity`；
  - 物理解释：H.J. Andrews 这类森林陡坡浅层土壤中，优先流和土壤-基岩界面侧向连通通常在接近田间持水量后就可启动，不需要等到接近完全孔隙度；同时仍用事件前初始水分作为可排水下限，避免 wwj 分支中排到 `0.2*FC` 的过强假设。
- 长窗口实验结果：
  - 图件：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_long_ikw_if_fc_connectivity.png`；
  - 指标：NSE 从 `-0.2059` 小幅改善到 `-0.2043`，PBIAS 从 `-14.91%` 到 `-14.77%`；
  - 峰值略变差：峰值误差从 `33.94%` 到 `34.04%`，峰现仍偏晚 `7.92 h`；
  - `river_qi_sum` 最大值从 `0.00651` 增至 `0.03897 m3/s`，`QI` 完整期占比从 `0.0281%` 增至 `0.2029%`；
  - 2 月 7 日事件 `QI` 占比从 `0.0142%` 增至 `0.1119%`，2 月 9-10 日从 `0.0171%` 增至 `0.1187%`。
- 结论：
  - 该 `IKW_IF` 修改方向符合物理解释，并且确实让壤中流更容易连通到河道，可作为后续工作的机制基础保留；
  - 但它仍没有解决主问题：事件期 `QS` 仍约 `86-87%`，峰值仍由地表快流主导；
  - 下一步应检查 `SUR_SGA -> DEP_FS -> IKW_OL` 的地表快流水量链路，尤其是森林坡面上 `EXCP` 是否应有一部分作为近地表滞蓄/侧向慢释，而不是全部立即进入 `QS` 快流；这一步需要避开正在被另一 agent 修改的 `DEP_FS`，或先审查该改动后再继续。

## 2026-06-05 AndrewsForest SUR_SGA/DEP_FS/IKW_OL 水量闭合诊断

- 针对用户提出的原始设计逻辑，重新核对 `SUR_SGA -> DEP_FS -> IKW_OL` 三段状态变量语义：
  - `DPST/m_sd` 是静态洼地蓄水，当前已经在 `SUR_SGA` 中作为旧水量加入 `hWater = NEPR + DPST_old + snowmelt`；
  - 因此 `DEP_FS` 中使用 `totalWater = EXCP` 是合理的，不能简单改成 `m_sd + m_exsPcp`，否则旧洼地水会重复计入；
  - `SURU/m_sr` 是坡面运动波水深，不适合直接送回 `DEP_FS` 再填洼；物理上它应有继续下渗机会，但更合理的位置是在 `IKW_OL` 中按剩余入渗能力和坡面滞留体积处理。
- 新增环境变量控制的诊断输出，默认不开启、不改变模拟结果：
  - `SUR_SGA_balance.csv`：记录逐步 `hWater = INFIL + EXCP` 闭合，并记录旧 `DPST`、旧 `SURU`、潜在入渗和剩余入渗能力；
  - `DEP_FS_balance.csv`：记录逐步 `EXCP = DPST_new + runoff_added` 闭合；
  - `IKW_OL_balance.csv`：记录逐步坡面运动波体积闭合，即初始地表水体积、上游入流、坡面出流、二次下渗和末态地表水体积。
- 编译与同步：
  - 执行 `cmake --build build --target SUR_SGA DEP_FS IKW_OL -j4` 通过；
  - 将新编译的 `libSUR_SGA.dylib`、`libDEP_FS.dylib`、`libIKW_OL.dylib` 从 `build/seims/bin/seims_project/` 同步到 `build/lib/` 与 `build/install/lib/`；
  - 编译仅出现既有 macOS SDK 常量转换警告。
- 长窗口诊断运行：
  - 时间：`2015-02-04 06:00:00` 至 `2015-02-16 12:00:00`；
  - 命令通过 `SEIMS_WB_DIAG=1 SEIMS_IKW_CH_DIAG=1 SEIMS_IKW_IF_DIAG=1 python run_andrews_storm.py ...` 执行；
  - 输出图：`data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_long_water_balance_diag.png`；
  - 诊断文档：`data/AndrewsForest/andrews_forest_model/storm/water_balance_closure_diagnosis.md`。
- 运行指标与上一轮一致：
  - NSE `-0.2043`；
  - PBIAS `-14.77%`；
  - 峰值误差 `34.04%`，模拟峰值 `14.005 m3/s`，实测峰值 `10.449 m3/s`；
  - 峰现偏晚 `7.92 h`。
- 闭合结果：
  - `SUR_SGA` 最大闭合误差为 `0 mm-cell`；
  - `DEP_FS` 最大闭合误差为 `7.94e-06 mm-cell`，累计绝对误差 `6.39e-04 mm-cell`，仅为浮点量级；
  - `IKW_OL` 最大闭合误差约 `0.970 m3`，累计绝对误差约 `94.22 m3`，相对于坡面内部通量可忽略；
  - 说明当前问题不是显式漏水/造水导致，而是水量路径分配导致。
- 关键水量结果：
  - 全期累计净雨 `105.43 mm`；
  - `SUR_SGA` 入渗仅 `2.32 mm`；
  - `DEP_FS` 新增地表水 `runoff_added` 为 `102.54 mm`；
  - `IKW_OL` 二次下渗仅约 `0.0005 mm`；
  - 末态 `DPST` 约 `0.704 mm`，末态 `SURU` 约 `1.316 mm`；
  - 河道侧向来源中 `QS=50.61 mm`、`QI=0.139 mm`、`QG=17.66 mm`，出口模拟总水量约 `69.99 mm`。
- 事件窗口诊断：
  - 2 月 7 日事件净雨 `44.32 mm`，`SUR_SGA` 入渗 `0 mm`，`DEP_FS` 新增地表水 `44.32 mm`，`IKW_OL` 二次下渗 `0 mm`，河道侧向来源约 `QS/QI/QG = 84.17%/0.14%/15.21%`；
  - 2 月 9-10 日事件净雨 `44.17 mm`，`SUR_SGA` 入渗 `0 mm`，`DEP_FS` 新增地表水 `44.17 mm`，`IKW_OL` 二次下渗 `0 mm`，河道侧向来源约 `QS/QI/QG = 86.32%/0.12%/12.99%`。
- 结论：
  - 当前 `SUR_SGA -> DEP_FS` 闭合逻辑成立；
  - 残留 `SURU` 物理上应有继续下渗机会，当前代码在 `IKW_OL` 中有二次下渗接口，但实际几乎不起作用；
  - 峰值偏高和 `QS` 主导不是由模块水量不闭合造成，而是事件水几乎全部从 `DEP_FS` 进入地表快流，缺少有效的近地表滞蓄/壤中流转化。
- 下一步建议：
  - 先不要把 `m_surfRf` 直接加回 `SUR_SGA::hWater`，避免把运动水重新送入填洼逻辑；
  - 扩展 `IKW_OL` 诊断，明确 `DEP_FS` 新增 `SURU` 中有多少真正进入河道 `QS`、多少仍在坡面、多少发生二次下渗；
  - 检查 `SUR_SGA` 事件期入渗为 0 的直接原因，是 `theta >= porosity`、活动层入渗容量为 0，还是 Green-Ampt 累计入渗记忆过强；
  - 若确认 `SURU` 长时间停留但二次下渗关闭，应优先改造 `IKW_OL` 的二次下渗/近地表滞蓄机制，再调 `MANNING/CH_N`。
