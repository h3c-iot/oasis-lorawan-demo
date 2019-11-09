# oasis-lorawan-sdk
绿洲LoRaWAN终端接入SDK是新华三提供的基于LoRaWAN终端提供快速接入[**绿洲**](https://oasis.h3c.com/)的软件开发套件。

## 一、功能简介
SDK基于[LoRaMAC-node4.4.2](https://github.com/Lora-net/LoRaMac-node)开发, 扩展LoRaMAC-node的功能, 同时融入第三方模组的功能, 在此基础上提供绿洲增值功能和应用案例。

### 1. 支持的开发板

* NucleoL073 

### 2. 支持的频段

* CN470-510MHz

### 3. 应用案例

* newClassA 

根据SDK提供的classA 案例代码, 提供信道探测入网, 数据收发, 设备保活等功能

* FwUpdate 

基于本地串口, 使用xmodel协议对光宝WSL305模组进行升级的案例

### 4. 支持的模组

* WSL305S 

光宝WSL305S(L)模组是基于AT指令控制的lora模组

* AcsipS78F 

群登S78F是基于SPI控制的lora模组

## 二、后续计划
* 绿洲增值服务功能

* 低功耗实现

* 适配Class C的案例

* 空中升级

* 更多的开发板支持

## 三、版本修订记录

### 2019-11-08, V1.0.0
  1. 提供LADAPTER框架适配底层SPI和AT控制模组
  
  2. 提供基于SDK的class A案例
  
  3. 提供信道探测入网案例
  
  4. 提供光宝wsl305s模组升级功能以及案例



