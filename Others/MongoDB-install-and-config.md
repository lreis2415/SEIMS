MongoDB的安装预配置
---------------------------

by Huiran Gao

Latest Updated：May.31, 2016 

---------------------------
# 目录

[**1. MongoDB安装**](#1-mongodb安装)

1. [安装](#i-安装)

2. [测试](#ii-测试)

[**2. MongoDB配置**](#2-mongodb配置)

[**3. MongoVUE**](#3-mongovue)

# 1. MongoDB安装
## i. 安装
+ 运行`mongodb-win32-x86_64-2.6.1.msi`，实际上是解压程序，将解压后的所有文件放到一个**全英文且没有空格**的文件夹中，比如`E:\MongoDB`
+ 在MongoDB文件夹内新建logs文件夹，用于存储日志文件，比如`E:\MongoDB\logs`，并在此文件夹内新建空文件`mongodb.log`
+ 在MongoDB文件夹内新建db文件夹，用于存放数据库文件，比如`E:\MongoDB\db`
+ 启动MongoDB:
  以系统管理员身份运行cmd，输入以下命令：

  ```shell
  cd E:\MongoDB\bin
  E:
  mongod.exe --dbpath=e:\mongodb\db
  ```


  如看到控制台最后一行类似:

  ```
  Tue Oct 09 11:50:55 [websvr] admin web console watiing for connections on port 27017
  ```

  说明启动成功,MongoDB占用系统27017端口。

## ii. 测试
测试MongoDB，以管理员身份新建一个cmd窗口：

```shell
cd E:\MongoDB\bin
E: 
mongo
```

如出现`connecting to:test`说明测试通过。

![](http://i.imgur.com/Wkr40Z2.png)

继续测试：

```shell
use test
db.foo.save({hello:0})
db.foo.find()
```

如果出现类似`{ "_id" : ObjectId("574cfef6c02688f96a5819a1"), "Hello" : 0 }`之类信息，说明测试成功数据已经插入数据库，

![](http://i.imgur.com/Zz9caSo.png)

然后输入`exit`退出。

# 2. MongoDB配置

+ **注册MongoDB为系统服务**
  以系统管理员身份运行cmd（必须以系统管理员身份运行cmd，否则会报错）：

  ```shell
  cd E:\MongoDB\bin
  E: 
  mongo.exe --dbpath=d:\MongoDB\db --logpath=d:\MongoDB\logs\mongodb.log --install --serviceName "MongoDB"
  ```

  如果控制台出现类似 `Tue Oct 09 12:05:15 Service can be started from the command line with 'net start MongoDB'`这样的语句，说明服务已经注册成功。

+ **启动MongoDB数据库服务**
  在cmd中输入:

  ```shell
  net start MongoDB
  ```
  即可启动MongoDB数据库服务，此时控制台输出Mongo DB服务已经启动成功，说明系统启动成功。

  注意：MongoDB的安装目录需全英文且没有空格

+ **MongoDB 安装为Windows服务，设置为自动启动**
+ 
  以管理员方式运行cmd,输入以下命令:

  ```shell
  mongod --install --serviceName "MongoDB" --serviceDisplayName "MongoDB" --logpath d:\MongoDB\logs\mongodb.log --logappend --dbpath d:\MongoDB\db --directoryperdb  
  ```

  不提示错误即表示设置成功！

  `--install`：安装MongoDB服务

  `--serviceName`：安装Windows服务时使用的服务名

  `--serviceDisplayName`：在Windows服务管理器中显示的服务名

  `--logpath`：MongoDB日志输出文件名称。虽说该参数直译是“日志路径”，其实要指定的是一个具体的完整文件名。这里我使用的是C盘根目录下的MongoDB.Log文件。该文件不用事先创建，直接指定就是了。

  `--dbpath`：指定MongoDB数据存放的路径。这个就是最关键的参数了，不仅该目录要存在，并且最好不要以“\”结尾。

  `--directoryperdb`：这个参数很好理解，让MongoDB按照数据库的不同，针对每一个数据库都建立一个目录，所谓的“目录每数据库”。

# 3. MongoVUE

详见[MongoDB及MongoVUE](Windows#（2）-mongodb及mongovue)


Good luck!



