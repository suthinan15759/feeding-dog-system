#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include <Servo.h> 

const char* ssid     = "iPhonedew";
const char* password = "0963617429";

#define APPID    "FeedingDogSystem"
#define KEY     "FIx3d1RoJhjLVMl"
#define SECRET  "PZaJZWhoADYV2pSLi8fe3UWYu"
#define ALIAS   "Nodemcu"

WiFiClient client;
AuthClient *authclient;
Servo myservo; 
int relayPin = LED_BUILTIN;
int limit = 0; 
MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  Serial.print(topic);
  Serial.print(" : ");
  char strState[msglen];
  for (int i = 0; i < msglen; i++) {
    strState[i] = (char)msg[i];
    Serial.print((char)msg[i]);
  }
  Serial.println();

  String stateStr = String(strState).substring(0, msglen);
    if (limit == 0){
      if (stateStr == "OPEN") {
      
        myservo.write(70);
         microgear.chat("web", "OK Done! OPEN Feed");
        
      } else if (stateStr == "CLOSE") {
        limit = 1; 
       myservo.write(0);
         microgear.chat("web", "OK Done! 1  CLOSE Feed");
      }
      else if (stateStr == "OPEN_1") {
       myservo.write(15);
        microgear.chat("web", "OK Done! level 1");
      }
      else if (stateStr == "OPEN_2") {
        myservo.write(30);
        microgear.chat("web", "OK Done! level 2");
      }
      else if (stateStr == "OPEN_3") {
        myservo.write(45);
        microgear.chat("web", "OK Done! level 3");
      }
     
    }
    else if (limit==1){
      if (stateStr == "RESET") {
        limit = 0; 
         Serial.println("limit=0************************************");
        microgear.chat("web", "RESET !! Now you can feed your dog again ");
        
      }
      if( limit == 1){
      Serial.println("limit=1*******************************");
      microgear.chat("web", "Sorry you can't feed food to your dog because you used qouta today!!  ");
      }
      
      }
 
  
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setName("Nodemcu");
  myservo.write(0);
}

void setup() {
  Serial.begin(115200);

  Serial.println("Starting...");
  myservo.attach(4);
  pinMode(relayPin, OUTPUT);

  microgear.on(MESSAGE,onMsghandler);
  microgear.on(CONNECTED,onConnected);

  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //uncomment the line below if you want to reset token -->
    microgear.resetToken();
    microgear.init(KEY, SECRET);
    microgear.connect(APPID);
  }
}


void loop() {
  if (microgear.connected()) {
    microgear.loop();
    Serial.println("connect...");
  } else {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
  delay(1000);
}
