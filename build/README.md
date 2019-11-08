# ���̴
* ����Ŀʹ��CMake���й��̵Ĵ������ʹ��CMake���ٱ�������Makefile����ִ��make�������ɿ�ִ���ļ� *

## CMake���뻷��׼��
### cmake������װ
��cmake�����������µ�CMake����������װCMake��������װ��ɺ�ȷ��cmake�İ�װ·�����뵽PATH�У��ڿ���̨����ִ��cmake --version�鿴�Ƿ���Ч

### ������빤������װ
����Ŀ����ARM cortex-M��Ϊ�������ARM��������arm-none-eabi���湤������Ҳ����ʹ�������汾�Ľ�����빤����������ARM��armcc

### �makefileִ�л���
�����windows��������������MinGW�������ṩmake���ߣ���ȷ��MinGW��װ·�����뵽PATH�У��ڿ���̨����ִ��make --version�鿴�Ƿ���Ч��
�����linux������ϵͳ��װĬ�ϻ��make���ߣ����û������Ҫ��װ

## Cmake����Makefile�ű�
### windows����
a. ��cmd.exe
b. ���뵽����Ŀ¼��������buildĿ¼
c. ִ��cmake�ű����룬���磺cmake -DCMAKE_TOOLCHAIN_FILE="..\cmake\toolchain-arm-none-eabi.cmake" -DTOOLCHAIN_PREFIX="C:/Program Files (x86)/GNU Tools ARM Embedded/8 2019-q3-update" -DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/make.exe" -G "MinGW Makefiles" -DMBED_RADIO_SHIELD="SX1276MB1LAS" -DREGION_EU868="ON" -DREGION_CN470="OFF" -DACTIVE_REGION="LORAMAC_REGION_EU868" ..
   -DCMAKE_TOOLCHAIN_FILE ָ��CMake�Ľű��ļ�
   -DTOOLCHAIN_PREFIX ָ��������빤��·��
   -DCMAKE_MAKE_PROGRAM ָ��make����
   -G "MinGW Makefiles" ָ���������ɵ�Makefile����ΪMinGW Makefiles
   -DMBED_RADIO_SHIELD="SX1276MB1LAS" -DREGION_EU868="ON" -DREGION_CN470="OFF" -DACTIVE_REGION="LORAMAC_REGION_EU868"��ָ�����̱���ѡ�ͣ�����ο������½ڽ���
d. ִ�гɹ��󣬻���buildĿ¼������Makefile�ļ��Լ���������ļ���Ŀ¼
e. ���Ҫ���±��룬����ɾ��buildĿ¼�����ɵ������ļ�������ɾ��CMakeCache.txt�ļ����ټ���ִ��cmake����

### linux����
ͬwindows����

## ����������ļ�
ִ��make���ɿ�ʼ���룬�������ɵĶ������ļ����ھ����Ӧ��application�ļ�����

# ����ѡ��
## MBED_RADIO_SHIELD
��Ƶѡ�񣬿�ѡ�������£�
* SX1272MB2DAS 
* SX1276MB1LAS 
* SX1276MB1MAS
* SX1261MBXBAS 
* SX1262MBXCAS 
* SX1262MBXDAS
* SX1278H3C     
* SX1278ACSIPS78F  ��Ĭ��
* WSL305S
����ͨ��-DMBED_RADIO_SHIELD="SX1276MB1LAS"������ѡ��


## ����Ƶ�ι��ܿ���
* REGION_EU868 : Ĭ�� OFF
* REGION_US915 : Ĭ�� OFF
* REGION_CN779 : Ĭ�� OFF
* REGION_EU433 : Ĭ�� OFF
* REGION_AU915 : Ĭ�� OFF
* REGION_AS923 : Ĭ�� OFF
* REGION_CN470 : Ĭ�� ON
* REGION_KR920 : Ĭ�� OFF
* REGION_IN865 : Ĭ�� OFF
* REGION_RU864 : Ĭ�� OFF
����ͨ��-DREGION_XXXXX="ON"������ͨ��-DREGION_XXXXX="OFF"�رգ�֧�ֶ����ͬʱ������������Ҫ����һ��

## ACTIVE_REGION
��ǰ�汾�����Ĺ���Ƶ�Σ���ѡ�������£�
* LORAMAC_REGION_EU868 
* LORAMAC_REGION_US915 
* LORAMAC_REGION_CN779
* LORAMAC_REGION_EU433 
* LORAMAC_REGION_AU915 
* LORAMAC_REGION_AS923 
* LORAMAC_REGION_CN470 : Ĭ��
* LORAMAC_REGION_KR920 
* LORAMAC_REGION_IN865 
* LORAMAC_REGION_RU864
����ͨ��-DACTIVE_REGION="LORAMAC_REGION_XXXXX"������ѡ��

## APPLICATION
ָ����ǰ�����Ӧ�ð���������ͨ��-DAPPLICATION="XXXXXX"ѡ�񣬿�ѡ�������£�
* classA
  �ٷ���ClassA��������
* classC
  �ٷ���classC��������
* classB
  �ٷ���classB����������Ҫ����Class Bʹ��
* newclassA ��Ĭ��
  �ں�SDK��ܺ��ṩ��class A���������ṩ̽����������
* FwUpdate
  ��ԹⱦWSL305Sģ��ı�����������  
  
## CLASSB_ENABLED
ClassBʹ�ܿ��أ�����ͨ��-DCLASSB_ENABLED="ON"������Ĭ�Ϲرա�