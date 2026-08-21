/*
***********************************
* Temerature and humidity sensor DHT22 an NodeMCU ESP32
***********************************
*/

//#include <ESP8266WiFi.h>
//#include <esp_eap_client.h>
#include <WiFi.h>
//#include <WebServer.h>
#include <DHTesp.h>

//D0 - GPIO16 , wake
//D1 - GPIO5
//D2 - GPIO4
//D3 - GPIO0
//D4 - GPIO2
#define DEBUG true

// Uncomment one of the lines below for whatever DHT sensor type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHTesp::DHT22   // DHT 22  (AM2302), AM2321

// DHT Sensor
uint8_t DHTPin = 4; 
uint8_t DHTPower = 2; 

// Initialize DHT sensor.
DHTesp dht;

float Temperature;
float Humidity;

// WLAN Zugangsdaten
// const char* ssid      = "donnerwetter";
// const char* password  = "my_sc_donner";
const char* ssid      = "home.net";
const char* password  = "my_secret";
// Host zum senden der Daten
const char* datahost  = "192.168.2.251";

// 0.5h 32bit counter aber nur die haelfte nurbar, signed int ? 1800s
//const int sleepTimeS = 1800;  
//const int sleepTimeS = 6;     
const int sleepTimeS = 60;     
   

WiFiServer server(80);


// Verbindung zum WLAN aufbauen
void verbinden() {
  if(DEBUG) {
    delay(10);
    Serial.println("");
    Serial.print(F("Verbinde zu WLAN-Netzwerk '"));
    Serial.print(ssid);
    Serial.print("' ");
  }

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
 
  if(DEBUG) {
    Serial.println(F("-> Verbunden"));  
    Serial.print(F("IP-Adresse: "));
    Serial.print(WiFi.localIP());
    Serial.println("");
  }
}

// Verbindung zu Host herstellen und Sensordaten übermitteln
void sendData (const char* datahost) 
{
  if(DEBUG) {
    Serial.println("");
    Serial.print(F("Verbinde zu '"));
    Serial.print(datahost);
    Serial.println("'");
    } 
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(datahost, httpPort)) {
    Serial.println(F(" Fehler beim Verbinden zum Host"));
    //return;
  }

  Temperature = dht.getTemperature(); // Gets the values of the temperature in  Celsius
  Temperature = ( Temperature * 1.8) + 32; //convert to Fahrenheit for weewx
  Humidity = dht.getHumidity(); // Gets the values of the humidity 

  String url = "/sensors/cgi-bin/dht22.py";
  url += "?sensor=dht22-01";
  url += "&temperature=";
  url += Temperature;
  url += "&humidity=";
  url += Humidity;

  if(DEBUG) {
    Serial.println("");
    Serial.print("URL-Anfrage: ");
    Serial.println(url);
   }
     
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + datahost + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("[Client Timeout]");
      client.stop();
      return;
    }
  }
   
  // Lese alle Daten aus der Antwort des Servers
  while(client.available()){
    String line = client.readStringUntil('\r');
    if(DEBUG) {
      Serial.print(line);
    }
  }

    if(DEBUG) {
  Serial.println("");
  Serial.print(F("Verbindung zu '"));
  Serial.print(datahost);
  Serial.println(F("' beendet."));
    }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(DHTPin, INPUT);
  pinMode(DHTPower, OUTPUT);

  //switch on DHT22
  digitalWrite(DHTPower, HIGH);
  
  dht.setup(DHTPin, DHTTYPE);
  delay(1000);
   
  //connect WiFi
  verbinden();
  //measure and send
  sendData(datahost);
}

void loop() {

  if(DEBUG) {
    Serial.println("");
    Serial.print("Gehe schlafen fuer ");
    Serial.print(sleepTimeS);
    Serial.println(" Sekunden und schalte DHT22 ab");
  }

  //switch off DHT22
  digitalWrite(DHTPower, LOW);
  
//<<esp8266 variant with GPIO pint to rst input
//  ESP.deepSleep(sleepTimeS * 1000000);
//  //wake up after interupt

//  simple sleep
  delay(sleepTimeS * 1000);

//esp32 variant
//  esp_sleep_enable_timer_wakeup(sleepTimeS * 1000000);
//  esp_deep_sleep_start();

  //switch back on DHT22
  if(DEBUG) {
    Serial.println("Schalte DHT22 ein und warte 1s");
  }

  digitalWrite(DHTPower, HIGH);
  dht.setup(DHTPin, DHTTYPE);
  delay(1000);
  
  if(WiFi.status() != WL_CONNECTED) {
    verbinden();
  }
  sendData(datahost);
}


