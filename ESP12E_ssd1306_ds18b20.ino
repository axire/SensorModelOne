/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

//#include <SPI.h>          // is this really needed? try!
#include <Wire.h>
#include <Adafruit_GFX.h>           // ssd1306 display
#include <Adafruit_SSD1306.h>       // ssd1306 display
#include <OneWire.h>                // Temp 
#include <DallasTemperature.h>      // Temp 




#define OLED_RESET LED_BUILTIN           //4
Adafruit_SSD1306 display(OLED_RESET);



#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif



// Data wire is plugged into port 4 on the Arduino
#define ONE_WIRE_BUS D4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


void setup()   {                
  //Serial.begin(9600);
   
  // Start up the library
  sensors.begin();

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
   
  display.setTextSize(1);
  display.setTextColor(WHITE);

  drawtexttestscreen();     // just to make sure it works ok
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  delay(1000);


//  // draw a single pixel
//  display.drawPixel(10, 10, WHITE);
//  // Show the display buffer on the hardware.
//  // NOTE: You _must_ call display after making any drawing commands
//  // to make them visible on the display hardware!
//  display.display();
//  delay(2000);
//  display.clearDisplay();

}


void loop() {
  display.clearDisplay();
  drawlabels();
  drawvalues();
  
  display.display();
//  delay(1000);
//  display.clearDisplay();
//  display.display();
//  delay(1);
}

void drawvalues()
{
  //display.setCursor(64-((128/20)*7),0);
//  display.print(sensors.getTempCByIndex(0),2);
  //Serial.println(sensors.getTempCByIndex(0)); 

 
   sensors.requestTemperatures(); // Send the command to get temperatures
   //delay(10);
  //Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.

  display.setCursor((128/20)*7,0);
  display.print(sensors.getTempCByIndex(0));  
  display.setCursor((128/20)*7,8);
  display.print(sensors.getTempCByIndex(1));  
  display.setCursor((128/20)*7,16);
  display.print(sensors.getTempCByIndex(2));  
}


void drawlabels()
{
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
  display.print("Temp3: "); 
}

void drawtexttestscreen()
{
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


