/* 
 * File:   LCD.h
 * Author: User
 *
 * Created on January 18, 2022, 5:30 PM
 */

#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "main.h"
#include "cgram.h"

#define RS 		PORTEbits.RE0
#define RW      PORTEbits.RE1
#define EN 		PORTEbits.RE2
#define BUSY    PORTDbits.RD7
#define LCD     PORTD
#define CDR     TRISE
#define DDR     TRISD
    
// Commands
#define CLR     0b00000001
#define HOME    0b00000010
#define ENTRY   0b00000100
#define DISP    0b00001000
#define MOV     0b00010000
#define FUNCS   0b00100000
#define ADD_C   0b01000000
#define ADD_D   0b10000000  // address of DDRAM
    
// Entry Mode
#define I_ND    0b0010  // increment or not decrement
#define SHIFT_D 0b1     // shift display
    
// display
#define D_ON    0b100   // display on
#define C_ON    0b010   // cursor on
#define BLINK   0b001
    
// mov
#define D_NC    0b1000  // display or not cursor
#define R_NL    0b0100  // right or not left
    
// function
#define D_LEN   0b10000
#define NLINE   0b01000
#define FONT    0b00100
        
void LCD_Init(void);
void LCD_Command(unsigned char value);
void LCD_Data(unsigned char value);
void LCD_busy(void);

void LCD_custom(unsigned char *icon);

#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

