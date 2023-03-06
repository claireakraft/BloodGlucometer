// created by Claire, Kasia, Jake, Megan and Jack for Senior Design
// Group B28: Simulated blood Glucometer



// Headers
#include <ArduinoBLE.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include <stdint.h>
#include "TouchScreen.h"


// Initializing Bluetooth
BLEService newService("180A"); // creating the service
BLEFloatCharacteristic reading("2A57", BLERead | BLEWrite); // creating the Analog Value characteristic
long previousMillis = 0;

// Initializing Screen

#define TFT_CS 7        // wiring CS to digital 7
#define TFT_DC 6        // wiring DC to digital 6
#define TFT_RST -1 // RST can be set to -1 if you tie it to Arduino's reset
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
// SoftSPI - note that on some processors this might be *faster* than hardware SPI! (we are not doing!!)
//Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, MOSI, SCK, TFT_RST, MISO);

// Initializing touch screen
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 3   // can be a digital pin
#define XP 4   // can be a digital pin
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// initializing redLED

#define LED_PIN 2

int value1;
int ready=0;
int wait = 1;

void setup() {

  // Bluetooth Setup below:
  Serial.begin(9600);    // initialize serial communication
  while (!Serial);       //starts the program if we open the serial monitor.

  //initialize ArduinoBLE library
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");
    while (1);
  }

  BLE.setLocalName("Blood Glucometer"); //Setting a name that will appear when scanning for Bluetooth® devices
  BLE.setAdvertisedService(newService);

  newService.addCharacteristic(reading);

  BLE.addService(newService);  // adding the service

  reading.writeValue(0);

  BLE.advertise(); //start advertising the service
  Serial.println(" Bluetooth® device active, waiting for connections...");

  //Screen Setup below:

  Serial.begin(9600);
  Serial.println("HX8357D Test!"); 

  tft.begin();

  tft.setRotation(1);

  Serial.print(F("Lines                    "));
  Serial.println(testLines(HX8357_CYAN));
  delay(500);

  // waiting for bluetooth connection
  //tft.setRotation(2);
  //tft.fillScreen(0xDEF8);
  //tft.setCursor(50, 170);
  //tft.setTextColor(HX8357_BLACK);  
  //tft.setTextSize(3);
  //tft.print("Waiting for");
  //tft.setCursor(50, 200);
  //tft.print("Bluetooth");
  //tft.setCursor(50, 230);
  //tft.print("Connection...");  


    

  // LED setup 
  pinMode(LED_PIN, OUTPUT);

  waitScreen();
  


}




void loop() {
  
  BLEDevice central = BLE.central(); // wait for a Bluetooth® Low Energy central

  int ID = 0;
  int scan = 0;
  int v = 0;
  //int wait = 0;

  if (central) {  // if a central is connected to the peripheral
    Serial.print("Connected to central: ");
    wait = 0;
    // set up home screen
    homescreen();

    //Serial.println(central.address()); // print the central's BT address
    
    //digitalWrite(LED_BUILTIN, HIGH); // turn on the LED to indicate the connection

    // check the battery level every 200ms
    // while the central is connected:
    while (central.connected()) {
      long currentMillis = millis();
      
      if (currentMillis - previousMillis >= 10) { // if 200ms have passed, we check the battery level
        previousMillis = currentMillis;

        if (reading.written()) {
          //getValue();
          value1 = reading.value();

        }

        switch (ready){

          case 1121:
          	homescreen();
            ID = 0;
            scan = 0;
            v = 0;
            ready = 0;
	          break;
          case 0:
	          ready = homeCheck();
	          break;

          case 1:
	          // call function for user scan
            while (ID == 0){
              IDScreen();  
              ID++;        
            }
            // call function to check ID screen
            ready = checkID();
            //Serial.println(ready);
	          break;

          case 11:
	
	          // turn on red LED
            while (scan == 0){
              scanScreen();  
              scan++;        
            }

            ready = checkScan();
          
        	  break;

          case 12:
	          homescreen();
            ready = 0;
            ID = 0;
	          break;

          case 112:
	
	          delay(1000);
            while(v == 0){
              getValue();
              v++;
            }

            ready = checkValue();
          	break;

          case 113:
	          IDScreen();
            scan=0;
           ready=1;
          	break;

          //case 1121:
          //	homescreen();
          //  ID = 0;
          //  scan = 0;
          //  v = 0;
          //  ready = 0;
	        //  break;
          //default:
        }

      }

      
    }
    
    digitalWrite(LED_BUILTIN, LOW); // when the central disconnects, turn off the LED
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
  else if (!central){  
    while (wait == 0){
      waitScreen();  
      wait++; 
      ID = 0;
      scan = 0;
      v = 0;
      ready = 0;       
    }  
  }

}

unsigned long testLines(uint16_t color) {
  unsigned long start;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.fillScreen(HX8357_BLACK);

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);

  return micros() - start;
}

void waitScreen(){
  // waiting for bluetooth connection
  tft.setRotation(2);
  tft.fillScreen(0xDEF8);
  tft.setCursor(50, 170);
  tft.setTextColor(HX8357_BLACK);  
  tft.setTextSize(3);
  tft.print("Waiting for");
  tft.setCursor(50, 200);
  tft.print("Bluetooth");
  tft.setCursor(50, 230);
  tft.print("Connection...");  
}

void getValue(){
  int x;
  //char buffer[30];

  //value1 = reading.value();
  x = value1 / 10;
  //sprintf(buffer, "the modulo is %d", x);
  //Serial.println(buffer);
  if((x) >= 10){
    tft.fillScreen(0xDEF8);
    tft.setCursor(10, 170);
    tft.setTextColor(HX8357_BLACK);  tft.setTextSize(13);
    tft.print(value1); 
    tft.setCursor(220, 255);
    tft.setTextColor(HX8357_BLACK);  tft.setTextSize(2);
    tft.println(" mg/dL");

    tft.drawRect(60, 350, 200, 75, HX8357_BLACK);
    tft.setCursor(76, 380);
    tft.print("Done");
            
  }
  else{          
    tft.fillScreen(0xDEF8);
    tft.setCursor(0, 170);
    tft.setTextColor(HX8357_BLACK);  tft.setTextSize(13);
    tft.print(" ");
    tft.print(value1); 
    tft.setCursor(220, 245);
    tft.setTextColor(HX8357_BLACK);  tft.setTextSize(2);
    tft.println(" mg/dL");

    tft.drawRect(60, 350, 200, 75, HX8357_BLACK);
    tft.setCursor(76, 380);
    tft.print("Done");
            
  }    
          
  //sprintf(buffer, "the value is %d", value1);
  //Serial.println(buffer);
  return;

}

void homescreen(){

  // home screen setup
  //tft.fillScreen(HX8357_BLACK);
  tft.setTextColor(HX8357_BLACK);
  tft.fillScreen(0xDEF8);
  //tft.fillScreen(0xCE56);
  tft.setCursor(10, 10);
  tft.setTextColor(HX8357_BLACK);  
  tft.setTextSize(3);
  tft.print("Main Menu:");
  tft.drawRect(5, 40, 180, 0, HX8357_BLACK);
  tft.drawRect(60, 150, 200, 75, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(90, 180);
  tft.print("Patient Test");

  tft.drawRect(60, 250, 200, 75, HX8357_BLACK);
  tft.setCursor(90, 280);
  tft.print("Control Test");

  tft.drawRect(60, 350, 200, 75, HX8357_BLACK);
  tft.setCursor(76, 380);
  tft.print("Review Results");
  //delay(50000);
}

float homeCheck(){
// touch screen stuff
  
    TSPoint p = ts.getPoint();
    if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 575 && p.y < 675) {
    
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     

     return 1;
    }
    else if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 430 && p.y < 530) {
  
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     tft.fillScreen(HX8357_BLUE);     
     return 2;
    }
    else if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 300 && p.y < 400) {
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     tft.fillScreen(HX8357_RED);
     return 3;
    }
    

  return 0;

}

void IDScreen(){

    tft.setTextColor(HX8357_BLACK);
    tft.fillScreen(0xDEF8);
    tft.setTextColor(HX8357_BLACK);
    tft.drawRect(60, 150, 200, 75, HX8357_BLACK);
    tft.setTextSize(2);
    tft.setCursor(90, 180);
    tft.print("User ID");
    tft.drawRect(60, 250, 200, 75, HX8357_BLACK);
    tft.setCursor(90, 280);
    tft.print("Back");
}

float checkID(){
  TSPoint p = ts.getPoint();
    if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 575 && p.y < 675) {
    
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     

     return 11;
    }
    else if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 430 && p.y < 530) {
  
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);   
     return 12;
    }

  return 1;
}

void scanScreen(){
  // home screen setup
  tft.setTextColor(HX8357_BLACK);
  tft.fillScreen(0xDEF8);
  tft.setTextColor(HX8357_BLACK);  
  tft.setTextSize(3);
  tft.setCursor(50, 10);
  tft.print("Scanner");
  tft.drawRect(5, 40, 180, 0, HX8357_BLACK);
  tft.drawRect(60, 150, 200, 75, HX8357_BLACK);
  tft.setTextSize(2);
  tft.setCursor(90, 180);
  tft.print("Turn on");

  tft.drawRect(60, 250, 200, 75, HX8357_BLACK);
  tft.setCursor(90, 280);
  tft.print("Turn off");

  tft.drawRect(60, 350, 200, 75, HX8357_BLACK);
  tft.setCursor(76, 380);
  tft.print("Back");

}

float checkScan(){
   TSPoint p = ts.getPoint();
    if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 575 && p.y < 675) {
    
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     
    // turn LED on
    digitalWrite(LED_PIN, HIGH);
     return 11;
    }
    else if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 430 && p.y < 530) {
  
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);  

     // turn LED off
     digitalWrite(LED_PIN, LOW);
     return 112;
    }
    else if (p.z > ts.pressureThreshhold && p.x > 415 && p.x < 730 && p.y > 300 && p.y < 400) {
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     return 113;
    }
    

  return 11;

}

float checkValue(){
  TSPoint p = ts.getPoint();
  if (p.z > ts.pressureThreshhold && p.x > 300 && p.x < 800 && p.y > 200 && p.y < 500) {
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
     return 1121;
    }
  return 112;
}

