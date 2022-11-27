#include <Arduino.h>

// FS.h must be importer before all other libraries.
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include <WIFI.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <aws-config.h>

struct Timer
{
  unsigned long previousMillis = 0;

  void reset()
  {
    previousMillis = millis();
  }

  unsigned long getElapsedTime()
  {
    return millis() - previousMillis;
  }

  bool isFinished(unsigned long interval)
  {
    return getElapsedTime() >= interval;
  }
};

struct AnalogMultiplexerPinPair
{
  const int enable;
  const int input;
};

const String AWS_CONFIG_PATH = "/aws-config.json";

const String TOPIC = "devices/" + String(DEVICE_ID) + "/temperature";

const AnalogMultiplexerPinPair LIGHT_PINS = {
    .enable = 17,
    .input = 33,
};

const AnalogMultiplexerPinPair NTC_PINS[] = {
    {.enable = 16, .input = 32},
    {.enable = 4, .input = 35},
    {.enable = 2, .input = 39},
    {.enable = 15, .input = 36},
};

const int TMP_36_INPUT_PIN = 34;

const int TMP_36_ENABLE_PINS[] = {
    13,
    12,
    14,
    27,
    26,
};

const int AVERAGE_WINDOW_SIZE = 5;

struct NtcConfig
{
  // Value of  resistor used for the voltage divider
  const float rRef = 1000;
  // Nominal resistance at 25⁰C
  const float nominalResistance = 1000;
  // The beta coefficient or the B value of the thermistor
  // (usually 3000-4000) check the datasheet for the accurate value.
  const float transistorBeta = 3730;
  // temperature for nominal resistance (almost always 25⁰ C)
  const float nominalTemperature = 25;
  // Reference voltage of the ADC
  const float vRef = 3.3;
} const NTC_CONFIG;

const int DOOR_OPEN_LIGHT_THRESHOLD = 2000;

const unsigned long LIGHT_CHECK_DELAY = 500;
const unsigned long DOOR_CLOSE_DELAY = 60000;
const unsigned long TEMP_PUBLISH_DELAY_DOOR_OPEN = 1000;
const unsigned long TEMP_PUBLISH_DELAY_DOOR_CLOSED = 10000;

Timer tempPublisherTimer;
Timer lightCheckTimer;
Timer doorCloseDelayTimer;

WiFiClientSecure wifiClient;
PubSubClient pubSubClient(wifiClient);

bool doorOpen;
unsigned long tempPublishDelay = TEMP_PUBLISH_DELAY_DOOR_CLOSED;

DynamicJsonDocument publishTempJsonDoc(1024);

float getNtcTemp(AnalogMultiplexerPinPair pins)
{
  digitalWrite(pins.enable, HIGH);
  int samples = 0;
  for (int i = 0; i < AVERAGE_WINDOW_SIZE; i++)
  {
    samples += analogRead(pins.input);
  }
  digitalWrite(pins.enable, LOW);
  float average = samples / AVERAGE_WINDOW_SIZE;
  // Serial.print(average);
  float voltage = average * (NTC_CONFIG.vRef / 4096.0);
  float resistance = ((NTC_CONFIG.rRef / voltage) * NTC_CONFIG.vRef) - NTC_CONFIG.rRef;
  float steinhart = resistance / NTC_CONFIG.nominalResistance; // (R/Ro)
  steinhart = log(steinhart);                                  // ln(R/Ro)
  steinhart /= NTC_CONFIG.transistorBeta;                      // 1/B * ln(R/Ro)
  steinhart += 1.0 / (NTC_CONFIG.nominalTemperature + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                                 // Invert
  float temp = steinhart - 273.15;
  return temp;
}

float getTMP36Temp(int pin)
{
  float volts;
  float tempC;
  float temps[AVERAGE_WINDOW_SIZE];
  float sum = 0;

  digitalWrite(pin, HIGH);
  for (int i = 0; i < AVERAGE_WINDOW_SIZE; i++)
  {
    int temp = analogRead(TMP_36_INPUT_PIN);
    volts = temp / 1024.0;
    tempC = (volts - 0.5) * 100;
    temps[i] = tempC;
    sum += temps[i];
  }
  digitalWrite(pin, LOW);

  return sum / AVERAGE_WINDOW_SIZE;
}

void updateDoorOpen()
{
  digitalWrite(LIGHT_PINS.enable, HIGH);
  int light = analogRead(LIGHT_PINS.input);
  digitalWrite(LIGHT_PINS.enable, LOW);
  doorOpen = light > DOOR_OPEN_LIGHT_THRESHOLD;
}

void setupPins()
{
  pinMode(LIGHT_PINS.enable, OUTPUT);
  pinMode(LIGHT_PINS.input, INPUT);
  digitalWrite(LIGHT_PINS.enable, LOW);

  for (auto pin : NTC_PINS)
  {
    pinMode(pin.enable, OUTPUT);
    pinMode(pin.input, INPUT);
    digitalWrite(pin.enable, LOW);
  }

  for (auto pin : TMP_36_ENABLE_PINS)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
}

void setupWifi()
{
  WiFiManager wifiManager;

  // Maximum time for configuration portal to stay open
  wifiManager.setTimeout(180);

  // Auto connects to the saved WiFi network or opens a configuration portal
  if (!wifiManager.autoConnect("TempSensorSetup"))
  {
    Serial.println("WiFi setup failed.");
    Serial.println("Rebooting ...");
    ESP.restart();
  }

  Serial.println("Connected to WiFi network: " + wifiManager.getWiFiSSID());
}

void checkWifiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("No WiFi connection.");
    Serial.println("Rebooting ...");
    ESP.restart();
  }
}

void pubSubCallback(char *topic, byte *payload, unsigned int length)
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

void configurePubSub()
{
  wifiClient.setCACert(AWS_CERT_CA);
  wifiClient.setCertificate(AWS_CERT_CRT);
  wifiClient.setPrivateKey(AWS_CERT_PRIVATE);

  pubSubClient.setCallback(pubSubCallback);
  pubSubClient.setServer(AWS_IOT_ENDPOINT, 8883);
}

void checkPubSubConnection()
{
  while (!pubSubClient.connected())
  {
    Serial.println("Connecting to PubSub broker ...");
    if (pubSubClient.connect(DEVICE_ID))
    {
      Serial.println("PubSub connected!");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void resetTimers()
{
  lightCheckTimer.reset();
  tempPublisherTimer.reset();
  doorCloseDelayTimer.reset();
}

void updateTempPublisherDelay()
{
  if (lightCheckTimer.isFinished(LIGHT_CHECK_DELAY))
  {
    lightCheckTimer.reset();

    updateDoorOpen();

    if (doorOpen)
    {
      tempPublishDelay = TEMP_PUBLISH_DELAY_DOOR_OPEN;
      doorCloseDelayTimer.reset();
    }

    if (doorCloseDelayTimer.isFinished(DOOR_CLOSE_DELAY))
    {
      tempPublishDelay = TEMP_PUBLISH_DELAY_DOOR_CLOSED;
    }
  }
}

void postTempValues()
{

  publishTempJsonDoc["d"] = doorOpen;
  publishTempJsonDoc["bbr"] = getNtcTemp(NTC_PINS[0]);
  publishTempJsonDoc["tbr"] = getNtcTemp(NTC_PINS[1]);
  publishTempJsonDoc["tbl"] = getNtcTemp(NTC_PINS[2]);
  publishTempJsonDoc["bbl"] = getNtcTemp(NTC_PINS[3]);
  publishTempJsonDoc["o"] = getTMP36Temp(TMP_36_ENABLE_PINS[0]);
  publishTempJsonDoc["tfr"] = getTMP36Temp(TMP_36_ENABLE_PINS[1]);
  publishTempJsonDoc["bfr"] = getTMP36Temp(TMP_36_ENABLE_PINS[2]);
  publishTempJsonDoc["tfl"] = getTMP36Temp(TMP_36_ENABLE_PINS[3]);
  publishTempJsonDoc["bfl"] = getTMP36Temp(TMP_36_ENABLE_PINS[4]);
  
  String payload;
  serializeJson(publishTempJsonDoc, payload);

  pubSubClient.publish(TOPIC.c_str(), payload.c_str());
}

void updateTempPublisher()
{
  if (tempPublisherTimer.isFinished(tempPublishDelay))
  {
    postTempValues();
    tempPublisherTimer.reset();
  }
}

void setup()
{
  Serial.begin(115200);

  setupPins();
  setupWifi();
  configurePubSub();
  checkPubSubConnection();
  resetTimers();
}

void loop()
{
  checkPubSubConnection();
  checkWifiConnection();
  updateTempPublisherDelay();
  updateTempPublisher();
}
