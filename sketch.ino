#include <SPI.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

// lvgl libraries for display and touch
#include "Arduino_H7_Video.h"
#include "lvgl.h"
#include "Arduino_GigaDisplayTouch.h"

// TODO: move this to secrets!
#define SECRET_SSID "SSIDHERE"
#define SECRET_PASS "pskpw"

// initialize wifi
WiFiClient wifi;
int status = WL_IDLE_STATUS;

// initialize the display shield
Arduino_H7_Video Display(800, 480, GigaDisplayShield);
Arduino_GigaDisplayTouch TouchDetector;

// initialize JSON
JsonDocument doc;

// Server URL
char serverAddress[] = "192.168.1.1";  //  IP address here have pi-star wpsd
int port = 80;
HttpClient client = HttpClient(wifi, serverAddress, port);

// Create text labels for displaying JSON data
lv_obj_t *time_label, *mode_label, *callsign_label, *name_label;
lv_obj_t *callsign_suffix_label, *target_label, *src_label, *duration_label, *loss_label;

void setup() {
  Display.begin();
  TouchDetector.begin();

  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
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

  //Display & Grid Setup
  lv_obj_t* screen = lv_scr_act();
  lv_obj_set_size(screen, Display.width(), Display.height());

  static lv_coord_t col_dsc[] = { 750, LV_GRID_TEMPLATE_LAST };
  static lv_coord_t row_dsc[] = { 410, LV_GRID_TEMPLATE_LAST };

  lv_obj_t* grid = lv_obj_create(screen);
  lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
  lv_obj_set_size(grid, Display.width(), Display.height());

  // TODO: fix labels (font too small, use squareline studio!)
  
  // Create labels for displaying JSON data
  time_label = lv_label_create(grid);
  lv_obj_set_grid_cell(time_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
  lv_label_set_text(time_label, "Time UTC: ");

  mode_label = lv_label_create(grid);
  lv_obj_set_grid_cell(mode_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_label_set_text(mode_label, "Mode: ");

  callsign_label = lv_label_create(grid);
  lv_obj_set_grid_cell(callsign_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_label_set_text(callsign_label, "Callsign: ");

  name_label = lv_label_create(grid);
  lv_obj_set_grid_cell(name_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);
  lv_label_set_text(name_label, "Name: ");

  callsign_suffix_label = lv_label_create(grid);
  lv_obj_set_grid_cell(callsign_suffix_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 4, 1);
  lv_label_set_text(callsign_suffix_label, "Callsign Suffix: ");

  target_label = lv_label_create(grid);
  lv_obj_set_grid_cell(target_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 5, 1);
  lv_label_set_text(target_label, "Target: ");

  src_label = lv_label_create(grid);
  lv_obj_set_grid_cell(src_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 6, 1);
  lv_label_set_text(src_label, "Source: ");

  duration_label = lv_label_create(grid);
  lv_obj_set_grid_cell(duration_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 7, 1);
  lv_label_set_text(duration_label, "Duration: ");

  loss_label = lv_label_create(grid);
  lv_obj_set_grid_cell(loss_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 8, 1);
  lv_label_set_text(loss_label, "Loss: ");
}

void loop() {
  lv_timer_handler();

  if (WiFi.status() == WL_CONNECTED) { // Check the current connection status
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

        // Update labels with the extracted data
        lv_label_set_text(time_label, ("Time UTC: " + time_utc).c_str());
        lv_label_set_text(mode_label, ("Mode: " + mode).c_str());
        lv_label_set_text(callsign_label, ("Callsign: " + callsign).c_str());
        lv_label_set_text(name_label, ("Name: " + name).c_str());
        lv_label_set_text(callsign_suffix_label, ("Callsign Suffix: " + callsign_suffix).c_str());
        lv_label_set_text(target_label, ("Target: " + target).c_str());
        lv_label_set_text(src_label, ("Source: " + src).c_str());
        lv_label_set_text(duration_label, ("Duration: " + duration).c_str());
        lv_label_set_text(loss_label, ("Loss: " + loss).c_str());
      } else {
        Serial.println("JSON Parsing error");
      }
    } else {
      Serial.println("Something bad happened.");  
    }
  }

  delay(1500); // Make a request every 10 seconds
}
