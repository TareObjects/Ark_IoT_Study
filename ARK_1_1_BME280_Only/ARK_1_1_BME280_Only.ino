//  ArkSystems IoT classroom sample program 1 of 1st session
// 
//  BME280から気温,気圧湿度高度を読んでシリアルポートに出力
//
//  2016-11-04 : fix i2c address. bme280's default address was different.
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
}

void loop() {
  // put your main code here, to run repeatedly:

  float temperature = mySensor.readTempC();
  float pressure    = mySensor.readFloatPressure() / 100.0;
  float altitude    = mySensor.readFloatAltitudeMeters();
  float humidity    = mySensor.readFloatHumidity();

  
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
  
  delay(1000);
}
