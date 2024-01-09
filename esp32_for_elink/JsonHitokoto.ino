//****** 解析一言数据
void ParseHitokoto(String content, struct Hitokoto* data)
{
  DynamicJsonDocument json(1536); //分配内存,动态
  DeserializationError error = deserializeJson(json, content); //解析json
  //serializeJson(json, Serial);//构造序列化json,将内容从串口输出
  if (error)   //检查API是否有返回错误信息，有返回则返回错误信息
  {
    Serial.print("一言加载json配置失败:");
    Serial.println(error.c_str());
    Serial.println(" ");
    String z = "一言json配置失败:" + String(error.c_str()) + " " + content;
  }
  if (json["status_code"].isNull() == 0) //检查到不为空
  {
    strcpy(data->status_code, json["status_code"]);
    String z = "一言异常:" + String(yiyan.status_code);
    Serial.print("一言异常:"); Serial.println(yiyan.status_code);
  }
  else
  {
    if (json["hitokoto"].isNull() == 0){
      strcpy(data->hitokoto, json["hitokoto"]);
      } 
    else strcpy(data->hitokoto, "\"hitokoto\"没有数据");
  }
  // 复制字符串 ,先检查是否为空，空会导致系统无限重启
  // isNull()检查是否为空 空返回1 非空0
}
void ParseMindDay(String content, struct MindDay* data)
{
  DynamicJsonDocument json(128);//分配内存
  DeserializationError error = deserializeJson(json, content); //解析json
  serializeJson(json, Serial);//构造序列化json,将内容从串口输出
  if (error)
  {
    Serial.print("倒计时时间错误");
  }
  else
  {//{"Year":2024,"Month":2,"Day":4,"JieRi":"测试"}
  int Year = json["Year"]; // 2024
  int Month = json["Month"]; // 2
  int Day = json["Day"]; // 4
  //char* JieRi = json["JieRi"]; // "测试"
  data->year = Year;
  data->month = Month;
  data->day = Day;
  strcpy(data->festival,json["JieRi"]);
  //strcpy(data->year,json["Year"]);
  // strcpy(data->month,json["Month"]);
  // strcpy(data->day,json["Day"]);
  // strcpy(data->festival,json["JieRi"]);//提取出各个时间部分
  }
}