#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_3C.h>
//#include <GxEPD2_BW.h>//适用于中景园电子的屏幕，黑白
#include <ArduinoJson.h>
#include <Timezone.h>
//GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(GxEPD2_420_GDEY042T81(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); GDEY042T81, 400x300, SSD1683 (no inking)
GxEPD2_3C<GxEPD2_420c, GxEPD2_420c::HEIGHT> display(GxEPD2_420c(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));  // GDEW042Z15
#include "gb2312.c"
#include "u8g2_mflansong_36_gb2312a.h"
#include "u8g2_mflansong_24_gb2312a.h"
#include "imagedata.h"
#include "Webserver.h"
#include "PubSubClient.h"  //MQTT
#include <dht11.h>
#include <NTPClient.h>
#include <WiFi.h>  // for WiFi shield
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <UnixTime.h>
extern const uint8_t chinese_city_gb2312[239032] U8G2_FONT_SECTION("chinese_city_gb2312");
#define secondsPerDay 86400;
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
//中断配置
const byte interruptPin_0 = 14;     //这就是我们设置中断的目标对应的那个引脚
const int inputPin = 33;            //输入引脚用于检测5v
const int outputPin1 = 32;          //控制无线充电
const int outputPin2 = 26;          //控制有线充电
//portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  //声明一个portMUX_TYPE类型的变量，利用其对主代码和中断之间的同步进行处理
//******MQTT服务配置**********
const char* topic = "esp32calender";                //主题名字，可在巴法云控制台自行创建，名称随意
#define ID_MQTT "e2a9e2c5559e48afa9c7eb4145002640"  //修改，你的Client ID
const char* mqtt_server = "bemfa.com";              //默认，MQTT服务器
const int mqtt_server_port = 9501;                  //默认，MQTT服务器
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;  //计时
//****** HTTP服务器配置 ******
String language = "zh-Hans";                   // 请求语言
String url_yiyan = "https://v1.hitokoto.cn/";  //一言获取地址
String url_ActualWeather;                      //实况天气地址
String url_FutureWeather;                      //未来天气地址
//****** 天气数据
//我们要从此网页中提取的数据的类型
struct ActualWeather {
  char status_code[64];   //错误代码
  char city[16];          //城市名称
  char weather_name[16];  //天气现象名称
  char weather_code[4];   //天气现象代码
  char temp[5];           //温度
  char last_update[25];   //最后更新时间
};
ActualWeather actual;  //创建结构体变量 目前的
struct FutureWeather {
  char status_code[64];  //错误代码
  char date0[14];             //今天日期
  char date0_text_day[20];    //白天天气现象名称
  char date0_code_day[4];     //白天天气现象代码
  char date0_text_night[16];  //晚上天气现象名称
  char date0_code_night[4];   //晚上天气现象代码
  char date0_high[5];         //最高温度
  char date0_low[5];          //最低温度
  char date0_humidity[5];     //相对湿度
  char date0_wind_scale[5];   //风力等级
  char date1[14];             //明天日期
  char date1_text_day[20];    //白天天气现象名称
  char date1_code_day[4];     //白天天气现象代码
  char date1_text_night[16];  //晚上天气现象名称
  char date1_code_night[4];   //晚上天气现象代码
  char date1_high[5];         //最高温度
  char date1_low[5];          //最低温度

  char date2[14];             //后天日期
  char date2_text_day[20];    //白天天气现象名称
  char date2_code_day[4];     //白天天气现象代码
  char date2_text_night[16];  //晚上天气现象名称
  char date2_code_night[4];   //晚上天气现象代码
  char date2_high[5];         //最高温度
  char date2_low[5];          //最低温度
};
FutureWeather future;  //创建结构体变量 未来
struct LifeIndex       //生活指数
{
  char status_code[64];  //错误代码
  char uvi[10];          //紫外线指数
};
LifeIndex life_index;  //创建结构体变量 未来
struct News            //新闻API
{
  char status_code[64];  //错误代码
  char title[11][64];
};
struct Hitokoto  //一言API
{
  char status_code[64];  //错误代码
  char hitokoto[64];
};
struct MindDay  //倒计时未来时间
{
  char festival[64];  //节日
  uint16_t year;//年
  uint8_t month;//月
  uint8_t day;//日
};
//****** 一些变量 ******
String webServer_news = " ";
boolean night_updated = 1;  //夜间更新 1-不更新 0-更新 为了省电还是不更新吧
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com");  //NTP服务器地址
int k = 0;          //用于判断页面
int FLAG_KEYIT = 0; //按键
int FLAG_CHARGE = 0;//充电
//RTC临时数据
#define RTC_hour_dz 0            //小时地址
#define RTC_night_count_dz 1     //夜间计数地址
#define RTC_peiwang_state_dz 2   //配网状态地址
uint32_t RTC_hour = 100;         //小时
uint32_t RTC_night_count = 0;    //24-6点，夜间不更新计数
int32_t night_count_max = 0;     //需要跳过几次
uint32_t RTC_peiwang_state = 0;  //配网状态 1-需要
Hitokoto yiyan;                          //创建结构体变量 一言
News xinwen;                             //创建结构体变量 新闻
MindDay jieri = { "元旦", 2024, 1, 1 };  //创建结构体变量 节日
//测试
int udc = 0;              //更新次数记录
unsigned long epochTime;  //当前时间戳
UnixTime stamp(8);        //设置为北京时间 GMT 8

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID_MQTT)) {
      Serial.println("connected");
      Serial.print("subscribe:");
      Serial.println("esp32calender");
      //订阅主题，如果需要订阅多个主题，可发送多条订阅指令client.subscribe(topic2);client.subscribe(topic3);
      client.subscribe("esp32calender");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
/*中断函数*/
//MQTT接收中断
String msg = "";
void callback(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Topic:");//topic消息接收
  // Serial.println(topic);
  msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  // Serial.print("Msg:");
  // Serial.println(msg);
  FLAG_KEYIT++;
}
//充电检测中断
void IRAM_ATTR handleInput() {
  if (digitalRead(inputPin) == 1) {
    FLAG_CHARGE = 1;  //启用有线
  } else if (digitalRead(inputPin == 0)){
    FLAG_CHARGE = 2;  //启用无线
  }
}
//按键中断回调函数
void handleInterrupt() {
  //delay(20);  //延时20ms作为消抖，由于经过测试，中断回调函数中加入了延时有几率触发重启，故弃用，采用消抖电容
  if (digitalRead(interruptPin_0) == 0)  //因为是下拉触发，所以在消抖时间完后读取引脚高低电平，如果还是为低那么就代表出现了一次稳定的中断
  {
    FLAG_KEYIT++;
  }
}
void setup() {
  Serial.begin(115200);
  EEPROM.begin(4096);
  display.init();
  u8g2Fonts.begin(display);  //将u8g2连接到display
  display.firstPage();
  display.display(1);
  pinMode(interruptPin_0, INPUT_PULLUP);                                             //先把引脚设置为上拉输入
  pinMode(inputPin, INPUT_PULLDOWN);                                                 // 设置D33为输入并启用内部下拉
  pinMode(outputPin1, OUTPUT);                                                       // 设置D32为输出
  pinMode(outputPin2, OUTPUT);                                                       // 设置D26为输出
  digitalWrite(outputPin1, HIGH);                                                    //D32高电平启用无线充电
  digitalWrite(outputPin2, LOW);                                                     //D26低电平屏蔽有线充电 默认启用无线
  attachInterrupt(digitalPinToInterrupt(interruptPin_0), handleInterrupt, FALLING);  //按键中断切换页面
  attachInterrupt(digitalPinToInterrupt(inputPin), handleInput, CHANGE);             // 在D33上设置中断处理函数
  wifi_init();
  //NTP服务 启动！
  timeClient.begin();
  timeClient.setTimeOffset(28800);  // + 1区 偏移3600， +8区 ：3600×8 = 28800
  //MQTT  启动！
  client.setServer(mqtt_server, mqtt_server_port);
  client.setCallback(callback);
}

void loop() {
  if (settingMode)  //联网判断
  {
    dnsServer.processNextRequest();
  }
  webServer.handleClient();
  //Serial.println(settingMode);
  if (!settingMode) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    //获取时间
    timeClient.update();
    epochTime = timeClient.getEpochTime();
    // Serial.print("Epoch Time: ");
    // Serial.println(epochTime);//串口输出当时的Unix时间戳
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int weekDay = timeClient.getDay();
    struct tm* ptm = gmtime((time_t*)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    long now = millis();
    if (k == 0 ||(now - lastMsg > 600000))  //10分钟 刷新一次
    {
      lastMsg = now;
      k = 1;
      FLAG_KEYIT = 0;
      GetData();
      // 开始显示
      display.fillScreen(GxEPD_WHITE);
      get_weather();
      timedisplay(currentMonth, monthDay, weekDay, currentHour, currentMinute);
      hitokoto();
      newsdisplay();
      gettem();
      //特殊图标测试
      u8g2Fonts.setFont(u8g2_font_siji_t_6x10);
      u8g2Fonts.setForegroundColor(GxEPD_BLACK);  // 设置前景色
      u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // 设置背景色
      u8g2Fonts.drawGlyph(382, 30, 0x0e21a);      //显示Wi-Fi图标
      display.nextPage();
      udc++;//用于新闻内容的切换，10分钟++，一天24*60/10=144次，用个一两年问题不大不重启
      Serial.println("页面一 显示完毕 ##");
      Serial.println(FLAG_KEYIT);
    }
    if (k == 1&&FLAG_KEYIT == 1) {
      // 由于无法做到局部刷新，故在展示日程倒计时时，不得不对一言以及天气等进行刷新
      // Serial.println("0号中断对应的引脚发生中断!");
      k = 2;
      display.fillScreen(GxEPD_WHITE);
      get_weather();
      timedisplay(currentMonth, monthDay, weekDay, currentHour, currentMinute);
      hitokoto();
      ParseMindDay(msg, &jieri);  //json 解析
      displayday(epochTime, jieri, currentHour, currentMinute);
      gettem();
      u8g2Fonts.setFont(u8g2_font_siji_t_6x10);
      u8g2Fonts.setForegroundColor(GxEPD_BLACK);  // 设置前景色
      u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // 设置背景色
      u8g2Fonts.drawGlyph(382, 30, 0x0e21a);      //显示Wi-Fi图标
      display.nextPage();
      //Serial.print("over");
      // FLAG_KEYIT = 0;
      Serial.println("页面二 显示完毕 ##");
      Serial.println(FLAG_KEYIT);
    }else if (FLAG_KEYIT == 2){
      k = 0;
      Serial.println(FLAG_KEYIT);

      
    }
    //充电选择判断
    if (FLAG_CHARGE == 1) {
      digitalWrite(outputPin1, LOW);
      digitalWrite(outputPin2, HIGH);
      // Serial.println("启用有线充电");
      FLAG_CHARGE = 0;
    } else if (FLAG_CHARGE == 2) {
      digitalWrite(outputPin1, HIGH);
      digitalWrite(outputPin2, LOW);
      // Serial.println("启用无线充电");
      FLAG_CHARGE = 0;
    }
  }
  delay(1000);  //延时1s
 /*深度睡眠会导致内存中数据丢失让新闻切换出问题，故弃用
 * esp_sleep_enable_timer_wakeup(20000000);
esp_deep_sleep_start();*/
};
