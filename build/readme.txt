��һ������װCMake���

�ڶ������������뻷������ARM��������arm-none-eabi������빤��������װ

�������������windows��������Ҫ����MinGW������ṩmake���ߣ������linux�����������ʡ��

���Ĳ������룬ִ��cmake -DCMAKE_TOOLCHAIN_FILE="..\cmake\toolchain-arm-none-eabi.cmake" -DTOOLCHAIN_PREFIX="C:/Program Files (x86)/GNU Tools ARM Embedded/8 2019-q3-update" -DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/make.exe" -G "MinGW Makefiles" -DMBED_RADIO_SHIELD="SX1276MB1LAS" -DREGION_EU868="ON" -DREGION_CN470="OFF" -DACTIVE_REGION="LORAMAC_REGION_EU868" ..