

/*********************************************************************
Demo of:
  Simple Sensor One

Features:
  Read temperature from 3 sensors   (send info)
  Turn on or off a LED              (act on command)


Hardware:
  ESP12E      
  SSD1306     Display monocrome 128x64
  DS18B20     Temperature sensor from Dallas instruments.  1-wire protocol needed.
  MQTT        PubSubClient is used 



| Feature       | Status        | Stage |
| ------------- |:-------------:| -----:|
| ESP12E        | done          |     1 |
| SSD1306       | done          |     2 |
| DS18B20       | done          |     2 |
| WIFI          | done          |     3 |
| MQTT          | done          |     4 |
| NTP           | done          |     5 |
| WS2812b LED strip | done      |     6 |
| OTA           | todo          |  need to fix old toolchain |

// commands to run on the openhab/mqtt server
 mosquitto_sub -d -t /+/+
 sudo tail -f /var/log/openhab/openhab.log

 


  
Based on work written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <ESP8266WiFi.h>            // Network
#include <PubSubClient.h>           // MQTT
#include <Wire.h>
#include <Adafruit_GFX.h>           // ssd1306 display
#include <Adafruit_SSD1306.h>       // ssd1306 display
#include <OneWire.h>                // Temp 
#include <DallasTemperature.h>      // Temp 
#include <NTPClient.h>              // Time
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>      // ws2812b LED strip

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Sensor config start/////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

const char* sensorname = "tempbot01";
// MQTT Topics     [area] ??  /[unit]/[temp/humid/light/sound]/[]/[]
char* temp1Topic = "/tempbot01/temp1";

// Switch1
int switch1 = 0;
char* switch1Topic = "/tempbot01/switch1";
int switch1Port = D8;       // connect to esp12 port wemos mini d1

#define MQTT_SERVER "192.168.x.x"  // MQTT
const char* ssid = "................";           // WIFI
const char* password = "............";    // WIFI

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Sensor config end////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

String mqttClientName = "";     // sets to macadr at reconnect

// Time
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
WiFiUDP ntpUDP;                     // Time
NTPClient timeClient(ntpUDP);       // Time

// LED Setup
  
  // Which pin on the Arduino is connected to the NeoPixels?
  // On a Trinket or Gemma we suggest changing this to 1
  #define PIN            2
  
  // How many NeoPixels are attached to the Arduino?
  #define NUMPIXELS      4
  
  // When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
  // Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
  // example for more information on possible values.
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
  
  int delayval = 500; // delay for half a second


// declaration problem much?? 
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
String macToStr(const uint8_t* mac);

 // Wifi
WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

#define OLED_RESET LED_BUILTIN           //4   wat?!
Adafruit_SSD1306 display(OLED_RESET);

// keep this in case of tool chain changes
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Data wire is plugged into port 4 on the Arduino. D4 for ESP12E.
//#define ONE_WIRE_BUS D4
#define ONE_WIRE_BUS D7

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

 
void drawtexttestscreen() {
  display.setCursor(64-((128/20)*4),32);
  display.print("center");
  display.setCursor(0,0);
  display.print("upleft");
  display.setCursor(128-((128/20)*8),0);
  display.print("upright");
  display.setCursor(0,56);
  display.print("downleft");
  display.setCursor(128-((128/20)*10),56);
  display.print("downright");
} 

void drawvalues()  {
  sensors.requestTemperatures(); // Send the command to get temperatures
  display.setCursor((128/20)*7,0);
  display.print(sensors.getTempCByIndex(0));  // After we got the temperatures, we can print them here.
  display.setCursor((128/20)*7,8);
  display.print(sensors.getTempCByIndex(1));  // After we got the temperatures, we can print them here.
  display.setCursor((128/20)*9,16);
  display.print(String(switch1));  
//  display.setCursor((128/20)*7,16);
//  display.print(sensors.getTempCByIndex(2));  
}


void drawlabels()  {
 /*
  for (uint8_t f=0; f< 3; f++) {
    display.println(f + "Temp: ");
  }
  */
  display.setCursor(0,0);
  display.print("Temp1: ");
  display.setCursor(0,8);
  display.print("Temp2: ");
  display.setCursor(0,16);
  display.print("Switch1: ");
//  display.setCursor(0,24);
//  display.print("Temp3: ");

  display.setCursor(0,32);
  display.print("IP: ");
  display.print(WiFi.localIP());

  display.setCursor(0,40);
  display.print("Flash size: ");
  display.print(ESP.getFlashChipSize());
 
  display.setCursor(0,48);
  display.print("Time: ");
  display.println(timeClient.getFormattedTime()); 
}


void setup()   {  
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setPixelColor(0, pixels.Color(0,150,0));   // LED indicating startup progress
  pixels.show(); // This sends the updated pixel color to the hardware.
  //start the serial line for debugging
  Serial.begin(115200);
  delay(100);
  //Serial.begin(9600);
  Serial.println("");
  Serial.println("Running setup... ");
  Serial.print("Name: ");
  Serial.println(String(sensorname));
  Serial.print("Flash size: ");
  Serial.println(ESP.getFlashChipSize());
  Serial.println("Setting pimodes.");  
  pinMode(D8, OUTPUT); 
  Serial.print("Starting ds18b20 sensors: ");
  sensors.begin();  // Start up the library ds20b18
  Serial.println("Ok!");
  Serial.print("Starting network: ");
  WiFi.begin(ssid, password);     //start wifi subsystem
  Serial.print("Wifi started - ");
  reconnect();                    //attempt to connect to the WIFI network and then connect to the MQTT server
  Serial.println("Connected.");
  //delay(2000);                  //wait a bit before starting the main loop
  pixels.setPixelColor(1, pixels.Color(0,150,0));   // LED indicating startup progress
  pixels.show(); // This sends the updated pixel color to the hardware.
  
  Serial.print("Starting display: ");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  Serial.println("init done.");
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  Serial.println("Splashscreen?");
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();   
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  drawtexttestscreen();     // just to make sure it works ok
  display.display();
  Serial.println("Testscreen.");
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  Serial.println("Is this delay doing someting good?");
  //delay(1000);  //orig
  delay(100);    //20161222

  Serial.println("Display test... done!");
  pixels.setPixelColor(2, pixels.Color(0,150,0));   // LED indicating startup progress
  pixels.show(); // This sends the updated pixel color to the hardware.
  
  Serial.print("Setting up time: ");
  //
  Serial.print("Time service started: ");
  timeClient.begin();
  Serial.print("Ok. Running update: ");
  timeClient.update();
  Serial.print("Ok. Time: ");
  Serial.println(timeClient.getFormattedTime());

  pixels.setPixelColor(3, pixels.Color(0,150,0));   // LED indicating startup progress
  pixels.show(); // This sends the updated pixel color to the hardware.
  
  Serial.println("Running setup... done!");
}

unsigned long previousMillis = 0;
const long interval = 15000;


//////////////////// Loop() ///////////////////////////////////////////////////////////////////////

void loop() {
  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {
    reconnect();
  }

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 

  display.clearDisplay();
  drawlabels();
  drawvalues();
  
  display.display();
  //  delay(1000);
  //  display.clearDisplay();
  //  display.display();
  //  delay(1);
  static int counter = 0;
//  
//  String payload = "{\"micros\":";
//  payload += micros();
//  payload += ",\"counter\":";
//  payload += counter;
//  payload += "}";
//  IPAddress ip = WiFi.localIP();
 /* broadCast[3] = 255;
  Serial.println(broadCast);
*/
  
  String payload = String(sensorname) + "@" + timeClient.getFormattedTime() + "@";
  payload += counter +"@";
  payload += WiFi.localIP().toString() + "@" +String(mqttClientName);
  
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
    if (client.connected()) {
    
      Serial.println("Publish [topic:message] " + String(temp1Topic) + ":" + String(sensors.getTempCByIndex(0),2));
  
      //temporarily holds data from vals
      char charVal[10];                
            //4 is mininum width, 3 is precision; float value is copied onto buff
      dtostrf(sensors.getTempCByIndex(1), 4, 3, charVal);
      client.publish(temp1Topic, charVal);

      // this is for status feedback /test/test can be /status/type or something... 
      payload += " posting [topic:message]: ";
      payload += String(temp1Topic) + ":" + String(charVal);
      Serial.println("Sending payload to /test/test: ");
      Serial.println(payload);
      if (client.publish("/test/test", (char*) payload.c_str())) {
     // if (client.publish("/test/test", payload)) {
        Serial.println("Publish ok");
      }
      else {
        Serial.println("Publish failed");
      }
    
      ++counter;
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Print out some debugging info
  Serial.print("Callback update. ");

  String r="";
  char c;
  for (int i=0; i<length; i++)  {
    c = payload[i]; 
      if (c!=0) 
        r+=c;
    }
 
  //convert topic to string to make it easier to work with
  String topicStr = topic; 
 
  //Print out some debugging info
  Serial.println(timeClient.getFormattedTime());
  Serial.print("Topic: ");
  Serial.println(topicStr);
  Serial.print("Payload: ");
  Serial.println(r);
  Serial.print("Length: ");
  Serial.println(length);
  
    if (topicStr == switch1Topic) {
      //turn the light on if the payload is '1' and publish to the MQTT server a confirmation message
      if(payload[0] == '1'){
        digitalWrite(switch1Port, HIGH);    //HIGH on wemos d1 mini and LOW on MCU?
        switch1 = 1;
        rainbowCycle(1);
 //       client.publish("/test/confirm", "Light On");
        }
    
      //turn the light off if the payload is '0' and publish to the MQTT server a confirmation message
      else if (payload[0] == '0'){
        digitalWrite(switch1Port, LOW);   // HIGH on MCU and low on wemos d1 mini?
        switch1 = 0;
        for (int i = 0; i<4; i++) {
          pixels.setPixelColor(i, pixels.Color(0,0,0));   // LED indicating startup progress
          pixels.show(); // This sends the updated pixel color to the hardware.
          delay(10);
        }        
//        client.publish("/test/confirm", "Light Off");
      }
    }
}



void reconnect() {

  delay(5000);
  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  
  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
    //print out some more debug once connected
    Serial.println("WiFi connected.");  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
      mqttClientName = clientName;

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str())) {
        Serial.println("MTQQ Connected as: " + clientName);
        if (client.subscribe(switch1Topic))
          Serial.println("MTQQ Subscribed to led16Topic.");
       // client.subscribe(temp1Topic);
       // client.subscribe("/+/+");   // all
      }
      //otherwise print failed for debugging
      else{Serial.println("\tFailed."); abort();}
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5){
      result += ':';
    }
  }
  return result;
}


