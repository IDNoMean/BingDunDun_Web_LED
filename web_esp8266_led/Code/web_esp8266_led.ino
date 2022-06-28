#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <FS.h>  
#include <SPIFFS.h>  
#include <ArduinoJson.h>  
#include <stdlib.h>
#include <string.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN      4 // 设置灯带连接引脚

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 7 // 设置灯带灯珠数

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

WiFiMulti wifiMulti;     // 建立WiFiMulti对象
WebServer esp8266_server(80);    // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）

int mod = 1;
int r = random(0,255);
int g = random(0,255);
int b = random(0,255);
//int rpin = 4;
//int gpin = 2;
//int bpin = 3;

bool flag = true;
bool statue = true;
void handleUserRequet();
void lightType();
void rgb();
void led_breath();
void led_flash();
void setled(int rv,int gv,int bv);
bool handleFileRead(String path);

void setup() {
  Serial.begin(9600);          // 启动串口通讯
  Serial.println("");

  pixels.begin();
  // pixels.setBrightness(50)
  //pinMode(rpin,OUTPUT);
  // pinMode(gpin,OUTPUT);
  // pinMode(bpin,OUTPUT);
  
  wifiMulti.addAP("HMKJG", "hmkjg12345"); // 将需要连接的一系列WiFi ID和密码输入这里
  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2"); // ESP8266-NodeMCU再启动后会扫描当前网络
  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3"); // 环境查找是否有这里列出的WiFi ID。如果有
  Serial.println("Connecting ...");                            // 则尝试使用此处存储的密码进行连接。
  
  int i = 0;  
  while (wifiMulti.run() != WL_CONNECTED) { // 尝试进行wifi连接。
    delay(1000);
    Serial.print(i++); Serial.print(' ');
  }
  
  // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP

  if(SPIFFS.begin()){                       // 启动闪存文件系统
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }

  esp8266_server.begin();                           // 启动网站服务
  
  esp8266_server.onNotFound(handleUserRequet);      // 告知系统如何处理用户请求
  
  esp8266_server.on("/lightType",lightType);       //设置响应
  
  esp8266_server.on("/rgb",rgb);                   //设置响应
   
  Serial.println("HTTP server started");
}

void loop(void) {


  pixels.clear(); // Set all pixel colors to 'off'

  
  esp8266_server.handleClient();                    // 处理用户请求

  switch(mod){

    case 1:
     setled(r, g, b);
      
      break;
    case 2:
      rainbow(10);
      break;
    case 3:
      led_flash();
  }
 
}


void led_flash(){
  
    setled(r, g, b);
    delay(DELAYVAL*2);
    setled(0, 0, 0);
    delay(DELAYVAL*2);
}

void setled(int rv,int gv,int bv){

  
  for(int j=0; j<NUMPIXELS; j++) { // For each pixel...

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    pixels.setPixelColor(j, pixels.Color(rv,gv,bv));

      // Send the updated pixel colors to the hardware.
  }
  pixels.show(); 
}

void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 1*65536; firstPixelHue += 64) {
    for(int i=0; i<pixels.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue;
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
      if(statue != flag){break;}
    }
    pixels.show(); // Update strip with new contents
    if(statue != flag){break;}
    delay(wait);  // Pause for a moment
  }
  statue = flag;
}



void handleUserRequet() {         
     
  // 获取用户请求网址信息
  String webAddress = esp8266_server.uri();
  
  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(webAddress);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp8266_server.send(404, "text/plain", "404 Not Found"); 
  }
}


void lightType(){
  String type = esp8266_server.arg("type"); // 获取用户请求中的PWM数值
  Serial.print("type = ");
  Serial.println(type);
  
  if(type=="static"){
     mod = 1;
     flag = !flag;
  }else if(type=="breath"){
     mod = 2;
     flag = !flag;
  }else if(type=="flash"){
     mod = 3;
     flag = !flag;
  }
}

void rgb(){
  
  r = atoi(esp8266_server.arg(0).c_str() );
  Serial.print("r = ");
  Serial.println(r);

  g = atoi(esp8266_server.arg(1).c_str() );
  Serial.print("g = ");
  Serial.println(g);

  b = atoi(esp8266_server.arg(2).c_str() );
  Serial.print("b = ");
  Serial.println(b);
  
 }

bool handleFileRead(String path) {            //处理浏览器HTTP访问

  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(path);  // 获取文件类型
  
  if (SPIFFS.exists(path)) {                     // 如果访问的文件可以在SPIFFS中找到
    File file = SPIFFS.open(path, "r");          // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    return true;                                 // 返回true
  }
  return false;                                  // 如果文件未找到，则返回false
}

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
