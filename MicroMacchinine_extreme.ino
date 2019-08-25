// Davide Caminati Demo Microscope 2019 Extreme edition
// Demo based on:
// UTFT_Demo by Henning Karlsen
// web: http://www.henningkarlsen.com/electronics

// TODO: Create podio animations and sounds

#include <Ucar.h>
#include "Adafruit_GFX_AS.h"
#include "Adafruit_R61581_AS.h"

Adafruit_R61581_AS myGLCD = Adafruit_R61581_AS();       

String grassLine =    "000000000000000000000000000"; 
String treeLine =     "000100000100000001000000100"; 
String asphaltLine =  "111111111111111111111111111";
String arr[11] = {grassLine, treeLine, grassLine, asphaltLine, asphaltLine, asphaltLine, 
                  asphaltLine, asphaltLine, asphaltLine, grassLine, treeLine};
int trackW, trackH ;
int trackLength; 
Ucar car_0(R61581_GREEN, 0, 4, 0, "GREEN");
Ucar car_1(R61581_RED, 0, 5, 0, "RED");
Ucar car_2(R61581_BLUE, 0, 6, 0, "BLUE");
Ucar * cars[] = {&car_0, &car_1, &car_2}; 
Ucar enemy_0(R61581_WHITE, trackLength, 0, 0, ""); 
Ucar enemy_1(R61581_WHITE, trackLength, 0, 0, ""); 
Ucar * enemies[] = {&enemy_0, &enemy_1}; 
int carCount, enemiesCount;
String trees = treeLine + treeLine;
int treePos = 0;
long oldTimerStreet, oldTimerCar, oldTimerEnemy, oldTimerFinishLine;
int timerStreet  = 250, acceleration = 10, actual_speed = 0;
int timerCar = 1019;
int timerEnemy = 139;
int timerFinishLine = 1000;
byte endRaceFlagBits = 0b00000000;

void setup(){
  Serial.begin(9600);
  randomSeed(analogRead(0));
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  delay(10);
  digitalWrite(7, HIGH);
  myGLCD.begin(R61581B);
  myGLCD.setRotation(3);
  trackW = sizeof(arr) / sizeof(arr[0]);
  trackH = arr[0].length();
  carCount = (sizeof(cars) / sizeof(cars[0]));
  enemiesCount = (sizeof(enemies) / sizeof(enemies[0]));
  trackLength = asphaltLine.length()-5;
  Serial.println(trackLength);
  drawTrack();
  startPlayers();
  startEnemies();
  oldTimerCar = millis(); oldTimerStreet = millis(); oldTimerEnemy = millis(); oldTimerFinishLine = millis();
}

void loop(){
  if (not isFinished()){
    if ((oldTimerStreet + timerStreet - actual_speed) < millis()){
      moveTrees();
      actual_speed += (actual_speed < timerStreet/1.5) ? acceleration:-acceleration;
      oldTimerStreet = millis();
      return;
    }
    if ((oldTimerCar + timerCar) < millis()){
      for (int c =0 ; c < carCount; c++)
        moveCar((*cars[c]),cars[c]->x + 1 - cars[c]->penality,cars[c]->y);
      oldTimerCar = millis();
      return;
    }
    if ((oldTimerEnemy + timerEnemy) < millis()){
      for (int e = 0; e < enemiesCount; e++)
        moveEnemy((*enemies[e]),enemies[e]->x - 1,enemies[e]->y);
      checkCollision();
      oldTimerEnemy = millis();
      return;
    }
  }
  if ((oldTimerFinishLine + timerFinishLine) < millis())
    finishFlagBlow();
}

void finishFlagBlow(){
  endRaceFlagBits = 0b00000011 & (endRaceFlagBits += 0b00000001 );
  for (int f = 0; f < 6; f++){
    int wallColor = 0;
    wallColor += (endRaceFlagBits == 0b00000001) ? R61581_RED : R61581_BLACK;
    wallColor += (endRaceFlagBits == 0b00000010) ? R61581_GREEN : R61581_BLACK;
    wallColor += (endRaceFlagBits == 0b00000011) ? R61581_BLUE : R61581_BLACK;
    myGLCD.drawPixel(trackLength+1, 3+f, wallColor); 
  }
  oldTimerFinishLine = millis();
}

bool isFinished(){
  for (int c = 0; c < carCount; c++)
    if (cars[c]->x > trackLength+1) return true;
  return false;
}

void checkCollision(){
  for (int e = 0; e < enemiesCount; e++){
    for (int c = 0; c < carCount; c++){
        if (((*enemies[e]).x == (*cars[c]).x) && ((*enemies[e]).y == (*cars[c]).y)) {
          cars[c]->penality += 1;
          (*enemies[e]).x = trackLength;
          (*enemies[e]).y = random(3,7);
      }
    }
  }
}

void drawTrack(){
  myGLCD.fillScreen(R61581_WHITE);
  for (int x = 0; x < trackH; x++){
    for (int y = 0; y < trackW; y++){
      if (arr[y].charAt(x) == '1') myGLCD.drawLine(x, y, x, y,R61581_BLACK);
    }
  }
}

void startPlayers(){
  for (int c = 0; c < carCount; c++)
    moveCar(*cars[c],cars[c]->x,cars[c]->y);
}

void startEnemies(){
  for (int e = 0; e < enemiesCount; e++)
    moveCar((*enemies[e]),enemies[e]->x ,enemies[e]->y += random(3,7));
}

bool canMove(int x,int y){
  return ((y > 2) && (y < 9));
}

void moveCar(Ucar &car,int x, int y){
  if (canMove(x,y)){
    int old_color = 0x0000, new_color = 0x0000;
    int old_x = car.x, old_y = car.y;
    for (int c = 0; c < carCount; c++)
      old_color += ((old_x == cars[c]->x) && (old_y == cars[c]->y) && (cars[c]->color != car.color)) ? cars[c]->color : R61581_BLACK;
    myGLCD.drawPixel(old_x, old_y, old_color); 
    car.x = x; car.y = y; car.penality = 0;
    for (int c = 0; c < carCount; c++)
      new_color += ((x == cars[c]->x) && (y == cars[c]->y)) ? cars[c]->color : R61581_BLACK;
    myGLCD.drawPixel(x, y, new_color);
  }
}

// TODO don't destroy car (BLACK)
void moveEnemy(Ucar &car,int x, int y){
  myGLCD.drawPixel(car.x, car.y, R61581_BLACK);
  car.x = x; car.y = y;
  myGLCD.drawPixel(car.x, car.y, car.color);
  if (car.x < 0) {car.y = random(3, 7); car.x = trackLength ;}
}

void moveTrees(){
  for (int x = 0; x < trackH-1 ; x++){
      myGLCD.drawLine(x, 1, x, 1,((trees.charAt(x+treePos) == '0') ? R61581_WHITE : R61581_BLACK));
      myGLCD.drawLine(x, 10, x, 10,((trees.charAt(x+treePos) == '0') ? R61581_WHITE : R61581_BLACK));
  }
  treePos = (treePos < trackH-1) ? treePos += 1:0;
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    switch (inChar){
      case 'q': moveCar((*cars[0]),cars[0]->x,cars[0]->y -1); break;
      case 'a': moveCar((*cars[0]),cars[0]->x,cars[0]->y +1); break;
      case 'w': moveCar((*cars[1]),cars[1]->x,cars[1]->y -1); break;
      case 's': moveCar((*cars[1]),cars[1]->x,cars[1]->y +1); break;
      case 'e': moveCar((*cars[2]),cars[2]->x,cars[2]->y -1); break;
      case 'd': moveCar((*cars[2]),cars[2]->x,cars[2]->y +1); break;
    }
  }
}
