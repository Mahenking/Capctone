#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==========================================
// 1. PIN DEFINITIONS
// ==========================================

// --- Front Wheels ---
#define PWMA_1 4   // Wheel 1 PID (Speed)
#define AIN1_1 5   // Wheel 1 DIR
#define AIN2_1 6   // Wheel 1 DIR
#define PWMB_1 7   // Wheel 2 PID (Speed)
#define BIN1_1 15  // Wheel 2 DIR
#define BIN2_1 16  // Wheel 2 DIR
#define STBY   12  // TB6612 Enable

// --- Back Wheels ---
#define PWMA_2 17  // Wheel 3 PID (Speed)
#define AIN1_2 18  // Wheel 3 DIR
#define AIN2_2 8   // Wheel 3 DIR
#define PWMB_2 9   // Wheel 4 PID (Speed)
#define BIN1_2 10  // Wheel 4 DIR
#define BIN2_2 11  // Wheel 4 DIR

// --- Encoders ---
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

#define LED_PIN 48 // Built-in LED

// ==========================================
// 2. NETWORK & MQTT SETTINGS
// ==========================================

const char* ssid = "YOUR_WIFI_NAME";         // CHANGE THIS
const char* password = "YOUR_WIFI_PASSWORD"; // CHANGE THIS
const char* mqtt_server = "broker.emqx.io";  // Public test broker

WiFiClient espClient;
PubSubClient client(espClient);


// ==========================================
// 3. HARDWARE CONTROL FUNCTIONS
// ==========================================

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

void setWheel2(int speed) {
  if (speed > 0) {
    digitalWrite(BIN1_1, HIGH);
    digitalWrite(BIN2_1, LOW);
    analogWrite(PWMB_1, speed);
  } else if (speed < 0) {
    digitalWrite(BIN1_1, LOW);
    digitalWrite(BIN2_1, HIGH);
    analogWrite(PWMB_1, -speed);
  } else {
    digitalWrite(BIN1_1, LOW);
    digitalWrite(BIN2_1, LOW);
    analogWrite(PWMB_1, 0);
  }
}

void setWheel3(int speed) {
  if (speed > 0) {
    digitalWrite(AIN1_2, HIGH);
    digitalWrite(AIN2_2, LOW);
    analogWrite(PWMA_2, speed);
  } else if (speed < 0) {
    digitalWrite(AIN1_2, LOW);
    digitalWrite(AIN2_2, HIGH);
    analogWrite(PWMA_2, -speed);
  } else {
    digitalWrite(AIN1_2, LOW);
    digitalWrite(AIN2_2, LOW);
    analogWrite(PWMA_2, 0);
  }
}

void setWheel4(int speed) {
  if (speed > 0) {
    digitalWrite(BIN1_2, HIGH);
    digitalWrite(BIN2_2, LOW);
    analogWrite(PWMB_2, speed);
  } else if (speed < 0) {
    digitalWrite(BIN1_2, LOW);
    digitalWrite(BIN2_2, HIGH);
    analogWrite(PWMB_2, -speed);
  } else {
    digitalWrite(BIN1_2, LOW);
    digitalWrite(BIN2_2, LOW);
    analogWrite(PWMB_2, 0);
  }
}

// ==========================================
// 4. NETWORK COMMUNICATION FUNCTIONS
// ==========================================

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// This function runs every time the ESP32 receives a message from Flutter
void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  
  Serial.print("Command received via [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(messageTemp);

  // --- COMMAND LOGIC ---
  
  // Example: Flutter sends a speed value (-255 to 255) to move forward/backward
  if (String(topic) == "mower/drive") {
    int speed = messageTemp.toInt();
    setWheel1(speed);
    setWheel2(speed);
    setWheel3(speed);
    setWheel4(speed);
  }
  
  // Example: Flutter turns the cutting blades ON or OFF
  if (String(topic) == "mower/rotor") {
    if(messageTemp == "ON"){
      setRotorSpeed(200); // Adjust default cutting speed here
    } else if(messageTemp == "OFF"){
      setRotorSpeed(0);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Mower-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Re-subscribe to the topics Flutter sends messages to
      client.subscribe("mower/drive");
      client.subscribe("mower/rotor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ==========================================
// 5. MAIN SETUP & LOOP
// ==========================================

void setup() {
  Serial.begin(115200);
  
  // --- Pin Modes ---
  pinMode(PWMA_1, OUTPUT); pinMode(AIN1_1, OUTPUT); pinMode(AIN2_1, OUTPUT);
  pinMode(PWMB_1, OUTPUT); pinMode(BIN1_1, OUTPUT); pinMode(BIN2_1, OUTPUT);
  pinMode(STBY, OUTPUT);

  pinMode(PWMA_2, OUTPUT); pinMode(AIN1_2, OUTPUT); pinMode(AIN2_2, OUTPUT);
  pinMode(PWMB_2, OUTPUT); pinMode(BIN1_2, OUTPUT); pinMode(BIN2_2, OUTPUT);

  pinMode(ENC_A_1, INPUT_PULLUP); pinMode(ENC_B_1, INPUT_PULLUP);
  pinMode(ENC_A_2, INPUT_PULLUP); pinMode(ENC_B_2, INPUT_PULLUP);
  pinMode(ENC_A_3, INPUT_PULLUP); pinMode(ENC_B_3, INPUT_PULLUP);
  pinMode(ENC_A_4, INPUT_PULLUP); pinMode(ENC_B_4, INPUT_PULLUP);

  pinMode(RPWM, OUTPUT); pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT); pinMode(L_EN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // --- Initial Hardware States ---
  digitalWrite(STBY, HIGH); 
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  
  setWheel1(0); setWheel2(0); setWheel3(0); setWheel4(0);
  setRotorSpeed(0);

  // --- Network Setup ---
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  Serial.println("ESP32 IoT Mower Initialized & Ready.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Listen for incoming commands from Flutter App

  
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    client.publish("mower/status", "Online and waiting for commands");
  }

  delay(10); 
}
