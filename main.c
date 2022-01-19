#include "main.h"
#include "LCD.h"

void HP_ISR(void);
void LP_ISR(void);

void __interrupt(high_priority) high_isr(void)
{
    HP_ISR();
}

void __interrupt(low_priority) lo_isr(void)
{
    LP_ISR();
}

unsigned char storage[STR_MAX][32];

void init(void);
void init_int(void);
void init_timer0(void);
void init_timer2(void);
void init_ccp1(void);
void init_uart(void);

void LCD_PStr(const char *str);
uint8_t uart_read(void);

void main(void)
{
    init();
    LCD_Init();
    init_int();
    init_timer0();
    init_timer2();
    init_ccp1();
    init_uart();
    
    T2CONbits.TMR2ON = 1;
    ADCON0bits.GO = 1;
    
    INTCONbits.GIE = 0;
    LCD_Command(ADD_D | 0x01);
    LCD_PStr("Hello");
    LCD_Command(ADD_D | 0x42);
    LCD_PStr("World");

    LCD_Command(ADD_D | 0x07);
    INTCONbits.GIE = 1;
    while(1){
    }
}

void init()
{
    ADCON0 = 0x01;  // CHANNEL 1
    ADCON1 = 0x0E;  // AN0 DIGITAL (AN0/RA0, PIN2)
    CDR = DDR = 0;
    EN = 0;
    
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0x00;   // CCP1 output
    
    PORTA = PORTB = PORTC = PORTD = PORTE = 0x00;
}

void init_int()
{
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    INTCONbits.INT0IE = 1;
    INTCON3bits.INT1IE = 1;
    INTCON3bits.INT2IE = 1;
    INTCONbits.TMR0IE = 1;
    PIE1bits.ADIE = 1;
    
    
    RCONbits.IPEN = 1;
    IPR1bits.ADIP = 0;
    
    // RCIE in mode handler
    PIE1bits.RCIE = 0;
    IPR1bits.RCIP = 1;
}

void init_timer0()
{
    INTCONbits.TMR0IF = 0;
    T0CONbits.PSA = 0;      // use pre-scaler
    T0CONbits.T0CS = 0;     // instruction cycle clock
    T0CONbits.T08BIT = 0;   // 16-bits timer
    T0CONbits.T0PS = 4;   // 1:32 pre-scaler
}

void init_timer2()
{
    // TA > 0.7u
    // OSC = 1M
    ADCON2 = 0x93;  // T(ACQ) = 4T(AD), T(AD) = T(OSC), right-justified
    T2CON = 0x7A; // 16x16 pre-scale/post-scale
    
    PR2 = 0xFF;
    TMR2 = 0;
}

void init_ccp1()
{
    CCP1CONbits.CCP1M = 0x0C;   // PWM
}

void LCD_PStr(const char *str)
{
    while(*str != '\0')
        LCD_Data(*str++);
}

void init_uart()
{
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    BAUDCONbits.BRG16 = 1;
    SPBRG = 25;
    
    RCSTAbits.SPEN = 1;
    RCSTAbits.RX9 = 0;
    RCSTAbits.CREN = 1;
    
    TXSTAbits.TX9 = 0;
    TXSTAbits.TXEN = 1;
    TXSTAbits.SENDB = 0;
    TXSTAbits.TRMT =  1;
}

#pragma interrupt HP_ISR
void HP_ISR(void)
{
    __delay_ms(100);
    // L R Button
    DDR = 0xFF;
    RS = 0;
    RW = 1;
    EN = 1;
    addr = LCD & 0x7F;
    EN = 0;
    DDR = 0x00;
    if(INTCON3bits.INT1IF && INTCON3bits.INT2IF){
        INTCON3bits.INT1IF = 0;
        INTCON3bits.INT2IF = 0;
        if(mode == MODE_0){
            if(PORTBbits.RB1)
                if(addr == 0x00)
                    LCD_Command(ADD_D | 0x4F);
                else if(addr == 0x40)
                    LCD_Command(ADD_D | 0x0F);
                else
                    LCD_Command(MOV);
            else if(PORTBbits.RB2)
                if(addr == 0x4F)
                    LCD_Command(ADD_D | 0x00);
                else if(addr == 0x0F)
                    LCD_Command(ADD_D | 0x40);
                else
                    LCD_Command(MOV | R_NL);
            while(PORTBbits.RB1 || PORTBbits.RB2)
                ;
        }
        else if(mode == MODE_TMR){
            if(PORTBbits.RB1){
                if(T0CONbits.T0PS < 7)
                    T0CONbits.T0PS += 1;
            }
            else if(PORTBbits.RB2){
                if(T0CONbits.T0PS > 0)
                    T0CONbits.T0PS -= 1;
            }
        }
        else if(mode == MODE_ANI){
            if(PORTBbits.RB1){
                if(ani_signal){
                    LCD_Data(1);
                    LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                    LCD_Command(MOV | D_NC | R_NL);
                    ani_signal = 0;
                }
                else{
                    LCD_Data(2);
                    LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                    LCD_Command(MOV | D_NC | R_NL);
                    ani_signal = 1;
                }
            }
            else if(PORTBbits.RB2){
                if(ani_signal){
                    LCD_Data(3);
                    LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                    LCD_Command(MOV | D_NC | R_NL);
                }
                else{
                    LCD_Data(0);
                    LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                    LCD_Command(MOV | D_NC | R_NL);
                }
            }
        }
        else if(mode == MODE_DIS){
            LCD_Command(CLR);
            LCD_Command(HOME);
            LCD_PStr(storage[dis_index]);
            dis_index = (dis_index + 1) % str_count;
        }
        while(PORTBbits.RB1 || PORTBbits.RB2)
                ;
    }
    else if(INTCON3bits.INT1IF){
        INTCON3bits.INT1IF = 0;
        if(mode == MODE_0){
            if(addr == 0x00)
                LCD_Command(ADD_D | 0x4F);
            else if(addr == 0x40)
                LCD_Command(ADD_D | 0x0F);
            else
                LCD_Command(MOV);
        }
        else if(mode == MODE_TMR){
            if(T0CONbits.T0PS < 7)
                T0CONbits.T0PS += 1;
        }
        else if(mode == MODE_ANI){
            if(ani_signal){
                LCD_Data(1);
                LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                LCD_Command(MOV | D_NC | R_NL);
                ani_signal = 0;
            }
            else{
                LCD_Data(2);
                LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                LCD_Command(MOV | D_NC | R_NL);
                ani_signal = 1;
            }
        }
        else if(mode == MODE_DIS){
            LCD_Command(CLR);
            LCD_Command(HOME);
            LCD_PStr(storage[dis_index]);
            dis_index = (dis_index + 1) % str_count;
        }
        while(PORTBbits.RB1)
                ;
    }
    else if(INTCON3bits.INT2IF){
        INTCON3bits.INT2IF = 0;
        if(mode == MODE_0){
            if(addr == 0x4F)
                LCD_Command(ADD_D | 0x00);
            else if(addr == 0x0F)
                LCD_Command(ADD_D | 0x40);
            else
                LCD_Command(MOV | R_NL);
        }
        else if(mode == MODE_TMR){
            if(T0CONbits.T0PS < 7)
                T0CONbits.T0PS += 1;
        }
        else if(mode == MODE_ANI){
            if(ani_signal){
                LCD_Data(3);
                LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                LCD_Command(MOV | D_NC | R_NL);
            }
            else{
                LCD_Data(0);
                LCD_Command(ADD_D | ((addr==0x40) ? 0x67 : addr - 1));
                LCD_Command(MOV | D_NC | R_NL);
            }
        }
        else if(mode == MODE_DIS){
            LCD_Command(CLR);
            LCD_Command(HOME);
            LCD_PStr(storage[dis_index]);
            dis_index = (dis_index + 1) % str_count;
        }
        while(PORTBbits.RB2)
            ;
    }
    // Next Button
    else if(INTCONbits.INT0IF){
        INTCONbits.INT0IF = 0;
        
        // before mode switch
        if(mode == MODE_KEY || mode == MODE_ENC || mode == MODE_STR){
            PIE1bits.RCIE = 0;
        }
        if(mode == MODE_DIS){
            str_index = str_count = dis_index = 0;
        }
        
        mode = (mode+1) % MODE_CNT;
        LCD_Data(mode + '0');
        
        // after mode switch
        if(mode == MODE_0){
            LCD_Command(CLR);
            LCD_Command(ADD_D | 0x01);
            LCD_PStr("Hello");
            LCD_Command(ADD_D | 0x42);
            LCD_PStr("World");
        }
        else if(mode == MODE_TMR){
            timer = 0;
            LCD_Command(CLR);
            LCD_Command(ADD_D | 0x01);
            LCD_PStr("Timer");
            LCD_Command(ADD_D | 0x43);
            T0CONbits.T0PS = 0b100;       // 1:32 pre-scaler
            TMR0 = 0xFFFF - 7032;       // 1 sec - 100ms delay
            LCD_Data((timer%1000)/100 + '0');
            LCD_Data((timer%100)/10 + '0');
            LCD_Data((timer%10) + '0');
            T0CONbits.TMR0ON = 1;
        }
        else if(mode == MODE_KEY || mode == MODE_ENC || mode == MODE_STR){
            LCD_Command(CLR);
            LCD_Command(HOME);
            PIE1bits.RCIE = 1;
            if(mode == MODE_ENC){
                LCD_PStr("Encoding");
                LCD_Command(ADD_D | 0x40);
            }
            else if(mode == MODE_STR){
                LCD_PStr("Store Line");
                LCD_Command(ADD_D | 0x40);
            }
        }
        else if(mode == MODE_ANI){
            LCD_Command(CLR);
            LCD_Command(HOME);
            LCD_Command(ADD_D | 0x41);
            LCD_PStr("Wave");
            LCD_Command(ADD_D | 0x40);
        }
        else if(mode == MODE_DIS){
            LCD_Command(CLR);
            LCD_Command(HOME);
        }
        while(PORTBbits.RB0)
            ;
    }
    else if(INTCONbits.TMR0IF){
        INTCONbits.TMR0IF = 0;
        T0CONbits.TMR0ON = 0;
        TMR0 = 0xFFFF - 7032;       // 1 sec - 100ms delay (1:32 pre-scaler)
        if(mode == MODE_TMR){
            timer += 1;
            LCD_Command(ADD_D | 0x43);
            LCD_Data((timer%1000)/100 + '0');
            LCD_Data((timer%100)/10 + '0');
            LCD_Data((timer%10) + '0');
            T0CONbits.TMR0ON = 1;
        }
    }
    else if(PIR1bits.RCIF){
        if(mode == MODE_KEY){
            if(RCSTAbits.OERR){
                RCSTAbits.CREN = 0;
                asm("nop");
                RCSTAbits.CREN = 1;
            }
            c = RCREG;
            DDR = 0xFF;
            RS = 0;
            RW = 1;
            EN = 1;
            addr = LCD & 0x7F;
            EN = 0;
            DDR = 0x00;
            __delay_us(5);
            if((addr == 0x10) || (addr == 0x50) || (c == '\r') || (c == '\n')){
                if((addr&0xF0) >= 0x40)
                    LCD_Command(ADD_D | 0x00);
                else
                    LCD_Command(ADD_D | 0x40);
                if((addr == 0x10) || (addr == 0x50))
                    LCD_Data(c);
            }
            else if(c == 0x7F){
                if(addr == 0x00)
                    LCD_Command(ADD_D | 0x4F);
                else if(addr == 0x40)
                    LCD_Command(ADD_D | 0x0F);
                else
                    LCD_Command(MOV);
                LCD_Data(0xFE);
                if(addr == 0x00)
                    LCD_Command(ADD_D | 0x4F);
                else if(addr == 0x40)
                    LCD_Command(ADD_D | 0x0F);
                else
                    LCD_Command(MOV);
            }
            else{
                LCD_Data(c);
            }
        }
        else if(mode == MODE_ENC){
            if(RCSTAbits.OERR){
                RCSTAbits.CREN = 0;
                asm("nop");
                RCSTAbits.CREN = 1;
            }
            c = RCREG;
            DDR = 0xFF;
            RS = 0;
            RW = 1;
            EN = 1;
            addr = LCD & 0x7F;
            EN = 0;
            DDR = 0x00;
            __delay_us(5);
            if((addr == 0x50) || (c == '\r') || (c == '\n')){
                LCD_Command(ADD_D | 0x40);
                if(addr == 0x50)
                    LCD_Data(c);
            }
            else if(c == 0x7F){
                if(addr == 0x40)
                    LCD_Command(ADD_D | 0x4F);
                else
                    LCD_Command(MOV);
                LCD_Data(0xFE);
                if(addr == 0x40)
                    LCD_Command(ADD_D | 0x4F);
                else
                    LCD_Command(MOV);
            }
            else if((((c >= 'a') && (c <= 'z'))) || ((c >= 'A') && (c <= 'Z'))){
                c = (unsigned char)((int)c & 0xFC) | ((int)c & 0x01) << 1 | ((int)c & 0x02) >> 1;
                LCD_Data(c);
            }
            else{
                LCD_Data(c);
            }
        }
        else if(mode == MODE_STR){
            if(RCSTAbits.OERR){
                RCSTAbits.CREN = 0;
                asm("nop");
                RCSTAbits.CREN = 1;
            }
            c = RCREG;
            DDR = 0xFF;
            RS = 0;
            RW = 1;
            EN = 1;
            addr = LCD & 0x7F;
            EN = 0;
            DDR = 0x00;
            __delay_us(5);
            if((addr == 0x50) || (c == '\r') || (c == '\n')){
                if(str_count == STR_MAX){
                    LCD_Command(CLR);
                    LCD_Command(HOME);
                    LCD_PStr("NO MORE SPACE");
                }
                else{
                    storage[str_count][str_index] = '\0';
                    str_index = 0;
                    str_count += 1;
                    LCD_Command(CLR);
                    LCD_Command(HOME);
                    LCD_PStr("Store Line");
                    LCD_Command(ADD_D | 0x40);
                }
            }
            else if(c == 0x7F){
                str_index -= 1;
                if(addr == 0x40)
                    LCD_Command(ADD_D | 0x4F);
                else
                    LCD_Command(MOV);
                LCD_Data(0xFE);
                if(addr == 0x40)
                    LCD_Command(ADD_D | 0x4F);
                else
                    LCD_Command(MOV);
            }
            else{
                storage[str_count][str_index++] = c;
                LCD_Data(c);
            }
        }
    }
    else{
        INTCONbits.INT0IF = 0;
        INTCON3bits.INT1IF = 0;
        INTCON3bits.INT2IF = 0;
    }
}

#pragma interruptlow LP_ISR
void LP_ISR(void)
{
    if(PIR1bits.ADIF == 1){
        CCPR1L = (unsigned char)((ADRES & 0xFFFC) >> 2);
        CCP1CONbits.DC1B = ADRES & 0x03;
        PIR1bits.ADIF = 0;
        _delay(9);          // 2T(AD) = 8T(OSC)
        ADCON0bits.GO = 1;
    }
}