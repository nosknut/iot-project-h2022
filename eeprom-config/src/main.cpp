#include <FS.h>          //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

#ifdef ESP32
#include <SPIFFS.h>
#endif

#include <Arduino.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#include <PubSubClient.h>

/****************************************
 * Variables
 ****************************************/

// flag for saving data
bool shouldSaveConfig = false;

// variables from WM
char wm_ssid[40]; 
char wm_pw[40];
char mqtt_server[40] = "";
char device_label[40] = "";

// timers for publishing
const int PUBLISH_FREQUENCY = 1000; // Update rate in milliseconds
unsigned long timer;

WiFiClient espClient;
PubSubClient client(espClient);

// pins for sensors
const int analogPin = 34;
const int lightPin = 17;
const int lightPin_A = 33;
const int ntc1_pin_D = 16;
const int ntc1_pin_A = 32;
const int ntc2_pin_D = 4;
const int ntc2_pin_A = 35;
const int ntc3_pin_D = 2;
const int ntc3_pin_A = 39;
const int ntc4_pin_D = 15;
const int ntc4_pin_A = 36;
const int tmp31_1_pin = 13;
const int tmp31_2_pin = 12;
const int tmp31_3_pin = 14;
const int tmp31_4_pin = 27;
const int tmp31_5_pin = 26;

// Variables for NTC calculation
const int samplingrate = 5;            // number of samples to take
const float Rref = 1000;               // Value of  resistor used for the voltage divider
const float nominal_resistance = 1000; // Nominal resistance at 25⁰C
const float beta = 3730;               // The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value.
const float nominal_temperature = 25;  // temperature for nominal resistance (almost always 25⁰ C)
const float Vref = 3.3;                // Reference voltage of the ADC

// variables for temp calculation
int temp;
float volts;
float tempC;
float temps[10];

// timers for publishing to Node-RED
int TEMP_PUBLISH_FREQUENCY = 5000;
unsigned long temp_timer;
unsigned long light_timer;


/****************************************
 * Auxiliar Functions
 ****************************************/

// pinModes for pins
void pinPins()
{
  pinMode(lightPin, OUTPUT);
  pinMode(ntc1_pin_D, OUTPUT);
  pinMode(ntc2_pin_D, OUTPUT);
  pinMode(ntc3_pin_D, OUTPUT);
  pinMode(ntc4_pin_D, OUTPUT);
  pinMode(tmp31_1_pin, OUTPUT);
  pinMode(tmp31_2_pin, OUTPUT);
  pinMode(tmp31_3_pin, OUTPUT);
  pinMode(tmp31_4_pin, OUTPUT);
  pinMode(tmp31_5_pin, OUTPUT);
  digitalWrite(lightPin, LOW);
  digitalWrite(ntc1_pin_D, LOW);
  digitalWrite(ntc2_pin_D, LOW);
  digitalWrite(ntc3_pin_D, LOW);
  digitalWrite(ntc4_pin_D, LOW);
  digitalWrite(tmp31_1_pin, LOW);
  digitalWrite(tmp31_2_pin, LOW);
  digitalWrite(tmp31_3_pin, LOW);
  digitalWrite(tmp31_4_pin, LOW);
  digitalWrite(tmp31_5_pin, LOW);
}

// resistance temp get
float getTempNtc(int D_pin, int A_pin)
{
  digitalWrite(D_pin, HIGH);
  int samples = 0;
  for (int i = 0; i < samplingrate; i++)
  {
    samples += analogRead(A_pin);
  }
  digitalWrite(D_pin, LOW);
  float average = samples / samplingrate;
  // Serial.print(average);
  float voltage = average * (Vref / 4096.0);
  float resistance = ((Rref / voltage) * Vref) - Rref;
  float steinhart = resistance / nominal_resistance; // (R/Ro)
  steinhart = log(steinhart);                        // ln(R/Ro)
  steinhart /= beta;                                 // 1/B * ln(R/Ro)
  steinhart += 1.0 / (nominal_temperature + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                       // Invert
  float temp = steinhart - 273.15;
  return temp;
}

// TMP36 temp get
float getTemp(int pin)
{
  digitalWrite(pin, HIGH);
  for (int i = 0; i < 10; i++)
  {
    temp = analogRead(analogPin);
    volts = temp / 1024.0;
    tempC = (volts - 0.5) * 100;
    temps[i] = tempC;
  }
  digitalWrite(pin, LOW);
  float sum = 0;
  for (int i = 0; i < 10; i++)
  {
    sum += temps[i];
  }
  return sum / 10;
}

// door check with light sensor
int getLight(int D_pin, int A_pin)
{
  digitalWrite(D_pin, HIGH);
  int light = analogRead(A_pin);
  digitalWrite(D_pin, LOW);
  // if (light > 1000) {
  //   return true;
  // }
  // return false;
  return light;
}

// combines temp values into one string and publishes to Node-RED
void postTempValues()
{
  // NTC1/NTC2/NTC3/NTC4/TMP31_1/TMP31_2/TMP31_3/TMP31_4/TMP31_5
  String temps = String(getLight(lightPin, lightPin_A)) + "," 
  + String(getTempNtc(ntc1_pin_D, ntc1_pin_A)) + "," 
  + String(getTempNtc(ntc2_pin_D, ntc2_pin_A)) + "," 
  + String(getTempNtc(ntc3_pin_D, ntc3_pin_A)) + "," 
  + String(getTempNtc(ntc4_pin_D, ntc4_pin_A)) + "," 
  + String(getTemp(tmp31_1_pin)) + "," 
  + String(getTemp(tmp31_2_pin)) + "," 
  + String(getTemp(tmp31_3_pin)) + "," 
  + String(getTemp(tmp31_4_pin)) + "," 
  + String(getTemp(tmp31_5_pin));

  Serial.println("Temps: ");
  Serial.println(temps);
  client.publish("esp32/temps", temps.c_str());
}

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

  // pins pins
  pinPins();

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
  if (!client.connected())
  {
    reconnect();
  }

  // loop that publishes temps to Node_RED every TEMP_PUBLISH_FREQUENCY
  if (millis() - temp_timer > TEMP_PUBLISH_FREQUENCY)
  {
    postTempValues();
    temp_timer = millis();
  }
}

