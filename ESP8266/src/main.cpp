#include <Arduino.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define DHTPIN1 5
#define DHTPIN2 4

// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
//#define DHTTYPE DHT21 // DHT 21 (AM2301)

// Initialize DHT sensor. change the line below whatever DHT type you're using DHT11, DHT21 (AM2301), DHT22 (AM2302, AM2321)
DHT dht[] = {{DHTPIN1, DHTTYPE}, {DHTPIN2, DHT11}};
// initialize temperatures and humidities array to store read() values
float humids[2];
float temps[2];
float ldr[2];
float moisture[3];

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(4800);
  while (!Serial)
    continue;
  // DHTs setup
  for (auto &sensor : dht)
  {
    sensor.begin();
  }

  delay(2000);
}

void dhtsEnvi()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  for (int index = 0; index < 2; index++)
  {
    humids[index] = dht[index].readHumidity();
    temps[index] = dht[index].readTemperature();
  }
}

void receiver()
{
  if (Serial.available())
  {
    // Create new documents must be bigger than sender because it must store the strings
    StaticJsonDocument<300> doc;
    // Read the JSON document from the "link" serial port
    DeserializationError err = deserializeJson(doc, Serial);

    if (err == DeserializationError::Ok)
    {
      // Print the values
      // (we must use as<T>() to resolve the ambiguity)
      Serial.print("inside light = ");
      Serial.print(doc["light1"].as<float>());
      Serial.print("  outside light = ");
      Serial.println(doc["light2"].as<float>());

      Serial.print("moisture 1 = ");
      Serial.print(doc["moisture1"].as<float>());
      Serial.print("  moisture 2 = ");
      Serial.print(doc["moisture2"].as<float>());
      Serial.print("  moisture 3 = ");
      Serial.println(doc["moisture3"].as<float>());

      Serial.print("mode code = ");
      Serial.print(doc["modecode"].as<char>());
      Serial.print("  standard code= ");
      Serial.println(doc["standardcode"].as<String>());
    }
    else
    {
      // Print error to the "debug" serial port
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());

      // Flush all bytes in the "link" serial port buffer
      while (Serial.available() > 0)
        Serial.read();
    }
  }
}

void debug()
{
  // debug here!!
  delay(200);
}

void loop()
{
  dhtsEnvi();
  receiver();
  // debug();
}