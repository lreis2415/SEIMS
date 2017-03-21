SEIMS 开发环境
-----------------------

by Huiran Gao

Latest Updated：May.31, 2016 

-----------------------
# 目录

[**1. Windows下VS编译**](#1-windows下vs编译)

[**2. 开发辅助工具-VAssistX介绍**](#2-开发辅助工具-vassistx介绍)

1. [VAssistX安装与说明](#i-vassistx安装与说明)

2. [VAssistX的使用](#ii-vassistx的使用)

# Windows下VS编译
SEIMS模型采用C++编写，开发工具主要是**VS 2010**。

安装Microsoft Visual Studio 2010后，打开“开始” ->  Microsoft Visual Studio 2010 -> Visual Studio Tools->Visual Studio 命令提示 (2010)，以**管理员方式运行**，切换到VS工程目录，VS工程目录是在SEIMS源码外新建的一个文件夹，比如 `D:\SEIMS\SEIMS_prj`。

然后使用CMake命令生成VS 2010工程，详细参考[**Windows下SEIMS主程序的编译**](Windows#ii-windows下seims主程序的编译)

编译完成后，打开VS工程，生成解决方案，工程目录结构如下：

![](http://i.imgur.com/hR7E7ZN.png)

# 开发辅助工具-VAssistX介绍
**VAssistX**（Visual Assist X）是开发环境的辅助工具，具有强大的编辑特色，可以完全集成到开发环境中，使用该工具可以更加轻松地编写代码。支持C/C++，C#，Java和HTML等语言，能自动识别各种关键字、系统函数、成员变量、自动给出输入提示、自动标示错误等，有助于提高开发过程的自动化和开发效率。

在SEIMS中，为了使模型代码具有更好的可读性、传承性，我们使用[doxygen](http://www.doxygen.nl/)规则对代码进行注释，在VS2010中借助VAssistX进行快速添加注释。

## i. VAssistX安装与说明
VAssistX（破解版）下载地址：[http://pan.baidu.com/s/1qXI4UPe](http://pan.baidu.com/s/1qXI4UPe)

选择本地安装的VS版本 -> Install。

![](http://i.imgur.com/htMVHUl.png)

安装完成后，打开VS2010，菜单栏中会出现VAssistX菜单项，说明安装成功。

![](http://i.imgur.com/G62YwdG.png)

## ii. VAssistX的使用

打开VAssistX选项自定义注释，Suggestion -> Edit VA Snippets,自定义注释方法如图所示：

![](http://i.imgur.com/3I0xXvB.jpg)

添加一些符合doxygen规则的C++注释模板，为了方便起见，快捷键Shortcut统一设置为`/  *!`，这样，当需要输入注释模板时，输入`/*!`后便会出现只能提示，如下图，选择合适的便可快速插入注释。

![](http://gaohr.win/site/blogs/2016/2016-04-20-VAssistX/image006.png)

### VAssistX规范注释示例
+ 最基本的注释，对任何对象（file，function，class…）均适用，使用格式如下:
  
  ```cpp
  //! Brief description, can span only 1 line
  /*!
  * More detailed description.
  * Feel free to span as many lines
  * as you wish... All allowed.
  */
  ```
  在VA中设置如下（Title，Description可以按照自己想法设置，下面均不再重复）：
  
  ```
  Title: generalCommentBlock
  Shortcut: /*!
  Descrition: General Doxygen Commen Block
  Code:
  ```

  ```cpp
  //! $end$
  /*!
  * \ingroup 
  *
  */
  ```

+ 对于一些变量或函数的定义，希望在一行内对其注释，使用格式如下：

  ```cpp
  void function1();    //!< Correct
  void function2(int i);    //!< WRONG! Comment on parameter i, but function not documented!
  void function3(int i);    //!< Some int. Correct - both function and parameter   documented
  ```

  上面的例子说明，对于含有参数的函数定义，VA中的设置如下：
  
  ```cpp
  //!<
  ```

+ 在`*.h`, `*.cpp`文件头部的注释,使用格式：

  ```cpp
  /*!
  * \file [filename]
  * \brief
  *
  * \author [your name]
  * \date
  *
  * [your comment here]
  */
  ```

  在VA中设置：

  ```cpp
  /*!
  * \ingroup
  * \file $FILE_BASE$.$FILE_EXT$
  * \brief
  *
  * \author [your name]
  * \version
  * \date $MONTHLONGNAME$ $YEAR$
  *
  * $end$
  */
  ```

+ 类定义的注释

  ```cpp
  /*!
  * \ingroup 
  * \class [class name]
  *
  * \brief [brief description]
  *
  * [detailed description]
  *
  * \author [your name]
  * \date
  */
  ```

  VA中的设置：
  
  ```cpp
  /*!
  * \ingroup
  * \class $end$
  *
  * \brief
  *
  * \author [your name]
  * \date $MONTHLONGNAME$ $YEAR$
  */
  ```

+ 函数注释块
  
  ```cpp
  /*!
  * \ingroup 
  * \brief [brief description]
  *
  * [detailed description]
  *
  * \param[in] [name of input parameter] [its description]
  * \param[out] [name of output parameter] [its description]
  * \return [information about return value]
  * \sa [see also section]
  * \note [any note about the function you might have]
  * \warning [any warning if necessary]
  */
  ```
  
  VA中设置：
  
  ```cpp
  /*!
  * \ingroup
  * \brief
  *
  * $end$
  *
  * \param[in]
  * \param[out]
  * \return
  * \sa
  * \note
  * \warning
  */
  ```