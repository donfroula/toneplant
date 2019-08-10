# toneplant
This project allows an Atmega328p-based Arduino to be used as a "tone plant" for vintage telephone switches.

Numerous modern and vintage US and UK tones are provided. 

Tone selection is performed using a four position DIP switch. 16 unique tones are supported.

Cadence selection is performed using a 4 position DIP switch. 16 unique cadences are supported.

Any cadence may be applied to any town.

Tones are generated using four voice wavetable synthesis. The waveform, frequency and amplitude of each voice is performed internally
and mixed to generate the final output tone.

The output of the tone is in PWM (pulse width modulation) format. It is passed through two stage low-pass filter to reconstitute the original waveform. It is then fed to an LM386 audio amplifier through a level control. Output is designed for 8 ohms. The on-board amplification is adequate for most purposes.

The board also provides an open-collector cadence output that is synchronized to the tone cadence. It may be used to drive an external interrupter relay or circuit with suitable additional circuitry.

The circuit is powered from 12vdc MAX.

Tone Settings - S2  
0000 Modulated Low Tone (600x120)  
0001 Modulated Ringing 1 (420x40)  
0010 Modulated Ringing 2 (500x40)  
0011 Old High Tone (500)  
0100 Precise Dial Tone (350+440)  
0101 Precise Ringing (440+480)  
0110 Precise Low Tone (480+620)  
0111 Receiver Off-Hook (ROH) tone (1400+2060+2450+2600)  
1000 New High Tone (480)  
1001 Test Tone (1004)  
1010 UK Old Dial Tone (400x33)  
1011 UK Old Ringing (400x133)  
1100 UK Precise Dial Tone (350+450)  
1101 UK PreciseRinging (400+450)  
1110 UK Equipment Engaged Tone/Congestion/Number Unavailable tone (400)  
1111 Crybaby (US Vacant Number) (200 to 400 to 200 at 1 Hz,   
     interrupted every 6 seconds for 500ms)  
     Note: For "Crybaby" tones, cadence must be set to "Continuous"  

Cadence Settings - S1  
0000 Always ON (Continuous Tone)  
0001 250 ON 250 OFF (Precise Reorder)  
0010 200 ON 300 OFF (Toll Reorder)  
0011 300 ON 200 OFF (Local Reorder)  
0100 500 ON 500 OFF (Busy)  
0101 100 ON 100 OFF (Receiver off hook)  
0110 2000 ON 4000 OFF (Standard US Ring)  
0111 1000 ON 1000 OFF 1000 ON 3000 OFF (No. 5 Xbar Coded Ring 2)  
1000 1500 ON 500 OFF 500 ON 3500 OFF (No. 5 Xbar Coded Ring 3)  
1001 1500 ON 500 OFF 500 ON 500 OFF 500 ON 2500 OFF (No. 5 Xbar Coded Ring 4)  
1010 1500 ON 500 OFF 500 ON 500 OFF 1500 ON 1500 OFF (No. 5 Xbar Coded Ring 5)  
1011 400 ON 200 OFF 400 ON 2000 OFF (UK Ring)  
1100 400 OFF 200 ON 400 OFF 2000 ON (UK Inverted Ring)  
1101 375 ON 375 OFF (UK Busy)  
1110 400 ON 350 OFF 225 ON 525 OFF (UK Reorder)  
1111 750 ON 750 OFF (UK Old Busy)  
