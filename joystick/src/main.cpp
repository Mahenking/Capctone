#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// WIFI
const char* ssid = "IQOO Z9 5G";
const char* password = "wannasif";

// WEBSOCKET
WebSocketsServer webSocket = WebSocketsServer(81);

float joyX = 0;
float joyY = 0;
bool emergencyStop = false;

SemaphoreHandle_t xMutex;

void setMotorSpeed(int left, int right)
{
  Serial.printf("Left: %d | Right: %d\n", left, right);
}

// JOYSTICK PROCESS
void processJoystick(float x, float y)
{
  if (abs(x) < 0.1) x = 0;
  if (abs(y) < 0.1) y = 0;

  int left = (y * 255) + (x * 255);
  int right = (y * 255) - (x * 255);

  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  setMotorSpeed(left, right);
}

// WEBSOCKET EVENT
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  if (type == WStype_TEXT)
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) return;

    xSemaphoreTake(xMutex, portMAX_DELAY);

    if (doc.containsKey("stop"))
    {
      emergencyStop = true;
    }
    else
    {
      joyX = doc["x"];
      joyY = doc["y"];
      emergencyStop = false;
    }

    xSemaphoreGive(xMutex);
  }
}

// TASK 1
void TaskWiFi(void *pvParameters)
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  vTaskDelete(NULL); // selesai
}

// TASK 2
void TaskWebSocket(void *pvParameters)
{
  while (true)
  {
    webSocket.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// TASK 3
void TaskMotor(void *pvParameters)
{
  float x, y;
  bool stopFlag;

  while (true)
  {
    xSemaphoreTake(xMutex, portMAX_DELAY);

    x = joyX;
    y = joyY;
    stopFlag = emergencyStop;

    xSemaphoreGive(xMutex);

    if (stopFlag)
    {
      setMotorSpeed(0, 0);
    }
    else
    {
      processJoystick(x, y);
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); 
  }
}

//  SETUP 

void setup()
{
  Serial.begin(115200);

  xMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskWiFi, "WiFi Task", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskWebSocket, "WebSocket Task", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskMotor, "Motor Task", 4096, NULL, 2, NULL, 1);
}

void loop()
{

}