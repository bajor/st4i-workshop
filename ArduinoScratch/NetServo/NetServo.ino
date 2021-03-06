
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <Servo.h> 
#include "ST4iWifiManager.h"
#include <WebSocketClient.h>
#include <JsonParser.h>

using namespace ArduinoJson::Parser;


ST4iWifiManager wifi; 
Servo servo1; 

#define SERVER      "node.seb.ly"
#define PATH         "/"
#define PORT         8102


#define WIFI "SebsAirport"
#define PASSWORD "Internet"

String id = String("Seb");

JsonParser<100> parser;

const int ledPin = 44; 
const int buttonPin = 21;
const int wifiLed = 2; 
const int heartbeatFrequency = 20000; 
const int servoPin = 7;

boolean buttonPushed = false; 

uint32_t ip;
unsigned long lastSend;
int servoPosition = 10; 

WebSocketClient wsclient;

void setup() {

  Serial.begin(115200);

  // wifi.init(int okLedPin, int brightness (0 to 255, but only works on PWM pins))
  wifi.init(wifiLed, 30); 
  wifi.connect(WIFI, PASSWORD, WLAN_SEC_WPA2); 

  pinMode(ledPin, OUTPUT); 
  pinMode(buttonPin, INPUT); 

  digitalWrite(ledPin, HIGH); 
  delay(1000); 
  digitalWrite(ledPin, LOW);


  Serial.println("connecting to socket server");


  while(!wsclient.connect(&wifi.client, SERVER, PATH, PORT)) { 
    Serial.println("couldn't connect :( Trying again..."); 
    delay(1000);  
  }
  
  servo1.attach(servoPin);
   
  Serial.println("Socket connected!"); 
  // socketGreenLight();
  wsclient.setDataArrivedDelegate(dataArrived);
  //String sendstring = String("{\"type\" : \"register\", \"id\" : \""+"Seb"+"\"}");
  String sendstring = String(String("{\"type\" : \"register\", \"id\" : \"")+id+String("\"}"));
  wsclient.send(sendstring);
  lastSend = millis(); 

}

void loop() {

  
  // check wifi !
  wifi.monitor(); 
  // check websocket
  wsclient.monitor();

 //int position = map(analogRead(A5),0,1024,0,180);
 // servo1.write(position);

  // heartbeat code to keep the connection alive 
  if(millis() - lastSend > heartbeatFrequency) { 
    Serial.print("*"); 
    wsclient.send("{\"type\" : \"heartbeat\"}");
    lastSend = millis();
  }


  if(!digitalRead(buttonPin) != buttonPushed) { 
    buttonPushed = !buttonPushed; 
    
 //   servo1.write(buttonPushed?20:180); 
    String msg; 
    if(buttonPushed) 
      msg = String("{\"type\" : \"light\", \"data\" : 1, \"id\":\"Seb\"}");
    else 
      msg = String("{\"type\" : \"light\", \"data\" : 0, \"id\":\"Seb\"}");

    wsclient.send(msg);
  }
 

}

void dataArrived(WebSocketClient wsclient, String data) {

  char jsonchar[data.length()+1];
  data.toCharArray(jsonchar, data.length()+1);

  // should poss parse json? 
  JsonObject json = parser.parse(jsonchar);
  Serial.println("Data Arrived: " + data);

  const char* msgid = json["id"]; 
  
  if(strcmp(msgid, id.c_str())!=0) return; 
  
  const char* type = json["type"]; 

  Serial.println(type); 
  if(strcmp(type,"light")==0) { 

    Serial.println("found light"); 
    int state = json["data"]; 


    if(state==1) {
      digitalWrite(ledPin, HIGH); 
      servoPosition = 180; 
      

    } 
    else {
      digitalWrite(ledPin, LOW); 
      servoPosition = 20; 
    }
    Serial.println(servoPosition); 
    servo1.write(servoPosition);
  }
}






