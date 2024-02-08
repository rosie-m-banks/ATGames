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

///////////////////////////////////////////////////////////////////SOUND
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
bool playSound = true;
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
  reset();
  playSound = true;
}

void loop() {
  
  if (!gameEnd) winner = winCheck();
  if (winner == 1){
    gameEnd = true;
    color(1);
    //Serial.println("reset");
    if (prevCursorP2 == 0 && digitalRead(cursorP2) == 1){
      playerOneTurn = false;
      reset();
    }
  }
  if (winner == 2){
    gameEnd = true;
    color(2);
    //Serial.println("reset");
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
    if (prevDropP1 == 0 && digitalRead(dropP1) == 1 && !pixelState[iVal][columns-1] && canGo){
      canGo = false;
      dropRow(iVal, 1); //make animation
      playerOneTurn = false;
      sendCmd(0x03, 2, 0, false); //play track 002 in folder 01   
      Serial.println("im getting called");         
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
    if (prevDropP2 == 0 && digitalRead(dropP2) == 1 && !pixelState[iVal][columns-1] && canGo){
      canGo = false;
      dropRow(iVal, 2); //make animation
      playerOneTurn = true;
      sendCmd(0x03, 2, 0, false); //play track 002 in folder 01
      Serial.println("im getting called");
      iVal = -1;
    }
    prevDropP2 = digitalRead(dropP2);
  }

  //Serial.println(playSound);
  if (gameEnd && playSound){
    Serial.println("Stop anything playing"); //in case anything is already playing on reset
    sendCmd(0x0E, 0, 0, false);
    delay(200); //give a delay for the DF Player to execute the command
    sendCmd(0x03, 3, 0, false); //play track 003 in folder 01
    Serial.println("now me");
    delay(200);
    playSound = false;
  }   
  
  delay(50);

}

void dropRow(int dropSpot, int color){
  sendCmd(0x03, 1, 0, false); //play track 003 in folder 01
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
  Serial.println("Stop anything playing"); //in case anything is already playing on reset
  sendCmd(0x0E, 0, 0, false);
  delay(200); //give a delay for the DF Player to execute the command
  playSound = true;
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
    if (sel[i] == 1) strip.setPixelColor(selector[i], strip.Color(20,25,255));
    if (sel[i] == 2) strip.setPixelColor(selector[i], strip.Color(43,255,50)); 
    //blue rgb = rgba(59,104,231) 
    //red rgb =  rgba(215,43,134)
    //this is in grb format            
    
    for (int j = 0; j < columns; j++){
      if (pixelState[i][j] == 0){
        strip.setPixelColor(remap[i][j][0], strip.Color(0,0,0));
        strip.setPixelColor(remap[i][j][1], strip.Color(0,0,0));
      }
      if (pixelState[i][j] == 1){
        strip.setPixelColor(remap[i][j][0], strip.Color(20,25,255));
        strip.setPixelColor(remap[i][j][1], strip.Color(20,25,255));
      } 
      if (pixelState[i][j] == 2) {
        strip.setPixelColor(remap[i][j][0], strip.Color(43,255,60));
        strip.setPixelColor(remap[i][j][1], strip.Color(43,255,60));
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
