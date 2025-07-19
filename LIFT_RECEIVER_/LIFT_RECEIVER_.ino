#include <WiFi.h>
#include <esp_now.h>

#define RELAY1 18
#define RELAY2 19
#define TIMEOUT_MS 500  // Stop relays if no data for 500ms

typedef struct struct_message {
  bool btn1Hold;
  bool btn2Hold;
  bool heartbeat;
} struct_message;

struct_message incomingMsg;
unsigned long lastReceivedTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Receiver ready...");
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&incomingMsg, data, sizeof(incomingMsg));
  lastReceivedTime = millis();

  // Enforce mutual exclusion: Only one relay on at a time
  if (incomingMsg.btn1Hold && !incomingMsg.btn2Hold) {
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, LOW);
  } else if (incomingMsg.btn2Hold && !incomingMsg.btn1Hold) {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, HIGH);
  } else {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
  }

  Serial.printf("Relay1: %s, Relay2: %s\n",
                incomingMsg.btn1Hold ? "ON" : "OFF",
                incomingMsg.btn2Hold ? "ON" : "OFF");
}

void loop() {
  // If no data received in TIMEOUT_MS, turn off relays
  if (millis() - lastReceivedTime > TIMEOUT_MS) {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
  }
}
