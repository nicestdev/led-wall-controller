# LED Wall Controller
This project allows you to control a RGBW LED wall built using SK6812 LEDs over WiFi using UDP packets. It's designed for boards compatible with Adafruit NeoPXL8, such as the Raspberry Pi Pico W (with NeoPXL8) or ESP32-S2/S3 variants.

## Panel Structure
```cpp
#define PANEL_WIDTH     20
#define PANEL_HEIGHT    40
#define ACTIVE_OUTPUTS  2
```
The panel has 800 LEDs, is split across 2 NeoPXL8 data outputs, with serpentine wiring per column.

## Network Configuration
```cpp
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"
#define UDP_PORT  80
```
Change these in the code to match your network. The controller listens on UDP port 80.

## Command Format
```
[0] X Position  (0–19)
[1] Y Position  (0–39)
[2] Red         (0–255)
[3] Green       (0–255)
[4] Blue        (0–255)
[5] White       (0–255)
```
### Special System Commands
Send a command starting with `\xFF`:

- `\xFF \x00` — Clear buffer (turn all LEDs off, not yet shown)
- `\xFF \x01` — Show buffer (refresh LED output using DMA)
- `\xFF \xFF` — Replies to sender with `"Pong"` for discovery
- `\xFF \x??` — Rainbow display (turn all LEDs on, not yet shown)

## Usage example
Since communication runs over UDP, you _can_ use broadcasting like `192.168.178.255`. However, I would not recommend this, as it sends unnecessary data across the entire network. Instead, I recommend using the discovery command for this purpose.
```bash
# 1. Clear internal framebuffer (resets all LED values to 0) (optional)
echo -ne '\xFF\x00\x00\x00\x00\x00' | nc -u -w1 192.168.178.100 80

# 2. Set individual pixels in the framebuffer
echo -ne '\x00\x00\x00\x00\x00\xFF' | nc -u -w1 192.168.178.100 80  # (0,0) white
echo -ne '\x01\x01\xFF\x00\x00\x00' | nc -u -w1 192.168.178.100 80  # (1,1) red
echo -ne '\x02\x02\x00\xFF\x00\x00' | nc -u -w1 192.168.178.100 80  # (2,2) green

# 3. Push updated framebuffer to LEDs using DMA
echo -ne '\xFF\x01\x00\x00\x00\x00' | nc -u -w1 192.168.178.100 80
```
