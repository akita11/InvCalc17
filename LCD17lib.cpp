#include <Arduino.h>
#include "LCD17lib.h"
// for LH02183
// http://blog.livedoor.jp/yokoshima_m/archives/6585052.html
// https://github.com/KenKenMkIISR/LH02813B-TDC-1

// pin: (10->1) NC/LEDK/LEDA/VLCC/VCC(3.3V)/GND/DAT/WR/RD/CS

#define PIN_DAT 2
#define PIN_WR  3
#define PIN_CS  4
#define PIN_RD  5 // unused
//mode
#define MODE_CMD	 0x04	//comannd mode
#define MODE_WRITE 0x05	//write_mode

#define BUF_LEN 0x25 // =37
char lcd_buf[BUF_LEN][2]; //LCD buffer

#define lcd_display_on()   lcd_command(0x03)
#define lcd_display_off()  lcd_command(0x02)

// Font Data
//   E0
// E2  E1
//   E3
// O1  O0
//   O2  O3
const char seg_data[16][2] =//charactor
{
  //Even, Odd
  {0x7, 0x7}, //0
  {0x2, 0x1}, //1
  {0xb, 0x6}, //2
  {0xb, 0x5}, //3
  {0xe, 0x1}, //4
  {0xd, 0x5}, //5
  {0xd, 0x7}, //6
  {0x3, 0x1}, //7
  {0xf, 0x7}, //8
  {0xf, 0x5}, //9
  {0xf, 0x3}, //A
  {0xc, 0x7}, //B
  {0x5, 0x6}, //C
  {0xa, 0x7}, //D
  {0xd, 0x6}, //E
  {0xd, 0x2}  //F
};

// addr for each digit (left -> right)
const char digit_addr[17] = { 0x22, 0x20, 0x1e, 0x1c, 0x1a, 0x18, 0x16, 0x14, 0x12, 0x10, 0x0e, 0x0c, 0x00, 0x02, 0x04, 0x06, 0x08 };

void lcd_mode(char mode)
{
  // dat= 3bit, MSB first, rising edge of WR
  char n;
  digitalWrite(PIN_WR, 1);
  for (n = 0; n < 3; n++) {
    if ((mode & 0x04) != 0) digitalWrite(PIN_DAT, 1);
    else digitalWrite(PIN_DAT, 0);
    digitalWrite(PIN_WR, 0); digitalWrite(PIN_WR, 1); 
    mode = mode << 1;
  }
}

void lcd_send_addr(char addr)
{
  char n;
  // 6bit, MSB first, rising edge of WR
  digitalWrite(PIN_WR, 1);
  for (n = 0; n < 6; n++){//read address send
    if((addr & 0x20) != 0) digitalWrite(PIN_DAT, 1);
    else digitalWrite(PIN_DAT, 0);
    digitalWrite(PIN_WR, 0); digitalWrite(PIN_WR, 1); 
    addr = addr << 1;
  }
}	

void lcd_write_data(char addr, char dat)
{
  char n;
  // CS=0 -> mode -> send_adrs -> 4bit, LSB first, rising edge of WR -> CS=1
  digitalWrite(PIN_CS, 0);
  lcd_mode(MODE_WRITE); //writ mode send
  lcd_send_addr(addr); //address send
  for (n = 0; n < 4; n++){
    if((dat & 0x01) != 0) digitalWrite(PIN_DAT, 1);
    else digitalWrite(PIN_DAT, 0);
    digitalWrite(PIN_WR, 0); digitalWrite(PIN_WR, 0); // wait
    digitalWrite(PIN_WR, 1); digitalWrite(PIN_WR, 1); // wait
    dat = dat >> 1;
  }
  digitalWrite(PIN_CS, 1);
}

void lcd_command(char command)
{
  char n;
  // CS=0 -> mode -> 8bit, MSB first + '1', rising edfe of WR -> CS=1
  digitalWrite(PIN_CS, 0);
  lcd_mode(MODE_CMD); //writ mode send
  for (n = 0; n < 8; n++){
    if((command & 0x80) != 0) digitalWrite(PIN_DAT, 1);
    else digitalWrite(PIN_DAT, 0);
    digitalWrite(PIN_WR, 0); digitalWrite(PIN_WR, 1); 
    command = command << 1;
  }
  digitalWrite(PIN_DAT, 1); // '1'
  digitalWrite(PIN_WR, 0); digitalWrite(PIN_WR, 1);  
  digitalWrite(PIN_CS, 1);
}

void lcd_init(void)
{
  pinMode(PIN_CS, OUTPUT);  digitalWrite(PIN_CS, 1);
  pinMode(PIN_DAT, OUTPUT); digitalWrite(PIN_DAT, 1);
  pinMode(PIN_WR, OUTPUT);  digitalWrite(PIN_WR, 1);
  lcd_command(0x01); // enable SYS
  delay(10);
  lcd_display_on();
  delay(10);
}

void lcd_clear(void)
{ 
  char n;
  for(n = 0; n <= BUF_LEN; n++) lcd_write_data(n, 0);
}

void lcd_show_num(char position, char number)
{
  //position*=2;
  lcd_write_data(digit_addr[position],   seg_data[number][0]);
  lcd_write_data(digit_addr[position]+1, seg_data[number][1]);
}

void lcd_show_segment(char position, char dat)
{
  // dat: -GFEDCBA
  char datE = 0, datO = 0;
  if (dat & 0x40) datE |= 0x08; // G
  if (dat & 0x20) datE |= 0x02; // F
  if (dat & 0x10) datO |= 0x01; // E
  if (dat & 0x08) datO |= 0x04; // D
  if (dat & 0x04) datO |= 0x02; // C
  if (dat & 0x02) datE |= 0x04; // B
  if (dat & 0x01) datE |= 0x01; // A
  
//   E0
// E2  E1
//   E3
// O1  O0
//   O2  O3
  lcd_write_data(digit_addr[position],   datE);
  lcd_write_data(digit_addr[position]+1, datO);
}
