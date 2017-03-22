模块开发Demo
-------------

以一个简单Demo为例介绍，模块开发的整体流程。

一般来说，模块编写基本步骤如下：

+ 1. 阅读相关文献和模型文档，编写模型文档的理论部分；
+ 2. 设计模块名，基本命名规则为`过程_算法`，如`PET_H`为蒸散发过程（PET）的Hargreaves方法实现；缩写规范自行查找，比如`YLD`可以是作物产量`Yield`的缩写，没有指定具体算法的，命名只有`过程`也可；
+ 3. 根据模型的相关理论，参考已有成熟模型的代码，列出模型的输入（包括以其他模块的输出作为输入）、输出，编写元数据函数(`MetadataInformation`)代码，即`api.cpp`；
+ 在头文件中定义所需变量，并在实现文件(`.cpp`)中做参数初始化（构造函数）、赋值（`SetValue`, `Set1DData`, `Set2DData`系列函数）和有效性检查（`CheckInputData`系列函数）的工作；
+ 根据公式，编写`Execute`函数的代码，实现模拟功能
+ 编写输出代码，`Get1DData`，`Get2DData`等
+ 代码检查、测试


# 目录

1. [新建模块](#1-新建模块)

2. [编写元数据函数](#2-编写元数据函数)

3. [头文件定义](#3-头文件定义)

4. [实现函数编写](#4-实现函数编写)

5. [模块测试](#5-模块测试)


## 1. 新建模块

这里，默认你已经掌握SEIMS代码基本结构和`CMAKE`编译知识，并已经成功编译出SEIMS的VS工程，如果没有，请往前阅读相关文档。

**模块功能设计**：

+ 测试模型对单层栅格和多层栅格数据的支持
+ 分别读取Raster1D和Raster2D数据，做简单运算之后，输出
+ 模块命名为`IO_TEST`

**利用CMAKE新建模块工程**：

+ 在`SEIMS/src/modules/test`下新建目录`IO_TEST`，并在`SEIMS/src/modules/test/CMakeLists.txt`中添加语句`ADD_SUBDIRECTORY(./IO_TEST)`；
+ 复制`SEIMS/src/modules/test/template/`目录下的元数据模板`api.cpp`，头文件模板`template.h`，实现函数模板`template.cpp`，以及`CMakeLists.txt`到`IO_TEST`目录下；
+ 根据需要，对头文件和实现函数文件进行重命名，这里我们重命名为`IO_test.h`和`IO_test.cpp`,并修改`CMakeLists.txt`中相关设置，具体如下图所示
![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/SEIMS-wiki%2Fcmakelist-modification.jpg)
+ 一切准备就绪，打开Visual Studio 命令提示工具进行编译，随后打开VS，便可看到新增的模块工程，如下图：
![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/SEIMS-wiki%2Fnew-module-created.jpg)

[返回目录](#目录)

## 2. 编写元数据函数

为了便于统一修改，并保持SEIMS整体的一致性，`api.cpp`中所有字符串类型的元数据信息均在`src/base/util/text.h`头文件中以宏定义`#define`的形式进行管理。

### 2.1 SetClass
定义模块所属模块组及描述，在`text.h`中定义为`MCLS_`及`MCLSDESC_`，如：

```cpp
/// Potential Evapotranspiration Modules
#define MCLS_PET		"Potential Evapotranspiration"
#define MCLSDESC_PET	"Calculate the potential evapotranspiration"
```

### 2.2 SetID, SetDescription
定义该模块的唯一标识符ID和描述，在`text.h`中定义为`MID_`及`MDESC_`，如：

```cpp
#define MID_PET_H		"PET_H"
#define MDESC_PET_H		"Hargreaves method for calculating the potential evapotranspiration."
```
SetName 同 SetID

### 2.3 AddParameter

所有从数据库中原始读取的参数，均以AddParameter形式定义，这些参数在模拟开始前从数据库中读取，以指针的形式在不同模块间传递，模拟开始后，不重复读取！

共有5个输入：

+ 变量名，在`text.h`中定义为`VAR_`，如CN2值定义为`VAR_CN2`
+ 单位，在`text.h`中定义为`UNIT_`，如秒的单位为`UNIT_SECOND`
+ 描述，在`text.h`中定义为`DESC_`，如CN2值得描述为`DESC_CN2`
+ 来源，来源共有如下几种：
	+ `File_Input`:来自模型配置文件`file.in`,如模拟时间步长
	+ `Source_HydroClimateDB`：来自水文气象数据库，如气温
	+ `Source_ParameterDB`：来自模型参数数据库，包括MongoDB中`Parameters`表和`spatial`表
+ 数据类型，为`dimensionTypes`枚举型变量，主要包括：
	+ `DT_Single`：单个数值型
	+ `DT_Array1D`：一维数组，如站点平均气温
	+ `DT_Array2D`：二维数组，如栅格分层计算的层数及每层栅格的索引值
	+ `DT_Raster1D`：一维栅格数据，如DEM
	+ `DT_Raster2D`：二维栅格数据，如多层土壤属性栅格
	+ `DT_Scenario`：情景数据

### 2.4 AddInput
所有来源于其他模块输出的参数均以AddInput的形式定义，这部分参数在模块运行之前，检查其他模块是否有输出该参数，如果有，则获取指针；如果该参数尚未初始化，则由对应模块的`initialOutputs()`函数进行初始化并输出。

与AddParameter的设置基本一致，唯一的区别在于AddInput的变量来源必须是`Source_Module`。

二者的区别为：

+ AddParameter中的参数，在模型构建时（即开始逐时间步长计算之前）一次性读取（除气象水文数据外），具体参见`src/base/ModuleFactory.cpp`中的：

```cpp
void ModuleFactory::SetData()
```

+ 而AddInput的参数，在模型开始第一个时间步长运行前，获取该参数的内存地址：

```cpp
void ModuleFactory::GetValueFromDependencyModule()
```

### 2.5 AddOutput

AddOutput需要设置变量名、单位、描述、数据类型，不再赘述。


### 2.6 Demo

我们设定了测试模型输入输出的模块目的，这里我们可以这样定义AddParameter和AddOutput（暂不涉及从其他模块中读取变量）：读取一维栅格数据`VAR_CN2`和二维栅格数据`VAR_CONDUCT`，分别输出为`CN2_M`和`K_M`：

```cpp
/// set parameters from database
mdi.AddParameter(VAR_CN2, UNIT_NON_DIM, DESC_CN2, Source_ParameterDB, DT_Raster1D);
mdi.AddParameter(VAR_CONDUCT, UNIT_WTRDLT_MMH, DESC_CONDUCT, Source_ParameterDB, DT_Raster2D);
/// set the output variables
mdi.AddOutput("CN2_M",UNIT_NON_DIM, DESC_CN2, DT_Raster1D);
mdi.AddOutput("K_M",UNIT_WTRDLT_MMH,DESC_CONDUCT,DT_Raster2D);
```

[返回目录](#目录)

## 3. 头文件定义

### 3.1 利用VAssistX快速重命名

打开头文件`IO_test.h`之后，首先想将默认的继承于`SimulationModule`的子类名称改为`IO_TEST`，有了VassistX我们可以很方便的快速重命名，如下：

![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/SEIMS-wiki%2Fvs-rename.jpg)

![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/SEIMS-wiki%2Fvs-rename2.jpg)

### 3.2 定义输入、输出变量

与`api.cpp`中设置的变量一一对应，在`IO_TEST`类定义变量名，并做好注释，这里我的定义如下：

```cpp
private:
	/// valid cells number
	int m_nCells;
	/// input 1D raster data
	float* m_raster1D;
	/// maximum number of soil layers
	int m_soilLayers;
	/// input 2D raster data
	float** m_raster2D;
	/// output 1D raster data
	float* m_output1Draster;
	/// output 2D raster data
	float** m_output2Draster;
```

### 3.3 定义public或private函数

+ 构造函数和析构函数：（必需）
+ 算法执行函数：（必须），`virtual int Execute();`
+ 获取输入数据系列函数：（多选）
	+ `SetValue`：设置`DT_Single`类型数据
	+ `Set1DData`：设置`DT_Array1D`和`DT_Raster1D`类型数据
	+ `Set2DData`：设置`DT_Array2D`和`DT_Raster2D`类型数据
+ 获取输出数据系列函数：（多选）
	+ `GetValue`：读取`DT_Single`类型数据
	+ `Get1DData`：读取`DT_Array1D`和`DT_Raster1D`类型数据
	+ `Get2DData`：读取`DT_Array2D`和`DT_Raster2D`类型数据
+ 其他一些自定义的函数

## 4. 实现函数编写

接下来我们可以进入实现函数编写的阶段。

### 4.1 参数初始化

按照AddParameter，AddInput，AddOutput中参数定义顺序，在构造函数中依次进行参数初始化，基本原则为：

+ `DT_Single`类型数据设置为-1或者`NODATA`(`utils.h`中定义)
+ `DT_Array1D`, `DT_Raster1D`, `DT_Array2D`和`DT_Raster2D`类型变量设置为`NULL`

这里我们这样初始化：

```cpp
IO_TEST::IO_TEST(void):m_nCells(-1), m_raster1D(NULL), m_raster2D(NULL),
	m_output1Draster(NULL), m_output2Draster(NULL)
{
}
```

### 4.2 内存释放（析构函数）

基本原则是：凡是在该模块中new的内存均需要在析构函数中释放，除非该变量在其他地方被显式释放。对于一维数组利用`delete[]`即可，对于二维数组则需要遍历释放，并重新赋值为`NULL`避免“野指针”。

这里我们这样释放内存：

```cpp
IO_TEST::~IO_TEST(void)
{
	if (m_output1Draster != NULL)
		delete[] m_output1Draster;
	m_output1Draster = NULL;
	if (m_output2Draster != NULL)
	{
		for(int i = 0; i < m_nCells; i++)
			delete[] m_output2Draster[i];
		delete[] m_output2Draster;
		m_output2Draster = NULL;
	}
}
```

由于释放内存频繁用到，因此，在`utils.h`中定义了模板函数用于释放内存：

```cpp
#include "utils.h"
if(m_output1Draster != NULL) Release1DArray(m_output1Draster);
if(m_output2Draster != NULL) Release2DArray(m_nCells, m_output2Draster)
```

### 4.3 变量赋值

赋值时首先进行数据长度检查（`CheckInputSize()`）,在进行长度检查时，顺便给栅格个数（`m_nCells`）赋值，随后对变量名进行匹配（`StringMatch()`），如果匹配则赋值，以`Set1DData`为例:

```cpp
void IO_TEST::Set1DData(const char* key, int n, float* data)
{
	if(!this->CheckInputSize(key,n)) return;
	string sk(key);
	if(StringMatch(sk, VAR_CN2)) this->m_raster1D = data;
}

bool IO_TEST::CheckInputSize(const char* key, int n)
{
	if(n<=0)
		throw ModelException("IO_TEST","CheckInputSize","Input data for "+string(key) +" is invalid. The size could not be less than zero.");
	if(this->m_nCells != n)
	{
		if(this->m_nCells <=0) this->m_nCells = n;
		else
			throw ModelException("IO_TEST","CheckInputSize","Input data for "+string(key) +" is invalid. All the input data should have same size.");
	}
	return true;
}
```

### 4.4 参数有效性检查

有效性检查函数`CheckInputData()`用于逐个判断SetValue系列函数中设置的输入是否均以成功赋值，如没有，则抛出异常。

```cpp
bool IO_TEST::CheckInputData()
{
	if(this->m_date == -1) /// m_date is protected variable member in base class SimulationModule.
		throw ModelException("IO_TEST","CheckInputData","You have not set the time.");
	if(m_nCells <= 0)
		throw ModelException("IO_TEST","CheckInputData","The dimension of the input data can not be less than zero.");
	if(m_raster1D == NULL)
		throw ModelException("IO_TEST","CheckInputData","The 1D raster input data can not be less than zero.");
	if(m_raster2D == NULL)
		throw ModelException("IO_TEST","CheckInputData","The 2D raster input data can not be less than zero.");
	return true;
}
```

### 4.5 编写Execute函数

在Execute函数中，首先需要为输出变量开辟内存空间，并赋初值，然后进行模块功能的实现。

这里我们以简单的算法为例：

```cpp
int IO_TEST::Execute()
{
	/// Initialize output variables
	if (m_output1Draster == NULL)
	{
		m_output1Draster = new float[m_nCells];
#pragma omp parallel for
		for (int i = 0; i < m_nCells; ++i)
			m_output1Draster[i] = 0.f;
	}
	if (m_output2Draster == NULL)
	{
		m_output2Draster = new float*[m_nCells];
#pragma omp parallel for
		for (int i = 0; i < m_nCells; ++i)
		{
			m_output2Draster[i] = new float[m_soilLayers];
			for(int j = 0; j < m_soilLayers; j++)
				m_output2Draster[i][j] = 0.f;
		}
	}
	/// Execute function
#pragma omp parallel for
	for (int i = 0; i < m_nCells; ++i)
	{
		m_output1Draster[i] = m_raster1D[i] * 0.5f;
		for(int j = 0; j < m_soilLayers; j++)
			m_output2Draster[i][j] = m_raster2D[i][j] + 2.f;
	}
	return true;
}
```

与内存释放类似，为方便参数初始化，在`util.h`中定义了模板函数，调用示例如下：

```cpp
/// Initialize output variables
if(m_output1Draster == NULL) Initialize1DArray(m_nCells, m_output1Draster, 0.f)
if(m_output2Draster == NULL) Initialize2DArray(m_nCells, m_soilLayers, m_output2Draster, 0.f)
```

### 4.6 编写变量输出代码

与变量赋值代码类似，首先进行变量名匹配，然后进行赋值，以`Get2DData`为例：

```cpp
void IO_TEST::Get2DData(const char* key, int* n, int* col, float*** data)
{
	string sk(key);
	if(StringMatch(sk, "K_M"))
	{
		*data = this->m_output2Draster;
		*n = this->m_nCells;
		*col = this->m_soilLayers;
	}
}
```

至此，这个简单的`IO_TEST`模块的代码编写工作便完成了！

[返回目录](#目录)

## 5. 模块测试

>值得一提的是，SEIMS同时也是一个模块化框架，单独运行某一个模块也是可以的(当然是在保证气象输入正确的前提下)。

对于我们编写的这个IO测试模块，可以通过配置模型输入文件，仅运行这一模块，并得到输出。

### 5.1 config.fig

通过之前的学习，相信你对config.fig的输入格式已经有所了解，不再赘述，具体设置如下：

```
### FORMAT
### MODULE CLASS NO. | PROCESS NAME | METHOD NAME | MODULE ID
### -1 | Module Test
0 | IO test| IO | IO_TEST
```

### 5.2 file.in

既然仅测试这一模块，那就只设置模拟一天（其实file.in对IO_TEST没用）

```
MODE|Daily
INTERVAL|1
STARTTIME|2014-01-01 00:00:00
ENDTIME|2014-01-02 00:00:00
```
### 5.3 file.out

分别设置2个输出，注意设置开始和结束时间要在file.in中模拟时间段之内，即：

```
### -1 Module IO TEST
OUTPUTID|CN2_M
COUNT | 1
TYPE | MEAN
STARTTIME|2014-01-01 00:00:00
ENDTIME|2014-01-02 00:00:00
FILENAME | CN2_M.tif

OUTPUTID|K_M
COUNT | 1
TYPE | MEAN
STARTTIME|2014-01-01 00:00:00
ENDTIME|2014-01-02 00:00:00
FILENAME | K_M.tif
```

### 5.4 运行模型

我的运行命令为：

```
D:\Compile\SEIMS_OMP\Release\seims_omp E:\code\Hydro\SEIMS\model_data\TEST\model_dianbu_30m_longterm  6 0 127.0.0.1 27017
```

值得注意的是，模型目录文件夹名称一定要与MongoDB数据库名一致！

运行后在该目录下可以看到`output`文件夹：

![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/SEIMS-wiki%2FIO_test_run.jpg)

![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/SEIMS-wiki%2FIO_test_run_output.jpg)


至此，一个简单的SEIMS框架下的模块便编写完成了！

[返回目录](#目录)
