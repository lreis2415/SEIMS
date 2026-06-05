# AndrewsForest storm 水量闭合诊断

## 诊断目的

本次诊断回答两个问题：

1. 上一时刻残留的地表水 `SURU/m_sr` 是否应按物理逻辑继续参与下渗；
2. 当前 `SUR_SGA -> DEP_FS -> IKW_OL` 三段计算是否逐时间步满足水量闭合。

诊断运行窗口为 `2015-02-04 06:00:00` 至 `2015-02-16 12:00:00`。运行时启用：

```bash
SEIMS_WB_DIAG=1
SEIMS_IKW_CH_DIAG=1
SEIMS_IKW_IF_DIAG=1
```

输出图件：

```text
data/AndrewsForest/andrews_forest_model/storm/OUTPUT_D8_DOWNUP--/q_pcp_comparison_long_water_balance_diag.png
```

主要诊断文件：

```text
SUR_SGA_balance.csv
DEP_FS_balance.csv
IKW_OL_balance.csv
IKW_CH_diag.csv
IKW_IF_diag.csv
```

## 残留 SURU 是否应参与下渗

物理上，上一时刻残留在坡面的地表薄水层如果仍与土壤表面接触，就应当有继续下渗的机会。但在当前 SEIMS 的算子分裂结构里，需要区分两个状态：

- `DPST/m_sd`：静态洼地蓄水，代表还停留在局部洼地中的水；
- `SURU/m_sr`：运动波模块中的坡面地表水，代表已经进入坡面汇流路径的水膜/水深。

当前代码已经把上一时刻 `DPST` 加入 `SUR_SGA` 的可下渗水量：

```text
hWater = NEPR + DPST_old + snowmelt
```

因此 `DEP_FS` 中继续使用：

```text
totalWater = EXCP
```

是合理的。这里不能再写成 `DPST_old + EXCP`，否则旧洼地水会被重复计算。

对于上一时刻残留的 `SURU`，当前代码没有把它重新加入 `SUR_SGA` 的 `hWater`，而是让它在 `IKW_OL` 中通过 `INFILCAPSURPLUS` 做二次下渗。这种结构是合理的：如果 `SURU` 表示正在坡面运动的水，最好在运动波路由中按滞留体积和剩余入渗能力处理，而不是重新送回 `DEP_FS` 再填洼。

但当前结果显示，`IKW_OL` 的二次下渗几乎没有发挥作用：全期二次下渗仅约 `0.0005 mm`，而 `SURU` 在 3132 个时间步中为正，最大流域平均残留地表水深约 `43.68 mm`。所以问题不是“残留 SURU 完全不该下渗”，而是当前二次下渗机制太弱，未能成为有效的近地表滞蓄/再入渗路径。

## 闭合方程

本次逐步检查的方程为：

```text
SUR_SGA:
hWater = INFIL + EXCP

DEP_FS:
EXCP = DPST_new + SURU_added

IKW_OL:
SURU_initial_volume + upstream_inflow
= overland_outflow + reinfiltration + SURU_final_volume
```

注意：`SUR_SGA` 的 `EXCP` 不是独立外源降雨量。它包含每一步的旧 `DPST` 状态水，因此长时段累计 `EXCP` 不能直接与累计降雨比较。要看新进入地表快流的量，应使用 `DEP_FS` 的 `runoff_added`。

## 闭合结果

| 模块 | 最大闭合误差 | 累计绝对误差 | 判断 |
| --- | ---: | ---: | --- |
| `SUR_SGA` | `0 mm-cell` | `0 mm-cell` | 完全闭合 |
| `DEP_FS` | `7.94e-06 mm-cell` | `6.39e-04 mm-cell` | 仅浮点误差 |
| `IKW_OL` | `0.970 m3` | `94.22 m3` | 相对于坡面内部通量可忽略 |

全期流域平均量级：

| 项目 | 数值 |
| --- | ---: |
| 累计净雨 `NEPR` | `105.43 mm` |
| `SUR_SGA` 入渗 | `2.32 mm` |
| `DEP_FS` 新增地表水 `runoff_added` | `102.54 mm` |
| 末态 `DPST` | `0.704 mm` |
| 末态 `SURU` | `1.316 mm` |
| `IKW_OL` 二次下渗 | `0.0005 mm` |
| 河道侧向 `QS` 输入 | `50.61 mm` |
| 河道侧向 `QI` 输入 | `0.139 mm` |
| 地下水 `QG` 输入 | `17.66 mm` |
| 出口模拟总水量 `Q` | `69.99 mm` |

`DEP_FS` 的外源水量闭合也成立：

```text
NEPR + initial DPST - INFIL - runoff_added - final DPST ~= 0
```

代入当前结果：

```text
105.431 + 0.141 - 2.324 - 102.544 - 0.704 ~= 0
```

## 事件窗口结果

### 2 月 7 日事件

窗口：`2015-02-06 12:00:00` 至 `2015-02-08 18:00:00`

| 项目 | 数值 |
| --- | ---: |
| 净雨 | `44.32 mm` |
| `SUR_SGA` 入渗 | `0.00 mm` |
| `DEP_FS` 新增地表水 | `44.32 mm` |
| `IKW_OL` 二次下渗 | `0.00 mm` |
| 河道侧向来源：`QS/QI/QG` | `84.17% / 0.14% / 15.21%` |

### 2 月 9-10 日事件

窗口：`2015-02-09 00:00:00` 至 `2015-02-11 00:00:00`

| 项目 | 数值 |
| --- | ---: |
| 净雨 | `44.17 mm` |
| `SUR_SGA` 入渗 | `0.00 mm` |
| `DEP_FS` 新增地表水 | `44.17 mm` |
| `IKW_OL` 二次下渗 | `0.00 mm` |
| 河道侧向来源：`QS/QI/QG` | `86.32% / 0.12% / 12.99%` |

## 结论

1. 当前 `SUR_SGA -> DEP_FS` 的主水量闭合是正确的，`DEP_FS` 不应简单改成 `m_sd + m_exsPcp`。
2. 当前事件雨量几乎全部通过 `DEP_FS` 转为新增地表水，事件期 `SUR_SGA` 入渗为 `0`，`IKW_OL` 二次下渗也几乎为 `0`。
3. 峰值偏高、`QS` 主导的问题不是由这三个模块的显式水量不闭合造成，而是由水量路径分配造成：新降雨基本进入 `QS`，没有足够进入浅层壤中流或近地表滞蓄。
4. 残留 `SURU` 从物理上应有继续下渗机会；当前代码在 `IKW_OL` 中提供了这个机会，但实际诊断显示该路径几乎关闭。

## 下一步建议

1. 不要把 `m_surfRf` 直接加回 `SUR_SGA` 的 `hWater` 作为第一方案，因为这会把运动水重新送入填洼逻辑，可能造成状态语义混乱。
2. 先扩展 `IKW_OL` 诊断，明确 `DEP_FS` 新增 `SURU` 中有多少真正进入河道 `QS`、多少仍在坡面、多少发生二次下渗。
3. 检查 `SUR_SGA` 事件期入渗为 0 的直接原因：是 `theta >= porosity`、活动层入渗容量为 0，还是 Green-Ampt 累计入渗记忆导致潜在入渗受限。
4. 如果确认 `SURU` 长时间停留在坡面且二次下渗为 0，应优先改造 `IKW_OL` 的二次下渗/近地表滞蓄机制，而不是继续加大 `MANNING` 或直接照搬纯幅度参数。

## 追查结果：SUR_SGA 入渗关停原因

本次扩展了 `SUR_SGA_balance.csv`，新增了入渗容量和关停原因分类：

- `raw_potential_infil_mm_cell`：Green-Ampt 原始潜在入渗；
- `active_depth_mm_cell`：当前活动湿润锋深度；
- `infil_cap_mm_cell`：活动层可用入渗库容；
- `saturated_cells`：`theta >= porosity` 导致不入渗；
- `cap_zero_cells`：活动层库容为 0 导致不入渗；
- `potential_zero_cells`：Green-Ampt 原始潜在入渗为 0；
- `positive_infil_cells`：实际发生入渗的湿像元数。

诊断结论很明确：原长窗口事件期入渗被关到 0，不是 `theta >= porosity`，也不是 Green-Ampt 原始潜在入渗为 0，而是 `cap_zero_cells` 主导。直接原因是 `ACTIVE_DEPTH_MAX` 在数据库中为 `VALUE=0`，但 `MIN=10`，`DataCenter::GetAdjustedValue()` 会把 0 裁成 10，使 `SUR_SGA` 实际只使用 10 mm 活动层。这个 10 mm 活动层在初始洼地水和早期降雨后很快被 `m_accumuDepth` 填满，后续 `remainingEventStorage = eventStorage - m_accumuDepth` 变为 0，导致事件期 `infilCap=0`。

三组对照结果如下：

| 情景 | `ACTIVE_DEPTH_MAX` 实际含义 | NSE | PBIAS | 主结果 |
| --- | --- | ---: | ---: | --- |
| 锁死 10 mm | `VALUE=0` 被 `MIN=10` 裁剪 | `-0.2043` | `-14.77%` | 全期入渗仅 `2.32 mm`，`cap_zero_cells` 占湿像元步 `99.55%`，`DEP_FS` 新增地表水 `102.54 mm` |
| 自动全剖面 | 修正 `MIN=0` 后 `VALUE=0` 使用约 `1500 mm` 剖面 | `-2.6720` | `-71.01%` | 入渗 `105.57 mm`，`EXCP=0`，`QS=0`，洪峰消失 |
| 显式 150 mm | 当前数据库已设为 `VALUE=150, CHANGE=RC, IMPACT=1` | `-1.3095` | `-42.23%` | 入渗 `51.69 mm`，`DEP_FS` 新增地表水 `53.18 mm`，但 2 月 9-10 日仍几乎关停入渗 |

显式 150 mm 情景进一步说明：活动层不能固定得过浅，也不能直接用全剖面。2 月 7 日窗口中 `cap_zero_cells` 占 `73.20%`，仍有 `34.40 mm` 入渗；但 2 月 9-10 日窗口中 `cap_zero_cells` 占 `99.76%`，入渗仅 `0.214 mm`，`DEP_FS` 新增地表水 `43.94 mm`。这说明后段异常峰更像是 Green-Ampt 累计入渗记忆/活动层库容缺少降雨间歇期恢复，而不是土壤真正饱和。

## 追查结果：IKW_OL 中 SURU 的去向

本次扩展了 `IKW_OL_balance.csv`，新增：

- `stream_cell_qs_raw_m3`：坡面模块原始传给河网格的地表流量体积；
- `potential_reinfiltration_m3`：按 `INFILCAPSURPLUS` 计算的潜在二次下渗体积；
- `unused_reinfil_capacity_m3`：未使用的二次下渗能力；
- `unused_capacity_with_water_m3`：有地表水同时有剩余入渗能力的体积；
- `water_bypassed_capacity_m3`：有剩余入渗能力时仍被路由走的出流体积；
- `final_surface_with_unused_capacity_m3`：末态仍有地表水且有剩余入渗能力的体积。

注意：`stream_cell_qs_raw_m3` 是 IKW_OL 原始口径。`IKW_CH` 当前有 `m_substeps=2`，每个子步使用 `m_qs / m_substeps`，所以实际进入河道的 `QS` 应以 `IKW_CH_diag.csv` 的 `qs_m3` 为准。

在锁死 10 mm 情景中，`DEP_FS` 全期新增地表水约 `102.54 mm`，`IKW_OL` 二次下渗只有约 `0.0005 mm`，`IKW_CH` 实际接收 `QS` 约 `50.61 mm`。事件窗口里 `SUR_SGA` 的 `INFILCAPSURPLUS` 也为 0，因此 IKW_OL 并不是“明明有大量二次下渗能力却没用”，而是上游 `SUR_SGA` 已经把剩余入渗能力关掉。

在显式 150 mm 情景中，全期 `IKW_CH` 实际接收 `QS` 约 `25.85 mm`，`IKW_OL` 二次下渗约 `0.165 mm`。2 月 7 日窗口实际 `QS` 约 `0.878 mm`，2 月 9-10 日窗口实际 `QS` 约 `16.52 mm`。这说明 150 mm 活动层能削弱前一个峰，但到 2 月 9-10 日累计记忆再次耗尽，后段降雨仍主要进入地表快流。

## 新结论与下一步

1. `SUR_SGA` 入渗关停的直接原因是活动层库容被 `m_accumuDepth` 消耗完，原来还叠加了 `ACTIVE_DEPTH_MAX=0` 被 `MIN=10` 错误裁剪的问题。
2. 把 `ACTIVE_DEPTH_MAX=0` 解释为完整土壤剖面会使入渗过强、洪峰消失；这不是合理最终方案。
3. 显式 `ACTIVE_DEPTH_MAX=150 mm` 是一个中间态，但后段事件仍因累计入渗记忆过强而再次关停入渗。
4. 下一步不应继续单纯增大活动层深度，而应实现降雨间歇期的 Green-Ampt/GAR 式恢复或湿润锋重分配：让 `m_accumuDepth` 随无雨时段和当前土壤水状态逐步恢复，同时限制不能把事件水全部吞入完整剖面。
5. `IKW_OL` 的二次下渗接口需要保留，但它当前受 `SUR_SGA` 输出的 `INFILCAPSURPLUS` 控制；若上游入渗容量关停，坡面运动波没有可用二次下渗能力。
