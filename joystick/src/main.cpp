#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char* ssid = "wifi";
const char* password = "pass";

WebSocketsServer webSocket = WebSocketsServer(81);

typedef struct 
{
  float x;
  float y;
  bool emergency;
} JoystickData;

QueueHandle_t joystickQueue;
TaskHandle_t controlTaskHandle;

void processJoystick(float x, float y) 
{

  // Deadzone
  if (abs(x) < 0.1) x = 0;
  if (abs(y) < 0.1) y = 0;

  int left = (y * 255) + (x * 255);
  int right = (y * 255) - (x * 255);

  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  Serial.printf("Processed → Left: %d | Right: %d\n", left, right);
}

void controlTask(void *pvParameters) 
{

  JoystickData data;

  while (true) 
  {

    if (xQueueReceive(joystickQueue, &data, portMAX_DELAY)) 
    {

      if (data.emergency) 
      {
        Serial.println("!!! EMERGENCY STOP !!!");
        Serial.println("Left: 0 | Right: 0");
        continue;
      }

      processJoystick(data.x, data.y);
    }
  }
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) 
{

  if (type == WStype_TEXT) 
  {

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) 
    {
      Serial.println("JSON Error");
      return;
    }

    JoystickData data;

    if (doc.containsKey("stop")) 
    {
      data.emergency = true;
    } else 
    {
      data.emergency = false;
      data.x = doc["x"] | 0.0;
      data.y = doc["y"] | 0.0;
    }

    xQueueSend(joystickQueue, &data, 0);
  }
}

void setup() {
  Serial.begin(115200);

  joystickQueue = xQueueCreate(10, sizeof(JoystickData));

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  xTaskCreatePinnedToCore(
    controlTask,        
    "Control Task",     
    4096,               
    NULL,               
    1,                  
    &controlTaskHandle,
    1                  
  );
}

void loop() 
{
  webSocket.loop(); 
}
