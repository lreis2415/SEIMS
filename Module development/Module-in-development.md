Modules status in SEIMS
-----------

Initial created		: Liang-Jun Zhu

Corporate editors	: 

Last Updated: 2016-5-30

----------------------

## 表格填写说明

+ 1.过程名：对应于C++代码中`text.h`中宏定义前缀为`MCLS_`的分类，细分到每一种流域过程。
+ 2.模块ID：对应于源码中的模块ID
+ 3.方法：简要描述改模块所用算法，有参考的增加参考
+ 4.开发状态：清晰描述开发状态，如Bug修复、原理确认、代码编写等
+ 5.测试状态：清晰描述测试状态，如OMP版本测试、MPI版本测试等
+ 6.TODO：将来做出哪些改进，或现有的处理不足之处等

----------------------

|过程名|模块ID|方法|开发状态|测试状态|TODO|
|-----|-----|-----|-----|------|-----|
|气象数据|TSD_RD|逐时间步长读取站点气象观测数据|完成|完成||
||ITP|泰森多边形插值|完成|完成||
|蒸散发|PET_H|Hargreaves|完成|完成||
||PET_PT|PriestleyTaylor|完成|完成||
||PET_PM|Penman-Monteith|完成|待测试|待生态模块完成之后测试|
|土壤温度|STP_FP|Finn Plauborg|完成|完成||
|截留|PI_MSM|Maximum Storage|完成|完成||
||PI_STORM|Maximum Storage|未更新|尚未检查||
|融雪|SRD_MB|Snow redistribution|完成|未测试|缺少输入数据|
||SSM_PE|Snow Sublimation(WetSpa)|完成|完成||
||SNO_DD|Degree-Day|完成|完成||
||SNO_WB|Water balance|完成|完成|需确认输入输出是否必须依赖SNO_DD|
||SNO_SP|Snowpeak Daily(SWAT)|完成|完成|需确认输入输出是否必须依赖SNO_DD|
|入渗、产流|SUR_MR|Modified rational（WetSpa）|完成|完成||
||SUR_CN|SCS-CN|完成|完成||
||SUR_SGA|Green-Ampt|未更新|未测试||
|填洼|DEP_LINSLEY|Linsley method|完成|完成||
||DEP_FS|Fill and Spill|未更新|未测试||
|渗漏|PER_PI|Darcy's Law and Brooks-Corey|未完成|测试出问题|模拟时段内某天土壤水分为负，需检查算法的正确性|
||PER_STR|Storage routing|未完成|测试出问题|同PER_PI|
||PER_DARCY|Darcy's Law|未更新|未测试||
|地下径流|SSR_DA|Darcy's Law and Kinematic|未更新|未测试||
|土壤蒸发|SET_LM|Linear method|未更新|未测试||
|地下水|GWA_RE|Linear reservoir|未更新|未测试||
||GW_RSVR|Linear reservoir|未更新|未测试|与GWA_RE有何区别，命名待统一|
|坡面侵蚀|MUSLE_AS|MUSLE model|未更新|未测试||
|溅蚀|SplashEro_Park|Park Equation|未更新|未测试|模块ID需简写|
|坡面泥沙汇流|KinWavSed_OL|Energy function(Govers) method|未更新|未测试|确定原理、简写模块ID|
|河道泥沙汇流|KinWavSed_CH|Srinivasan & Galvao function|未更新|未测试|确定原理、简写模块ID|
||SEDR_VCD|Variable channel dimension|未更新|未测试||
|坡面汇流|IUH_OL|Instantaneous unit hydrograph(WetSpa)|未更新|未测试|应更名OL_IUH|
||IKW_OL|Kinematic wave|未更新|未测试|应更名OL_IKW|
|河道汇流|MUSK_CH|Muskingum|未更新|未测试|应更名为CH_MUSK|
||IKW_CH|Kinematic wave|未更新|未测试|应更名为CH_IKW|
||CH_DW|Diffusive wave|未更新|未测试||
|||||||


## 重要的问题

### Hydrology和Hydrology_longterm

这两个模块分组原意为前者为次暴雨模块组，后者为长时段模块组，目前看二者用的算法高度重叠，比如河道汇流方法马斯京根法、运动波法、四点隐式差分法等，模块命名仅前后顺序相反，如CH_MSK和MUSK_CH。

这样很容易造成使用混淆，能否将二者合二为一？或在模块命名时加以区分？



