////////////////////////////////////////////////////////////////I2C STUFF
#include <Wire.h>

bool endGame = false;
bool appearsToBeEnded = false; 
int startTime;
/////////////////////////////////////////////////////////////////NEOPIXELS
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define RED_LED_PIN     2

#define LED_COUNT  24

#define BRIGHTNESS 255

Adafruit_NeoPixel redTeam(LED_COUNT, RED_LED_PIN);
bool onOffState[LED_COUNT];
//////////////////////////////////////////////////////////////BUTTONS 
int leftButton = 3;
int rightButton = 4;
int toggle = 5;
bool prevLeft = 1;
bool prevRight = 1;
bool prevTogg = 1;
/////////////////////////////////////////////////////////////GUESSING
int cursorVal = 0;
int character = 0;
String bmpArr[] = {"tbd:)", "tbd:|", "tbd:("};
////////////////////////////////////////////////////////////TFT SCREEN STUFF
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
#define SD_CS    4 // SD card select pin
#define TFT_CS  10 // TFT select pin
#define TFT_DC   8 // TFT display/command pin
#define TFT_RST  9 // Or set to -1 and connect to Arduino RESET pin
SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
Adafruit_ST7789      tft    = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
//////////////////////////////////////////////////////////////////////////////////////////END INIT                     

void setup() {
  ImageReturnCode stat;
  tft.init(240, 320);
  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
  Wire.begin();
  Serial.begin(9600);
  randomSeed(A0);
  reset();
  // put your setup code here, to run once:

}

void loop() {
  //check left button
  if (prevLeft == 1 && digitalRead(leftButton) == 0){
    prevLeft = 0;
    cursorVal --;
    if (cursorVal < 0) cursorVal += 24;
  }
  else if (prevRight == 1 && digitalRead(rightButton) == 0){ //check right button
    prevRight = 0;
    cursorVal = (cursorVal+1)%LED_COUNT;
  }
  else if (digitalRead(leftButton) == 1){ //call only to open button, not close
    prevLeft = 1;
  }  
  else if (digitalRead(rightButton) == 1){ //if val changes midcycle, buttons don't work as intended 
    prevRight = 1;
  }
  //time for the toggle
  if (prevTogg == 1 && digitalRead(toggle) == 0){
    prevTogg = 0;
    if (onOffState[cursorVal]){
      onOffState[cursorVal] = false; //on turns off TODO: add audio feedback for discernible off and on 
    }
    else{
      onOffState[cursorVal] = true; // off turns on
    }
  }
  else if (digitalRead(toggle) == 1){ //reset
    prevTogg = 1;
  }
  //check End game state
  if (appearsToBeEnded && millis() - startTime > 5000){
    endGame = true;
    startTime = millis();
  }
  if (endGame && millis() - startTime > 1000){
    reset(); // give enough time for i2c to transmit before resetting
  }  
  reFlashArray(cursorVal);
  //request i2c data
  if (!endGame){
    Wire.requestFrom(1, 0);
    while(Wire.available()){
      endGame = Wire.read(); 
      if (endGame) startTime = millis();
    }    
  }
  else{
    Wire.beginTransmission(0);
    Wire.write(endGame);
    Wire.endTransmission();
  }
  delay(10);
  
}
void reFlashArray(int cursor){
  bool allDark = true;
  for (int i = 0; i < LED_COUNT; i++){
    redTeam.setPixelColor(i, redTeam.Color(0,0,0)); //set automatically to off
    if (onOffState[i]){
      allDark = false;
      redTeam.setPixelColor(i, redTeam.Color(255, 255, 255)); //turn on if state agrees
    }
    if (i == cursorVal){
      redTeam.setPixelColor(i, redTeam.Color(0, 255, 0)); //grb lol
    }
    if (allDark && !appearsToBeEnded){ //if all dark, start the counter
      appearsToBeEnded = true;
      startTime = millis();
    }
    else if (!allDark) appearsToBeEnded = false;
    
  }
}
void reset(){
  prevLeft = 1;
  prevRight = 1;
  prevTogg = 1;
  cursorVal = 0;
  endGame = false;
  appearsToBeEnded = false;
  character = random(0, 24);
  for (int i = 0; i < LED_COUNT; i++){
    onOffState[i] = false;
  }
  reFlashArray(cursorVal);
  const char* myBmp = bmpArr[character].c_str();  
  reader.drawBMP(myBmp, tft, 0, 0);  
  delay(500);
  for (int i = 0; i < LED_COUNT; i++){
    onOffState[i] = true;
  }
  reFlashArray(cursorVal);
}

