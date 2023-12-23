#include <WiFi.h>
#include "esp_http_server.h"

const int numLeds = 10;
int ledPins[numLeds] = { 2, 4, 5, 12, 13, 14, 15, 16, 17, 18 };
const char* ssid = "RAJAN";
const char* password = "rajan123";

httpd_handle_t server = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Actuator control with ESP32 Board</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 20px;
        }
        button {
            padding: 10px;
            margin: 5px;
        }
    </style>
</head>
<body>
    <h2>Actuator control with ESP32 Board</h2>
    <label for="buttonCount">Enter the number of buttons for LED:</label>
    <input type="number" id="buttonCount" min="1" value="1">
    <button onclick="createButtons()">Create Buttons</button>
    <div id="buttonContainer"></div>
    <script>
        function createButtons() {
            var buttonCount = document.getElementById("buttonCount").value; // Get the number of buttons from the input
            document.getElementById("buttonContainer").innerHTML = ''; // Clear the existing buttons
            for (var i = 1; i <= buttonCount; i++) {
                var button = document.createElement("button");
                button.innerText = "Button " + i + " - OFF"; // Set initial text
                button.setAttribute("data-state", "OFF"); // Set initial state to OFF
                button.onclick = function(buttonNumber) {
                    return function() {
                        toggleState(buttonNumber);
                    };
                }(i);
                document.getElementById("buttonContainer").appendChild(button);
            }
        }
        function toggleState(buttonNumber) {
            var button = document.querySelector('button:nth-child(' + buttonNumber + ')');
            var currentState = button.getAttribute("data-state");
            var newState = currentState === "ON" ? "OFF" : "ON";
            var command = newState.toLowerCase() + "_" + buttonNumber;
            sendHttpRequest(command);
            button.setAttribute("data-state", newState);
            button.innerText = "Button " + buttonNumber + " - " + newState;
            alert("Button " + buttonNumber + " toggled! New state: " + newState);
        }
        function sendHttpRequest(command) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/action?go=" + command, true);
            xhr.send();
        }
    </script>
</body>
</html>

)rawliteral";

static esp_err_t index_handler(httpd_req_t* req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char*)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t cmd_handler(httpd_req_t* req) {
  char* buf;
  size_t buf_len;
  char variable[32] = {
    0,
  };

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if (!buf) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
        Serial.print("Received message: ");
        Serial.println(variable);
        String variableString(variable);
        if (variableString.startsWith("on_")) {
          int ledIndex = variableString.substring(3).toInt();
          if (ledIndex >= 1 && ledIndex <= numLeds) {
            digitalWrite(ledPins[ledIndex - 1], HIGH);  // Turn on the specified LED
            Serial.println("LED turned ON");
          } else {
            Serial.println("Invalid LED index");
          }
        } else if (variableString.startsWith("off_")) {
          int ledIndex = variableString.substring(4).toInt();
          if (ledIndex >= 1 && ledIndex <= numLeds) {
            digitalWrite(ledPins[ledIndex - 1], LOW);  // Turn off the specified LED
            Serial.println("LED turned OFF");
          } else {
            Serial.println("Invalid LED index");
          }
        }
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  httpd_resp_send(req, "OK", 2);
  return ESP_OK;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("HTTP Server Ready! Go to: http://");
  Serial.println(WiFi.localIP());
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t index_uri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = index_handler,
      .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &index_uri);
    httpd_uri_t cmd_uri = {
      .uri = "/action",
      .method = HTTP_GET,
      .handler = cmd_handler,
      .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &cmd_uri);
  }
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
}
void loop() {
}
