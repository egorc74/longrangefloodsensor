My firmware is designed for an early flood warning sensor that detects rising water levels. I used two ESP32 boards as the main controllers—one for the measurement device and the other for the transmitting device.

Communication between the two boards is handled via LoRa SX1278. The measurement device (firmware: sensorunit.cpp) is powered by a solar panel with a battery, so it needs to be power-efficient. To achieve this, I use the ESP32’s built-in deep sleep mode.

The transmitting module is powered via USB and communicates with the server using HTTP. The server side of the project runs on a Django + React (Vite) stack and is kept in a private repository for security reasons.

To-Do List:
Encrypt LoRa messages
Use HTTPS instead of HTTP
Add a gateway on the server side to support multiple measurement units connected to a single transmission unit
