/*                                                                         /*
  Telephone Switch Tone Plant Generator
  by: Don Froula
  7/20/2019


  Tonegen library supports up to 4 simultaneous tones
  on a single output pin. The output uses a 40KHz sampling frequency and 62.5KHz PWM frequency.
  These frequencies must be filtered from the output with a low pass filter.
  The PWM library generates tones with a sine wave/square wave lookup table, varying the pulse width of the output according
  to values in the 8-bit table. The low pass filter integrates the varying pulse width to a varying
  analog voltage. Nominal voltage at silence ("127") is 2.5 volts. Using differential output modes
  increases this to 5 volts, doubling the tone output voltage and increasing the volume.

*/


#include "tonegen.h"  //Tone generation library

const unsigned char mLowtone =      B0000;    //Modulated Low Tone (600x120)
const unsigned char mRinging1 =     B0001;    //Modulated Ringing 1 (420x40)
const unsigned char mRinging2 =     B0010;    //Modulated Ringing 2 (500x40)
const unsigned char hz500 =         B0011;    //Old High Tone (500)
const unsigned char pDialtone =     B0100;    //Precise Dial Tone (350+440)
const unsigned char pRinging =      B0101;    //Precise Ringing (440+480)
const unsigned char pLowtone =      B0110;    //Precise Low Tone (480+620)
const unsigned char rohtones =      B0111;    //Receiver Off-Hook (ROH) tone (1400+2060+2450+2600)
const unsigned char hz480 =         B1000;    //New High Tone (480)
const unsigned char hz1004 =        B1001;    //Test Tone (1004)
const unsigned char ukOlddialtone = B1010;    //UK Old Dial Tone (400x33)
const unsigned char ukOldringing =  B1011;    //UK Old Ringing (400x133)
const unsigned char ukpDialtone =   B1100;    //UK Precise Dial Tone (350+450)
const unsigned char ukpRinging =    B1101;    //UK Ringing (400+450)
const unsigned char hz400 =         B1110;    //UK Equipment Engaged Tone/Congestion/Number Unavailable tone (400)
const unsigned char crybaby =       B1111;    //Crybaby (US Vacant Number) (200 to 400 to 200 at 1 Hz, interrupted every 6 seconds for 500ms)

const unsigned char  continuous =      B0000;  //Always ON (Continuous Tone)
const unsigned char  precisereorder =  B0001;  //250 ON 250 OFF (Precise Reorder)
const unsigned char  tollreorder =     B0010;  //200 ON 300 OFF (Toll Reorder)
const unsigned char  localreorder =    B0011;  //300 ON 200 OFF (Local Reorder)
const unsigned char  busy =            B0100;  //500 ON 500 OFF (Busy)
const unsigned char  roh =             B0101;  //100 ON 100 OFF (Receiver off hook)
const unsigned char  ring =            B0110;  //2000 ON 4000 OFF (Standard US Ring)
const unsigned char  partyline2 =      B0111;  //1000 ON 1000 OFF 1000 ON 3000 OFF (No. 5 Xbar Coded Ring 2)
const unsigned char  partyline3 =      B1000;  //1500 ON 500 OFF 500 ON 3500 OFF (No. 5 Xbar Coded Ring 3)
const unsigned char  partyline4 =      B1001;  //1500 ON 500 OFF 500 ON 500 OFF 500 ON 2500 OFF (No. 5 Xbar Coded Ring 4)
const unsigned char  partyline5 =      B1010;  //1500 ON 500 OFF 500 ON 500 OFF 1500 ON 1500 OFF (No. 5 Xbar Coded Ring 5)
const unsigned char  ukring =          B1011;  //400 ON 200 OFF 400 ON 2000 OFF (UK Ring)
const unsigned char  ukinvertedring =  B1100;  //400 OFF 200 ON 400 OFF 2000 ON (UK Inverted Ring)
const unsigned char  ukbusy =          B1101;  //375 ON 375 OFF (UK Busy)
const unsigned char  ukreorder =       B1110;  //400 ON 350 OFF 225 ON 525 OFF (UK Reorder)
const unsigned char  ukoldbusy =       B1111;  //750 ON 750 OFF (UK Old Busy)

unsigned char pwm_mode; //Storage for PWM libray output mode and ouput pin assignments

unsigned char selected_tone = mLowtone;
unsigned char selected_cadence = continuous;

//Instaniate PWM tone generator
tonegen tonePlayer;


void setup() {
  // Serial.begin(9600); // Initialize serial port for debugging

  // Configure the tone select pins for input - Upper nibble of Port D, Atmega328p pins 13,12,11,6 (from MSB to LSB)
  DDRD = DDRD &
         ~ (
           (1 << DDD4) |  // pinMode( 4, INPUT ); // Set to input - Pin 6
           (1 << DDD5) |  // pinMode( 5, INPUT ); // Set to input - Pin 11
           (1 << DDD6) |  // pinMode( 6, INPUT ); // Set to input - Pin 12
           (1 << DDD7)    // pinMode( 7, INPUT ); // Set to input - Pin 13
         );

  // Enable the pullups
  PORTD = PORTD |
          (
            (1 << PORTD4) |  // digitalWrite( 4, HIGH ); // Enable the pullup
            (1 << PORTD5) |  // digitalWrite( 5, HIGH ); // Enable the pullup
            (1 << PORTD6) |  // digitalWrite( 6, HIGH ); // Enable the pullup
            (1 << PORTD7)    // digitalWrite( 7, HIGH ); // Enable the pullup
          );

  // Configure the cadence select pins for input - Lower nibble of Port B, Atmega328p pins 17,16,15,14 (from MSB to LSB)
  DDRB = DDRB &
         ~ (
           (1 << DDB0) |  // pinMode( 8, INPUT ); // Set to input - Pin 14
           (1 << DDB1) |  // pinMode( 9, INPUT ); // Set to input - Pin 15
           (1 << DDB2) |  // pinMode( 10, INPUT ); // Set to input - Pin 16
           (1 << DDB3)    // pinMode( 11, INPUT ); // Set to input - Pin 17
         );

  // Enable the pullups
  PORTB = PORTB |
          (
            (1 << PORTB0) |  // digitalWrite( 8, HIGH ); // Enable the pullup
            (1 << PORTB1) |  // digitalWrite( 9, HIGH ); // Enable the pullup
            (1 << PORTB2) |  // digitalWrite( 10, HIGH ); // Enable the pullup
            (1 << PORTB3)    // digitalWrite( 11, HIGH ); // Enable the pullup
          );

  //Set audio output physical pin 5 as output
  DDRD |= (1 << DDD3);

  //Set relay driver physical pin 4 as output
  DDRD |= (1 << DDD2);

  //Set cadence LED pin as output
  DDRB |= (1 << DDB5);

  //Turn off relay output
  PORTD = PORTD & B11111011;

  //Turn off cadence LED
  PORTB = PORTB & B11011111;

//Uncomment ONE of the following three lines to set audio output mode and pin
  //pwm_mode = CHA;  //Single ended signal in CHA pin (11) - Physical Pin 17 on chip
  pwm_mode = CHB;  //Single ended signal on CHB pin (3) - Physical Pin 5 on chip
  //pwm_mode = DIFF; //Differntial signal on CHA and CHB pins (11,3) - Physical Pin 17 and Physical Pin 5 on chip

  tonePlayer.begin(pwm_mode); //Sets up output pins and single-ended or differential mode, according to the uncommented mode above.

  //Set default wave table for each voice
  tonePlayer.setWave(SINE, SINE, SINE, SQUARE);

  tonePlayer.setVolume(60, 60, 60, 60);

}


//Main loop
void loop()
{

  // Read all input pins and set tone and cadence variables
  
  //Read Port D, shift to the right four places, take the complement, and mask the upper 4 bits
  selected_tone = (~((PIND & ( (1 << PIND4) | (1 << PIND5) | (1 << PIND6) | (1 << PIND7) )) >> 4) & B00001111);

  //Read Port B, take the complement, and mask the upper 4 bits
  selected_cadence = (~(PINB & ( (1 << PINB0) | (1 << PINB1) | (1 << PINB2) | (1 << PINB3) )) & B00001111);
  
  playcadence();

}
//End of loop


void playcadence()
{
  switch (selected_cadence) {
    case continuous:
      playtone(0);
      break;
    case precisereorder:
      playtone(250);
      delay(250);
      break;
    case tollreorder:
      playtone(200);
      delay(300);
      break;
    case localreorder:
      playtone(300);
      delay(200);
      break;
    case busy:
      playtone(500);
      delay(500);
      break;
    case roh:
      playtone(100);
      delay(100);
      break;
    case ring:
      playtone(2000);
      delay(4000);
      break;
    case partyline2:
      playtone(1000);
      delay(1000);
      playtone(1000);
      delay(3000);
      break;
    case partyline3:
      playtone(1500);
      delay(500);
      playtone(500);
      delay(3500);
      break;
    case partyline4:
      playtone(1500);
      delay(500);
      playtone(500);
      delay(500);
      playtone(500);
      delay(2500);
      break;
    case partyline5:
      playtone(1500);
      delay(500);
      playtone(500);
      delay(500);
      playtone(1500);
      delay(1500);
      break;
    case ukring:
      playtone(400);
      delay(200);
      playtone(400);
      delay(2000);
      break;
    case ukinvertedring:
      playtone(200);
      delay(400);
      playtone(2000);
      delay(400);
      break;
    case ukbusy:
      playtone(375);
      delay(375);
      break;
    case ukreorder:
      playtone(400);
      delay(350);
      playtone(225);
      delay(525);
      break;
    case ukoldbusy:
      playtone(750);
      delay(750);
      break;
    default:
      playtone(0);
      break;
  }
}


// Total volume on a tone for all four voices MAY NOT EXCEED 250!!!!!!!!
void playtone(unsigned long length)
{
  switch (selected_tone) {
    case mLowtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(80, 80, 58, 22);
      tonePlayer.setVolume(80, 80, 80, 0);
      playPWM(480, 720, 600, 0, length);
      break;
    case mRinging1:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(80, 80, 80, 0);
      playPWM(380, 460, 420, 0, length);
      break;
    case mRinging2:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      //tonePlayer.setVolume(80, 80, 68, 12);
      tonePlayer.setVolume(80, 80, 80, 0);
      playPWM(460, 540, 500, 0, length);
      break;
    case hz500:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(240, 0, 0, 0);
      playPWM(500, 0, 0, 0, length);
      break;
    case pDialtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(350, 440, 0, 0, length);
      break;
    case pRinging:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(440, 480, 0, 0, length);
      break;
    case pLowtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(480, 620, 0, 0, length);
      break;
    case rohtones:
      tonePlayer.setWave(SINE, SINE, SINE, SINE);
      tonePlayer.setVolume(60, 60, 60, 60);
      playPWM(1400, 2060, 2450, 2600, length);
      break;  
    case hz480:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(240, 0, 0, 0);
      playPWM(480, 0, 0, 0, length);
      break;
    case hz1004:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(240, 0, 0, 0);
      playPWM(1004, 0, 0, 0, length);
      break;
    case ukOlddialtone:
      tonePlayer.setWave(SQUARE, SQUARE, SQUARE, SQUARE);
      tonePlayer.setVolume(20, 20, 20, 180);
      playPWM(434, 366, 400, 33, length);
      break;
    case ukOldringing:
      tonePlayer.setWave(SQUARE, SQUARE, SQUARE, SQUARE);
      tonePlayer.setVolume(20, 20, 20, 180);
      //playPWM(433,367,400,133,length);
      playPWM(434, 366, 400, 133, length);
      break;
    case ukpDialtone:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(350, 450, 0, 0, length);
      break;
    case ukpRinging:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(120, 120, 0, 0);
      playPWM(400, 450, 0, 0, length);
      break;
    case hz400:
      tonePlayer.setWave(SINE, SINE, SINE, SQUARE);
      tonePlayer.setVolume(240, 0, 0, 0);
      playPWM(400, 0, 0, 0, length);
      break;
    case crybaby:
    if (selected_cadence == continuous) {  //Only play crybaby if set to continuous cadence, otherwise silence
      tonePlayer.setWave(RAMP, SINE, SQUARE, SQUARE);
      tonePlayer.setVolume(250, 0, 0, 0);
      starttones();
      for (int j = 1; j <= 6; j++) {
        for (int i = 0; i < 100; i++) {
           //Read exponential table to mimic capacitor discharge of the real Crybaby circuit (frequency rises)
           changetones(pgm_read_float(discharge_sequence + i), 0, 0, 0);
           delay(5);
           }
         for (int i = 0; i < 100; i++) {
          //Read exponential table to mimic capacitor charge of the real Crybaby circuit (frequency falls)
           changetones(pgm_read_float(charge_sequence + i), 0, 0, 0);
           delay(5);
         }
        }
      stoptones();
      delay(500);
      }
      else {  //Play silence
        tonePlayer.setWave(SINE, SINE, SINE, SINE);
        tonePlayer.setVolume(0, 0, 0, 0);
        playPWM(0, 0, 0, 0, length);
        }
      break;
  }
}


//Play PWM-generated mixed tones
void playPWM(float freq1, float freq2, float freq3, float freq4, unsigned long length)
{
  tonePlayer.setFrequency(0, freq1); //Set the new Tone 1 frequency
  tonePlayer.setFrequency(1, freq2); //Set the new Tone 2 frequency
  tonePlayer.setFrequency(2, freq3); //Set the new Tone 3 frequency
  tonePlayer.setFrequency(3, freq4); //Set the new Tone 4 frequency
  tonePlayer.resume(); //Turn on PWM interrupts
  PORTD = PORTD | B00000100;  //Turn on relay output
  PORTB = PORTB | B00100000;  //Turn on cadence LED
  if (length > 0) {
    delay(length); //Wait for the tone duration
    tonePlayer.suspend(); //Turn off PWM interrupts
    PORTD = PORTD & B11111011;  //Turn off relay output
    PORTB = PORTB & B11011111;  //Turn off cadence LED
    tonePlayer.setFrequency(0, 0); //Set for silence
    tonePlayer.setFrequency(1, 0); //Set for silence
    tonePlayer.setFrequency(2, 0); //Set for silence
    tonePlayer.setFrequency(3, 0); //Set for silence
  }
}

//Change frequencies
void changetones(float freq1, float freq2, float freq3, float freq4)
{
  tonePlayer.setFrequency(0, freq1); //Set the new Tone 1 frequency
  tonePlayer.setFrequency(1, freq2); //Set the new Tone 2 frequency
  tonePlayer.setFrequency(2, freq3); //Set the new Tone 3 frequency
  tonePlayer.setFrequency(3, freq4); //Set the new Tone 4 frequency
}

void starttones()
{
  tonePlayer.setFrequency(0, 0); //Set for silence
  tonePlayer.setFrequency(1, 0); //Set for silence
  tonePlayer.setFrequency(2, 0); //Set for silence
  tonePlayer.setFrequency(3, 0); //Set for silence
  tonePlayer.resume(); //Turn on PWM interrupts
  PORTD = PORTD | B00000100;  //Turn on relay output
  PORTB = PORTB | B00100000;  //Turn on cadence LED
}

void stoptones()
{
  tonePlayer.suspend(); //Turn off PWM interrupts
  PORTD = PORTD & B11111011;  //Turn off relay output
  PORTB = PORTB & B11011111;  //Turn off cadence LED
  tonePlayer.setFrequency(0, 0); //Set for silence
  tonePlayer.setFrequency(1, 0); //Set for silence
  tonePlayer.setFrequency(2, 0); //Set for silence
  tonePlayer.setFrequency(3, 0); //Set for silence
}
