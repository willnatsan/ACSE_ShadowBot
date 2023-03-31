/*
 * File:   autoBraking.c
 * Author: asusv
 *
 * Created on March 31, 2023, 7:59 AM
 */


#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#pragma config OSC = HS     //High speed resonator
#pragma config WDT = OFF    //watchdog timer off
#pragma config LVP = OFF    //Low voltage programmer disabled
#pragma config PWRT = ON    //Power up timer on
#define Leftmotor1A LATAbits.LA4    //Direction bits Left motor
#define Leftmotor2A LATAbits.LA5
#define Rightmotor3A LATBbits.LB0   //Direction bits Right motor
#define Rightmotor4A LATBbits.LB1
#define _XTAL_FREQ 10000000         // to use __ms_delay

void configPWM(void);       //Configure PWM
void goforward(void);       //Turn both motors forward
void initialiseIO(void);
unsigned int read_channel_left(void);
unsigned int read_channel_right(void);
float moving_average_10(int new_reading);
void delay(int ms);

unsigned int markspaceL;    //Mark space ratio for Left motor
unsigned int markspaceR;    //Mark space ratio for Right motor
unsigned int setpoint;      //target distance
unsigned int max_speed;
unsigned int min_speed;
int pwm;
int error;
int kp;

#define ARRAY_SIZE 150
int sensor_input[ARRAY_SIZE] = {0};

int main(void) {
    initialiseIO();
    TRISA = 0b11001111;    //Set PORTA pins
    TRISB = 0;              	 //Set all PORTB pins to outputs
    TRISC = 0b00111001;    //Set PORTC pins
    LATB = 0;              	 //Turn LEDs off
    configPWM();          	  //Configure PWM
    kp = 1;
    setpoint = 60;
    max_speed = 300;
    min_speed = 0;
    pwm = 0;
    while(1){
//        markspaceL=250;   //Left motor speed 
//        markspaceR=250;   //Right motor speed
//        goforward();     	 //Turn both motor forward
//        delay(4000);
//        markspaceL=500;   //Left motor speed 
//        markspaceR=500;   //Right motor speed
//        goforward();
//        delay(4000);
        
     
        int avg = moving_average_10(read_channel_left());
        
        if ((avg <= 110) && (avg >= 80)) {
            LATB = 0;
            LATB = 0b00100000;
        } else if ((avg <= 75) && (avg >= 46)) {
            LATB = 0;
            LATB = 0b00010000;
        } else if ((avg <= 45) && (avg >= 31)) {
            LATB = 0;
            LATB = 0b00001000;
        } else if ((avg <= 20) && (avg >= 10)) {
            LATB = 0;
            LATB = 0b00000100;
        } else {
            LATB = 0;
        }
       
        
        
//        int avg = moving_average_10(read_channel_left());
//        if (avg < setpoint) avg = setpoint;
//        else if (avg > 250) avg = max_speed + setpoint;
//                
//        error = avg - setpoint;
//        pwm = error * kp;
//        
//        markspaceL = max_speed - pwm;
//        markspaceR = max_speed - pwm;
//        
//        if ((markspaceL > max_speed) || (markspaceR > max_speed)) {
//            markspaceL = max_speed;
//            markspaceR = max_speed;
//        }   else if ((markspaceL < min_speed) || (markspaceR < min_speed)) {
//            markspaceL = min_speed;
//            markspaceR = min_speed;
//        }
//        goforward();
//        delay(10);
    }
}


float moving_average_10(int new_input) {
    static int sum = 0;
    static int value_index = 0;
    
    sum -= sensor_input[value_index];
    sum += new_input;
    sensor_input[value_index] = new_input;
    value_index = (value_index + 1) % ARRAY_SIZE;
    float average = (float)sum / ARRAY_SIZE;
    return (((int)average + 5) / 10) * 10; // rounds of to the nearest 10
}

void initialiseIO(void) {
    ADCON1 = 0b00001101; 
    ADCON2 = 0b10000010; // Fosc/32, A/D result right justified
}

void delay(int ms) {
    int c;
    // Maybe could be rewritten to use the same internals as __delay_ms macro so
    // it looks nicer...
    for (c = 0; c < ms; c++)
        __delay_ms(1);
    return;
}

unsigned int read_channel_left(void) {
    ADCON0 = 0b00000011; //select A/D channel AN0,start conversion
    while (ADCON0bits.GO); //do nothing while conversion in progress
    return (unsigned int) ((ADRESH << 8) + ADRESL); //Combines high and low A/D bytes into one
} // value and returns this A/D value 0-1023

unsigned int read_channel_right(void) {
    ADCON0 = 0b00000111; //select A/D channel AN0,start conversion
    while (ADCON0bits.GO); //do nothing while conversion in progress
    return (unsigned int) ((ADRESH << 8) + ADRESL); //Combines high and low A/D bytes into one
}

void configPWM(void){   //Configures PWM
PR2 = 0b11111111 ;     //set period of PWM,610Hz
T2CON = 0b00000111 ;   //Timer 2(TMR2)on, prescaler = 16 
CCP1CON = 0b00001100;   //enable CCP1 PWM
CCP2CON = 0b00001100;   //enable CCP2 PWM
CCPR1L = 0;             //turn left motor off
CCPR2L = 0;             //turn Right motor off
return;
}

void goforward(void){
Leftmotor1A = 0;    //Left motor forward;
Leftmotor2A = 1;    
Rightmotor3A = 0;   //Right motor forward;
Rightmotor4A = 1;
CCP1CON = (0x0c)|((markspaceL&0x03)<<4);//0x0c enables PWM,then insert the 2 LSB
CCPR1L = markspaceL>>2; //of markspaceL into CCP1CON and the higher 8 bits into
CCP2CON = (0x0c)|((markspaceR&0x03)<<4); //CCPR1L.  Same as above but for 
CCPR2L = markspaceR>>2;                  // CCP2CON and CCPR2L
return;                 
}
