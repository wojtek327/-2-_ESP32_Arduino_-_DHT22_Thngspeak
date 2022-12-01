#include <WiFi.h>
#include "DHT.h"

#define uS_TO_S_FACTOR 1000000  
#define TIME_TO_SLEEP  50      

#define DHTTYPE DHT22   // DHT 22
uint8_t DHTPin = 4; 

#define ONE_WIRE_LINE 2

String apiKey = "API_KEY";     

const char* serverAddress = "api.thingspeak.com";  
char ssid[] = "SSID";  // your network SSID (name)
char pass[] = "PASS"; // your network password

unsigned long previousMillis = 0;  
const long interval = 10000;   

float Temperature;
float Humidity;

WiFiClient client;
DHT dht(DHTPin, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(DHTPin, INPUT);
  dht.begin();
     
  int connectTries = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
   
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    connectTries++;
    if(connectTries > 30){
      ESP.deepSleep(30000000);
    }
  }
  Serial.println("Wifi connected");

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      Temperature = dht.readTemperature(); // Gets the values of the temperature
      Humidity = dht.readHumidity(); // Gets the values of the humidity 

      if(!isnan(Temperature) && !isnan(Humidity))
      {
        Serial.print("Temp: ");
        Serial.print(Temperature, 3);
        Serial.print(" st.C Humid: ");
        Serial.print(Humidity, 3);
        Serial.println(" %;");
      }
      else
      {
        Serial.println("Temp: 0 st.C Humid: 0%;");
        Temperature = 0.0;
        Humidity = 0.0;
      }
      communicateWithServer(Temperature, Humidity);
  }
}

void communicateWithServer(float temp, float humid)
{
    if(client.connect(serverAddress,80)){
        String dataToSend = apiKey;
        dataToSend += "&field1=";
        dataToSend += String(temp);
        dataToSend += "&field2=";
        dataToSend += String(humid);
   
        Serial.println(dataToSend);
  
        dataToSend += "\r\n\r\n";

        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(dataToSend.length());
        client.print("\n\n");
        client.print(dataToSend);
        delay(500);
  }
  esp_deep_sleep_start();
}
