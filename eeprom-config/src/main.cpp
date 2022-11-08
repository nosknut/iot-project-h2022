#include <FS.h>          //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#ifdef ESP32
#include <SPIFFS.h>
#endif

#include <Arduino.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#include <PubSubClient.h>

// flag for saving data
bool shouldSaveConfig = false;

// ssid/pw and token from WM
char wm_ssid[40]; // Put here your Wi-Fi SSID
char wm_pw[40];   // Put here your Wi-Fi password

// define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40] = "";
char device_label[40] = "";

// timers for publishing
const int PUBLISH_FREQUENCY = 1000; // Update rate in milliseconds
unsigned long timer;

WiFiClient espClient;
PubSubClient client(espClient);

/****************************************
 * Auxiliar Functions
 ****************************************/

// callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// MQTT callback
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

// MQTT reconnect​
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("esp32/online", device_label);
      // ... and resubscribe
      // client.subscribe("inTopic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  // clean FS, for testing
  SPIFFS.format();

  // read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      // file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if (!deserializeError)
        {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success())
        {
#endif
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(device_label, json["device_label"]);
        }
        else
        {
          Serial.println("failed to load json config");
        }
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }
  // end read

  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_port("server", "MQTT Server", mqtt_server, 40);
  WiFiManagerParameter custom_device_label("device_label", "Device Label", device_label, 40);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  // set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // ADD ALL YOUR PARAMETERS HERE
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_device_label);

  // reset settings - for testing
  wifiManager.resetSettings();

  // set minimu quality of signal so it ignores AP's under that quality
  // defaults to 8%
  wifiManager.setMinimumSignalQuality();

  // sets timeout until configuration portal gets turned off
  wifiManager.setTimeout(180);

  // fetches ssid and pass and tries to connect if not connected starts AP
  if (!wifiManager.autoConnect("ESP32_Temp", "password"))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  // if you get here you have connected to the WiFi
  Serial.println("device has connected to your wifi...yeey :)");

  // read updated CUSTOM parameters
  strcpy(mqtt_server, custom_mqtt_port.getValue());
  strcpy(device_label, custom_device_label.getValue());
  Serial.println("The values in the file are: ");
  Serial.println("\tmqtt_token : " + String(mqtt_server));
  Serial.println("\tdevice_label : " + String(device_label));

  // from line 436,439 in WifiManager.h
  // helper to get saved password and ssid, if persistent get stored, else get current if connected
  // converting ssid/pw String to Char for UBIDOTS connect
  int ssid_len = wifiManager.getWiFiSSID().length() + 1;
  wifiManager.getWiFiSSID().toCharArray(wm_ssid, ssid_len);
  int pw_len = wifiManager.getWiFiPass().length() + 1;
  wifiManager.getWiFiPass().toCharArray(wm_pw, pw_len);

  Serial.println("\tyour_ssid : " + String(wm_ssid));
  Serial.println("\tyour_pw : " + String(wm_pw));

  // save the custom parameters to FS
  if (shouldSaveConfig)
  {
    Serial.println("saving config");
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
#endif
    json["wm_ssid"] = wm_ssid;
    json["wm_pw"] = wm_pw;
    json["mqtt_server"] = mqtt_server;
    json["device_label"] = device_label;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
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
    // end save
  }

  // MQTT setup and connection
  const char *MQTT_IP = mqtt_server;
  client.setServer(MQTT_IP, 1883);
  client.setCallback(callback);
}

void loop()
{
  // Node-RED reconnect
  reconnect();

  // test publishing to Node-RED
  if (millis() - timer > PUBLISH_FREQUENCY)
  { 
    String var = "100";
    client.publish("esp32/wm_test", var.c_str());
  }
  timer = millis();
}
