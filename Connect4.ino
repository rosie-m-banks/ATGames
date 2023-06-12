/////////////////////////////////////////////////////////////////NEOPIXELS
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define RED_LED_PIN     2
#define BLUE_LED_PIN    3
#define LED_COUNT  42

#define BRIGHTNESS 255

Adafruit_NeoPixel redTeam(LED_COUNT, RED_LED_PIN);
Adafruit_NeoPixel blueTeam(LED_COUNT, BLUE_LED_PIN);
const int rows = 7;
const int columns = 6;
int pixelState[rows][columns];
//////////////////////////////////////////////////////////////BUTTONS 
int cursorP1 = 4; 
int dropP1 = 5;
int prevCursorP1 = 0;
int prevDropP1 = 1;
int cursorP2 = 6;
int dropP2 = 7;
int prevCursorP2 = 0;
int prevDropP2 = 1;
int iVal = 0;
int columnVal = 0;
bool playerOneTurn;
bool gameEnd;
int winner = 0;
bool initialize;
bool availableI[rows] = {1,1,1,1,1,1};
void setup() {
  // put your setup code here, to run once:
  pinMode(cursorP1, INPUT_PULLUP);
  pinMode(dropP1, INPUT_PULLUP);
  pinMode(cursorP2, INPUT_PULLUP);
  pinMode(dropP2, INPUT_PULLUP);
  playerOneTurn = true;
  gameEnd = false;
  initialize = false;
  reset();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (playerOneTurn){
    if (prevCursorP1 == 0 && digitalRead(cursorP1) == 1){ //off is on and on is off
      if (initialize) pixelState[iVal][columns-1] = 0;
      iVal = newI();
      pixelState[iVal][columns-1] = 3; //p1cursor
      initialize = true;
    }
    prevCursorP1 = digitalRead(cursorP1);
    if (prevDropP1 == 1 && digitalRead(dropP1) == 0){
      columnVal = dropRow(iVal); 
      pixelState[iVal][columns - 1] = 0;
      pixelState[iVal][columnVal] = 1; //p1Val
      playerOneTurn = false;
      initialize = false;
    }
    prevDropP1 = digitalRead(dropP1);
  }
  else{
    if (prevCursorP2 == 0 && digitalRead(cursorP2) == 1){
      if(initialize) pixelState[iVal][columns-1] = 0;
      iVal = newI();
      pixelState[iVal][columns-1] = 4; //p2cursor
      initialize = true;
    }
    prevCursorP2 = digitalRead(cursorP2);
    if (prevDropP2 == 1 && digitalRead(dropP2) == 0){
      columnVal = dropRow(iVal); 
      pixelState[iVal][columns - 1] = 0;
      pixelState[iVal][columnVal] = 2; //p2Val
      playerOneTurn = true;
      initialize = false;
    }
    prevDropP2 = digitalRead(dropP2);
  }
  reflashArray();
  
  if (!gameEnd) winner = winCheck();
  if (winner == 1){
    color(1);
    if (prevCursorP2 == 1 && digitalRead(cursorP2) == 0){
      playerOneTurn = false;
      reset();
    }
  }
  if (winner == 2){
    color(2);
    if (prevCursorP1 == 1 && digitalRead(cursorP1) == 0){
      playerOneTurn = true;
      reset();
    }
  }
  if (winner == 0 && sum() == 0){
    draw();
    if ((prevCursorP1 == 1 && digitalRead(cursorP1) == 0) || (prevCursorP2 == 1 && digitalRead(cursorP2) == 0)){
      playerOneTurn = false;
      reset();
    }
  }
  delay(50);

}
int dropRow(int dropSpot){
  for (int i = 0; i < columns; i++){
    if (pixelState[dropSpot][i] == 0){
      return i;
    }
  }
  return columns;
}
int newI(){
  int firstAv = -1;
  for (int i = 0; i < rows; i++){
    if (firstAv < 0 && availableI[i]) firstAv = i;
    if (!initialize){
      if (availableI[i]) return i; 
    }
    if (availableI[i] && i > iVal) return i;
  }
  return firstAv;
}
void reset(){
  for (int i = 0; i < rows; i++) availableI[i] = 1;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++)pixelState[i][j] = 0;
  }
  gameEnd = false;
  initialize = false;
  winner = 0;
}
int winCheck(){
  int winner = 0;
  for (int i = 0; i < rows; i++){
    if (pixelState[i][columns-1] == 1 || pixelState[i][columns-1] == 2) availableI[i] = 0;
    for (int j = 0; j < columns; j++){
      if (i <= rows - 4){
        if (pixelState[i][j] == 1 && pixelState[i+1][j] == 1 && pixelState[i+2][j] == 1 && pixelState[i+3][j] == 1) winner = 1;
        if (pixelState[i][j] == 2 && pixelState[i+1][j] == 2 && pixelState[i+2][j] == 2 && pixelState[i+3][j] == 2) winner = 2;
      }
      if (j <= columns - 4){
        if (pixelState[i][j] == 1 && pixelState[i][j+1] == 1 && pixelState[i][j+2] == 1 && pixelState[i][j+3] == 1) winner = 1;
        if (pixelState[i][j] == 2 && pixelState[i][j+1] == 2 && pixelState[i][j+2] == 2 && pixelState[i][j+3] == 2) winner = 2;
      }
      if (i <= rows-4 && j <= columns - 4){
        if (pixelState[i][j] == 1 && pixelState[i+1][j+1] == 1 && pixelState[i+2][j+2] == 1 && pixelState[i+3][j+3] == 1) winner = 1;
        if (pixelState[i][j] == 2 && pixelState[i+1][j+1] == 2 && pixelState[i+2][j+2] == 2 && pixelState[i+3][j+3] == 2) winner = 2;
      }
    }
  }
  return winner;
}
void reflashArray(){
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < columns; j++){
      redTeam.setPixelColor(i+rows*j, 0, 0, 0);
      blueTeam.setPixelColor(i+rows*j, 0, 0, 0);
      if (pixelState[i][j] == 1 || pixelState[i][j] == 3){
        redTeam.setPixelColor(i+rows*j, 0, 255, 0);
        blueTeam.setPixelColor(i+rows*j, 0, 255, 0);
      }
      if (pixelState[i][j] == 2 || pixelState[i][j] == 4){
        redTeam.setPixelColor(i+rows*j, 255, 0, 0);
        blueTeam.setPixelColor(i+rows*j, 255, 0, 0);
      }
    }
  }
  redTeam.show();
  blueTeam.show();
}
void color(int player){
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < columns; j++){
      pixelState[i][j] = player;
    }
  }
}
void draw(){
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < columns; j++){
      pixelState[i][j] = 2;
      if (i%2 == 0) pixelState[i][j] = 1;
    }
  }  
}
int sum(){
  int a;
  for (int i = 0; i < rows; i++){
    a+= availableI[i];
  }
  return a;
}