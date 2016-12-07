考虑上下游关系的地块划分
=======================
用户手册 V2.0
----------------

Original Developer: Hui Wu
Reviewed		  : Liang-Jun Zhu
Latest Update     : 2016-6-12

----------------------------

## Change log

+ 1. 修复根据流向、河网、DEM数据寻找流域出口的Bug
+ 2. 增加对TauDEM流向编码的支持
+ 3. 增加对GeoTiff输入输出的支持，并修复输出文件没有坐标系的Bug
+ 4. 增加输入数据检查，文件名中含有dem、flow、stream、landuse以及mask关键字即可（大小写不敏感）

------------------------------

## Download and installation

+ 1. 从Github上下载或Clone源码
+ 2. 利用CMake命令生成VS工程或直接编译出可执行程序，参考[SEIMS编译步骤](https://github.com/seims/SEIMS/wiki/Develop-environment)

## Data preparation

当前版本的地块划分需要准备5个输入栅格：

+ 1. DEM
+ 2. LANDUSE
+ 3. FLOW
+ 4. STREAM
+ 5. MASK

利用ArcGIS或者TauDEM提取均可，需要注意以下几点：

+ 1. 5个栅格行列号及Extent必须完全一致
+ 2. 文件命名必须包含以上关键字，大小写不限

## Execution

程序调用命令如下：

```cpp
Command:fieldpartition <Raster files path> [<threshold>] [<FlowDirection method>]

1. Rasters include dem, mask, landuse, flow, streamk MUST be located in raster files path, the format can be ASC or GTIFF; 
2. Threshold MUST be greater than 0, the default is 50;
3. Flow direction method CAN be 0 (TauDEM) or 1 (ArcGIS), the default is 0.
```

> 值得注意的是，栅格文件夹内除了这5个栅格文件，不要存放其他文件！



