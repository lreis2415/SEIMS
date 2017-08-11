# Demo Data
## 测试数据

以店埠河数据（dianbu）为例进行模型测试

examples中按照模型的5个方面进行了举例说明。

# 1. 预处理
```shell
# 打开data\examples\1.preprocess\dianbu2_30m_omp_win.ini文件修改对应的路径
cd C:\z_code\Hydro\SEIMS
python -m seims.preprocess.main -ini C:\z_code\Hydro\SEIMS\data\examples\1.preprocess\dianbu2_30m_omp_win.ini
```

# 2. 运行模型
# 2.1 长时段模型
```shell
cd D:\compile\bin\seims_longterm
seims_omp C:\z_code\Hydro\SEIMS\data\dianbu\model_dianbu2_30m_demo 4 0 127.0.0.1 27017 0
```

