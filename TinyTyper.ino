#include <WiFi.h>
#include <WebServer.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <SPIFFS.h>

USBHIDKeyboard Keyboard;

// Wi-Fi credentials for the access point
const char* ssid = "TinyTyper";
const char* password = "TinyTyper";  // Minimum 8 characters

// Create a web server on port 80
WebServer server(80);

// Global variables
String inputText = "";
bool typingInProgress = false;
bool isPaused = false;
bool isStopped = false;

// Typing settings
int minTypingSpeed = 50;    // Minimum delay between characters (ms)
int maxTypingSpeed = 200;   // Maximum delay between characters (ms)
int wrongCharPercentage = 5; // Percentage of wrong characters (0-100)

// HTML for the webpage
const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>TinyTyper</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f4f4f9;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }
    .container {
      background-color: #ffffff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      text-align: center;
      width: 90%;
      max-width: 600px;
      position: relative;
    }
    h1 {
      color: #333333;
      margin-bottom: 20px;
    }
    textarea {
      width: 100%;
      height: 150px;
      padding: 10px;
      margin-bottom: 20px;
      border: 1px solid #cccccc;
      border-radius: 5px;
      font-size: 16px;
      resize: vertical;
    }
    .button-row {
      display: flex;
      justify-content: space-between;
      gap: 10px;
      margin-bottom: 10px;
    }
    .button-row button {
      flex: 1;
      background-color: #008080; /* Teal color */
      color: white;
      padding: 10px;
      border: none;
      border-radius: 5px;
      font-size: 16px;
      cursor: pointer;
    }
    .button-row button:hover {
      background-color: #006666; /* Darker teal for hover */
    }
    .button-row button:active {
      background-color: #004d4d; /* Even darker teal for active state */
    }
    .settings-icon {
      position: absolute;
      top: 20px;
      right: 20px;
      font-size: 24px;
      cursor: pointer;
    }
    .modal {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background-color: rgba(0, 0, 0, 0.5);
      justify-content: center;
      align-items: center;
    }
    .modal-content {
      background-color: #ffffff;
      padding: 20px;
      border-radius: 10px;
      width: 90%;
      max-width: 400px;
    }
    .modal-content h2 {
      margin-top: 0;
    }
    .modal-content label {
      display: block;
      margin: 10px 0 5px;
    }
    .modal-content input[type="range"] {
      width: 100%;
    }
    .modal-content .button-row {
      margin-top: 20px;
    }
  </style>
  <script>
    function sendCommand(command) {
      const text = document.querySelector('textarea[name="text"]').value;
      fetch('/' + command, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: 'text=' + encodeURIComponent(text),
      })
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => console.error('Error:', error));
    }

    function clearInput() {
      document.querySelector('textarea[name="text"]').value = '';
    }

    function openSettings() {
      document.getElementById('settingsModal').style.display = 'flex';
    }

    function closeSettings() {
      document.getElementById('settingsModal').style.display = 'none';
    }

    function saveSettings() {
      const minSpeed = document.getElementById('minSpeed').value;
      const maxSpeed = document.getElementById('maxSpeed').value;
      const wrongChar = document.getElementById('wrongChar').value;

      fetch('/saveSettings', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `minSpeed=${minSpeed}&maxSpeed=${maxSpeed}&wrongChar=${wrongChar}`,
      })
        .then(response => response.text())
        .then(data => {
          console.log(data);
          closeSettings();
        })
        .catch(error => console.error('Error:', error));
    }

    // Load saved settings when the page loads
    window.onload = function() {
      fetch('/getSettings')
        .then(response => response.json())
        .then(data => {
          document.getElementById('minSpeed').value = data.minSpeed;
          document.getElementById('maxSpeed').value = data.maxSpeed;
          document.getElementById('wrongChar').value = data.wrongChar;
        })
        .catch(error => console.error('Error:', error));
    };
  </script>
</head>
<body>
  <div class="container">
    <h1>TinyTyper</h1>
    <p>By Ahmed</p>
    <div class="settings-icon" onclick="openSettings()">⚙️</div>
    <textarea name="text" placeholder="Enter paragraph to type..." required></textarea>
    <div class="button-row">
      <button type="button" onclick="sendCommand('type')">Type</button>
      <button type="button" onclick="clearInput()">Clear</button>
    </div>
    <div class="button-row">
      <button type="button" onclick="sendCommand('pause')">Pause/Resume</button>
      <button type="button" onclick="sendCommand('stop')">Stop</button>
    </div>
  </div>

  <!-- Settings Modal -->
  <div class="modal" id="settingsModal">
    <div class="modal-content">
      <h2>Settings</h2>
      <label for="minSpeed">Minimum Typing Speed (ms):</label>
      <input type="range" id="minSpeed" name="minSpeed" min="10" max="500" value="50">
      <label for="maxSpeed">Maximum Typing Speed (ms):</label>
      <input type="range" id="maxSpeed" name="maxSpeed" min="10" max="500" value="200">
      <label for="wrongChar">Percentage of Wrong Characters (%):</label>
      <input type="range" id="wrongChar" name="wrongChar" min="0" max="100" value="5">
      <div class="button-row">
        <button type="button" onclick="closeSettings()">Cancel</button>
        <button type="button" onclick="saveSettings()">Save</button>
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";

// Function to mimic human typing
void typeText(String text) {
  typingInProgress = true;
  isStopped = false;
  isPaused = false;

  for (int i = 0; i < text.length(); i++) {
    if (isStopped) {
      break;  // Stop typing if the stop command is received
    }

    while (isPaused) {
      delay(100);  // Wait while paused
    }

    // Random delay between characters (minTypingSpeed to maxTypingSpeed)
    int delayTime = random(minTypingSpeed, maxTypingSpeed);
    delay(delayTime);

    // Occasionally introduce a backspace (wrongCharPercentage chance)
    if (random(100) < wrongCharPercentage && i > 0) {
      Keyboard.write(KEY_BACKSPACE);
      delay(random(50, 150));  // Random delay after backspace
      i--;  // Go back one character
      continue;
    }

    // Type the character
    Keyboard.print(text[i]);

    // Occasionally add a longer delay after a space or newline (20% chance)
    if ((text[i] == ' ' || text[i] == '\n') && random(100) < 20) {
      delay(random(200, 500));  // Longer delay after a space or newline
    }
  }

  // Add a newline at the end (if not stopped)
  if (!isStopped) {
    Keyboard.println();
  }

  // Clear the input text and mark typing as complete
  inputText = "";
  typingInProgress = false;
  isStopped = false;
  isPaused = false;

  // Reload the webpage
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// Task to handle typing
void typingTask(void* parameter) {
  while (1) {
    if (inputText.length() > 0 && !typingInProgress) {
      Serial.println("Typing task started");
      typeText(inputText);
      Serial.println("Typing task completed");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Small delay to prevent busy-waiting
  }
}

// Task to handle Wi-Fi and web server
void wifiTask(void* parameter) {
  // Start the Wi-Fi access point
  WiFi.softAP(ssid, password);
  Serial.println("Wi-Fi Hotspot Started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  // Load saved settings
  loadSettings();

  // Define server routes
  server.on("/", handleRoot);           // Handle the root path
  server.on("/type", handleType);       // Handle the form submission
  server.on("/pause", handlePause);     // Handle pause/resume command
  server.on("/stop", handleStop);       // Handle stop command
  server.on("/saveSettings", handleSaveSettings); // Handle save settings
  server.on("/getSettings", handleGetSettings);   // Handle get settings

  // Captive portal: Redirect all requests to the root page
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  // Start the server
  server.begin();
  Serial.println("HTTP Server Started");

  while (1) {
    server.handleClient();
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Small delay to prevent busy-waiting
  }
}

void setup() {
  // Start Serial communication
  Serial.begin(115200);

  // Initialize USB and HID Keyboard
  USB.begin();
  Keyboard.begin();
  delay(1000);

  // Create tasks for Wi-Fi and typing
  xTaskCreatePinnedToCore(
    wifiTask,     // Task function
    "WiFiTask",  // Task name
    10000,       // Stack size
    NULL,        // Task parameters
    1,           // Task priority
    NULL,        // Task handle
    1            // Core (Core 1 for Wi-Fi)
  );

  xTaskCreatePinnedToCore(
    typingTask,  // Task function
    "TypingTask", // Task name
    10000,       // Stack size
    NULL,        // Task parameters
    1,           // Task priority
    NULL,        // Task handle
    0            // Core (Core 0 for typing)
  );
}

void loop() {
  // FreeRTOS handles the tasks, so nothing is needed here
}

// Handle the root path
void handleRoot() {
  server.send(200, "text/html", htmlContent);
}

// Handle the form submission
void handleType() {
  if (server.method() == HTTP_POST) {
    // Get the text input from the form
    inputText = server.arg("text");
    Serial.print("Received Text: ");
    Serial.println(inputText);

    // Send a response back to the client
    server.send(200, "text/plain", "Typing started: " + inputText);
  }
}

// Handle the pause/resume command
void handlePause() {
  isPaused = !isPaused;
  server.send(200, "text/plain", isPaused ? "Typing paused" : "Typing resumed");
}

// Handle the stop command
void handleStop() {
  isStopped = true;
  server.send(200, "text/plain", "Typing stopped");
}

// Handle saving settings
void handleSaveSettings() {
  if (server.method() == HTTP_POST) {
    minTypingSpeed = server.arg("minSpeed").toInt();
    maxTypingSpeed = server.arg("maxSpeed").toInt();
    wrongCharPercentage = server.arg("wrongChar").toInt();

    // Ensure maxTypingSpeed is greater than minTypingSpeed
    if (maxTypingSpeed <= minTypingSpeed) {
      maxTypingSpeed = minTypingSpeed + 10;
    }

    // Save settings to SPIFFS
    saveSettings();

    server.send(200, "text/plain", "Settings saved");
  }
}

// Handle getting settings
void handleGetSettings() {
  String settings = "{\"minSpeed\":" + String(minTy pingSpeed) +
                    ",\"maxSpeed\":" + String(maxTypingSpeed) +
                    ",\"wrongChar\":" + String(wrongCharPercentage) + "}";
  server.send(200, "application/json", settings);
}

// Save settings to SPIFFS
void saveSettings() {
  File file = SPIFFS.open("/settings.txt", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.println(minTypingSpeed);
  file.println(maxTypingSpeed);
  file.println(wrongCharPercentage);
  file.close();
  Serial.println("Settings saved to SPIFFS");
}

// Load settings from SPIFFS
void loadSettings() {
  File file = SPIFFS.open("/settings.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  minTypingSpeed = file.readStringUntil('\n').toInt();
  maxTypingSpeed = file.readStringUntil('\n').toInt();
  wrongCharPercentage = file.readStringUntil('\n').toInt();
  file.close();
  Serial.println("Settings loaded from SPIFFS");
}
