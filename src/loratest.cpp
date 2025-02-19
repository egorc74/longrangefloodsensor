#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <iostream>
#include <vector>
#include <string>

#include <WiFiManager.h>
#define ss 5
#define rst 14
#define dio0 2
#define touchPin 15
String SN, type, communication, status;


bool bootFlag=true;



const int threshold = 20;
bool touchFlag=true;
String SerialNumber="123456";
std::string CppSerialNumber=std::string(SerialNumber.c_str());

std::vector<std::string> SNlist = {CppSerialNumber,""};

volatile bool interruptFlag=false;

const char* communicationGateway = "http://209.38.6.208:8000/floodpro-sensor-server/api/ccu-data-entrypoint/";
// const char* communicationGateway = "http://209.38.6.208:8000/floodpro-sensor-server/api/ccu-data-entrypoint/";


void serverPostRequest(String SN,String type,String communication,String status){
  std::string SN_cpp = std::string(SN.c_str());
   if (WiFi.status() == WL_CONNECTED && std::find(SNlist.begin(), SNlist.end(), SN_cpp) != SNlist.end()) {
    HTTPClient http;
    http.begin(communicationGateway);
    delay(10);
    http.addHeader("Content-Type", "application/json");
    // Creating JSON payload
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["type_code"] = type;             // Set type value
    jsonDoc["serial_number"] = SN;     // Set serialnumber value
    jsonDoc["communication_code"] = communication;    // Set communication value
    jsonDoc["status"] = status;         // Set status value
    String requestBody;
    serializeJson(jsonDoc, requestBody);
    // const char * headerkeys[] = {"User-Agent","Set-Cookie","Cookie","Date","Content-Type","Connection"} ;
    // size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
    // http.collectHeaders(headerkeys,headerkeyssize);
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response:");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}



void sendMessage(String message){
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  // Serial.print("Sending message via LoRa: " + message);

}
String receiveMessage(){
  String LoRaData;
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      // Serial.print(LoRaData);
    }
    return LoRaData;
  }
  else{
    return "";
  }
}

void bootFunction() {
  sendMessage(SerialNumber + "101" + "0"+"1");
}

void settings(){
WiFiManager wifiManager;
  //wifiManager.resetSettings();
  if (!wifiManager.autoConnect("ESP32_AP")) {
      Serial.println("Failed to connect and hit timeout.");
      ESP.restart();
    }
  pinMode(touchPin,INPUT);
  Serial.begin(115200);
  while(!Serial);
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
    }
  LoRa.setSyncWord(0x12);
}


void extractValues(String data) {
    if (data.length() >= 10) { // Check if the string is long enough
        SN = data.substring(0, 6);               // First 6 characters
        type = data.substring(6, 9);             // Next 3 characters
        communication = data.substring(9,10);          // Next character
        status = data.substring(10,11);                // Last character
    } else {
        Serial.println("Error: Input data is too short.");
    }
}




void setup(){
  settings();

}
void loop(){
  if(bootFlag){
      for(int i=0; i<100; i++){
        bootFunction();
      }
  }
  for(int i=0; i<=1000; i++){
    String message = receiveMessage();
    if (message.length() > 5) {
      Serial.print("\nreceived message: " + message);
      extractValues(message);
      if(message==SN + "101" + "1"+"1"){
      bootFlag=false;
      }
      serverPostRequest(SN, type, communication, status);
      delay(1000);
      break;
    }
  }
}