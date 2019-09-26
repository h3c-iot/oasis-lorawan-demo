第一步：安装CMake软件

第二步：搭建交叉编译环境，到ARM官网下载arm-none-eabi交叉编译工具链并安装

第三步：如果是windows环境，需要下载MinGW软件，提供make工具，如果是linux环境，则可以省略

第四步：编译，执行cmake -DCMAKE_TOOLCHAIN_FILE="..\cmake\toolchain-arm-none-eabi.cmake" -DTOOLCHAIN_PREFIX="C:/Program Files (x86)/GNU Tools ARM Embedded/8 2019-q3-update" -DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/make.exe" -G "MinGW Makefiles" -DMBED_RADIO_SHIELD="SX1276MB1LAS" -DREGION_EU868="ON" -DREGION_CN470="OFF" -DACTIVE_REGION="LORAMAC_REGION_EU868" ..