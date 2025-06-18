void lcd_command(char command);
void lcd_init(void);
void lcd_clear(void);
void lcd_show_num(char position, char number);
void lcd_show_segment(char position, char segment);
/*
  A=0x01, B=0x02, ... G=0x40
    A
  B   F
    G
  C   E
    D
*/