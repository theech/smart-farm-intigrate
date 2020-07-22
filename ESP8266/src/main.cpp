#include <Arduino.h>
#include <DHT.h>
#include <ArduinoJson.h>

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
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
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
    StaticJsonDocument<600> doc;
    DeserializationError err = deserializeJson(doc, Serial);

    if (err == DeserializationError::Ok)
    {
      // resive the massage from Arduino
      ldr[0] = doc["light1"];
      ldr[1] = doc["light2"];
      moisture[0] = doc["moisture1"];
      moisture[1] = doc["moisture2"];
      moisture[2] = doc["moisture3"];
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
  Serial.print("inside humid: ");
  Serial.print(humids[0]);
  Serial.print("  intside temp: ");
  Serial.print(temps[0]);
  Serial.print("  outside humid: ");
  Serial.print(humids[1]);
  Serial.print("  outside temp: ");
  Serial.print(temps[1]);

  Serial.print("  outside light = ");
  Serial.print(ldr[0]);
  Serial.print("  outside light = ");
  Serial.print(ldr[1]);

  // Print the values
  Serial.print("  moisture 1 = ");
  Serial.print(moisture[0]);
  Serial.print("  moisture 2= ");
  Serial.print(moisture[1]);
  Serial.print("  miosture 3 = ");
  Serial.println(moisture[2]);
  delay(200);
}

void loop()
{
  dhtsEnvi();
  receiver();
  debug();
}