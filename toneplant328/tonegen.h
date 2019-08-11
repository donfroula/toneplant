#ifndef _TONEGEN
#define _TONEGEN
//*************************************************************************************
//  Arduino Tone Generator
// Generates four simultaneous square/sine wave tones at any relative amplitude
//*************************************************************************************
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "tables.h"

#define DIFF 1
#define CHA 2
#define CHB 3

#define SINE      0
#define SQUARE    1
#define RAMP      2

#define FS 40000.0                            //-Rate must be evenly divisable into 16,000,000. Higher rates have worse frequency error, but better resolution. Not amy issue with 32-bit phase accumulators.
                                              //-Must be at least twice the highest frequency. Frequency resolution is FS/(2^32). (2^32) = the size of the phase accumulator.

#define SET(x,y) (x |=(1<<y))		        	  	//-Bit set/clear macros
#define CLR(x,y) (x &= (~(1<<y)))       			// |
#define CHK(x,y) (x & (1<<y))           			// |
#define TOG(x,y) (x^=(1<<y))            			//-+

volatile unsigned long PCW[4] = {
  0, 0, 0, 0};			            //-Wave phase accumulators - Index into wave table (upper 8 bits of 32)
volatile unsigned long FTW[4] = {
  0, 0, 0, 0};                  //-Wave frequency tuning words - Actual frequency being produced
volatile unsigned char AMP[4] = {
  0, 0, 0, 0};                  //-Wave amplitudes [0-255]
volatile unsigned int wavs[4];  //-Wave table selector [address of wave table in memory]
volatile unsigned char output_mode;

//*********************************************************************************************
//  Audio driver interrupt
//*********************************************************************************************

SIGNAL(TIMER1_COMPA_vect)
{
  //---------------------------------------------------------
  //  Audio mixer - Total must not be > 250 to avoid clipping
  //---------------------------------------------------------
  //pgm_read_byte reads a byte from the wave table in program memory. The function requires the memory address of the byte to be retrieved.
  //Here, the start address of the wave table array, wavs[X], is added to the most-significant 8 bits of the current value of the (32-bit phase accumulator + 32-bit tuning word).
  //This is the address of the retrieved wave table value.
  //The wave table signed 256-bit value is then multiplied by a 256-bit unsigned volume adjustment constant and divided by 256 to render a level-adjusted wave table sample.
  
   OCR2A = OCR2B = 127 +  //Start at 127 or 2.5 volts as zero level after capacitor coupling the output. Output voltage swings 5 volts p-p.
    (
    (((signed char)pgm_read_byte(wavs[0] + ((PCW[0]+=FTW[0]) >> 24)) * AMP[0]) >> 8) +
    (((signed char)pgm_read_byte(wavs[1] + ((PCW[1]+=FTW[1]) >> 24)) * AMP[1]) >> 8) +
    (((signed char)pgm_read_byte(wavs[2] + ((PCW[2]+=FTW[2]) >> 24)) * AMP[2]) >> 8) +
    (((signed char)pgm_read_byte(wavs[3] + ((PCW[3]+=FTW[3]) >> 24)) * AMP[3]) >> 8)
    );

}

class tonegen
{
private:

public:

  tonegen()
  {
  }

  //*********************************************************************
  //  Startup default 
  //*********************************************************************

  void begin()
  {
    output_mode=CHA;
    TCCR1A = 0x00;                                  //-Start audio interrupt
    TCCR1B = 0x09;
    TCCR1C = 0x00;
    OCR1A=16000000.0 / FS;			                    //-Auto sample rate
    SET(TIMSK1, OCIE1A);                            //-Start audio interrupt
    sei();                                          //-+

    TCCR2A = 0x83;                                  //-8 bit audio PWM
    TCCR2B = 0x01;                                  // |
    OCR2A = 127;                                    //-+
    SET(DDRB, 3);            //-PWM pin             //Enable PWM pins - CHA Complimentary Pin (5) - differential output on both normal and complimentary pins.
  }

  //*********************************************************************
  //  Startup, selecting various output modes
  //*********************************************************************

  void begin(unsigned char d)
  {
  //Timer 1 is used for PWM audio clock
    TCCR1A = 0x00;                                  //-Start audio interrupt
    TCCR1B = 0x09;
    TCCR1C = 0x00;
    OCR1A=16000000.0 / FS;			                    //-Auto sample rate
    SET(TIMSK1, OCIE1A);                            //-Start audio interrupt
    sei();                                          //-+

    output_mode=d;

    switch(d)
    {

      
    case DIFF:                                        //-Differntial signal on CHA and CHB pins (11,3)
      TCCR2A = 0xB3;                                  //-8 bit audio PWM
      TCCR2B = 0x01;                                  // |
      OCR2A = OCR2B = 127;                            //-+
      SET(DDRB, 3);                                   //-PWM pin
      SET(DDRD, 3);                                   //-PWM pin
      break;

    case CHB:                                         //-Single ended signal on CHB pin (3)
      TCCR2A = 0x23;                                  //-8 bit audio PWM
      TCCR2B = 0x01;                                  // |
      OCR2A = OCR2B = 127;                            //-+
      SET(DDRD, 3);                                   //-PWM pin
      break;

    case CHA:
    default:
      output_mode=CHA;                                //-Single ended signal in CHA pin (11)
      TCCR2A = 0x83;                                  //-8 bit audio PWM
      TCCR2B = 0x01;                                  // |
      OCR2A = OCR2B = 127;                            //-+
      SET(DDRB, 3);                                   //-PWM pin
      break;
    }
  }



  //*********************************************************************
  //  Setup waves
  //*********************************************************************

  void setWave(unsigned char wave1, unsigned char wave2, unsigned char wave3, unsigned char wave4)
  {

    if(wave1 == SQUARE) {wavs[0] = SquareTable;} 
      else if (wave1 == SINE){wavs[0] = SinTable;} 
      else if (wave1 == RAMP){wavs[0] = RampTable;}
      else {wavs[0] = SinTable;}
    if(wave2 == SQUARE) {wavs[1] = SquareTable;} 
      else if (wave2 == SINE){wavs[1] = SinTable;} 
      else if (wave2 == RAMP){wavs[1] = RampTable;}
      else {wavs[1] = SinTable;}
    if(wave3 == SQUARE) {wavs[2] = SquareTable;} 
      else if (wave3 == SINE){wavs[2] = SinTable;} 
      else if (wave3 == RAMP){wavs[2] = RampTable;}
      else {wavs[2] = SinTable;}   
    if(wave4 == SQUARE) {wavs[3] = SquareTable;} 
      else if (wave4 == SINE){wavs[3] = SinTable;} 
      else if (wave4 == RAMP){wavs[3] = RampTable;}
      else {wavs[3] = SinTable;}

  }


  //*********************************************************************
  //  Set frequency direct
  //*********************************************************************

  void setFrequency(unsigned char voice,float f)
  {
    //32-bit phase accumulator provides 0.000009313 Hz. resolution at 40KHz sampling rate (FS/(2^32))
    //FTW[voice]=f/(FS/4294967295.0); //frequency/(sample_rate/max_32bit_count)
    //f * 1/(40,000/4294967295.0) = f * 107,374.182375
    FTW[voice]=f * 107374.182375;
  }

  
  //*********************************************************************
  //  Set volume
  //*********************************************************************

  void setVolume(unsigned char vol0, unsigned char vol1, unsigned char vol2,unsigned char vol3)
  {
    AMP[0] = vol0;
    AMP[1] = vol1;
    AMP[2] = vol2;
    AMP[3] = vol3;
  }
  //*********************************************************************
  //  Suspend/resume tonegen
  //*********************************************************************

  void suspend()
  {
    CLR(TIMSK1, OCIE1A);                            //-Stop audio interrupt 
    while(OCR2A != 127){                            //-Ramp down voltage to zero for click reduction
      if(OCR2A > 127){OCR2A = OCR2B -= 1;}
      if(OCR2A < 127){OCR2A = OCR2B += 1;}
      delayMicroseconds(8);                         //Sets ramp down speed, 1.008 mS worst case (126*0.008)
    }
    //Resetting phase accumulators to zero assures voltage always starts at zero for consistent "on" click.
    PCW[0] = PCW[1] = PCW[2] = PCW[3] = 0;          //-Reset phase accumulators to avoid weird on/off click phasing patterns
  }
  
  void resume()
  {
    SET(TIMSK1, OCIE1A);                            //-Start audio interrupt
  }

};

#endif
