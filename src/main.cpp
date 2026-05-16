#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// STBY/使能
static const int STBY = 23;

// 左轮（A通道）
static const int PWMA = 13;  // 左轮PWM调速
static const int AIN1 = 14;  // 左轮方向1
static const int AIN2 = 27;  // 左轮方向2

// 右轮（B通道）
static const int PWMB = 12;  // 右轮PWM调速
static const int BIN1 = 26;  // 右轮方向1
static const int BIN2 = 25;  // 右轮方向2
// ===================================

// PWM 参数
static const int PWM_FREQ = 20000;
static const int PWM_RES  = 8;      // 0~255
static const int CH_A = 0;
static const int CH_B = 1;

int speedVal = 180; // 默认速度 0~255

// dir: 1=前进, -1=后退, 0=停止(滑行)
void setMotor(int in1, int in2, int pwmCh, int pwm, int dir) {
  pwm = constrain(pwm, 0, 255);

  if (dir > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(pwmCh, pwm);
  } else if (dir < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(pwmCh, pwm);
  } else {
    ledcWrite(pwmCh, 0);
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void stopCar() {
  setMotor(AIN1, AIN2, CH_A, 0, 0);
  setMotor(BIN1, BIN2, CH_B, 0, 0);
}

void forward() {
  setMotor(AIN1, AIN2, CH_A, speedVal,  1);
  setMotor(BIN1, BIN2, CH_B, speedVal,  1);
}

void backward() {
  setMotor(AIN1, AIN2, CH_A, speedVal, -1);
  setMotor(BIN1, BIN2, CH_B, speedVal, -1);
}

void leftTurn() {
  // 原地左转：左轮后退、右轮前进
  setMotor(AIN1, AIN2, CH_A, speedVal, -1);
  setMotor(BIN1, BIN2, CH_B, speedVal,  1);
}

void rightTurn() {
  setMotor(AIN1, AIN2, CH_A, speedVal,  1);
  setMotor(BIN1, BIN2, CH_B, speedVal, -1);
}

void handleCommand(char c) {
  // 调速：0~9
  if (c >= '0' && c <= '9') {
    int level = c - '0';
    speedVal = map(level, 0, 9, 80, 255);
    Serial.printf("Speed=%d\n", speedVal);
    SerialBT.printf("Speed=%d\n", speedVal);
    return;
  }

  switch (c) {
    case 'F': forward();  break;
    case 'B': backward(); break;
    case 'L': leftTurn(); break;
    case 'R': rightTurn();break;
    case 'S': stopCar();  break;
    default: break; // 忽略换行等
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH); // 解除TB6612待机

  ledcSetup(CH_A, PWM_FREQ, PWM_RES);
  ledcSetup(CH_B, PWM_FREQ, PWM_RES);
  ledcAttachPin(PWMA, CH_A);
  ledcAttachPin(PWMB, CH_B);

  stopCar();

  const char* btName = "ESP32_CAR";
  SerialBT.begin(btName);
  Serial.printf("Bluetooth started: %s\n", btName);
  Serial.println("Commands: F,B,L,R,S and 0-9 for speed.");
}

void loop() {
  if (SerialBT.available()) {
    handleCommand((char)SerialBT.read());
  }

  // 串口也能控制
  if (Serial.available()) {
    handleCommand((char)Serial.read());
  }
}