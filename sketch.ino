#include <SPI.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "Arduino_GigaDisplay_GFX.h"

#define SECRET_SSID "VA3IAN-11"
#define SECRET_PASS "C0mmunic@tions"

#define BLACK 0x0000
#define WHITE 0xFFFF
#define YELLOW 0xFFE0

// initialize wifi
WiFiClient wifi;
int status = WL_IDLE_STATUS;

// define screen
GigaDisplay_GFX display;  // create the object

// initialize JSON
JsonDocument doc;

// Server URL
char serverAddress[] = "172.16.1.16";
int port = 80;
HttpClient client = HttpClient(wifi, serverAddress, port);

void clearScreen() {
  display.fillScreen(BLACK);
}

void setup() {
  display.begin();  //init library
  clearScreen();    // fill screen black

  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    display.setTextSize(5);

    // print callsign
    display.setCursor(10, 10);
    display.print("Connecting ...");

    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(SECRET_SSID);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(SECRET_SSID, SECRET_PASS);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");

  // print the ip address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  clearScreen();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {  // Check the current connection status
    client.beginRequest();
    client.get("/api/index.php");
    client.endRequest();

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String payload = client.responseBody();

    if (statusCode == 200) {
      // Parse JSON
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        // Access the first array element
        JsonArray array = doc.as<JsonArray>();
        JsonObject obj = array[0];

        // Extract data from the object
        String time_utc = obj["time_utc"].as<String>();
        String mode = obj["mode"].as<String>();
        String callsign = obj["callsign"].as<String>();
        String name = obj["name"].as<String>();
        String callsign_suffix = obj["callsign_suffix"].as<String>();
        String target = obj["target"].as<String>();
        String src = obj["src"].as<String>();
        String duration = obj["duration"].as<String>();
        String loss = obj["loss"].as<String>();

        // {
        //     "time_utc": "2024-08-05 18:43:51",
        //     "mode": "DMR Slot 2",
        //     "callsign": "HA5DT",
        //     "name": "Tomi",
        //     "callsign_suffix": "",
        //     "target": "TG 91",
        //     "src": "Net",
        //     "duration": "93.7",
        //     "loss": "4%",
        //     "bit_error_rate": "0.3%",
        //     "rssi": ""
        // },

        clearScreen();
        display.setTextSize(3);

        // print callsign
        display.setCursor(10, 10);
        display.print(callsign);

        // print name
        display.setCursor(10, 45);
        display.print(name);

        // print time
        display.setCursor(10, 80);
        display.print(time_utc); 

        // print callsign
        display.setCursor(10, 115);
        display.print(mode);

        // print name
        display.setCursor(10, 150);
        display.print(target);

        // print time
        display.setCursor(10, 185);
        display.print(duration); 

        // print time
        display.setCursor(10, 220);
        display.print(src); 

        // debug
        Serial.println(time_utc);
        Serial.println(mode);
        Serial.println(callsign);
        Serial.println(name);
        Serial.println(callsign_suffix);
        Serial.println(target);
        Serial.println(src);
        Serial.println(duration);
        Serial.println(loss);
      } else {
        Serial.println("JSON Parsing error");
      }
    } else {
      Serial.println("Something bad happened.");
    }
  }

  delay(1500);  // Make a request every 10 seconds
}
