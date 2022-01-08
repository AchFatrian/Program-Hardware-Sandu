#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "SMA RJ2";
const char* password = "rumahpakrt";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://sandu-api.herokuapp.com/api/data/12345678";

String data;
int suhu;
int hum;

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  }

String getValue(String dataA, char separator, int index){ 
  int found = 0; 
  int strIndex[] = {0, -1}; 
  int maxIndex = dataA.length()-1; 
 
  for(int i=0; i<=maxIndex && found<=index; i++){ 
    if(dataA.charAt(i)==separator || i==maxIndex){ 
        found++; 
        strIndex[0] = strIndex[1]+1; 
        strIndex[1] = (i == maxIndex) ? i+1 : i; 
    } 
  } 
  return found>index ? dataA.substring(strIndex[0], strIndex[1]) : ""; 
}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      if (Serial.available() > 0) {
        delay(1000);
        char d = Serial.read();
        data += d;
      }
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      if (data.length() > 0){
        suhu = getValue(data,',',0).toInt();
        tinggi = getValue(data,',',1).toInt();
        berat = getValue(data,',',2).toInt();
        detak = getValue(data,',',1).toInt();
        String httpRequestData = "&suhu=" + String(suhu) + "&tinggi=" + String(hum) + "&berat=" + String(berat) + "&detak=" + String(detak) ; 
         // Send HTTP POST request
        int httpResponseCode = http.POST(httpRequestData);
     
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }          
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
