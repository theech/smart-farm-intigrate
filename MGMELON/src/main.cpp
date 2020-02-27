#include <Arduino.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include "Wire.h"
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(19, 18); // RX, TX (Arduin Uno)

// what digital pin we're connected to
#define inDHT 10
#define outDHT 11
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define inDHTTYPE DHT22 // DHT 22  (AM2302), AM2321
#define outDHTTYPE DHT22
// photoresistor are defined
#define inLight A8
#define outLight A9
// define moisPin0 for Soil moisture sensor one
// define moisPin1 for Soil moisture sensor two
// define moisPin2 for Soil moisture sensor three
#define moisPin0 A0
#define moisPin1 A1
#define moisPin2 A2
// define keypad row and column
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {9, 8, 7, 6};
// Define status variable to stall the realtime data from inDHT & outDHT
int inHumdStatus = 0;
int outHumdStatus = 0;
int inTempStatus = 0;
int outTempStatus = 0;
// inLightStaus and outLightStatus are Read value from sensor directly.
int inLightStatus = 0;
int outLightStatus = 0;
int inLightPercent;
int outLightPercent;
// Standard photoresistor value is 1024 that mean it will start from 0 to 1023
int lightMin = 0;
int lightMax = 1023;
// define status variable to stall the realtime data form moisture sensor 1, 2, and 3;]
int moisStatus0 = 0;
int moisStatus1 = 0;
int moisStatus2 = 0;
// standart soil moisture
char standNum[8] = "50";
// Sending str to Nodemcu
// String str;
// Communication String
String strInH;
String strInT;
String strOutH;
String strOutT;
String strInLig;
String strOutLig;
String strMois0;
String strMois1;
String strMois2;
String strStandNum;
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht1(inDHT, inDHTTYPE);
DHT dht2(outDHT, outDHTTYPE);
// keypad map the key
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
// initialize the library with the numbers of the interface pins
//Adafruit_LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
int Contrast = 125;
//calling function
void keyPad();
void humidTemp();
void photoresis();
void soilMoisture();
void comnuni();
void LCDSetup();
void debug();

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.println("Smart Melon Farm [MEGE]");
  lcd.backlight();
  lcd.init(); // initialize the lcd
  // DHT22 begin
  dht1.begin();
  dht2.begin();
  // inform the MCU that photoresistor is an input
  pinMode(inLight, INPUT);
  pinMode(inLight, INPUT);
  // inform the MCU that Soil Moisture is an input
  pinMode(moisPin0, INPUT);
  pinMode(moisPin1, INPUT);
  pinMode(moisPin2, INPUT);
}

void loop()
{
  // keypad setup
  keyPad();
  // humidity and Temperature
  humidTemp();
  // photoresistor
  photoresis();
  // soil Moisture
  soilMoisture();
  // show in Nodemcu by serial Monitor as realtime
  comnuni();
  // LCD Disply
  LCDSetup();
  // debuging
  debug();
  // Wait a few seconds between measurements.
  //delay(2000);
}

// keypad function
void keyPad()
{
  char key = keypad.getKey();
  if (key == '*')
  {
    lcd.clear();
    do
    {
      lcd.setCursor(1, 0);
      lcd.print("*SMART MELON FARM*");
      lcd.setCursor(4, 1);
      lcd.print("SET SOIL:");
      lcd.setCursor(13, 1);
      lcd.print(standNum);
      lcd.setCursor(1, 2);
      lcd.print("CONFIRM PRESS [#]");
      lcd.setCursor(2, 3);
      lcd.print("RESET PRESS  [A]");

      if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9')
      {
        if (strlen(standNum) <= 2)
        {
          int num = +strlen(standNum);
          standNum[num] = key;
        }
        else
        {
          memset(standNum, 0, sizeof(standNum));
        }
      }
      else if (key == 'A')
      {
        memset(standNum, 0, sizeof(standNum));
        // Serial.println("RESET");
        lcd.setCursor(13, 1);
        lcd.print("   ");
      }
      key = keypad.getKey();
    } while (key != '#');
    lcd.clear();
  }
}

// humidity and Temperature function
void humidTemp()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  inHumdStatus = dht1.readHumidity();
  outHumdStatus = dht2.readHumidity();
  // Read temperature as Celsius (the default)
  inTempStatus = dht1.readTemperature();
  outTempStatus = dht2.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(inHumdStatus) || isnan(inTempStatus))
  {
    Serial.println("Failed to read from Inside DHT22 sensor!");
    return;
  }
  if (isnan(outHumdStatus) || isnan(outTempStatus))
  {
    Serial.println("Failed to read from Outside DHT22 sensor!");
    return;
  }
}

// photoresistor function
void photoresis()
{
  // Reading the value from sensor directly.
  inLightStatus = analogRead(inLight);
  outLightStatus = analogRead(outLight);
  // if inLightStatus is less than 0, set it equal 0, but if greater than 1023 set it equal 1023
  if (inLightStatus < lightMin)
    inLightStatus = lightMin;
  else if (inLightStatus > lightMax)
    inLightStatus = lightMax;
  // if outLightstatus is less than 0, set it equal 0, but if greater than 1023, set it equal 1023
  if (outLightStatus < lightMin)
    outLightStatus = lightMin;
  else if (outLightStatus > lightMax)
    outLightStatus = lightMax;
  // calculate to %
  inLightPercent = ((inLightStatus / 1023.0) * 100.0);
  outLightPercent = ((outLightStatus / 1023.0) * 100.0);
}

// Soil moisture function
void soilMoisture()
{
  // reading data from each moisture sensors directly
  moisStatus0 = (analogRead(moisPin0) / 1024.0) * 100.0;
  moisStatus1 = (analogRead(moisPin1) / 1024.0) * 100.0;
  moisStatus2 = (analogRead(moisPin2) / 1024.0) * 100.0;
}

// Serial Communication function
void comnuni()
{
  // in and out dht communication sending string
  strInH = String('H') + String(inHumdStatus);
  strInT = String('T') + String(inTempStatus);
  strOutH = String('I') + String(outHumdStatus);
  strOutT = String('U') + String(outTempStatus);
  Serial1.println(strInH);
  Serial1.println(strInT);
  Serial1.println(strOutH);
  Serial1.println(strOutT);
  // in and out light communication sending string
  strInLig = String('L') + String(inLightPercent);
  strOutLig = String('M') + String(outLightPercent);
  Serial1.println(strInLig);
  Serial1.println(strOutLig);
  // mois 1, 2, and 3 communication sending string
  strMois0 = String('N') + String(moisStatus0);
  strMois1 = String('O') + String(moisStatus1);
  strMois2 = String('P') + String(moisStatus2);
  Serial1.println(strMois0);
  Serial1.println(strMois1);
  Serial1.println(strMois2);
  // standard soil number
  strStandNum = String('Q') + String(standNum);
  Serial1.println(strStandNum);
}

void LCDSetup()
{
  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  lcd.print("IH:");
  lcd.print(inHumdStatus);
  lcd.setCursor(7, 0);
  lcd.print("IT:");
  lcd.print(inTempStatus);
  lcd.setCursor(14, 0);
  lcd.print("IL:");
  lcd.print(inLightStatus);
  lcd.setCursor(0, 1);
  lcd.print("OH:");
  lcd.print(outHumdStatus);
  lcd.setCursor(7, 1);
  lcd.print("OT:");
  lcd.print(outTempStatus);
  lcd.setCursor(14, 1);
  lcd.print("OL:");
  lcd.print(outLightStatus);
  lcd.setCursor(0, 2);
  lcd.print("S1:");
  lcd.print(moisStatus0);
  lcd.setCursor(7, 2);
  lcd.print("S2:");
  lcd.print(moisStatus1);
  lcd.setCursor(14, 2);
  lcd.print("S3:");
  lcd.print(moisStatus2);
  lcd.setCursor(0, 3);
  lcd.print("METER:24L/H");
  lcd.setCursor(12, 3);
  lcd.print("PUMP:OFF");
}

void debug() {
  Serial.println("\t\t\tSmart Melon Farm [MEGE]");
  Serial.println("------------------------------------------------------------------------");
  // show in MG module as realtime
  Serial.print("Inside Humidity: \t");
  Serial.print(inHumdStatus);
  Serial.print(" %\t|\t");
  Serial.print("Inside Temperature: \t");
  Serial.print(inTempStatus);
  Serial.println(" *C\t|");
  Serial.print("Outside Humidity: \t");
  Serial.print(outHumdStatus);
  Serial.print(" %\t|\t");
  Serial.print("Outside Temperature: \t");
  Serial.print(outTempStatus);
  Serial.println(" *C\t|");
  Serial.println("------------------------------------------------------------------------");
  // showing status in monitor
  Serial.print("Inside Light: \t\t");
  Serial.print(inLightPercent);
  Serial.print(" %\t|\tOutside Light: \t\t");
  Serial.print(outLightPercent);
  Serial.println(" %\t|");
  Serial.println("------------------------------------------------------------------------");
  // showing each moisture sensors to screen
  Serial.print("Soil moisture one \t");
  Serial.print(moisStatus0);
  Serial.print(" %\t|\t");
  Serial.print("Soil moisture two \t");
  Serial.print(moisStatus1);
  Serial.print(" %\t|\t");
  Serial.print("Soil moisture three \t");
  Serial.print(moisStatus2);
  Serial.println(" %\t|\t");
  Serial.println("-----------------------------------------------------------------------------------------------------------------");
  // standard soil number
  Serial.print("Standardi soil: ");
  Serial.println(standNum);
  Serial.println("----------------------------------------------------------------------------------------------------------------- \n\n");
}