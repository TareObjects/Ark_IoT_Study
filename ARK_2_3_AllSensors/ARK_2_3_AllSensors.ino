//  ArkSystems IoT classroom sample program 3 of 2nd session
//
//  BME280から気温,気圧湿度高度,PIRから1分間の動き, 空気質センサー…を読んでシリアルポートに出力し、
//  OLEDへ表示、1分ごとにThnkgSpeakへ送信
//
//  Thanks : Spark fun's Library and sample "I2C_ReadAllData.ino"
//  "BME280 Arduino and Teensy example
//   by Marshall Taylor @ SparkFun Electronics
//     May 20, 2015"
//     https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
//
//  Thanks : squix78's esp8266-oled-ssd1306 Library
//    The MIT License (MIT)
//    Copyright (c) 2016 by Daniel Eichhorn
//    Copyright (c) 2016 by Fabrice Weinberg
//    https://github.com/squix78/esp8266-oled-ssd1306/blob/master/examples/SSD1306UiDemo/SSD1306UiDemo.ino
//


//  BME280
#include "SparkFunBME280.h"
#include "Wire.h"
#include "SPI.h"

BME280 mySensor;



//
//  WiFi
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

//#include "../../private_ssid.h"
const char *ssid       = "*************";
const char *password   = "*************";
String apiKeyArk       = "*************";

WiFiClient client;


//
//  時間
#include <Time.h>
#include <TimeLib.h>

int prevMinute = -1;  //  分単位の処理をするためのフラグ


//
//  OLED
//
#include "SSD1306.h"

SSD1306   display(0x3c, 4, 5);



//
//  PIR Sensor
//
const int kPIRSensor = 12;
volatile int pirCounter = 0;  // volatileは最適化せずメモリに置いとけというコンパイラへの指示
// 割り込みの中でアクセスする変数などに指定。


//
//  タイマー割り込み関連
//

extern "C" {
#include "user_interface.h"
}

os_timer_t myTimer;

//
//  AirQuality
//


//
//  Setup
//
void setup() {
  //
  //  Serial
  Serial.begin(115200);

  //
  //  BME280
  //
  //  設定パラメータをセット
  mySensor.settings.commInterface = I2C_MODE;
  mySensor.settings.I2CAddress = 0x76;
  mySensor.settings.runMode = 3; //Normal mode
  mySensor.settings.tStandby = 0;
  mySensor.settings.filter = 0;
  mySensor.settings.tempOverSample = 1;
  mySensor.settings.pressOverSample = 1;
  mySensor.settings.humidOverSample = 1;

  // 検出開始
  Serial.println(mySensor.begin(), HEX);


  //
  //  PIR
  pinMode(kPIRSensor, INPUT); // PIR Sensorを入力モードで使う

  prevMinute = -1;

  //  Interval Timer
  os_timer_setfn(&myTimer, timerCountPIR, NULL);  // myTimerを使い, 割り込みが起こったらtimerCountPIRを呼ぶ
  os_timer_arm(&myTimer, 1000, true);             // myTimerの割り込みは1000mSecごとに、繰り返し(true)起こす


  //
  //  WiFi
  //
  //  接続＆接続確認
  Serial.print("WiFi Connectiong");
  WiFi.begin(ssid, password);
  //  while (WiFi.status() != WL_CONNECTED) {
  //    Serial.print(".");
  //    delay(500); //  0.5秒
  //  }
  //  Serial.println("connected");

  //
  //  OLED
  display.init();
  display.flipScreenVertically();
  display.displayOn();
  display.clear();
}




//
//  メインループ
//

void loop() {
  //
  //  BME280読み込み
  float temperature = mySensor.readTempC();
  float pressure    = mySensor.readFloatPressure() / 100.0;
  float altitude    = mySensor.readFloatAltitudeMeters();
  float humidity    = mySensor.readFloatHumidity();

  //
  //  文字列へ変換
  char temperatureBuf[10], humidityBuf[10], pressureBuf[10];
  dtostrf(temperature, 7, 2, temperatureBuf);
  dtostrf(humidity,    7, 2, humidityBuf);
  dtostrf(pressure,    7, 2, pressureBuf);

  //  空気質センサーの読みこみと文字列へ変換
  int airQuality = system_adc_read();
  char airQualityBuf[10];
  itoa(airQuality, airQualityBuf, 10); //  airQuality to airQuality Buf, in Decimal

  char pirBuf[10];
  //
  //  1分ごとに処理
  if (prevMinute != minute()) {
    pirBuf[0] = 0;  //  c文字列を空に
    if (prevMinute != -1) { //  最初の1回は飛ばす
      //
      //  PIRの値を文字列に変換
      itoa(pirCounter, pirBuf, 10); // air to pirBuffer, in Decimal

      //  変換し終わったら素早くカウンタを0にリセット
      pirCounter = 0;

      //
      //  人間の動きっぷりをシリアルへ出力
      Serial.print("Movement: ");
      Serial.print(pirBuf);
      Serial.println(" s/min");


      //  Thing Speak　
      String postStr = "&field1=" + String(temperatureBuf) + "&field2=" + String(humidityBuf)
                       + "&field3=" + String(pressureBuf)
                       + "&field4=" + String(airQualityBuf)   + "&field5=" + String(pirBuf);

      sendToThingSpeak(postStr);
    }

    prevMinute = minute();
  }



  //  シリアルに出力
  Serial.print("Temperature: ");
  Serial.print(temperatureBuf);
  Serial.println(" degrees C");

  Serial.print("Pressure: ");
  Serial.print(pressureBuf);
  Serial.println(" HPa");

  Serial.print("Altitude: ");
  Serial.print(altitude, 2);
  Serial.println("m");

  Serial.print("%RH: ");
  Serial.print(humidityBuf);
  Serial.println(" %");

  Serial.print("Air Quality: ");
  Serial.println(airQualityBuf);

  Serial.println();


  //
  //  OLEDへ表示
  //  OLEDへ表示
  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString( 0,  0, "Temperature");
  display.drawString( 0, 16, "Humidity");
  display.drawString( 0, 32, "Pressure");
  display.drawString( 0, 48, "Air/Move");

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);

  display.drawString(127,  0, temperatureBuf);
  display.drawString(127, 16, humidityBuf);
  display.drawString(127, 32, pressureBuf);
  display.drawString( 96, 48, airQualityBuf);
  display.drawString(127, 48, pirBuf);
  display.display();

  delay(1000);
}


void sendToThingSpeak(String inPostStr) {
  Serial.print("Connecting...");
  if (client.connect("184.106.153.149", 80)) {  //  api.thingspeak.com
    Serial.print("Connected....");
    String postStr = apiKeyArk + inPostStr + "\r\n\r\n";
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKeyArk + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("posted.");
  }
  client.stop();
}



//
//  タイマー割り込みでPIRがHIGHならカウントアップ
//
void timerCountPIR(void *temp) {
  if (digitalRead(kPIRSensor) == HIGH) {
    pirCounter ++;
  }
}
