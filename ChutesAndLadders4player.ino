#include <Adafruit_NeoPixel.h>
#include "DFRobotDFPlayerMini.h"
#include <Arduino.h>
#include "wiring_private.h" // pinPeripheral() function

#define LED_PIN     5
#define LED_COUNT  124

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 60 // Set BRIGHTNESS to about 1/5 (max = 255)
#define DFPLAYERVOLUME 25 //from 1 to 30

#define C565(c) (((c) & 0xF800ul) << 8 | ((c) & 0x07E0ul) << 5 | ((c) & 0x001Ful) << 3)

static const uint32_t ORANGE = C565(0xEA00);
static const uint32_t LIME = C565(0x07E0);
static const uint32_t RED = C565(0xF800);
static const uint32_t GREEN = C565(0x0400);
static const uint32_t BLUE = C565(0x001F);
static const uint32_t CYAN = C565(0x07FF);
static const uint32_t MAGENTA = C565(0xF81F);
static const uint32_t YELLOW = C565(0xFFE0);
static const uint32_t TURQUOISE = C565(0x067A);
static const uint32_t FOREST = C565(0x2444);
static const uint32_t MAROON = C565(0x8000);
static const uint32_t NAVY = C565(0x0010);
static const uint32_t BROWN = C565(0x9240);
static const uint32_t YELLOWORANGE = C565(0xFD68);
static const uint32_t WHITE = C565(0xFFFF);
static const uint32_t BLACK = C565(0x0000);

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);
DFRobotDFPlayerMini myDFPlayer;
Uart dfPlayer (&sercom2, 3, 4, SERCOM_RX_PAD_1, UART_TX_PAD_0);
void SERCOM2_Handler()
{
  dfPlayer.IrqHandler();
}

int Vals[100]; //what each square maps to
int playerPos[LED_COUNT];
int ladderBeginnings[] = {5, 29, 14, 37, 40, 50};
int ladderEnds[] =      {22, 48, 34, 56, 83, 73};
int chuteBeginnings[] = {20, 26, 54, 94, 96, 84};
int chuteEnds[] =        {1,  8, 36, 71, 79, 32};
int maxPlayers = 4;

int dicePins[] = {A1, 7, A4, A3};
int movePins[] = {A5, A2, 9, 2};
int hardMode[] = {0, 10, 0, 13};
int easyMode[] = {SDA, SCL, 11, 12};
int currPos[]  = {0, 0, 0, 0};

uint32_t colorIDs[] = {BLACK, BLUE, GREEN, TURQUOISE, YELLOW, FOREST, LIME, CYAN, RED, MAGENTA, MAROON, NAVY, ORANGE, BROWN, YELLOWORANGE, WHITE};

int playerTurn = 0;
bool diceTurn = true;
int diceRollVal = 0;
int timer = 0;
int currCounter = 0;
bool gameEnd = false;
bool pressOccur = false;
int prevRead = 1;
int winner = 3;

void pinDefs(){
  randomSeed(analogRead(A0));
  for (int i = 0; i < maxPlayers; i++){
    pinMode(dicePins[i], INPUT_PULLUP);
    pinMode(movePins[i], INPUT_PULLUP);
    pinMode(easyMode[i], INPUT_PULLUP);
    if (i%2 == 1) pinMode(hardMode[i], INPUT_PULLUP);
  }
  strip.begin(); strip.show(); strip.clear(); strip.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
  
}
void dfPlayerSetup(){
  dfPlayer.begin(9600);
  pinPeripheral(3, PIO_SERCOM_ALT);
  pinPeripheral(4, PIO_SERCOM_ALT);
  delay(1000);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(dfPlayer, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(DFPLAYERVOLUME);  //Set volume value. From 0 to 30
  myDFPlayer.pause();  //Play the first mp3
}

void setup() {
  pinDefs();
  dfPlayerSetup();
  for (int i = 0; i < 100; i++){
    Vals[i] = i;
    for (int j = 0; j < 6; j++){
      if (i == ladderBeginnings[j]) Vals[i] = ladderEnds[j];
      if (i == chuteBeginnings[j]) Vals[i] = chuteEnds[j];
    }
  }
  delay(1000);
  reset();
}

void loop() {
  if (gameEnd){
    for (int i = 0; i < maxPlayers; i++) {
      if (digitalRead(dicePins[i]) == 0) {
        reset();
      }
      delay(50);
    }
  } else{
    
  if (playerTurn%2 == 1 && digitalRead(easyMode[playerTurn]) == 1 && digitalRead(hardMode[playerTurn]) == 1 && diceRollVal == 0){
    nextTurn();
  }
  if (diceTurn){
    if (digitalRead(dicePins[playerTurn]) == 0){
      diceRollVal = random(1,7);
      currCounter = diceRollVal;
      playerPos[6*playerTurn + diceRollVal - 1 + 100] = 1 << playerTurn;
      myDFPlayer.play(diceRollVal);
      diceTurn = false;
      timer = millis();
    }
  }
  else{
    if (currPos[playerTurn] + diceRollVal > 99){
      playerPos[6*playerTurn + diceRollVal - 1 + 100] = 0;
      nextTurn();
    }
    else if (digitalRead(movePins[playerTurn]) == 0 && prevRead == 1 || (pressOccur && digitalRead(easyMode[playerTurn]) == 0 && millis()-timer >= 600)){
      pressOccur = true;
      timer = millis();
      if (currPos[playerTurn] != 0) playerPos[currPos[playerTurn]] -= 1 << playerTurn;
      currPos[playerTurn] ++;
      playerPos[currPos[playerTurn]] += 1 << playerTurn;
      diceRollVal --;
      myDFPlayer.play(currCounter - diceRollVal);
      myDFPlayer.play(currCounter - diceRollVal);
      Serial.println(currCounter - diceRollVal);
      if (diceRollVal == 0){
        slide(currPos[playerTurn], playerTurn);
        playerPos[6*playerTurn + currCounter - 1 + 100] = 0;
        nextTurn();
      }
    }
  }
  prevRead = digitalRead(movePins[playerTurn]);
  updateStrip();
  delay(50);
  }
}



void reset(){
  Serial.println("hi");
  myDFPlayer.pause();
  playerTurn =  (winner + 1) % maxPlayers;
  diceRollVal = 0;
  currCounter = 0;
  prevRead = 1;
  diceTurn = true;
  for (int i = 0; i < LED_COUNT; i++) playerPos[i] = 0;
  timer = millis();
  gameEnd = false;
  pressOccur = false;
  for (int i = 0; i < maxPlayers; i++) currPos[i] = 0;
}



void nextTurn(){
  pressOccur = false;
  playerTurn ++;
  playerTurn %= maxPlayers;
  diceTurn = true;
  diceRollVal = 0;
}

void slide(int curr, int playerID){
  int ladderSound = 7; 
  int chuteSound = 8;
  if (Vals[curr] < curr){
    delay(300);
    myDFPlayer.play(chuteSound);
    myDFPlayer.play(chuteSound);
    for (int i = curr; i > Vals[curr]; i--){
      updateStrip();
      delay(100);
      playerPos[i] -= 1<< playerID;
      playerPos[i-1] += 1<<playerID;
    }
  }
  else if (Vals[curr] > curr){
    delay(300);
    myDFPlayer.play(ladderSound);
    myDFPlayer.play(ladderSound);
    for (int i = curr; i < Vals[curr]; i++){
      updateStrip();
      delay(100);
      playerPos[i] -= 1<< playerID;
      playerPos[i+1] += 1<<playerID;
    }
  }
  updateStrip();
  delay(50);
  currPos[playerID] = Vals[curr];
  if (currPos[playerID] == 99){
    gameEnd = true;
    endGame(playerID);
  }

}
void endGame(int playerID){
  winner = playerID;
  myDFPlayer.play(9);
  myDFPlayer.play(9);
  for (int i = 0; i < 100; i++){
    playerPos[i] = 0;
    if (i%4 == 0) playerPos[i] = 1 << playerID;
  }
  updateStrip();
  delay(50);
}

void updateStrip(){
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor((i+24)%LED_COUNT, colorIDs[playerPos[i]]);
  }
  strip.show();
}