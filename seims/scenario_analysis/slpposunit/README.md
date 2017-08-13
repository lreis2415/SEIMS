## 坡位单元划分

by Gao Huiran

Latest Updated：Feb.5, 2017

#### 代码说明

+ **主函数：`main.py`**

   参数说明
   + `dataDir`： 数据目录
   
   输入数据：
   
   + `slpPosfile`： 3类坡位确定性分类数据
   + `subBasinfile`： 研究区子流域数据
   + `maskfile`： 研究区掩膜数据
   
   输出数据：
   
   + `slpPosfile_`： 子流域-坡位单元（无编码）
   + `prePartion`： 唯一编码的子流域-坡位单元
   + `fieldPartion`： 平滑边界后的具有唯一编码的子流域-坡位单元（可选择使用）
   
+ **坡位划分执行函数：`fieldPartion.py`**

   函数说明
   
   + `ClipRaster(inraster, mask, outraster)`： 根据掩膜裁剪输入数据，可选；
   + `PrefieldParti(slpPos, subBasin)`： 根据坡位和子流域划分坡位单元；
   + `integRaster(prepartionfile, outputfile)`： 平滑边界
   + `extractSlpPos()`： 将Summit, Slope和Valley单独分离出来（用于生成流向关系）

+ **生成流向关系函数：`flowDirRlat.py`**

   函数说明
   
   + `SummitforSub(dataDir, subBasin)`： 统计summit与subbasin的一对多关系，生成json格式文件；
   + `SummitforSlp(dataDir, field)`： 统计summit与slope的一对多关系，生成json格式文件；
   + `SlpforVly(dataDir, field)`： 统计slope与valley的一对多关系，生成json格式文件；

+ **坡位功能指导下的BMPs空间配置：`bmpsConfig.py`**

   函数说明
   
   + `PosBMPs(dataDir)`： 提取json格式的BMPs空间配置知识文件中的信息，提取每种坡位单元上的可选BMPs类型；
   + `BMPConf(dataDir, confRate=0.5)`： 坡位指导BMPs空间配置，默认配置概率为0.5；
   + `BMPConf_random(field_num, BMPs, confRate=0.5)`： 随机BMPs空间配置，默认配置概率为0.5；
   + `BMPConf_field_Wh(dataDir, confRate=0.5)`： 考虑上下游关系的BMPs空间配置，默认配置概率为0.5；
   + `writeSceRaste(outdataDir, fieldPartion, BMPConf, wRaster=False)`： 输出情景BMPs空间分布栅格数据