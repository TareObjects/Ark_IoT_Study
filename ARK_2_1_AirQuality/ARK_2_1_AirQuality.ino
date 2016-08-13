//  ArkSystems IoT classroom sample program 1 of 2nd session
//
//  空気質センサの値をA/Dコンバータから読み込んでシリアルに出力する
//


//
//  A/Dコンバータ読み込みなどシステム関数を使うための宣言
//
extern "C" {
  #include "user_interface.h"
}


//
//  Setup
//
void setup() {
  //
  //  Serial
  Serial.begin(115200);
}




//
//  メインループ
//

void loop() {
  //
  //  Read Analog to Digital Converter(ADC) and Send it to Serial
  int airQuality = system_adc_read();
  Serial.print("Air Quality = ");
  Serial.println(airQuality);
  
  delay(1000);
}

