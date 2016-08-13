//  ArkSystems IoT classroom sample program 2 of 2nd session
//
//  PIRセンサの出力をGPIO 12から読み込んでシリアルに出力する
//


//
//  時間関連関数
//
#include <Time.h>
#include <TimeLib.h>

int prevMinute = -1;  //  分が変わったことを検知するための変数



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
//  Setup
//
void setup() {
  //
  //  Serial
  Serial.begin(115200);

  //
  //  GPIO
  pinMode(kPIRSensor, INPUT); // PIR Sensorを入力モードで使う

  //
  //  Interval Timer
  os_timer_setfn(&myTimer, timerCountPIR, NULL);  // myTimerを使い, 割り込みが起こったらtimerCountPIRを呼ぶ
  os_timer_arm(&myTimer, 1000, true);             // myTimerの割り込みは1000mSecごとに、繰り返し(true)起こす
  
  //
  //  fetch timing adjustment
  prevMinute = minute();
  while (prevMinute == minute()) {
    delay(100);
  }
  prevMinute = -1;
}




//
//  メインループ
//
void loop() {
  if (prevMinute != minute()) {
    if (prevMinute != -1) {
      Serial.print("PIR Counter = ");
      Serial.println(pirCounter);
      pirCounter = 0;
    }
    pirCounter = 0;
    prevMinute = minute();
  }

  if (digitalRead(kPIRSensor) == HIGH) {
    Serial.println("HIGH!");
  } else {
    Serial.println("");
  }
  
  delay(1000);
}



//
//  タイマー割り込みでPIRがHIGHならカウントアップ
//
void timerCountPIR(void *temp) {
  if (digitalRead(kPIRSensor) == HIGH) {
    pirCounter ++;
  }
}

