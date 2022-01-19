/* 
 * File:   main.h
 * Author: User
 *
 * Created on January 18, 2022, 5:31 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <pic18f4520.h>
#include <stdint.h>

#include "config.h"
    
#define _XTAL_FREQ 1000000UL
    
#define STR_MAX 8
    
typedef enum _mode_t{
    MODE_0,
    MODE_TMR,
    MODE_KEY,
    MODE_ENC,
    MODE_ANI,
    MODE_STR,
    MODE_DIS,
    MODE_CNT,
}mode_t;

mode_t mode = MODE_0;
uint8_t timer = 0;

unsigned int ani_signal = 0;
unsigned char c;
uint8_t addr = 0;

uint8_t str_index = 0, str_count = 0, dis_index = 0;
unsigned char storage[STR_MAX][32] = {'\0'};

#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

