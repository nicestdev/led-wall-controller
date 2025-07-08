#include <Adafruit_NeoPXL8.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define WIFI_SSID "SSID"
#define WIFI_PASS "PASSWORD"
#define UDP_PORT 80

#define COMMAND_SIZE 6

#define PANEL_WIDTH 20
#define PANEL_HEIGHT 40
#define COLOR_ORDER NEO_GRBW
#define ACTIVE_OUTPUTS 2

// Data pins for NeoPXL8 (only first ACTIVE_OUTPUTS will be used)
int8_t Pins[8] = { 15, 14, 13, 12, 11, 10, 9, 8 };
// Buffer for receiving LED commands (max. one command per LED + 2 special commands)
uint8_t Buffer[COMMAND_SIZE + (PANEL_WIDTH * PANEL_HEIGHT * COMMAND_SIZE) + COMMAND_SIZE];
// NeoPXL8 LED controller instance
Adafruit_NeoPXL8 LedWall((PANEL_WIDTH * PANEL_HEIGHT) / ACTIVE_OUTPUTS, Pins, COLOR_ORDER);
// UDP socket used to receive LED commands over WiFi
WiFiUDP UpdSocket;

void processCommand(uint8_t* cmd) {
  // Special system commands
  if (cmd[0] == 0xFF) {
    switch (cmd[1]) {
      case 0x00:
        LedWall.clear();
        break;
      case 0x01:
        LedWall.show();
        break;
      case 0xFF:
        UpdSocket.beginPacket(UpdSocket.remoteIP(), UpdSocket.remotePort());
        UpdSocket.write((const uint8_t*)"Pong", 4);
        UpdSocket.endPacket();
        break;
      default:
        LedWall.rainbow();
        break;
    }

    return;
  }

  uint8_t x = cmd[0];
  uint8_t y = cmd[1];
  uint16_t index;

  // Calculate pixel index based on layout (even/odd)
  if (x % 2 == 0) {
    index = (x * PANEL_HEIGHT) + (PANEL_HEIGHT - 1 - y);
  } else {
    index = (x * PANEL_HEIGHT) + y;
  }

  /* 
    Command structure:
      [0] X Position  (0–19)
      [1] Y Position  (0–39)
      [2] Red         (0–255)
      [3] Green       (0–255)
      [4] Blue        (0–255)
      [5] White       (0–255)
  */
  LedWall.setPixelColor(index, cmd[2], cmd[3], cmd[4], cmd[5]);
}

void setup() {
  LedWall.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  UpdSocket.begin(UDP_PORT);
}

void loop() {
  uint32_t packetSize = UpdSocket.parsePacket();

  if (packetSize <= 0) return;

  // Drop oversized packet
  if ((size_t)packetSize > sizeof(Buffer)) {
    while (UpdSocket.available()) UpdSocket.read();
    return;
  }

  int bytesRead = UpdSocket.read(Buffer, packetSize);

  // Drop invalid or incomplete data
  if (bytesRead != packetSize || (bytesRead % COMMAND_SIZE) != 0) return;

  const int cmdCount = bytesRead / COMMAND_SIZE;

  for (int i = 0; i < cmdCount; i++) {
    processCommand(&Buffer[i * COMMAND_SIZE]);
  }
}