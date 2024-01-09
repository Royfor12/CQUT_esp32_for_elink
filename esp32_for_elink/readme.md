# 基于ESP32的多功能电子日历 程序使用说明

🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻🧑‍💻

* 主程序为==esp32_for_elink.ion==
* ==displayCode.c==将数据显示在屏幕上
* ==Json**.c==用于处理获取到的Json数据
* ==gatData.ino==用于通过URL获取API返回的数据
* ==Webserver.h==用于Wi-Fi连接与强制门户认证
* ==CallHttps.ino==中封装了callHttps函数提供给==gatData.ino==中使用:facepunch:
* ==PubSubClient.h/.cpp==均为巴法云提供,用于连接MQTT服务
* ==imageData.c/.h==为对图片取模得到的数据，用于==displayCode.c==显示
* ==gb2312.c==与==u8g2**.c.h==为字库数据，==gb2312.c==包含更多的**汉字**,其余的有更加丰富的**字体与字号**:sweat_drops:

## ArduinoJson 版本为6.19.1
