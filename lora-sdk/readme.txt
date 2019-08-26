SDK-+
    |
    +--- apps : LoRaWAN之上的应用层协议，包含OASIS平台接入协议
    |
    +--- example : 案例代码，提供跟设备相关的BSP、HAL代码，OS代码以及main.c的案例代码
    |
    +--- loramac : 标准SPI控制SX127x/SX126x系列芯片
    |
    +--- lorawan_adapt : 屏蔽底层是标准的SPI控制芯片或通过AT指令控制模组
    |
    +--- system : SDK运行的支持模块，移植到一块新的设备上要实现system下的所有接口即可运行起来
    |
    +--- wsl30x : 光宝WSL305模组的控制指令