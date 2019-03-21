//BTN[0] -- Bit 0 -- Start
//BTN[1] -- Bit 1 -- Stop
//BTN[2] -- Bit 2 -- Lap
//BTN[3] -- Bit 3 -- Reset


#include "address_map_arm.h"

volatile int lookUpTable[17] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};


//Buttons
volatile int* BTN = (int*)KEY_BASE;

//Time address
volatile int* Timer = HPS_TIMER0_BASE;

volatile int centisecond_count = 0;
volatile int second_count = 0;
volatile int minute_count = 0;

volatile int lap_centisecond_count = 0;
volatile int lap_second_count = 0;
volatile int lap_minute_count = 0;

void DisplayHex(int minutes, int seconds, int centiseconds){

  volatile long *DISPLAY_ONES = HEX3_HEX0_BASE;
  volatile long *DISPLAY_TENS = HEX5_HEX4_BASE;

  volatile int centiseconds_ones = centiseconds % 10;
  volatile int centiseconds_tens = centiseconds / 10;

  volatile int seconds_ones = seconds % 10;
  volatile int seconds_tens = seconds / 10;

  volatile int minutes_ones = minutes % 10;
  volatile int minutes_tens = minutes / 10;



  *DISPLAY_ONES = lookUpTable[seconds_tens] * 0x1000000 + lookUpTable[seconds_ones] * 0x10000 + lookUpTable[centiseconds_tens] * 0x100 + lookUpTable[centiseconds_ones];
  *DISPLAY_TENS = lookUpTable[minutes_tens] * 0x100 + lookUpTable[minutes_ones];
}



int ReadSwitches(void){
  volatile int* SW_ptr = (int*)SW_BASE;
  return *SW_ptr&0xf;
}

void ReadButtons(void){
  // Start
  if ((((*BTN)&0x01) == 1)){
	DisplayHex(1,1, 1);
    Timer[2] = 0x03;
    while((((*BTN)&0x1) == 1));
  }

  // Stop
  if ((((*BTN)&0x02) == 2)){
	DisplayHex(2,2,2);
    Timer[2] = 0x00;
    while((((*BTN)&0x02) == 2));
  }
  

  // Lap
  if ((((*BTN)&0x04) == 4)){
	DisplayHex(3,3,3);
    //Set Lap counts to current time
    lap_centisecond_count = centisecond_count;
    lap_second_count = second_count;
    lap_minute_count = minute_count;
	
	while((((*BTN)&0x04) == 4));
  }

  // Reset
  if ((((*BTN)&0x08) == 8)){
	DisplayHex(4,4,4); 
	 
    //Reset time (current a lap) variables
    centisecond_count = 0;
    second_count = 0;
    minute_count = 0;

    lap_centisecond_count = 0;
    lap_second_count = 0;
    lap_minute_count = 0;

    while((((*BTN)&0x08) == 8));
  }
}


int main(void){
	


  // Load count value of 1,000,000
  // Timer is 100 MHz
  // Increment ( 1M * 1/100 MHZ) = 0.01 seconds
  Timer[0] = 0xF4240;

  // Set control register to zero
  Timer[2] = 0x00;


  while(1){
    ReadButtons();
	
    //Interrupt
    if(Timer[4] == 0x01){

      //Time Function
	  
      centisecond_count++;
      if(centisecond_count >= 100){
        centisecond_count = 0;
        second_count++;
        if(second_count >= 60){
          second_count = 0;
          minute_count++;
        }
      }

	  Timer[2] = 0x00;
      Timer[0] = 0xF4240;
      Timer[2] = 0x03;
    }
	
	if(ReadSwitches() == 0){
	  DisplayHex(minute_count, second_count, centisecond_count);
	}
		
	else{
		DisplayHex(lap_minute_count, lap_second_count, lap_centisecond_count);
	}
  }
}
