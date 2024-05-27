//
//  920MHz LoRa/FSK RF module ES920LR3 
//  receive data display
//
//  MaiaR Create 2022/06/05
//  add RSSI 2023/01/20
//
//  Applicable model
//  * M5Stack BASIC    with Module/UNIT
//  * M5Stack CORE2    with Module/UNIT
//  * M5Stack ATOM S3  with KIT/UNIT
//

#ifdef ARDUINO_M5STACK_Core2
#include <M5Core2.h>
#define TSIZE 2
#elif defined ARDUINO_M5Stack_Core_ESP32
#include <M5Stack.h>
#define TSIZE 4
#define RPOS 180
#define SX 270
#define SY 35
#define SR 15
#elif defined ARDUINO_M5Stack_ATOMS3
#include <M5AtomS3.h>
#define TSIZE 2
#define RPOS 100
#define SX 100
#define SY 80
#define SR 8
#endif

#include "LoRa.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <string.h>

WebServer server(80);  // WebServerオブジェクトを作る

char mbuf[5000];
int sensor_number = -1;
float mx = -1;
float my = -1;
float dv = -1;
int16_t rssi = -1000;
time_t start_time;
double elapsed_time;
char rsv_data[400];
TFT_eSprite img = TFT_eSprite(&M5.Lcd);

void handleEnv() { // "/env"をアクセスされたときの処理関数
  char buf[5000];  // HTMLを編集する文字配列
  char buf2[400];

  strcat(mbuf, rsv_data);
  strcat(mbuf, "<br>");
  sprintf(buf,  // HTMLに温度と湿度を埋め込む
      "<html>\
      <head>\
          <title>M5Stack EvnServer</title>\
          <meta http-equiv=\"Refresh\" content=\"5\">\
      <head>\
      <body>\
          <h1>M5Stack EnvServer</h1>\
          <p>time, sensor_number, mx,  my,  fspeed</p>\
          <p>%s</p>\
      </body>\
      </html>",
  mbuf);
  server.send(200, "text/html", buf);
}

void handleNotFound() {  // 存在しないファイルにアクセスされたときの処理関数
    server.send(404, "text/plain", "File Not Found\n\n");
    M5.Lcd.println("File Not Found");
}

void handleRoot() {  // "/"をアクセスされたときの処理関数
    server.send(200, "text/plain", "hello from M5Stack!");
    M5.Lcd.println("accessed on \"/\"");
}

void setup() {
  String config_ini;
  String ssid;
  String password;

  Serial.begin(115200);
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(TSIZE);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.println("LoRa");
  LoRaInit();
  M5.Lcd.println("Display");
  M5.Lcd.println("Stand-by");
#ifdef ARDUINO_M5STACK_Core2
  M5.Spk.begin();
#elif defined ARDUINO_M5Stack_Core_ESP32
  M5.Speaker.begin();
  M5.Speaker.setVolume(2);
#endif

  if (!SD.begin())                                         // SDカードの初期化
      Serial.println("Card failed, or not present");          // SDカードが未挿入の場合の出力
  else
      Serial.println("microSD card initialized.");
      /* ファイルオープン */
  File datFile = SD.open("/set/setup.ini");
  if( datFile ){
      Serial.println("File open successful");
      /* サイズ分ループ */
      while(datFile.available()){
          config_ini = config_ini + datFile.readString();
      }
      /* SSID取得 */
      config_ini = config_ini.substring(config_ini.indexOf("#SSID\r\n") + 7);
      ssid = config_ini.substring(0, config_ini.indexOf("\r\n"));
      /* パスワード取得 */
      config_ini = config_ini.substring(config_ini.indexOf("#SSID_PASS\r\n") + 12);
      password = config_ini.substring(0, config_ini.indexOf("\r\n"));
      /* ファイルクローズ */
      datFile.close();
  } 
  else{
      Serial.println("File open error");
      ssid = "Buffalo-G-7780";
      password = "reen8ukdr7a83";
  }

  WiFi.begin(ssid.c_str(), password.c_str());// Wi-Fi APに接続する
  while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
    delay(500);
    M5.Lcd.print(".");
  }
  M5.Lcd.print("\r\nWiFi connected\r\nIP address: ");
  M5.Lcd.println(WiFi.localIP());
  Serial.print(WiFi.localIP());

  if (MDNS.begin("m5stack")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/env", handleEnv);
  server.onNotFound(handleNotFound);

  server.begin();
  M5.Lcd.println("HTTP server started");

  clock_t start_time, current_time;
  start_time = clock();

}

void loop(){
  if (Serial2.available() > 0) {
    String rxs = Serial2.readString();
    Serial.print(rxs);
    char Buf[5];
    rxs.toCharArray(Buf, 5);
    rssi = strtol(Buf,NULL,16);
    rxs = rxs.substring(4);
    M5.Lcd.fillScreen(BLACK);

    int buffer_len = rxs.length() + 1;
    char buffer[buffer_len];
    rxs.toCharArray(buffer, buffer_len);
    sscanf(buffer, "fs:%d mx:%f my:%f sp:%f", &sensor_number, &mx, &my, &dv);

    time_t current_time = time(NULL);
    elapsed_time = difftime(current_time, start_time);
    sprintf(rsv_data, "%.1f, %d, %.1f, %.1f, %.1f, %d", elapsed_time, sensor_number, mx, my, dv, rssi);
    //rssi -> dB

    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println(WiFi.localIP());
    M5.Lcd.setCursor(0, sensor_number*60);
    M5.Lcd.print(rsv_data);

    delay(400);
    server.handleClient();
  }
}
