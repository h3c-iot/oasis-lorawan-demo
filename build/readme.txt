本项目使用CMake进行工程的搭建，可以使用CMake快速编译生成Makefile，再执行make编译生成可执行文件。

一、CMake编译环境准备
1、cmake软件安装
到cmake官网下载最新的CMake软件，并安装CMake软件，安装完成后确保cmake的安装路径加入到PATH中，在控制台可以执行cmake --version查看是否生效

2、交叉编译工具链安装
本项目基于ARM cortex-M作为案例搭建，ARM官网下载arm-none-eabi交叉工具链，也可以使用其它版本的交叉编译工具链，比如ARM的armcc

3、搭建makefile执行环境
如果是windows环境，可以下载MinGW软件，提供make工具，并确保MinGW安装路径加入到PATH中，在控制台可以执行make --version查看是否生效。
如果是linux环境，系统安装默认会带make工具，如果没有则需要安装

二、Cmake制作Makefile脚本
1、windows环境
a. 打开cmd.exe
b. 进入到工程目录，并进入build目录
c. 执行cmake脚本编译，例如：cmake -DCMAKE_TOOLCHAIN_FILE="..\cmake\toolchain-arm-none-eabi.cmake" -DTOOLCHAIN_PREFIX="C:/Program Files (x86)/GNU Tools ARM Embedded/8 2019-q3-update" -DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/make.exe" -G "MinGW Makefiles" -DMBED_RADIO_SHIELD="SX1276MB1LAS" -DREGION_EU868="ON" -DREGION_CN470="OFF" -DACTIVE_REGION="LORAMAC_REGION_EU868" ..
   -DCMAKE_TOOLCHAIN_FILE 指定CMake的脚本文件
   -DTOOLCHAIN_PREFIX 指定交叉编译工具路径
   -DCMAKE_MAKE_PROGRAM 指定make工具
   -G "MinGW Makefiles" 指定编译生成的Makefile类型为MinGW Makefiles
   -DMBED_RADIO_SHIELD="SX1276MB1LAS" -DREGION_EU868="ON" -DREGION_CN470="OFF" -DACTIVE_REGION="LORAMAC_REGION_EU868"，指定工程编译选型，具体参考后续章节介绍
d. 执行成功后，会在build目录下生成Makefile文件以及其它相关文件和目录
e. 如果要重新编译，可以删除build目录下生成的所有文件，或者删除CMakeCache.txt文件，再继续执行cmake命令

2、linux环境
同windows类似

三、编译二进制文件
1、执行make即可开始编译，编译生成的二进制文件会在具体对应的application文件夹下

四、工程编译选型
1、MBED_RADIO_SHIELD
射频选择，可选参数如下：
* SX1272MB2DAS 
* SX1276MB1LAS 
* SX1276MB1MAS ：默认
* SX1261MBXBAS 
* SX1262MBXCAS 
* SX1262MBXDAS
可以通过-DMBED_RADIO_SHIELD="SX1276MB1LAS"来进行选择


2、国家频段功能开关
* REGION_EU868 : 默认 OFF
* REGION_US915 : 默认 OFF
* REGION_CN779 : 默认 OFF
* REGION_EU433 : 默认 OFF
* REGION_AU915 : 默认 OFF
* REGION_AS923 : 默认 OFF
* REGION_CN470 : 默认 ON
* REGION_KR920 : 默认 OFF
* REGION_IN865 : 默认 OFF
* REGION_RU864 : 默认 OFF
可以通过-DREGION_XXXXX="ON"开启，通过-DREGION_XXXXX="OFF"关闭，支持多项开关同时开启，但至少要开启一项

3、ACTIVE_REGION
当前版本工作的国家频段，可选参数如下：
* LORAMAC_REGION_EU868 
* LORAMAC_REGION_US915 
* LORAMAC_REGION_CN779
* LORAMAC_REGION_EU433 
* LORAMAC_REGION_AU915 
* LORAMAC_REGION_AS923 
* LORAMAC_REGION_CN470 : 默认
* LORAMAC_REGION_KR920 
* LORAMAC_REGION_IN865 
* LORAMAC_REGION_RU864
可以通过-DACTIVE_REGION="LORAMAC_REGION_XXXXX"来进行选择

4、APPLICATION
指定当前编译的应用案例，可以通过-DAPPLICATION="XXXXXX"选择，可选参数如下：
* classA : 默认
  官方的ClassA案例程序
* classC
  官方的classC案例程序
* classB
  官方的classB案例程序，需要开启Class B使能
  
5、CLASSB_ENABLED
ClassB使能开关，可以通过-DCLASSB_ENABLED="ON"开启，默认关闭。
