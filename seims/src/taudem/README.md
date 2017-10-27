# TauDEM
---------------------------
TauDEM(Terrain Analysis Using Digital Elevation Models) is a suite of Digital Elevation Model (DEM) tools for the extraction and analysis of hydrologic information from topography as represented by a DEM.

For more information on the development of TauDEM please refer to the wiki https://github.com/dtarb/TauDEM/wiki.

For the latest release and detailed documentation please refer to the website: http://hydrology.usu.edu/taudem.

TauDEM采用C/C++编写，矢栅数据读写基于GDAL库，因此可以跨平台编译。

## Windows
### Visual Studio
+ 安装MSMPI
+ 下载与本机Visual Studio对应版本的GDAL编译版本
+ 利用VS开发人员命令工具编译VS工程，如
`D:\compile\TauDEM>cmake -G "Visual Studio 12 2013 Win64" C:\z_code\DTA\TauDEM -DINSTALL_PREFIX=D:\compile\bin\taudem`
### Mingw64
基本思路为：
+ 安装MSMPI，并用msys的dlltool生成`.a`链接库
+ 利用mingw64编译GDAL，可用peacock辅助编译
+ 利用CLion IDE编辑、编译TauDEM，在`File->Settings->Build,Execution,Deployment->CMake->CMake options`中显式指定GDAL和MPI路径，如
`-DGEO_3RD_PARTY_ROOT=C:\peacock\windows\windows\mingw-4\x86_64 -DMPI_LIBRARIES=C:/mingw64/lib/libmsmpi.a -DMPI_INCLUDE_PATH=C:/mingw64/include`

参考：
+ http://www.math.ucla.edu/~wotaoyin/windows_coding.html
+ https://github.com/geoneric/peacock

## Linux

## macOS

+ 采用clang编译器，生成Xcode工程：`cmake -G "Xcode" <source path>`

> Note: 在执行cmake命令之前，请确保先打开Xcode，在Preference里的Locations，设置Command Line Tools为合适的版本，比如Xcode 8.2.

+ 直接编译安装
```shell
cmake <source path> -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```
+ 如果希望使用GCC编译器，则可在上述代码之前：
```shell
export CC=/usr/local/bin/gcc-4.9
export CXX=/usr/local/bin/g++-4.9
```
