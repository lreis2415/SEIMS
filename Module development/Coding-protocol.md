SEIMS开发规范
---------------------

+ 开发规范的制定是为了保持模型代码的一致性和可读性。
+ 开发规法包括代码风格统一和模块编写指南

## 1.代码格式

### 1.1.代码整理格式规范

SEIMS模型主要编程语言为C/C++和Python，采用CMake组织代码。由于不同开发者所用的编辑器各异，因此我们基于Google代码格式规定了C/C++和Python的代码书写格式规范，位于`./doc/style`文件夹下。这两个文件分别可以导入CLion和PyCharm软件，使用菜单栏命令“Code”->“Reformation Code”便可以一键整理代码。

### 1.2.禁用Tab键
不同IDE对Tab键的显示不同，Visual Studio下排版漂亮的代码在其他IDE下很可能错位，因此**强烈建议**在VS中将**Tab键用4个空格代替**。

打开Visual Studio，设置“工具”->“选项”->“文本编辑器”->“C/C++”->“制表符”->“缩进”选择“智能”，“制表符大小”和“缩进大小”均为4，选中“插入空格”。

### 1.3.不混用换行符
众所周知，`Windows`系统和`Linux/Unix`系统的换行符不同，而`Windows`下的Git默认情况下会在提交代码时自动替换换行符`LF`或`CR`为`CRLF`，这样对跨平台代码造成了极大的不便。

因此，我们**强烈建议**代码采用`Linux`编码`LF`即`\n`。

打开Git shell进行如下设置：

```
# AutoCRLF, 设置提交、检出时均不转换
git config --global core.autocrlf false
# SafeCRLF, 设置拒绝提交包含混合换行符的文件，并且在提交混合换行符的文件时给出警告，从而手动转换为LF换行符后提交
git config --global core.safecrlf true
git config --global core.safecrlf warn
```

详见[Wiki-Github使用说明](https://github.com/lreis2415/SEIMS2017/wiki/Git-guidance)。

## 2.代码风格

### 2.1.Python

### 2.2.C/C++

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

+ 3.2.1. 加入SEIMS开发组（https://github.com/orgs/lreis2415/teams/watershed_modeling），直接`clone`实验室账号下的SEIMS库（git@github.com:lreis2415/SEIMS.git）即可，不用`fork`到自己账号下；
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

