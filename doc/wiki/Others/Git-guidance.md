# Github使用说明
---------------

在开始之前，强烈建议阅读以下学习资料：
+ [廖学峰老师的Git教程](http://www.liaoxuefeng.com/wiki/0013739516305929606dd18361248578c67b8067c8c017b000/)
+ [常用的Git命令](http://www.codeceo.com/article/git-command-guide.html)

# 目录
1. [Windows下安装Git](#1windows下安装git)

2. [从Github中克隆代码库及代码修改](#2从github中克隆代码库及代码修改)

3. [代码同步与更新](#3代码同步与更新)

4. [subtree操作](#4subtree操作)

5. [submodule操作（弃用技术，留存备忘）](#5submodule操作) 


## 1.Windows下安装Git
Windows下推荐使用[msysgit](http://msysgit.github.io/)的Git工具，直接下载安装即可，安装完成后，`开始 -> Git -> Git Bash`，打开一个类似命令行的窗口，说明Git安装成功了。
安装完成后，在git bash 命令行里输入：

~~~
$ git config --global user.name "Your Name"
$ git config --global user.email "email@example.com"

e.g.,
$ git config --global user.name "crazyzlj"
$ git config --global user.email "crazyzlj@gmail.com"
~~~

这两行表示，这台机器上的所有Git仓库都使用这个配置(用户名和邮件地址)。

P.S. 如何在push的时候免输密码，可以参考这个[博客](http://www.cnblogs.com/ballwql/p/3462104.html)的方法二，设置简单。

<i class="fa fa-exclamation-triangle fa-2x"></i> **注意！**

### 1.1.Git中AutoCRLF与SafeCRLF换行符问题
众所周知，Windows系统和Linux/Unix系统的换行符不同，而Windows下的Git默认情况下会在提交代码时自动替换换行符LF或CR为CRLF，这样对跨平台代码造成了极大的不便。

因此，我们一般约定代码采用Linux编码，即 `LF \n`。打开Git shell进行如下设置：

```
# AutoCRLF, 设置提交、检出时均不转换
git config --global core.autocrlf false
# SafeCRLF, 设置拒绝提交包含混合换行符的文件，并且在提交混合换行符的文件时给出警告，从而手动转换为LF换行符后提交
git config --global core.safecrlf true
# 同时，在SEIMS/.gitattributes里我们已经设置了一些例外，如Windows批处理文件.bat即为CRLF
```

[返回目录](#目录)
## 2.从Github中克隆代码库及代码修改
配置好本地的Git环境之后，我们需要将代码库从Github上克隆到本地计算机。
+ 首先在Github.com上注册一个账号。注册好之后，需要设置SSH key，在`git bash`中输入：
~~~
ssh-keygen -t rsa -C youremail@example.com

e.g.,
ssh-keygen -t rsa -C crazyzlj@gmail.com
~~~

一路回车，如果顺利的话，在用户目录（如`C:\Users\ZhuLJ`）里能找到`.ssh`目录，里面有`id_rsa`和`id_rsa.pub`两个文件，这两个就是SSH Key的密钥对，前者是私钥，后者是公钥。
+ 然后，登录Github，打开 Account Settings，SSH key页面，点Add SSH Key，Title任意填，在Key文本框里填上`id_rsa.pub`的内容，点击Add key就好啦。
+ 打开[SEIMS模型的首页](https://github.com/lreis2415/SEIMS2017)， 点击`Fork`，来创建自己账户下的SEIMS克隆，克隆结束之后，复制克隆库的**SSH**地址。下一步，我们将克隆一份代码库到本地计算机。

~~~
cd <destination folder>
git clone git@github.com:<yourname>/SEIMS.git
#推荐只克隆dev分支，用法（需要有git-1.7.10以上版本）：
git clone <url> --branch <branch> --single-branch
e.g.,
cd e:/code/hydro
git clone git@github.com:crazyzlj/SEIMS.git
#只克隆dev分支示例：
git clone git@github.com:crazyzlj/SEIMS.git --branch dev --single-branch
~~~

+ 随后，`cd`到克隆库目录，添加上游远程仓库，这一步**很重要**，关系到之后的代码同步和更新：
~~~
cd SEIMS
git remote add upstream git@github.com:seims/SEIMS.git
~~~

+ 这样，我们就将SEIMS代码克隆到了本地计算机一份。接下来，我们尝试对本地SEIMS文件做一下修改，然后提交到远程库（你自己的，如`crazyzlj/SEIMS`，而不是`seims/SEIMS`），最后通过提交`pull request` 提交给SEIMS代码拥有者（即seims）。

为了说明问题，我简单修改了一下`README`文件，然后 `git status`  查看本地和远程的差别
随后
~~~
Git add README.md
Git commit –m "modification test”
Git push –u origin master
~~~

这样就把本地的本次修改提交到了**你自己的远程库**中。

+ 在Github目录下可以看到刚才提交的修改，这时候，如果想把这个修改，提交到`seims/SEIMS`，需要点击网页上的`pull request`按钮,然后点击`Create pull request`，Github会自动判断能否“自动合并”，如果可以，便把本次修改提交完成了，如果不能，就需要我们进行手动解决冲突。
此时，`SEIMS`拥有者`seims`就会收到你的`pull request`并处理了。

[返回目录](#目录)
## 3.代码同步与更新
以上我们知道了如何将本地的修改提交到源代码库，接下来，看看如何将**源代码库中的更新**同步到本地库中。

比如其他人在`seims/SEIMS`下修改了README.md文件，现在同步到本地。
+ 查看远程主机地址
~~~
git remote -v
~~~
经过以上的设置，正常情况应该看到类似结果,如果没有upstream则需添加上游远程库

![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/seims-img%2Fgitremotev.png)

+ 将源代码库更新的内容同步到本地，检查差异，然后再和本机分支合并，注意，下列代码同步的是`upstream`库的`master`分支，如果想同步其他分支，只需将第一行的`master`替换为相应分支名即可。
~~~
git fetch upstream master
git checkout master
git merge upstream/master
~~~

> fetch远程库的时候注意，如果仅用git fetch upstream命令，会把所有远程库分支下载，速度慢，因此推荐使用
> git fetch upstream master

+ 多数情况下，Git可以自动合并，如果出现冲突，则需手动处理后，再`commit`提交，冲突结果类似：

![](http://zhulj-blog.oss-cn-beijing.aliyuncs.com/seims-img%2Fconflictwhenmerge.png)

+ 处理步骤为：
	+ 打开出现合并冲突的文件
	+ 找到冲突的地方，即`<<<<<<<<<HEAD`和`>>>>>>>>>upstream/master`出现的地方
	+ 判断不同版本，并选择去留，并删掉 `<<<<<<<<<HEAD`和`>>>>>>>>>upstream/master`
	+ `git add .`
	+ `git commit -m "merge conflict fixed"`
	+ `git push -u origin master`

[返回目录](#目录)

## 4.Subtree操作
参考资料：[XA技术不宅的博客](http://aoxuis.me/post/2013-08-06-git-subtree)

git subtree是一条git子命令，本质上subtree是一种合并策略，从git v1.5.2，官方就推荐使用subtree代替submodule，所以它并不需要保存`.submodule`这样的元信息，平时使用时你甚至可以忘掉有subtree的存在，只需子项目管理者在适当的时候与子项目库进行双向同步更新。

### 4.1第一次添加子目录，并建立与git项目的关联

建立关联总共有2条命令：

+ 语法：`git remote add -f <子仓库名> <子仓库地址>`

解释：其中-f意思是在添加远程仓库之后，立即执行fetch。

+　语法：`git subtree add --prefix=<子目录名> <子仓库名> <分支> --squash`

解释：`--squash`意思是把subtree的改动合并成一次commit，这样就不用拉取子项目完整的历史记录。`--prefix`之后的=等号也可以用空格。


**示例**:

```
$git remote add -f wiki https://github.com/lreis2415/SEIMS2017.wiki.git 
$git subtree add --prefix=doc/wiki wiki master --squash
```

## 4.2从远程仓库更新子目录

更新子目录有2条命令:

+ 语法：`git fetch <远程仓库名> <分支>`

+ 语法：`git subtree pull --prefix=<子目录名> <远程分支> <分支> --squash`

**示例**:

```
$git fetch wiki master  
$git subtree pull --prefix=doc/wiki wiki master --squash
```

## 4.3从子目录push到远程仓库（确认你有写权限）

推送子目录的变更有1条命令:

+ 语法：`git subtree push --prefix=<子目录名> <远程分支名> 分支`

**示例**:

```
$git subtree push --prefix=doc/wiki wiki master
```

[返回目录](#目录)

## 5.Submodule操作

一个项目可以独立出来一些功能相对独立、允许其他人重复使用的通用模块，这个时候，就需要使用**子模块**的概念了。

### 5.1 包含子模块的仓库的克隆

项目根目录下的`.submodules`文件定义了该项目的子模块列表和地址信息，如果该文件存在且不为空，那么可采取下面的方式对该项目进行克隆。

参考资料：http://www.kafeitu.me/git/2012/03/27/git-submodule.html

```
git clone <parent-git-path>
git submodule update --init --recursive
```

### 5.2 为当前项目添加子模块

```
git submodule add <child-git-path> <current-store-path>
```
其中，`<child-git-path>`是指子模块仓库地址，`<current-store-path>`指将子模块放置在当前工程下的路径。
注意：路径不能以 `/` 结尾（会造成修改不生效）、不能是现有工程已有的目录（不能顺利 Clone）


### 5.3 删除子模块
To remove a submodule you need to:

1. Delete the relevant section from the .gitmodules file.
2. Stage the .gitmodules changes `git add .gitmodules`
3. Delete the relevant section from `.git/config`.
4. Run `git rm --cached path_to_submodule (no trailing slash)``.
5. Run `rm -rf .git/modules/path_to_submodule`
6. Commit `git commit -m "Removed submodule <name>"`
7. Delete the now untracked submodule files
8. rm -rf path_to_submodule

###

[返回目录](#目录)
