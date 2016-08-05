//  ArkSystems IoT classroom sample program 3 of 1st session
//
//  BME280から気温,気圧湿度高度を読んでシリアルポートに出力し、OLEDへ表示、1分ごとにThnkgSpeakへ送信
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
//  Thanks : Paul Stoffregen's Time Library
//    time.c :   Copyright (c) Michael Margolis 2009-2014
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//    https://github.com/PaulStoffregen/Time
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

//  #include "../../private_ssid.h"
const char *ssid       = "*************";
const char *password   = "*************";
String      apiKey = "U09JSMUGWE0D92PX";

WiFiClient client;


//
//  時間
#include <Time.h>
#include <TimeLib.h>


//
//  OLED
//
//#include <OLEDDisplay.h>
//#include <OLEDDisplayFonts.h>
//#include <OLEDDisplayUi.h>
//#include <SH1106.h>
//#include <SH1106Brzo.h>
//#include <SH1106Wire.h>
#include <SSD1306.h>
//#include <SSD1306Brzo.h>
//#include <SSD1306Spi.h>
//#include <SSD1306Wire.h>

SSD1306   display(0x3c, 4, 5);

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
  mySensor.settings.I2CAddress = 0x77;
  mySensor.settings.runMode = 3; //Normal mode
  mySensor.settings.tStandby = 0;
  mySensor.settings.filter = 0;
  mySensor.settings.tempOverSample = 1;
  mySensor.settings.pressOverSample = 1;
  mySensor.settings.humidOverSample = 1;

  // 検出開始
  Serial.println(mySensor.begin(), HEX);


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
  Serial.println("connected");

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

int prevMinute = minute();

void loop() {
  //
  //  BME280読み込み
  float temperature = mySensor.readTempC();
  float pressure    = mySensor.readFloatPressure() / 100.0;
  float altitude    = mySensor.readFloatAltitudeMeters();
  float humidity    = mySensor.readFloatHumidity();

  //
  //  文字列へ変換
  char tBuf[10], hBuf[10], pBuf[10], aBuf[10];
  dtostrf(temperature, 7, 2, tBuf);
  dtostrf(humidity,    7, 2, hBuf);
  dtostrf(pressure,    7, 2, pBuf);

  //  シリアルに出力
  Serial.print("Temperature: ");
  Serial.print(tBuf);
  Serial.println(" degrees C");

  Serial.print("Pressure: ");
  Serial.print(pBuf);
  Serial.println(" HPa");

  Serial.print("Altitude: ");
  Serial.print(altitude, 2);
  Serial.println("m");

  Serial.print("%RH: ");
  Serial.print(hBuf);
  Serial.println(" %");

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

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);

  display.drawString(127,  0, tBuf);
  display.drawString(127, 16, hBuf);
  display.drawString(127, 32, pBuf);
  display.display();


  //
  //  1分ごとにThingSpeakへ送出
  if (prevMinute != minute()) {

    if (WiFi.status() == WL_CONNECTED) {
      String postStr = "&field1=" + String(tBuf) + "&field2=" + String(hBuf) + "&field3=" + String(pBuf);

      sendToThingSpeak(postStr);
    }
    prevMinute = minute();
  }

  delay(1000);
}


void sendToThingSpeak(String inPostStr) {
  Serial.print("Connecting...");
  if (client.connect("184.106.153.149", 80)) {  //  api.thingspeak.com
    Serial.print("Connected....");
    String postStr = apiKey + inPostStr + "\r\n\r\n";
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("posted.");
  }
  client.stop();
}