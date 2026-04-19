#include <Arduino.h>

// Front Wheels
#define PWMA_1 4   // Wheel 1 PID (Speed)
#define AIN1_1 5   // Wheel 1 DIR
#define AIN2_1 6   // Wheel 1 DIR
#define PWMB_1 7   // Wheel 2 PID (Speed)
#define BIN1_1 15  // Wheel 2 DIR
#define BIN2_1 16  // Wheel 2 DIR
#define STBY   12  // TB6612 Enable

// Back Wheels
#define PWMA_2 17  // Wheel 3 PID (Speed)
#define AIN1_2 18  // Wheel 3 DIR
#define AIN2_2 8   // Wheel 3 DIR
#define PWMB_2 9   // Wheel 4 PID (Speed)
#define BIN1_2 10  // Wheel 4 DIR
#define BIN2_2 11  // Wheel 4 DIR

#define ENC_A_1 1  // PID feedback W1
#define ENC_B_1 2  
#define ENC_A_2 41 // PID feedback W2
#define ENC_B_2 42 
#define ENC_A_3 39 // PID feedback W3
#define ENC_B_3 40 
#define ENC_A_4 37 // PID feedback W4
#define ENC_B_4 38 

// --- Rotor Motor (BTS7960) ---
#define RPWM   13  // BTS7960 rotor speed/dir
#define LPWM   14  // BTS7960 rotor speed/dir
#define R_EN   21  // BTS7960 enable Right
#define L_EN   47  // BTS7960 enable Left


#define LED_PIN 48

void setRotorSpeed(int speed) {
  if (speed > 0) {
    analogWrite(LPWM, 0);
    analogWrite(RPWM, speed);
  } else if (speed < 0) {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, -speed);
  } else {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
  }
}

void setWheel1(int speed) {
  if (speed > 0) {
    digitalWrite(AIN1_1, HIGH);
    digitalWrite(AIN2_1, LOW);
    analogWrite(PWMA_1, speed);
  } else if (speed < 0) {
    digitalWrite(AIN1_1, LOW);
    digitalWrite(AIN2_1, HIGH);
    analogWrite(PWMA_1, -speed);
  } else {
    digitalWrite(AIN1_1, LOW);
    digitalWrite(AIN2_1, LOW);
    analogWrite(PWMA_1, 0);
  }
}


void setup() {
  Serial.begin(115200);
  
  pinMode(PWMA_1, OUTPUT);
  pinMode(AIN1_1, OUTPUT);
  pinMode(AIN2_1, OUTPUT);
  pinMode(PWMB_1, OUTPUT);
  pinMode(BIN1_1, OUTPUT);
  pinMode(BIN2_1, OUTPUT);
  pinMode(STBY, OUTPUT);

  pinMode(PWMA_2, OUTPUT);
  pinMode(AIN1_2, OUTPUT);
  pinMode(AIN2_2, OUTPUT);
  pinMode(PWMB_2, OUTPUT);
  pinMode(BIN1_2, OUTPUT);
  pinMode(BIN2_2, OUTPUT);

  pinMode(ENC_A_1, INPUT_PULLUP);
  pinMode(ENC_B_1, INPUT_PULLUP);
  pinMode(ENC_A_2, INPUT_PULLUP);
  pinMode(ENC_B_2, INPUT_PULLUP);
  pinMode(ENC_A_3, INPUT_PULLUP);
  pinMode(ENC_B_3, INPUT_PULLUP);
  pinMode(ENC_A_4, INPUT_PULLUP);
  pinMode(ENC_B_4, INPUT_PULLUP);

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);


  pinMode(LED_PIN, OUTPUT);

  
  digitalWrite(STBY, HIGH); 
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  
  
  setWheel1(0);
  setRotorSpeed(0);
 
  Serial.println("ESP32 IoT Mower Initialized.");
}



void loop() {
  delay(10); 
}
