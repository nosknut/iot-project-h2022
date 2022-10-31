#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <arduino.h>

// ubidots
#include "UbidotsEsp32Mqtt.h"
#include <Wire.h>
#include <SPI.h>

#ifdef ESP32
  #include <SPIFFS.h>
#endif

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson


//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
char mqtt_port[6] = "8080";
char api_token[34] = "YOUR_API_TOKEN";

//flag for saving data
bool shouldSaveConfig = false;

// ssid/pw from WM
char wm_ssid[40];      // Put here your Wi-Fi SSID
char wm_pw[40];      // Put here your Wi-Fi password

/****************************************
 * UBIDOTS PUBLISH CODE
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-boQsUTaILq6F7dLhQfThgBnG8g7k42";  // Put here your Ubidots TOKEN (Karl: BBFF-33AqonsLEHN5ujXN3N1q2qtcIqBN0K)
const char *DEVICE_LABEL = "Test";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "Temp"; // Put here your Variable label to which data  will be published

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds 

unsigned long timer;
uint8_t analogPin = 34; // Pin used to read data from GPIO34 ADC_CH6.
int tempPin = 34;

const int temp1_pin = 12;

int temp;
float volts;
float tempC;
float temps[10];

Ubidots ubidots(UBIDOTS_TOKEN);


/****************************************
 * Auxiliar Functions
 ****************************************/

// callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// ubidots callback
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

float getTemp(int pin) {
  digitalWrite(pin, HIGH);
  for (int i = 0; i < 10; i++) {
    temp = analogRead(tempPin);
    volts = temp / 1024.0;
    tempC = (volts - 0.5) * 100;
    temps[i] = tempC;
  }
  digitalWrite(pin, LOW);
  float sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += temps[i];
  }
  return sum / 10;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  // ESP32 PINS
  pinMode(temp1_pin, OUTPUT);
  digitalWrite(temp1_pin, LOW);

  //clean FS, for testing
  SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
#endif
          Serial.println("\nparsed json");
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(api_token, json["api_token"]);
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 40);
  WiFiManagerParameter custom_api_token("apikey", "API token", api_token, 40);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //ADD ALL YOUR PARAMETERS HERE
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_api_token);

  //reset settings - for testing
  wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "ESP32_Temp"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("ESP32_Temp", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("device has connected to your wifi...yeey :)");

  //read updated CUSTOM parameters
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(api_token, custom_api_token.getValue());
  Serial.println("The values in the file are: ");
  Serial.println("\tubidots_token : " + String(mqtt_server));
  Serial.println("\tmqtt_port : " + String(mqtt_port));
  Serial.println("\tapi_token : " + String(api_token));

  // from line 436,439 in WifiManager.h
  // helper to get saved password and ssid, if persistent get stored, else get current if connected
  // converting ssid/pw String to Char for UBIDOTS connect
  int pw_len = wifiManager.getWiFiPass().length() + 1;
  wifiManager.getWiFiPass().toCharArray(wm_ssid, pw_len);
  int ssid_len = wifiManager.getWiFiSSID().length() + 1;
  wifiManager.getWiFiPass().toCharArray(wm_pw, ssid_len);

  Serial.println("\tyour_pw : " + String(wm_ssid));
  Serial.println("\tyour_ssid : " +  String(wm_pw));

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif
    json["mqtt_port"] = mqtt_port;
    json["api_token"] = api_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!ubidots.connected())
  {
    const char *WIFI_SSID = wm_ssid;
    const char *WIFI_PASS = wm_pw;
    ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
    ubidots.setCallback(callback);
    ubidots.setup();
    ubidots.reconnect();

  timer = millis();
  }
    if (millis() - timer > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    ubidots.add("temperature1", getTemp(temp1_pin));
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
}