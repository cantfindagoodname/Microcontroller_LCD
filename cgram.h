/* 
 * File:   cgram.h
 * Author: User
 *
 * Created on January 18, 2022, 9:14 PM
 */

#ifndef CGRAM_H
#define	CGRAM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
unsigned int cgram_addr = 0;
    
// LCD_Data(0)
unsigned char cgram_l[8] = {
    0,0,0,0,0,0,31,0
};

// LCD_Data(1)
unsigned char cgram_u[8] = {
    7,4,4,4,4,4,28,0
};

// LCD_Data(2)
unsigned char cgram_d[8] = {
    28,4,4,4,4,4,7,0
};

// LCD_Data(3)
unsigned char cgram_h[8] = {
    31,0,0,0,0,0,0,0
};

#ifdef	__cplusplus
}
#endif

#endif	/* CGRAM_H */

