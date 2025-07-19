#include <WiFi.h>
#include <esp_now.h>

#define BUTTON1 14
#define BUTTON2 27

typedef struct struct_message {
  bool btn1Hold;
  bool btn2Hold;
  bool heartbeat;
} struct_message;

struct_message outgoingMsg;
uint8_t receiverMAC[] = {0x44, 0x1D, 0x64, 0xF7, 0x0A, 0x5C};

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 100; // Heartbeat interval in ms

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Sender ready...");
}

void loop() {
  bool btn1 = !digitalRead(BUTTON1); // Active LOW
  bool btn2 = !digitalRead(BUTTON2);

  // Only one button active at a time
  if (btn1 && !btn2) {
    outgoingMsg.btn1Hold = true;
    outgoingMsg.btn2Hold = false;
  } else if (btn2 && !btn1) {
    outgoingMsg.btn1Hold = false;
    outgoingMsg.btn2Hold = true;
  } else {
    outgoingMsg.btn1Hold = false;
    outgoingMsg.btn2Hold = false;
  }

  // Send every 100ms as heartbeat
  if (millis() - lastSendTime >= sendInterval) {
    outgoingMsg.heartbeat = true;
    esp_now_send(receiverMAC, (uint8_t *)&outgoingMsg, sizeof(outgoingMsg));
    lastSendTime = millis();
  }
}
