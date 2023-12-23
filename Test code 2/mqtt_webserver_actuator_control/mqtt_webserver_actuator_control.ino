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
  <title>MQTT Subscriber</title>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.0.1/mqttws31.min.js"></script>
  <script>
    let mqtt = new Paho.MQTT.Client('broker.hivemq.com', 8000, 'subscriberjs');
    mqtt.onConnectionLost = onConnectionLost;
    mqtt.onMessageArrived = onMessageArrived;
    mqtt.connect({
      onSuccess: onConnect,
      onFailure: onFailure
    });
    function onConnect() {
      console.log('Connected to MQTT server');
      mqtt.subscribe('MQTT/ESP32/LED');
    }
    function onFailure() {
      console.log('Failed to connect to MQTT server');
    }
    function onConnectionLost(responseObject) {
      if (responseObject.errorCode !== 0) {
        console.log('Connection lost: ' + responseObject.errorMessage);
      }
    }
    function onMessageArrived(message) {
      console.log('Message arrived: ' + message.payloadString);
      sendHttpRequest(message.payloadString);
      var messageContainer = document.getElementById('messageContainer');
      messageContainer.innerHTML += '<p>' + message.payloadString + '</p>';
      
    }
    function sendHttpRequest(x) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/action?go=" + x, true);
      xhr.send();
    }
  </script>
</head>
<body>
  <h2>MQTT Subscriber</h2>
  <div id="messageContainer"></div>
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

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("HTTP Server Ready! Go to: http://");
  Serial.println(WiFi.localIP());

  // Start the HTTP server
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
}

void loop() {
}
