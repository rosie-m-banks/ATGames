// NeoPixel test program showing use of the WHITE channel for RGBW
// pixels only (won't look correct on regular RGB NeoPixel strips).

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN     12

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  100

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 255 // Set BRIGHTNESS to about 1/5 (max = 255)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);
Adafruit_NeoPixel dice1Strip(28, 11);
Adafruit_NeoPixel dice2Strip(28, 10);
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
int chuteEnds[] =        {1,  8, 36, 70, 79, 32};
int dice1Vals[] = {2, 7, 11, 16, 21, 26};
int dice2Vals[] = {26, 21, 17, 12, 7, 2};

bool diceRoll1 = true;
bool diceRoll2 = false;

bool player1Turn = false;
bool player2Turn = false;

int desSpotP1 = 0;
int desSpotP2 = 0;
 
int person1 = 0;
int person2 = 0;
int button1 = A4;
int button2 = A1;

int dice1 = A3;
int dice2 = A2;
int button1State = 1;
int button2State = 1;
int constant = 5;
bool update = true;

bool diceRollTime = false;

void setup() {
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
  for (int i = 0; i < LED_COUNT; i++){
    Vals[i] = i;
    for (int j = 0; j < 6; j++){
      if (i == ladderBeginnings[j]) Vals[i] = ladderEnds[j];
      if (i == chuteBeginnings[j]) Vals[i] = chuteEnds[j];
    }
  }
  update = true;
  diceRollTime = false;
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
    setDice(dice1Vals[diceRollVal - 1], -1);
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
    setDice(-1, dice2Vals[diceRollVal - 1]);
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
      strip.setPixelColor(i, strip.Color(255, 0, 0)); 
    }
    if (i == player2){
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
    if (player1 == player2 && i == player1){
      strip.setPixelColor(i, strip.Color(0, 0, 255));
    }
  }
  strip.show();

}
void setDice(int dice1, int dice2){
	for(int i=0; i<dice1Strip.numPixels(); i++) { // For each pixel in strip...

      dice1Strip.setPixelColor(i, dice1Strip.Color(0, 0, 0));
      dice2Strip.setPixelColor(i, dice2Strip.Color(0, 0, 0));

      if (i == dice1){
        dice1Strip.setPixelColor(i, dice1Strip.Color(0, 255, 0)); 
      }
      if (i == dice2){
        dice2Strip.setPixelColor(i, dice2Strip.Color(255, 0, 0));
      }
      
  }
  dice1Strip.show();
  dice2Strip.show();
}
void oneFill(){
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      strip.setPixelColor(i, strip.Color(255, 0, 0)); 
    }
    strip.show();
}

void twoFill(){
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      strip.setPixelColor(i, strip.Color(0, 255, 0)); 
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
}

void slide(int currPos, int desPos, int player){
  if (currPos < desPos){ //is a ladder
    for (int i = currPos; i < desPos; i++){
      if (player == 0) setStrip(i, person2);
      if (player == 1) setStrip(person1, i);
      delay(50);
    }    
    
  }
  if (currPos > desPos){ //is a chute
    for (int i = currPos; i > desPos; i--){
      if (player == 0) setStrip(i, person2);
      if (player == 1) setStrip(person1, i);
      delay(50);
    }    
  }
  
}

void diceRoll(int diceVal, int dice){
  if (dice == 0){
    for (int i = 0; i < 6; i++){
      setDice(dice1Vals[i], -1);
      delay(60);
    }
    for (int i = 5; i <= diceVal; i--){
      setDice(dice1Vals[i], -1);
      delay(60);      
    }  
  }
  if (dice == 1){
    for (int i = 0; i < 6; i++){
      setDice(-1, dice2Vals[i]);
      delay(60);
    }
    for (int i = 5; i <= diceVal; i--){
      setDice(-1, dice2Vals[i]);
      delay(60);      
    }  
  }
}



