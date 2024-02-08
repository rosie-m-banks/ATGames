// NeoPixel test program showing use of the WHITE channel for RGBW
// pixels only (won't look correct on regular RGB NeoPixel strips).
//1 is ladder script
//2 is chutes
//3
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN     10

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  100

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 255 // Set BRIGHTNESS to about 1/5 (max = 255)

#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function
Uart mySerial (&sercom2, 3, 4, SERCOM_RX_PAD_1, UART_TX_PAD_0);
void SERCOM2_Handler()
{
  mySerial.IrqHandler();
}
//SoftwareSerial mySerial(2, 3); // RX, TX... D5 goes to 3rd pin down, D6 goes to 2nd pin down from top

void sendCmd(int cmd, int lb, int hb, bool reply = false)
{ // standard format for module command stream
  uint8_t buf[] = {0x7E, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};
  int16_t chksum = 0;
  int idx = 3; // position of first command byte

  buf[idx++] = (uint8_t)cmd; // inject command byte in buffer
  if (reply) buf[idx++] = 0x01;
  else buf[idx++] = 0x00;// set if reply is needed/wanted
  if (hb >= 0) // if there is a high byte data field
  buf[idx++] = (uint8_t)hb; // add it to the buffer
  if (lb >= 0) // if there is a low byte data field
  buf[idx++] = (uint8_t)lb; // add it to the buffer
  buf[2] = idx - 1; // inject command length into buffer
  buf[idx++] = 0xEF; // place end-of-command byte

  mySerial.write(buf, idx); // send the command to module
  // for (int i = 0; i < idx; i++) // send command as hex string to MCU
  // Serial.printf("%02X ", buf[i]);
  // Serial.println();
}


// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);
Adafruit_NeoPixel dice1Strip(28, 5);
Adafruit_NeoPixel dice2Strip(28, 13);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
int Vals[100];
int ladderBeginnings[] = {5, 29, 14, 37, 40, 50};
int ladderEnds[] =      {22, 48, 34, 56, 83, 73};
int chuteBeginnings[] = {20, 26, 54, 94, 96, 84};
int chuteEnds[] =        {1,  8, 36, 71, 79, 32};
int dice1Vals[] = {2, 7, 11, 16, 21, 26};
int dice2Vals[] = {26, 21, 17, 12, 7, 2};
int crownArr[] = {1,0,0,1,0,0,1,0,0,1,
                  0,0,1,0,0,1,0,0,1,0,
                  0,1,0,0,1,0,0,1,0,0,
                  1,0,0,1,0,0,1,0,0,1,
                  0,0,1,0,0,1,0,0,1,0,
                  0,1,0,0,1,0,0,1,0,0,
                  1,0,0,1,0,0,1,0,0,1,
                  0,0,1,0,0,1,0,0,1,0,
                  0,1,0,0,1,0,0,1,0,0,
                  1,0,0,1,0,0,1,0,0,1,};

bool diceRoll1 = true;
bool diceRoll2 = false;

bool player1Turn = false;
bool player2Turn = false;

int desSpotP1 = 0;
int desSpotP2 = 0;
 
int person1 = 0;
int person2 = 0;
int button1 = 9;
int button2 = 12;

int dice1 = 7;
int dice2 = 11;
int button1State = 1;
int button2State = 1;
int constant = 5;
bool update = true;

bool diceRollTime = false;
bool songPlay = true;


void setup() {
  songPlay = true;
  randomSeed(analogRead(A5));
  button1State = 1;
  button2State = 1;
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(dice1, INPUT_PULLUP);
  pinMode(dice2, INPUT_PULLUP);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show(); 
  strip.clear();           // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
  dice1Strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  dice1Strip.show(); 
  dice1Strip.clear();           // Turn OFF all pixels ASAP
  dice1Strip.setBrightness(BRIGHTNESS);
  dice2Strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  dice2Strip.show(); 
  dice2Strip.clear();           // Turn OFF all pixels ASAP
  dice2Strip.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
  delay(2000); //Allow Serial to start up

  Serial.println("starting ");
  mySerial.begin(9600); //communicate with DF Player
  pinPeripheral(3, PIO_SERCOM_ALT);
  pinPeripheral(4, PIO_SERCOM_ALT);
  delay(3000); //Allow SoftwareSerial to start up

  Serial.println("Stop anything playing"); //in case anything is already playing on reset
  sendCmd(0x0E, 0, 0, false);
  delay(200); //give a delay for the DF Player to execute the command

  Serial.println("Now volume ");
  sendCmd(6, 5, 0, false); //command code can be in decimal too.

  Serial.println("Starting Player");

  Serial.println("Stop anything playing"); //in case anything is already playing on reset
  sendCmd(0x0E, 0, 0, false);
  delay(200); //give a delay for the DF Player to execute the command
  for (int i = 0; i < LED_COUNT; i++){
    Vals[i] = i;
    for (int j = 0; j < 6; j++){
      if (i == ladderBeginnings[j]) Vals[i] = ladderEnds[j];
      if (i == chuteBeginnings[j]) Vals[i] = chuteEnds[j];
    }
  }
  update = true;
  diceRollTime = false;
  sendCmd(0x0E, 0, 0, false);

}

void loop() {
  if (diceRoll1 && digitalRead(dice1) == 0){ //is it your turn to roll the dice?
    int diceRollVal = random(1,7);
    desSpotP1 = person1 + diceRollVal;    //this is where you want to go
    diceRoll1 = false;
    player1Turn = true; //time to move there
    if (desSpotP1 > 99){
      player1Turn = false;
      diceRoll2 = true;      
    }
    Serial.println(desSpotP1);
    Serial.println("go");
    //diceRoll(diceRollVal -1, 0);
    if (person1 != 99 && person2 != 99)setDice(dice1Vals[diceRollVal - 1], -1, diceRollVal);
  }
  if (diceRoll2 && digitalRead(dice2) == 0){ //is it your turn to roll the dice?
    int diceRollVal = random(1,7);
    desSpotP2 = person2 + diceRollVal;    //this is where you want to go
    diceRoll2 = false;
    player2Turn = true; //time to move there
    if (desSpotP2 > 99){
      player2Turn = false;
      diceRoll1 = true;      
    }
    Serial.println(desSpotP2);
    Serial.println("go");
    //diceRoll(diceRollVal -1, 1);
    if (person1 != 99 && person2 != 99)setDice(-1, dice2Vals[diceRollVal - 1], diceRollVal);
  }
  update = false;
  int temp1 = digitalRead(button1); 
  if (button1State != temp1 && temp1 == 0 && player1Turn){ //is it your turn to press the button? and is this the first time the button is at a pressed state?
    update = true; //got to update the strip
    person1++; //go to the next spot
    if (person1 == desSpotP1){ // at the end of the line
      person1 = Vals[desSpotP1]; //go to special value if exists
      if (person1 != desSpotP1) slide(desSpotP1, person1, 0);
      player1Turn = false; //not your turn anymore
      diceRoll2 = true; //second player time to roll
    }
    Serial.println(person1);
  }
  button1State = temp1; 
  int temp2 = digitalRead(button2);
  if (button2State != temp2 && temp2 == 0 && player2Turn){
    update = true;
    person2++;
    if (person2 == desSpotP2){ // at the end of the line
      person2 = Vals[desSpotP2]; //go to special value if exists
      if (person2 != desSpotP2) slide(desSpotP2, person2, 1);
      player2Turn = false; //not your turn anymore
      diceRoll1 = true; //first player time to roll
    }
    Serial.println(person2);
  }
  person1 = min(LED_COUNT-1, person1);  
  person2 = min(LED_COUNT-1, person2);
  button2State = temp2;
  if (update == true){
    setStrip(person1, person2);
  }
if (person1 == 99)  {
  oneFill();
  if (digitalRead(dice2) == 0 || digitalRead(dice1) == 0){
    reset();
    diceRoll2 = true;
  }
}
if (person2 == 99) {
  twoFill();
  if (digitalRead(dice1) == 0 || digitalRead(dice2) == 0){
    reset();
    diceRoll1 = true;
  }
}
  delay(50);
}



void setStrip(int player1, int player2) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...

    strip.setPixelColor(i, strip.Color(0, 0, 0)); 

    if (i == player1){
      strip.setPixelColor(i, strip.Color(0, 0, 255)); 
    }
    if (i == player2){
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
    if (player1 == player2 && i == player1){
      strip.setPixelColor(i, strip.Color(0, 255, 255));
    }
  }
  strip.show();

}
void setDice(int dice1, int dice2, int desVal){
  sendCmd(0x03, desVal, 0, false); //play track 003 in folder 01
  //delay(10000);
  //sendCmd(0x0E, 0, 0, false); //send stop command
  delay(200);
  Serial.println(" ");
	for(int i=0; i<dice1Strip.numPixels(); i++) { // For each pixel in strip...

      dice1Strip.setPixelColor(i, dice1Strip.Color(0, 0, 0));
      dice2Strip.setPixelColor(i, dice2Strip.Color(0, 0, 0));

      if (i == dice1){
        
        dice1Strip.setPixelColor(i, dice1Strip.Color(0, 0, 255)); 
      }
      if (i == dice2){
        
        dice2Strip.setPixelColor(i, dice2Strip.Color(255, 0, 0));
      }
      
  }
  dice1Strip.show();
  dice2Strip.show();
}
void oneFill(){
  if (songPlay){
    sendCmd(0x03, 9, 0, false); //play track 003 in folder 01
    //delay(10000);
    //sendCmd(0x0E, 0, 0, false); //send stop command
    delay(200);
    songPlay = false;
  }
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      strip.setPixelColor(i, strip.Color(0, 0, 0));  
      if (crownArr[i] == 1)strip.setPixelColor(i, strip.Color(0, 0, 255)); 
      if (crownArr[i] == 2)strip.setPixelColor(i, strip.Color(0, 255, 255)); 
    }
  strip.show();
}

void twoFill(){
  if (songPlay){
    sendCmd(0x03, 9, 0, false); //play track 003 in folder 01
    //delay(10000);
    //sendCmd(0x0E, 0, 0, false); //send stop command
    delay(200);
    songPlay = false;
  }
  Serial.println(" ");
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      strip.setPixelColor(99-i, strip.Color(0, 0, 0));  
      if (crownArr[i] == 1)strip.setPixelColor(i, strip.Color(0, 255, 0)); 
      if (crownArr[i] == 2)strip.setPixelColor(i, strip.Color(0, 255, 255)); 
    }
  strip.show();
}

void reset(){
  diceRoll1 = false;
  diceRoll2 = false;

  player1Turn = false;
  player2Turn = false;

  desSpotP1 = 0;
  desSpotP2 = 0;
 
  person1 = 0;
  person2 =0;

  button1State = 1;
  button2State = 1;

  update = true;  
  diceRollTime = false;
  Serial.println("Stop anything playing"); //in case anything is already playing on reset
  sendCmd(0x0E, 0, 0, false);
  delay(200); //give a delay for the DF Player to execute the command
  songPlay = true;
}

void slide(int currPos, int desPos, int player){
  
  if (currPos < desPos){ //is a ladder
    sendCmd(0x03, 7, 0, false); //play track 003 in folder 01
    //delay(10000);
    //sendCmd(0x0E, 0, 0, false); //send stop command
    delay(200);
    Serial.println(" ");
    int count = 0;
    for (int i = currPos; i < desPos; i++){
      if (player == 0) setStrip(i, person2);
      if (player == 1) setStrip(person1, i);
      count ++;
      delay(50);
    }
    
  }
  if (currPos > desPos){ //is a chute
    sendCmd(0x03, 8, 0, false); //play track 003 in folder 01
    //delay(10000);
    //sendCmd(0x0E, 0, 0, false); //send stop command
    delay(200);
    Serial.println(" ");
    int count = 0;
    for (int i = currPos; i > desPos; i--){
      if (player == 0) setStrip(i, person2);
      if (player == 1) setStrip(person1, i);
      count ++;
      delay(50);
    }
    
  }
  
  
}




