//  ArkSystems IoT classroom sample program 2 of 1st session
// 
//  BME280から気温,気圧湿度高度を読んでシリアルポートに出力し、ThnkgSpeakへ送信
//
//  Thanks : Spark fun's Library and sample "I2C_ReadAllData.ino"
//  "BME280 Arduino and Teensy example
//   by Marshall Taylor @ SparkFun Electronics
//     May 20, 2015"
//     https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
//

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

#include "../../private_ssid.h"
//const char *ssid   = "*************";
//const char *pass   = "*************";
String      apiKey = "U09JSMUGWE0D92PX";

WiFiClient client;


//
//  時間
#include <Time.h>


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
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500); //  0.5秒
  }
  Serial.println("connected");
}




//
//  メインループ
//

int prevMinute = minute();

void loop() {
  //  BME280読み込み
  float temperature = mySensor.readTempC();
  float pressure    = mySensor.readFloatPressure() / 100.0;
  float altitude    = mySensor.readFloatAltitudeMeters();
  float humidity    = mySensor.readFloatHumidity();

  //  シリアルに出力
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" degrees C");

  Serial.print("Pressure: ");
  Serial.print(pressure, 2);
  Serial.println(" HPa");

  Serial.print("Altitude: ");
  Serial.print(altitude, 2);
  Serial.println("m");

  Serial.print("%RH: ");
  Serial.print(humidity, 2);
  Serial.println(" %");

  Serial.println();


  //  1分ごとにThingSpeakへ送出
  if (prevMinute != minute()) {
    char tBuf[10], hBuf[10], pBuf[10], aBuf[10];

    dtostrf(temperature, 7, 2, tBuf);
    dtostrf(humidity,    7, 2, hBuf);
    dtostrf(pressure,    7, 2, pBuf);

    String postStr = "&field1=" + String(tBuf) + "&field2=" + String(hBuf) + "&field3=" + String(pBuf);

    sendToThingSpeak(postStr);
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
