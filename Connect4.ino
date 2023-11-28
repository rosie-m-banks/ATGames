/////////////////////////////////////////////////////////////////NEOPIXELS
#include <Adafruit_DotStar.h>

Adafruit_DotStar dot = Adafruit_DotStar(1, 41, 40, DOTSTAR_BGR);
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN     10
#define LED_COUNT  91

#define BRIGHTNESS 155

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN);
const int rows = 7;
const int columns = 6;
int pixelState[rows][columns];
int remap[rows][columns][2];
//up and down snakelike pattern on the LEDs --> remap
int selector[] = {84, 85, 86, 87, 88, 89, 90};
int sel[] = {0, 0, 0, 0, 0, 0, 0};
//////////////////////////////////////////////////////////////BUTTONS 
int cursorP1 = 12; 
int dropP1 = 7;
int prevCursorP1 = 1;
int prevDropP1 = 1;

int cursorP2 = 11;
int dropP2 = 9;
int prevCursorP2 = 1;
int prevDropP2 = 1;

int iVal = -1;
int columnVal = 0;

bool playerOneTurn;
bool canGo = false;
bool gameEnd;
int winner = 0;
bool initialize;
void setup() {
  dot.begin();
  dot.show();
  // put your setup code here, to run once:
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);

  pinMode(cursorP1, INPUT_PULLUP);
  pinMode(dropP1, INPUT_PULLUP);
  pinMode(cursorP2, INPUT_PULLUP);
  pinMode(dropP2, INPUT_PULLUP);
  playerOneTurn = true;
  gameEnd = false;
  initialize = false;
  //remap all the values
  for (int i = 0; i < 84; i++){
    int f = i%42 + 6*int(i/42);
    int row = (i%42)/6;
    int col;
    if (i >= 42) row = 6-row;
    if (int(f/6)%2 == 0){
      //topDown
      col = 5-int(f%6);
    }
    else{
      //bottomUp
      col = int(f%6);
    }    
    if (i >= 42) remap[row][col][1] = i;
    if (i < 42) remap[row][col][0] = i;
  }
  Serial.begin(9600);
  reset();
}

void loop() {
  
  if (!gameEnd) winner = winCheck();
  if (winner == 1){
    gameEnd = true;
    color(1);
    Serial.println("reset");
    if (prevCursorP2 == 0 && digitalRead(cursorP2) == 1){
      playerOneTurn = false;
      reset();
    }
  }
  if (winner == 2){
    gameEnd = true;
    color(2);
    Serial.println("reset");
    if (prevCursorP1 == 0 && digitalRead(cursorP1) == 1){
      playerOneTurn = true;
      reset();
    }
  }
  if (winner == 0 && sum() == 0){
    gameEnd = true;
    draw();
    if ((prevCursorP1 == 0 && digitalRead(cursorP1) == 1) || (prevCursorP2 == 0 && digitalRead(cursorP2) == 1)){
      playerOneTurn = false;
      reset();
    }
  }

  
  // put your main code here, to run repeatedly:
  if (playerOneTurn){
    if (prevCursorP1 == 0 && digitalRead(cursorP1) == 1){ //off is on and on is off
      canGo = true;    
      iVal += 1;
      iVal %= rows;
      move(iVal, 1);
    }
    prevCursorP1 = digitalRead(cursorP1);
    if (prevDropP1 == 1 && digitalRead(dropP1) == 0 && !pixelState[iVal][columns-1] && canGo){
      canGo = false;
      dropRow(iVal, 1); //make animation
      playerOneTurn = false;
      iVal = -1;
    }
    prevDropP1 = digitalRead(dropP1);
  }
  else{
    if (prevCursorP2 == 0 && digitalRead(cursorP2) == 1){ //off is on and on is off
      canGo = true;
      iVal += 1;
      iVal %= rows;
      move(iVal, 2);
    }
    prevCursorP2 = digitalRead(cursorP2);
    if (prevDropP2 == 1 && digitalRead(dropP2) == 0 && !pixelState[iVal][columns-1] && canGo){
      canGo = false;
      dropRow(iVal, 2); //make animation
      playerOneTurn = true;
      iVal = -1;
    }
    prevDropP2 = digitalRead(dropP2);
  }
  
  delay(50);

}

void dropRow(int dropSpot, int color){
  sel[dropSpot] = 0;  
  int fast = 0;
  for (int i = columns-1; i >= 0; i--){
    if (pixelState[dropSpot][i] != 0){
      pixelState[dropSpot][i+1] = color;
      reFlashArray();
      return;
    }
    else{
      pixelState[dropSpot][i] = color;
      reFlashArray();
      delay(300-fast);
      fast += 50;
      pixelState[dropSpot][i] = 0;      
    }
  }
  pixelState[dropSpot][0] = color;
  reFlashArray();
}

void move(int spot, int color){
  sel[(spot-1+7)%7] = 0;
  sel[spot] = color;
  reFlashArray();    
}

void reset(){
  for (int i = 0; i < rows; i++) sel[i] = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++)pixelState[i][j] = 0;
  }
  gameEnd = false;
  canGo = false;
  initialize = false;
  winner = 0;
  reFlashArray();
}

int winCheck(){
  int winner = 0;
  for (int i = 0; i < rows; i++){
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
      if (i <= rows-4 && j >= 3){
      	if (pixelState[i][j] == 1 && pixelState[i+1][j-1] == 1 && pixelState[i+2][j-2] == 1 && pixelState[i+3][j-3] == 1) winner = 1;
        if (pixelState[i][j] == 2 && pixelState[i+1][j-1] == 2 && pixelState[i+2][j-2] == 2 && pixelState[i+3][j-3] == 2) winner = 2;
      }
    }
  }
  return winner;
}

void reFlashArray(){
  for (int i = 0; i < rows; i++){
    if (sel[i] == 0) strip.setPixelColor(selector[i], strip.Color(0,0,0));
    if (sel[i] == 1) strip.setPixelColor(selector[i], strip.Color(56,56,177));
    if (sel[i] == 2) strip.setPixelColor(selector[i], strip.Color(62,184,91)); 
    //blue rgb = rgba(56,56,177) 
    //red rgb =  rgba(184,62,91)
    //this is in grb format            
    
    for (int j = 0; j < columns; j++){
      if (pixelState[i][j] == 0){
        strip.setPixelColor(remap[i][j][0], strip.Color(0,0,0));
        strip.setPixelColor(remap[i][j][1], strip.Color(0,0,0));
      }
      if (pixelState[i][j] == 1){
        strip.setPixelColor(remap[i][j][0], strip.Color(56,56,177));
        strip.setPixelColor(remap[i][j][1], strip.Color(56,56,177));
      } 
      if (pixelState[i][j] == 2) {
        strip.setPixelColor(remap[i][j][0], strip.Color(62,184,91));
        strip.setPixelColor(remap[i][j][1], strip.Color(62,184,91));
      }
    }
  }
  strip.show();
}

void color(int player){
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < columns; j++){
      pixelState[i][j] = player;
    }
  }
  reFlashArray();
}

void draw(){
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < columns; j++){
      pixelState[i][j] = (i+j)%2+1;
    }
    
  }  
  reFlashArray();
}

int sum(){
  int a;
  for (int i = 0; i < rows; i++){
    for (int j = 0; j < columns; j++){
      if (!pixelState[i][j]) a++;
    }
  }
  return a;
}
