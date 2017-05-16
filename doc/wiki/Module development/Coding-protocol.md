SEIMS开发规范
---------------------

+ 开发规范的制定是为了保持模型代码的一致性和可读性。
+ 开发规法包括代码风格统一和模块编写指南

## 1.代码风格规范

### 1.1.代码整理格式规范

SEIMS模型主要编程语言为C/C++和Python，采用CMake组织代码。由于不同开发者所用的编辑器各异，因此我们基于Google代码格式规定了C/C++和Python的代码书写格式规范，位于`./doc/style`文件夹下。这两个文件分别可以导入CLion和PyCharm软件，使用菜单栏命令“Code”->“Reformation Code”便可以一键整理代码。

### 1.2.禁用Tab键
不同IDE对Tab键的显示不同，Visual Studio下排版漂亮的代码在其他IDE下很可能错位，因此**强烈建议**在VS中将**Tab键用4个空格代替**。

打开Visual Studio，设置“工具”->“选项”->“文本编辑器”->“C/C++”->“制表符”->“缩进”选择“智能”，“制表符大小”和“缩进大小”均为4，选中“插入空格”。

### 1.3.不混用换行符
众所周知，`Windows`系统和`Linux/Unix`系统的换行符不同，而`Windows`下的Git默认情况下会在提交代码时自动替换换行符`LF`或`CR`为`CRLF`，这样对跨平台代码造成了极大的不便。

因此，我们**强烈建议**代码采用`Linux`编码`LF`即`\n`，因此需要做到：
+ VS或其他IDE设置换行符为LF而非CRLF
+ 文本编辑器建议采用Notepad++，**禁止**使用Windows的记事本！

打开Git shell进行如下设置：

```
# AutoCRLF, 设置提交、检出时均不转换
git config --global core.autocrlf false
# SafeCRLF, 设置拒绝提交包含混合换行符的文件，并且在提交混合换行符的文件时给出警告，从而手动转换为LF换行符后提交
git config --global core.safecrlf true
git config --global core.safecrlf warn
```

详见[Wiki-Github使用说明](https://github.com/lreis2415/SEIMS2017/wiki/Git-guidance)。

### 1.4.行宽
行宽必须限制！！！

建议100字符为限，太长则应换行。

### 1.5.括弧
在复杂的条件表达式中，用括弧清楚地表示逻辑优先级。

### 1.6.断行与空白的花括号{}
避免使用精简风格，如：
```cpp
if (condition) DoSomething();
else    DoSomethingElse();
```
虽然这样可以节省几行空间，但是对于单步调试查看各个变量的变化情况非常不便，因此推荐采用以下两种风格：
```cpp
if (condition) {
    DoSomething();
}
else {
    DoSomethingElse();
}
```
或者
```cpp
if (condition)
{
    DoSomething();
}
else 
{
    DoSomethingElse();
}
```

### 1.7.分行
不要把多条语句放在一行！！！更严格地，不要把多个变量定义在一行，以下三行都是不可取的例子。

```cpp
int a =1, b = 2;
c = 3; d = 5.f;
if (fFoo)  Bar();
```

### 1.8.命名大小写
采用一个通用的做法：所有的类型、类、函数名都用Pascal形式（即所有单词的第一个字母都大写），所有的变量都用Camel形式（即第一个单词全部小写，随后单词随Pascal形式，也称lowerCamel）。

+ 类、类型、变量采用名词或组合名词，如Member、productInfo等；
+ 函数则用动词或动宾组合词来表示，如get/set、RenderPage()等。

### 1.9.注释规范
+ SEIMS采用Doxygen代码注释规范，在Visual Studio中配合VA Assist插件可高效插入注释，[查看详细配置](https://github.com/lreis2415/SEIMS/wiki/Develop-environment#%E5%BC%80%E5%8F%91%E8%BE%85%E5%8A%A9%E5%B7%A5%E5%85%B7-vassistx%E4%BB%8B%E7%BB%8D)。**以下为补充Tips**。
+ `#ifndef ... #endif`宏命令之后要注释上这是对应的哪个宏命令，以防嵌套使用时混淆不清，尤其是中间代码过长的时候，如：
	```cpp
	#ifdef windows
	#define Tag_ModuleDirectoryName "\\"
	#define SEP "\\"
	#define Tag_DyLib ".dll"
	#else
	#define Tag_ModuleDirectoryName "/"
	#define SEP "/"
	#define Tag_So "lib"
	#endif /* windows */
	#ifdef linux
	#define Tag_DyLib ".so"
	#elif (defined macos) || (defined macosold)
	#define Tag_DyLib ".dylib"
	#endif /* linux */
	```

## 2.代码设计规范


## 3.Git分支管理

### 3.1.各分支命名规范及功能定义

|分支|命名示例|功能|
|:---:|:---:|:---:|
|`master`|`master`|稳定版本发布，非工作区，仅管理员可操作|
|`dev`|`dev`|开发汇总，非工作区，开发组成员均可将完成的工作合并至此，当`dev`足够稳定之后，合并至`master`|
|`bug-<bugcode>-<author>`|`bug-ompconfused-zlj`|bug修复，临时工作区，修复并测试完成之后，将此分支合并至`dev`，并新建`issue`说明问题之后，删除该分支|
|`update-<module>-<author>`|`update-docs-zlj`|模块更新，常驻工作区，完成后合并至`dev`、新建`issue`、删除该分支|
|`new-<module>-<author>`|`new-lisemsed-zlj`|新增模块，常驻工作区，完成后合并至`dev`、新建`issue`、删除该分支|

所以，团队合作的项目分支看起来就像这样：

![分支管理示意图](http://i.imgur.com/Ya2n6vm.jpg)

### 3.2.分支管理一般步骤

+ 3.2.1. [加入SEIMS开发组](https://github.com/orgs/lreis2415/teams/watershed_modeling)，直接`clone`实验室账号下的SEIMS库（git@github.com:lreis2415/SEIMS.git）即可，不用`fork`到自己账号下；
+ 3.2.2. 发现代码中出现了bug、某个文档需要修改、某个模块需要更新、亦或是需要增加一个新的模块等，新建一个分支，修复它，合并至dev分支，并新建issue告知大家，也是备忘，完后删除这个分支，流程如下：

  ```
  # 假设目前cd到SEIMS所在文件夹了
  git checkout -b bug-IFoundU-zlj
  # 修改完成，提交修改
  git add .
  git commit -m "bug fixed of IFoundU"
  # 切换回dev分支，并合并刚才的修改
  git checkout dev
  git merge --no-ff -m "merge from bug-IFoundU-zlj" bug-IFoundU-zlj
  # 如果出现冲突，处理之后，可以删除bug分支了
  git branch -d bug-IFoundU-zlj
  # 去提交一个issue告知大家吧！https://github.com/lreis2415/SEIMS/issues
  ```

