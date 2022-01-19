#include "LCD.h"

void LCD_Init()
{
    __delay_ms(50);
    LCD_Command(FUNCS | D_LEN | NLINE);
    LCD_Command(CLR);
    __delay_ms(2);
    
    LCD_Command(DISP | D_ON | C_ON);
    LCD_Command(ENTRY | I_ND);
    LCD_Command(MOV | R_NL);
    
    // Build custom bitmap
    LCD_Command(ADD_C | 0x00 << 3);
    for(int i = 0;i < 8;++i)
        LCD_Data(cgram_l[i]);
    LCD_Command(ADD_C | 0x01 << 3);
    for(int i = 0;i < 8;++i)
        LCD_Data(cgram_u[i]);
    LCD_Command(ADD_C | 0x02 << 3);
    for(int i = 0;i < 8;++i)
        LCD_Data(cgram_d[i]);
    LCD_Command(ADD_C | 0x03 << 3);
    for(int i = 0;i < 8;++i)
        LCD_Data(cgram_h[i]);
    // AD at CDRAM here
    LCD_Command(HOME);
    __delay_ms(2);
}

void LCD_Command(unsigned char value)
{
    LCD_busy();
    RS = 0;
    __delay_us(5);
    RW = 0;
    __delay_us(5);
    LCD = value;
    __delay_us(25);
    EN = 1;
    __delay_ms(10);
    EN = 0;
    __delay_us(50);
}

void LCD_Data(unsigned char value)
{
    LCD_busy();
    RS = 1;
    __delay_us(5);
    RW = 0;
    __delay_us(5);
    LCD = value;
    __delay_us(25);
    EN = 1;
    __delay_us(5);
    EN = 0;
    __delay_ms(10);
}

void LCD_busy()
{
    unsigned char busy = 1;
    DDR = 0xFF;
    RS = 0;
    RW = 1;
    do{
        EN = 1;
        __delay_ms(15);
        busy = BUSY;
        EN = 0;
    }while(busy);
    DDR = 0;
    RW = 0;
}
