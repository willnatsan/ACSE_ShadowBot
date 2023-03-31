/*
 * File:   lineFollowing.c
 * Author: asusv
 *
 * Created on March 31, 2023, 7:58 AM
 */


#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#pragma config OSC = HS     //High speed resonator
#pragma config WDT = OFF    //watchdog timer off
#pragma config LVP = OFF    //Low voltage programmer disabled
#pragma config PWRT = ON    //Power up timer 
#define _XTAL_FREQ 10000000 // define clock frequency for __delay_10ms()
#define Leftmotor1A LATAbits.LA4    //Direction bits Left motor
#define Leftmotor2A LATAbits.LA5
#define Rightmotor3A LATBbits.LB0   //Direction bits Right motor
#define Rightmotor4A LATBbits.LB1
void I2C_Initialise(void);      	//Initialise I2C
void I2C_checkbus_free(void);   //Wait until I2C bus is free
void I2C_Start(void);           	//Generate I2C start condition
void I2C_RepeatedStart(void);  	 //Generate I2C Repeat start condition
void I2C_Stop(void);           	 //Generate I2C stop condition
void I2C_Write(unsigned char write);    //Generate I2C write condition
unsigned char I2C_Read(void);   	//Generate I2C read condition
void configPWM(void);       //Configure PWM
void move(void);            //Move
void brake(void);
void wait10ms(int del); 	//generates a delay in multiples of 10ms
int markspaceL;    //Mark space ratio for Left motor
int markspaceR;    //Mark space ratio for Right motor

//Main Body
int main(void)
{
   int theta,error=0,u,K=10,reference=0,lambda=1;
   unsigned char linesensor;    	 //Store raw data from sensor array
   int sensorArray[16][2]=         //Declaring Sensor Array
    {
        {0b11111110,-12},
        {0b11111100,-10},
        {0b11111101,-9},
        {0b11111001,-7},
        {0b11111011,-5},
        {0b11110011,-3},
        {0b11110111,-2},
        {0b11100111,0},
        {0b11101111,2},
        {0b11001111,3},
        {0b11011111,5},
        {0b10011111,7},
        {0b10111111,9},
        {0b00111111,10},
        {0b01111111,12},
        {0b00000000,0},

    };
   ADCON1 = 0b00001101;    //AN0,AN1 are analogue inputs,RA2 -RA5 are digital 
   TRISA = 0b11001111;    //Set PORTA pins
   TRISB = 0;              	 //Set all PORTB pins to outputs
   TRISC = 0b00111001;    //Set PORTC pins
   LATB = 0;              	 //Turn LEDs off
   configPWM();          	  //Configure 
   I2C_Initialise();             	//Initialise I2C Master 
   while(1){


            I2C_Start();                	//Send Start condition to slave
            I2C_Write(0x7C);            	//Send 7 bit address + Write to slave
            I2C_Write(0x11);            	//Write data, select RegdataA and send to slave
            I2C_RepeatedStart();        	//Send repeat start condition
            I2C_Write(0x7D);            	//Send 7 bit address + Read
            linesensor=I2C_Read();      	//Read  the IR sensors 
            LATB=linesensor;            	//Output to LEDs
            for (int i = 0;i<17;i++)
            {

                if(linesensor==sensorArray[i][0])
                {
                    theta=sensorArray[i][1];
                    error=reference-theta;
                    u=error*K;
                    markspaceL=250-lambda*u;
                    markspaceR=300+lambda*u;
                    move();             

                }                      

            }

            //printf("%c",linesensor);
            I2C_Stop();                 	//Send Stop condition


    
}
}


//Functions 
void configPWM(void){   //Configures PWM
PR2 = 0b11111111 ;     //set period of PWM,610Hz
T2CON = 0b00000111 ;   //Timer 2(TMR2)on, prescaler = 16 
CCP1CON = 0b00001100;   //enable CCP1 PWM
CCP2CON = 0b00001100;   //enable CCP2 PWM
CCPR1L = 0;             //turn left motor off
CCPR2L = 0;             //turn Right motor off
return;
}

void move(void){    //Moves the little shit you dumb shit
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

void brake(void){    //Moves the little shit you dumb shit
Leftmotor1A = 0;    //Left motor forward;
Leftmotor2A = 0;    
Rightmotor3A = 0;   //Right motor forward;
Rightmotor4A = 0;
CCP1CON = (0x0c)|((markspaceL&0x03)<<4);//0x0c enables PWM,then insert the 2 LSB
CCPR1L = markspaceL>>2; //of markspaceL into CCP1CON and the higher 8 bits into
CCP2CON = (0x0c)|((markspaceR&0x03)<<4); //CCPR1L.  Same as above but for 
CCPR2L = markspaceR>>2;                  // CCP2CON and CCPR2L
return;                 
}

void wait10ms(int del)		 //delay function in multiples of 10ms
{     	
	int c;
	for(c=0;c<del;c++)
    	__delay_ms(10);
	return;
}

void I2C_Initialise(void)  	//Initialise I2C
{
  SSPCON1 = 0b00101000; 	//set to master mode, enable SDA and SCL pins
  SSPCON2 = 0;         		 //reset control register 2
  SSPADD = 0x63;       		 //set baud rate to 100KHz
  SSPSTAT = 0;         		 //reset status register
  }
void I2C_checkbus_free(void)   	 //Wait until I2C bus is free
{
  while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));	//wait until I2C bus is free
}

void I2C_Start(void)        //Generate I2C start condition
{
  I2C_checkbus_free(); 	 //Test to see I2C bus is free
  SEN = 1;              	//Generate start condition,SSPCON2 bit 0 = 1
}

void I2C_RepeatedStart(void) 	//Generate I2C Repeat start condition
{
  I2C_checkbus_free();  		//Test to see I2C bus is free
  RSEN = 1;             		//Generate repeat start, SSPCON2 bit1 = 1
}

void I2C_Stop(void) 		//Generate I2C stop condition
{
  I2C_checkbus_free();  		//Test to see I2C bus is free
  PEN = 1;              		// Generate stop condition,SSPCON2 bit2 = 1
}

void I2C_Write(unsigned char write) 	//Write to slave
{
  I2C_checkbus_free();  		//check I2C bus is free
  SSPBUF = write;       		//Send data to transmit buffer
}

unsigned char I2C_Read(void)    //Read from slave
{
  unsigned char temp;
  I2C_checkbus_free(); 	 //Test to see I2C bus is free
  RCEN = 1;            	 //enable receiver,SSPCON2 bit3 = 1
  I2C_checkbus_free();  	//Test to see I2C bus is free
  temp = SSPBUF;        	//Read slave
  I2C_checkbus_free(); 	 //Test to see I2C bus is free
  ACKEN = 1;           	 //Acknowledge
  return temp;         	 //return sensor array data
}
