#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "themakerguy";
const char* password = "damnuman";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.43.176";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// DHT Sensor - GPIO 5 = D1 on ESP-12E NodeMCU board
#define INDHT D2
#define OUTDHT D3

// Initialize DHT sensor. change the line below whatever DHT type you're using DHT11, DHT21 (AM2301), DHT22 (AM2302, AM2321)
DHT dht[] ={
  {INDHT, DHT11},
  {OUTDHT, DHT11}
};

// Timers auxiliar variables
unsigned long now = millis();
unsigned long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("envsensors")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  for(auto& sensor : dht){
    sensor.begin();
  }

}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("envsensors");

  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 30000) {
    lastMeasure = now;
    // initialize temperatures and humidities array to store read() values
    float humidities[2];
    float temperatures[2];
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    for(int i = 0; i < 2; i++){
      humidities[i] = dht[i].readHumidity();
      temperatures[i] = dht[i].readTemperature();
    }

    // Check if any reads failed and exit early (to try again).
    //   if(isnan(humidities[0]) || isnan(temperatures[0])){
    //     Serial.println("Failed to read from inside DHT sensor!");
    //     return;
    //   } 
    //   if(isnan(humidities[1]) || isnan(temperatures[1])){
    //     Serial.println("Failed to read from outside DHT sensor!");
    //     return;
    //   }

    // Computes temperature values in Celsius
    static char inTemperature[7];
    static char outTemperature[7];
    dtostrf(temperatures[0], 6, 2, inTemperature);
    dtostrf(temperatures[1], 6, 2, outTemperature);
    
    // Uncomment to compute temperature values in Fahrenheit 
    // float hif = dht.computeHeatIndex(f, h);
    // static char temperatureTemp[7];
    // dtostrf(hic, 6, 2, temperatureTemp);
    
    static char inHumidity[7];
    static char outHumidity[7];
    dtostrf(humidities[0], 6, 2, inHumidity);
    dtostrf(humidities[1], 6, 2, outHumidity);

    // Publishes Temperature and Humidity values
    client.publish("in/temperature", inTemperature);
    client.publish("in/humidity", inHumidity);
    client.publish("out/temperature", outTemperature);
    client.publish("out/humidity", outHumidity);
    
    Serial.print("in Humidity: ");
    Serial.print(temperatures[0]);
    Serial.print(" %\t in Temperature: ");
    Serial.print(humidities[0]);
    Serial.println(" *C ");
    Serial.print("out Humidity: ");
    Serial.print(temperatures[1]);
    Serial.print(" %\t out Temperature: ");
    Serial.print(humidities[1]);
    Serial.println(" *C ");
  }
} 