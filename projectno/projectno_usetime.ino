#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include <Servo.h> 
#include <WiFiUdp.h>

const char* ssid     = "iPhonedew";      //wifi
const char* password = "0963617429";       //wifi pass

#define APPID    "FeedingDogSystem"
#define KEY     "FIx3d1RoJhjLVMl"
#define SECRET  "PZaJZWhoADYV2pSLi8fe3UWYu"
#define ALIAS   "Nodemcu"

int limit_hr = 00;
int limit_mm = 00;
int limit_ss = 00;

unsigned int localPort = 2390; 
IPAddress timeServer(129, 6, 15, 28);
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;
int hh;
int mm;
int ss;

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
        myservo.attach(4);
        myservo.write(45);
        delay(1000);
        myservo.detach();
         microgear.chat("web", "OK Done! OPEN Feed");
        
      } else if (stateStr == "CLOSE") {
        limit++; 
       myservo.attach(4);
        myservo.write(5);
        delay(1000);
        myservo.detach();
         microgear.chat("web", "OK Done! 1  CLOSE Feed");
      }
      else if (stateStr == "OPEN_1") {
       myservo.attach(4);
        myservo.write(20);
        delay(2000);                    //wait1
        myservo.write(5);
        delay(1000);
        myservo.detach();
        microgear.chat("web", "OK Done! level 1");
      }
      else if (stateStr == "OPEN_2") {
       myservo.attach(4);
        myservo.write(30);
        delay(1000);                  //wait2
        myservo.write(5);
        delay(1000);          
        myservo.detach();
        microgear.chat("web", "OK Done! level 2");
      }
      else if (stateStr == "OPEN_3") {
        myservo.attach(4);
        myservo.write(40);
        delay(1000);                 //wait3
        myservo.write(5);
        delay(1000);                  
        myservo.detach();
        microgear.chat("web", "OK Done! level 3");
      }
     
    }
    else if (limit==1) //limit dog feed **************************************************************
    {
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
  myservo.attach(4);
        myservo.write(5);
        delay(500);
        myservo.detach();
}

void setup() {
  Serial.begin(115200);

  Serial.println("Starting...");
  
  pinMode(relayPin, OUTPUT);

  microgear.on(MESSAGE,onMsghandler);
  microgear.on(CONNECTED,onConnected);

  if (WiFi.begin(ssid, password)) {

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      myservo.attach(4);
        myservo.write(5);
        delay(1000);
        myservo.detach();
      Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    udp.begin(localPort);
    //uncomment the line below if you want to reset token -->
    microgear.resetToken();
    microgear.init(KEY, SECRET);
    microgear.connect(APPID);
  }
}


void loop() {
  if (microgear.connected()) {
    microgear.loop();
    WiFi.hostByName(ntpServerName, timeServer); 
    sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
    Serial.println("connect...");
    int cb = udp.parsePacket();
  if (!cb) {
  }
  else {
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = (secsSince1900 - seventyYears)+(3600*7);
    // print Unix time:
    Serial.println(epoch);

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    hh = (epoch  % 86400L) / 3600;
    mm = (epoch  % 3600) / 60;
    ss = epoch % 60;
    if(hh==limit_hr  && mm==limit_mm && limit==1){          //reset day
       Serial.println("Reset"); 
       microgear.chat("web", "RESET !! New Day");
        limit = 0; 
    }
    Serial.print(hh); // print the hour (86400 equals secs per day)
    Serial.print(':');
    
    Serial.print(mm); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    
    Serial.println(ss); // print the second
     
  }
  // wait ten seconds before asking for the time again
  } else {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
  
  delay(1000);

   
   
}
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
