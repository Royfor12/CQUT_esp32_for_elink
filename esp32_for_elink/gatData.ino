//用于获取各种信息
void GetData()
{
  //拼装实况天气API地址
  url_ActualWeather = "https://api.seniverse.com/v3/weather/now.json";
  url_ActualWeather += "?key=" + xz_code;
  url_ActualWeather += "&location=ip" ;
  url_ActualWeather += "&language=" + language;
  url_ActualWeather += "&unit=c";
  //拼装实况未来API地址
  url_FutureWeather = "https://api.seniverse.com/v3/weather/daily.json";
  url_FutureWeather += "?key=" + xz_code;
  url_FutureWeather += "&location=ip";
  url_FutureWeather += "&language=" + language;
  url_FutureWeather += "&unit=c";
  url_FutureWeather += "&start=0";
  url_FutureWeather += "&days=3";
  //拼装生活指数
  String url_LifeIndex = "https://api.seniverse.com/v3/life/suggestion.json";
  url_LifeIndex += "?key=" + xz_code;
  url_LifeIndex += "&location=ip";
  //新闻地址
  String url_News="";
  if(udc%3==0){
    url_News="https://api.vvhan.com/api/hotlist?type=wbHot";//微博
  }else if(udc%3==1){
    url_News = "https://api.vvhan.com/api/hotlist?type=baiduRD";//央视新闻
  }else if(udc%3==2){
    url_News = "https://api.vvhan.com/api/hotlist?type=zhihuHot";//知乎
  }
//5小时更新一次天气 省电且节约次数10*30=300/60=5 
  if(udc%30==0){
    ParseActualWeather(callHttps(url_LifeIndex), &life_index); //获取生活指数
    ParseActualWeather(callHttps(url_ActualWeather), &actual);
    ParseFutureWeather(callHttps(url_FutureWeather), &future);
  }
  ParseHitokoto(callHttps(url_yiyan), &yiyan);//获取每日一言
  ParseNews(callHttps(url_News),&xinwen);//获取新闻
  //日程的获取通过MQTT中断获取，故不在此处
}
