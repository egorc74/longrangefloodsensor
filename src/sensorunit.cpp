#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>
#include <driver/adc.h>



#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10       /* Time ESP32 will go to sleep (in seconds) */
#define SwitchSensorPort 26
#define SwitchLoRaPOrt 33
// #define SensorPort 32
RTC_DATA_ATTR int sleepCount=0;
int healthCheckCount=24*3600/TIME_TO_SLEEP;

String SerialNumber="123456";
//LoRa settings
#define ss 5
#define rst 14
#define dio0 2
String SN, type, communication, status;
String healthCheckMessage=SerialNumber+"201"+"1"+"1";
String floodMessage=SerialNumber+"301"+"1"+"1";

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
   
  }
   delay(1000);
}

String createMessage(String SN, String type, String communication, String status){
  String message = SN + type + communication + status;
  return message;
}

void sendMessage(String message){
  for(int i=0; i<=100; i++){
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  delay(5);
  // Serial.print("S
   }  
}
String receiveMessage(){
  String LoRaData;
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet");

    // read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }
    return LoRaData;
  } 
  else{
    // Serial.print("\npaket was not received");
    return "";
  }
}

void extractValues(String data) {
    if (data.length() >= 10) { // Check if the string is long enough
        SN = data.substring(0, 6);               // First 6 characters
        type = data.substring(6, 9);             // Next 3 characters
        communication = data.substring(9,10);          // Next character
        status = data.substring(10,11);                // Last character
    } else {
        Serial.println("\n Error: Input data is too short.");
    }
}

void commandFunction(String receivedMessage){
  extractValues(receivedMessage);
  if (SN==SerialNumber){
    if(type.charAt(0) == '1'){
     //activate
      if(type=="101"){
        if(communication=="0"){
          //request
          status="1";
          String message=createMessage(SN,type,"1",status);
          sendMessage(message);
          Serial.print("\nmessage is sent: " + message);
        }
        else{
          //eror in response
          status="0";
           String message=createMessage(SN,type,"1",status);
          sendMessage(message);
        }
      }
    }
    else if(type.charAt(0) == '2'){
    //health
      if(type=="201"){
        //SU health check// 
        //Add battery check//
        // FUNCTIONALITY//
          if(communication=="0"){
            //request//
            status="1";
             String message=createMessage(SN,type,"1",status);
             delay(100);
          sendMessage(message);
          }
          else{
            //eror in response//
            status="0"; 
             String message=createMessage(SN,type,"1",status);
          sendMessage(message);
          }
      }
      if(type=="202"){
        //CCU health Check //
        status="0";
      }
    }
    else if(type.charAt(0) == '3'){
      //FLOOD//
      status="1";
    }
    
  }
  else{
  status="0";
  }
}




void settings(){

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(32, INPUT);            // Set GPIO32 as input
  pinMode(SwitchSensorPort,OUTPUT);
  pinMode(SwitchLoRaPOrt,OUTPUT);

  digitalWrite(SwitchSensorPort,LOW);
  digitalWrite(SwitchLoRaPOrt,HIGH);
  analogSetAttenuation(ADC_11db); // Set range to 0-3.3V
  analogReadResolution(12);
  Serial.begin(115200);
  while(!Serial);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //GoSleep settings
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500); 
  }
  LoRa.setSyncWord(0x12);

}

void dailyHealthCheck(int sleepCount){
  if(sleepCount==healthCheckCount){
    
    // Serial.print(sleepCount);
    sendMessage(healthCheckMessage);
    sleepCount=0;
  }
}
void blink()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}  

void sensorCheck(){
  analogRead(32);  // Read and discard
  delay(10);            // Allow ADC to stabilize
  int sensorValue = analogRead(32); // Read the analog value
  Serial.print(sensorValue);
  if(sensorValue<100){
  sendMessage(floodMessage);
  Serial.print("\nflood message is send:" + sensorValue);
  // delay(1000);
  }
}


void setup()
{ 
  settings(); 

  print_wakeup_reason();
  delay(100);
  sensorCheck();
  dailyHealthCheck(sleepCount);
  String receivedmessage;
  unsigned long startMillis = millis();
  for(int i=0; i<2000; i++ ){
    receivedmessage=receiveMessage();
    if (receivedmessage.length()>5){
      delay(500);
      commandFunction(receivedmessage);
      break;
    }
  }
   
  
 
  //####Going to Sleep####//
  unsigned long endMillis = millis();   // Record the end time
  unsigned long duration = endMillis - startMillis;
  digitalWrite(SwitchSensorPort,HIGH);
  digitalWrite(SwitchLoRaPOrt,LOW);


  Serial.print("\nLoop duration (ms): ");
  Serial.println(duration);  
  Serial.flush(); 
  ++sleepCount;
  esp_deep_sleep_start();
 
}
 
void loop() 
{ 
}


