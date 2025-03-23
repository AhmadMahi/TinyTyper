TinyTyper: Wireless Auto-Typing Keyboard using ESP32 🚀
TinyTyper is a wireless auto-typing keyboard built with an ESP32-S3. This project allows you to send text from a web interface to the ESP32, which then types it out like a real keyboard! It includes human-like typing simulation with random delays, pauses, and even backspaces to mimic natural typing.

🔹 Features:
✅ Turns ESP32-S3 into a USB HID keyboard
✅ Built-in Wi-Fi hotspot with a web-based interface
✅ Realistic typing with delays, pauses, and backspaces
✅ Pause/Resume/Stop controls for typing flexibility
✅ Uses FreeRTOS for multitasking

🔹 How It Works:
ESP32 creates a Wi-Fi hotspot (TinyTyper).

A web interface allows users to input text.

The text is sent to the ESP32, which simulates typing via USB HID.

Supports pause, resume, and stop commands.

🔹 Requirements:
ESP32-S3 (with native USB support)

Arduino IDE / PlatformIO

USBHID and WebServer libraries

🔹 Getting Started:
Flash the provided code onto your ESP32-S3.

Connect to the TinyTyper Wi-Fi network.

Open the web interface (192.168.4.1) and enter text.

Watch your ESP32 type automatically!

📌 Check out the full tutorial on YouTube: [Add your link]
📌 Contribute & Fork on GitHub: [Add your GitHub link]

#ESP32 #HIDKeyboard #WirelessTyping #Arduino #IoT #TinyTyper #AutoTyper #EmbeddedSystems
