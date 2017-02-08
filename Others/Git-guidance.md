# Github使用说明

为了便于群策群力完善模型，SEIMS采用Github平台进行版本控制，采用git subtree命令管理独立子库。

关于Git的详细教程，推荐阅读[廖学峰老师的网站](http://www.liaoxuefeng.com/wiki/0013739516305929606dd18361248578c67b8067c8c017b000/)

这里主要介绍我们需要用到的一些命令操作，也可参考[常用的Git命令](http://www.codeceo.com/article/git-command-guide.html)。
# 目录
1. [Windows下安装Git](#windows下安装git)

2. [从Github中克隆代码库及代码修改](#从github中克隆代码库及代码修改)

3. [代码同步与更新](#代码同步与更新)

4. [克隆Wiki库](#克隆wiki库)


## Windows下安装Git
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

[返回目录](#目录)
## 从Github中克隆代码库及代码修改
配置好本地的Git环境之后，我们需要将代码库从Github上克隆到本地计算机。
+ 首先在Github.com上注册一个账号。注册好之后，需要设置SSH key，在`git bash`中输入：
~~~
ssh-keygen -t rsa -C youremail@example.com

e.g.,
ssh-keygen -t rsa -C crazyzlj@gmail.com
~~~

一路回车，如果顺利的话，在用户目录（如`C:\Users\ZhuLJ`）里能找到`.ssh`目录，里面有`id_rsa`和`id_rsa.pub`两个文件，这两个就是SSH Key的密钥对，前者是私钥，后者是公钥。
+ 然后，登录Github，打开 Account Settings，SSH key页面，点Add SSH Key，Title任意填，在Key文本框里填上`id_rsa.pub`的内容，点击Add key就好啦。
+ 打开[SEIMS模型的首页](https://github.com/seims/SEIMS)， 点击`Fork`，来创建自己账户下的SEIMS克隆，克隆结束之后，复制克隆库的**SSH**地址。下一步，我们将克隆一份代码库到本地计算机。

~~~
cd <destination folder>
git clone git@github.com:<yourname>/SEIMS.git

e.g.,
cd e:/code/hydro
git clone git@github.com:crazyzlj/SEIMS.git
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
## 代码同步与更新
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

## 克隆Wiki库

+ SEIMS所有文档采用Github Wiki进行管理，Wiki目录是独立于SEIMS代码库的，因此需要单独克隆。
> 注意：这里需要克隆seims下的wiki，而不是你自己用户名下的，否则将无法push至seims/SEIMS.wiki!

```
cd <destination folder>
git clone git@github.com:seims/SEIMS.wiki.git

e.g.,
cd e:/code/hydro
git clone git@github.com:seims/SEIMS.wiki.git
```

+ 在该目录下，会出现`SEIMS.wiki`文件夹。

```
cd SEIMS.wiki
```

+ Github Wiki采用Markdown语法的文本文件管理，建议利用MarkdownPad2 软件作为编辑器。 对Wiki内文章做出修改后，利用Git命令提交即可。

## Git 子模块

一个项目可以独立出来一些功能相对独立、允许其他人重复使用的通用模块，这个时候，就需要使用**子模块**的概念了。

### 包含子模块的仓库的克隆

项目根目录下的`.submodules`文件定义了该项目的子模块列表和地址信息，如果该文件存在且不为空，那么可采取下面的方式对该项目进行克隆。

参考资料：http://www.kafeitu.me/git/2012/03/27/git-submodule.html

```
git clone <parent-git-path>
git submodule update --init --recursive
```

### 为当前项目添加子模块

```
git submodule add <child-git-path> <current-store-path>
```
其中，`<child-git-path>`是指子模块仓库地址，`<current-store-path>`指将子模块放置在当前工程下的路径。
注意：路径不能以 `/` 结尾（会造成修改不生效）、不能是现有工程已有的目录（不能顺利 Clone）


### 删除子模块
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
