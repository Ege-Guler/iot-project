#include <ESP8266WiFi.h>
#include <RTClib.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <iostream>

//DHT22
#define DHTPIN 2            //DHT22 PIN
#define DHTTYPE DHT22       // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin not used
#define SCREEN_ADDRESS 0x3C  // I2C address for OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Wi-Fi network settings
const char* ssid = "";
const char* password = "";

float hum;  //Stores humidity value
float temp; //Stores temperature value

void setup() {
  Serial.begin(57600);
  delay(10);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting to compile time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
}

void checkMemory() {
  Serial.print("Free RAM: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
}

String fetchPrediciton() {
  WiFiClient client;
  HTTPClient http;

  http.begin(client, "http://192.168.1.23:5000/predict");
  http.addHeader("Content-Type", "application/json");

  DateTime now = rtc.now();
  String payload = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + ",\"hour\":" + String(now.hour()) + ",\"day\":" + String(now.day()) + ",\"month\":" + String(now.month()) + "}";

  int httpCode = http.POST(payload);

  float serverTemp = -1;
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Server Response: " + response);
      http.end();
      return response;
  } else {
    Serial.println("Failed to fetch temperature from server");
  }

  http.end();
  return "error";
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Attempting reconnection...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nReconnected to WiFi");
  }

  hum = dht.readHumidity();
  temp = dht.readTemperature();

  String prediction = fetchPrediciton();

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.println(temp);

  Serial.println(prediction);

  delay(1000);
  checkMemory();

  DateTime now = rtc.now();
  Serial.print(now.hour());
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");
  if (now.second() < 10) Serial.print("0");
  Serial.print(now.second());
  Serial.print(" ");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.year());
  Serial.println();


  display.clearDisplay();
  display.setCursor(10, 5);

  displayTimeOled();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.print("Temp: ");
  display.print(temp);
  display.print((char)247);
  display.print("C");
  display.setCursor(0, 30);
  display.print("Server: ");
  display.print(prediction);
  display.print((char)247);
  display.print("C");
  display.display();
}

unsigned long getUnixTime() {
  DateTime now = rtc.now();
  return now.unixtime();
}

void displayTimeOled() {
  DateTime now = rtc.now();

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.print(now.hour());
  display.print(":");
  if (now.minute() < 10) {
    display.print("0");
  }
  display.print(now.minute());
  display.print(":");
  if (now.second() < 10) {
    display.print("0");
  }
  display.print(now.second());

  display.print("  ");
  display.print(now.day());
  display.print("/");
  display.print(now.month());
  display.print("/");
  display.print(now.year());

  display.setCursor(50, 10);
  switch (now.dayOfTheWeek()) {
    case 0: display.print("Sunday"); break;
    case 1: display.print("Monday"); break;
    case 2: display.print("Tuesday"); break;
    case 3: display.print("Wednesday"); break;
    case 4: display.print("Thursday"); break;
    case 5: display.print("Friday"); break;
    case 6: display.print("Saturday"); break;
  }

}


