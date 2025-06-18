#include "LCD17lib.h"

// MG-880 https://ja.m.wikipedia.org/wiki/%E3%82%B2%E3%83%BC%E3%83%A0%E9%9B%BB%E5%8D%93

#define KEY_AIM  6
#define KEY_FIRE 7

#define showNum(d) lcd_show_num(0, d)
void showMe(uint8_t d){
  switch(d){
    case 0: lcd_show_segment(1, 0x00); break;
    case 1: lcd_show_segment(1, 0x40); break;
    case 2: lcd_show_segment(1, 0x48); break;
    case 3: lcd_show_segment(1, 0x49); break;
  }
}

// sum = sum + num, if (sum % 10) == 0 -> UFO
// all clear -> fill Me

#define N_INV 15

int8_t inv[N_INV];

void clearAllInv(){
  for (uint8_t i = 0; i < N_INV; i++) inv[i] = -1;
}

void showInv(){
  for (uint8_t i = 0; i < N_INV; i++){
    if (inv[i] < 0) lcd_show_segment(i + 2, 0x00);
    else if (inv[i] >= 10) lcd_show_segment(i + 2, 0x54); // UFO
    else lcd_show_num(i + 2, inv[i]);
  }
}

void setAndScrollInv(uint8_t d){
  for (uint8_t i = 1; i < N_INV; i++) inv[i-1] = inv[i];
  inv[N_INV - 1] = d;
  showInv();
}

uint8_t getTopInvPos(){
  uint8_t i = 0;
  while(i < N_INV && inv[i] == -1) i++;
  return(i);
}

uint8_t getTopInv(){
  uint8_t i = 0;
  while(i < N_INV && inv[i] == -1) i++;
  return(inv[i]);
}

uint8_t eraseTopInv(){
  uint8_t i = 0;
  while(i < N_INV && inv[i] == -1) i++;
  if (i < N_INV){
    inv[i] = -1;
    showInv();
  }
  return(i);
}

void setup(){
  Serial.begin(9600);
  lcd_init();
  lcd_clear();
  pinMode(KEY_AIM, INPUT_PULLUP);  
  pinMode(KEY_FIRE, INPUT_PULLUP);  
  showNum(0);
  showMe(3);
  randomSeed(analogRead(0));
  clearAllInv();
}

uint8_t num = 0;
uint8_t me = 3;
uint8_t sum = 0;
bool fUFO = false;

uint16_t cnt = 0;
uint16_t Tcycle = 100;

uint16_t score = 0;

#define SCORE_UNIT 1
#define SCORE_UFO  10

void showOneDigit(uint8_t pos, uint8_t num, bool fZeroSupress = true){
  lcd_show_num(pos, num);
}

void showScore(){
  uint8_t d1000, d100,d10, d1;
  d1000 = score / 1000;
  d100 = (score / 100) % 10;
  d10 = (score / 10) % 10;
  d1 = score % 10;
  clearAllInv();
  showInv();
  for (uint8_t i = 0; i < 3; i++){
    showOneDigit(13, d1000); showOneDigit(14, d100); showOneDigit(15, d10); showOneDigit(16, d1);
    delay(500);
    showInv();
    delay(500);
  }
}

void loop()
{
  Tcycle = analogRead(A5) / 2 + 1; // analogRead(A5): 0 - 700
  if (digitalRead(KEY_AIM) == 0){
    num = (num + 1) % 10;
    showNum(num);
    while(digitalRead(KEY_AIM) == 0) delay(10);
  }
  if (digitalRead(KEY_FIRE) == 0){
//    num = getTopInv();
    if (getTopInv() == num || (getTopInv() >= 10 && num == 0)){
      if (getTopInv() < 10) score += SCORE_UNIT;
      else score += SCORE_UFO;
      eraseTopInv();
      sum = sum + num;
      if (sum >= 10) sum -= 10;
      if (sum == 0) fUFO = true;
      if (getTopInvPos() == 15){
        // all inv cleared
        me = 3; showMe(me);
        for (uint8_t i = 0; i < 15; i++){
          showOneDigit(i + 2, 8); delay(50);
        }
        clearAllInv(); showInv();
      }
    }
    while(digitalRead(KEY_FIRE) == 0) delay(10);
  }
  delay(10);
  cnt++;
  if (cnt >= Tcycle){
    cnt = 0;
    if (getTopInvPos() == 0){
      clearAllInv();
      showScore();
      if (me == 1){
        // game over
        Serial.print("Game over");
        while(digitalRead(KEY_AIM) == 1 && digitalRead(KEY_FIRE) == 1) showScore();
        me = 3; score = 0; showMe(me);
      }
      else{
        me--;
        showMe(me);
        clearAllInv(); 
        // show score
      }
    }
    else{
      uint8_t inv;
      if (fUFO == true) inv = 10; else inv = random(0, 10);
      fUFO = false;
      setAndScrollInv(inv);
    }
  }
}
