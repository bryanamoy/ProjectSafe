#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


typedef enum { RELEASED = 0, PRESSED } key_t;

#define LED_PIN0 (1 << PB0) //PB0 = 0000 1 << PB0 = 0001 (1)  Yellow
#define LED_PIN1 (1 << PB1) //PB1 = 0001 1<< PB1 = 0010 (2)   Green
#define LED_PIN2 (1 << PB2) //PB2 = 0010 1<<PB2 = 0100 (4)    Blue
#define LED_PIN3 (PB2)                                        //Red
//#define KEY_PIN0 (1 << PB3)
#define KEY0 (1 <<PB3)        //Button 1
#define KEY1 (1 << PB4)       //Button 2

#define SLOW 0x90

//--------------------EQUAL--------------------
/*
Test whether 6 bit passwords are equal
returns true if equal, false otherwise
*/
bool equal(char *code1, char *code2){
  for(int i = 0; i < 6; i++){
    if(code1[0] == code2[0] && code1[1] == code2[1] && code1[2] == code2[2] && code1[3] == code2[3] && code1[4] == code2[4] && code1[5] == code2[5]){
      return true;
    }

  }
  return false;
}

//--------------YELLOW BLINK--------------------
/*
Blinks Yellow if passwords are wrong, passes original password in parameter
*/
void blink(char *code){
uint8_t slow;
int time = 0;
DDRB = LED_PIN1|LED_PIN0;     //Lights Yellow LED
PORTB = LED_PIN1;

TCCR0B = 0b101;
slow = TCNT0 + SLOW;


while(time < 10){
  if (slow == TCNT0) {
    PORTB ^= LED_PIN1;
    slow += SLOW;
    time = time + 1;
  }
#ifdef BUSY
  switch (rand() % 4) {
  case 0:
    _delay_ms(10);
  case 1:
    _delay_ms(10);
  case 2:
    _delay_ms(10);
  case 3:
    _delay_ms(10);
  }
#endif
}
locked(code);
}

//----------------------------------------------

//---------------UNLOCKED MODE-------------------
/*
This method deals with unlocked/programming mode
*/
volatile int counter = 0; //counter for how many times buttons are pressed
void unlocked(){

volatile uint8_t history = 0;
volatile uint8_t history2 = 0;
key_t keystate = RELEASED;
key_t keystate2 = RELEASED;
char passcode[6];           //char array to store 6 bit binary password
key_t state;                //variables used to check the state changes
key_t statechange;          //from high to low
key_t state2;
key_t statechange2;
counter = 0;              //resets counter to 0 if method is called again
char bit = '0';           //char to set to label button presses, default 0

while (1) {

  DDRB = LED_PIN1|LED_PIN0;     //Set green and yellow led on while in unlocked/programming mode
  PORTB ^= LED_PIN1|LED_PIN0;
  //PORTB = LED_PIN1;

  //-----button---------------
  history = history << 1;
  history2 = history2 << 1;
  // state = keystate;
  // state2 = keystate2;
  if ((PINB & KEY0) == 0){ // low if button 1 is pressed!
    history = history | 0x1;
    bit = '0';            //sets bit to 0 if button 1 is pressed
  }
  if((PINB & KEY1) == 0){ //low uf button 2 is pressed
    history2 = history2 | 0x1;
    bit = '1';            //sets bit to 1 if button 2 is pressed
  }

  // Update the key state based on the latest 6 samples
  if ((history & 0b111111) == 0b111111){
    keystate = PRESSED;

  }
  if ((history2 & 0b111111) == 0b111111){
    keystate2 = PRESSED;

  }
  state = keystate;         //takes state when pressed
  state2 = keystate2;

  if ((history & 0b00111111) == 0){     //if button released green and yellow led on
    keystate = RELEASED;
    DDRB = LED_PIN1|LED_PIN0;
    // PB1 output is high, current flows from that pin, via yellow to PB0
    PORTB = LED_PIN1;
  }
  if ((history2 & 0b00111111) == 0){    //if button is released green and yellow on
    keystate2 = RELEASED;
    DDRB = LED_PIN1|LED_PIN0;
    // PB1 output is high, current flows from that pin, via yellow to PB0
    PORTB = LED_PIN1;
  }
  statechange = keystate;           //takes record of next states
  statechange2 = keystate2;

  if(state != statechange){         //compares to take instance of state change for button 1, if different means button 1 was pressed
    passcode[counter] = bit;      //when state changes add bit to password array
    counter = counter + 1;        //increment number of button presses

  }
  if(state2 != statechange2){     //compares to take instance of state change for button 2, if different button 2 pressed
    passcode[counter] = bit;      //when state state changes add bit to password array
    counter = counter + 1;        //increment number of button presses

  }

  // Turn on the LED based on the state
  if (keystate == PRESSED){             //if button 1 pressed turns blue led
    DDRB = LED_PIN0|LED_PIN1|LED_PIN2;
    PORTB = LED_PIN1;

  }
  if (keystate2 == PRESSED){          //if button 2 is pressed turns blue led
    DDRB = LED_PIN0|LED_PIN1|LED_PIN2;
    PORTB = LED_PIN1;
  }

  if(counter == 6){                 //when 6 bits are inputted, goes to locked state
    locked(passcode);               //passes set password as parameter
    return;
  }


  else{                             //otherwise yellow and gree

     DDRB = LED_PIN1|LED_PIN0;
     PORTB = LED_PIN1;

   }

}
}

//----------------------------------------------

//----------------LOCKED MODE----------------------
    //once button pressed 6 times go to this method
    //turn on red
    //add blue led button pressed again
    //if string is equal return to green and yellow
    volatile int countRed = 0;          //counter for locked state
  void locked(char *pass){
    volatile uint8_t historyRed = 0;
    volatile uint8_t historyRed2 = 0;

    countRed = 0;                   //resets if locked state called again

    key_t keystateRed = RELEASED;       //keystate for button 1
    key_t keystateRed2 = RELEASED;      //keystate for button 2

    key_t stateRed;                   //variables to keep track of current and next states
    key_t statechangeRed;

    key_t stateRed2;
    key_t statechangeRed2;

    char passcodeRed[6];              //char array to store locked state passcode
    char bitRed = '0';                //default bit code = 0

    while(1){
      DDRB = LED_PIN3|LED_PIN2;       //keeps red led on
      PORTB = LED_PIN2;

      historyRed = historyRed << 1;
      historyRed2 = historyRed2 << 1;

      if ((PINB & KEY0) == 0){                    //button 1
        historyRed = historyRed | 0x1;
        bitRed = '0';                           //button 1 labeled 0
      }
      if ((PINB & KEY1) == 0){                  //button 2
        historyRed2 = historyRed2 | 0x1;
        bitRed = '1';                         //button 2 labeled 1
      }
      if ((historyRed & 0b111111) == 0b111111) {
        keystateRed = PRESSED;

      }
      if ((historyRed2 & 0b111111) == 0b111111) {
        keystateRed2 = PRESSED;

      }
      stateRed = keystateRed;                 //takes current state starting at pressed
      stateRed2 = keystateRed2;

      if ((historyRed & 0b00111111) == 0) {
        keystateRed = RELEASED;
        DDRB = LED_PIN3|LED_PIN2;              //released red led button 1
        PORTB = LED_PIN2;
      }
      if ((historyRed2 & 0b00111111) == 0) {
        keystateRed2 = RELEASED;
        DDRB = LED_PIN3|LED_PIN2;              //released red led button 2
        PORTB = LED_PIN2;
      }
      statechangeRed = keystateRed;            //records next state
      statechangeRed2 = keystateRed2;
      if (stateRed != statechangeRed) {         //compares current and next state, if different button 1 pressed
        passcodeRed[countRed] = bitRed;          //adds bit to locked password
        countRed = countRed + 1;              //increment counter

      }
      else if (stateRed2 != statechangeRed2) {        //compares current and next state for button 2, if different button 2 pressed
        passcodeRed[countRed] = bitRed;             //adds bit to locked password
        countRed = countRed + 1;

      }

      if(keystateRed == PRESSED){           //flashed blue
        DDRB = LED_PIN1|LED_PIN2;
        PORTB = LED_PIN1;
      }
      if(keystateRed2 == PRESSED){          //flashes blue
        DDRB = LED_PIN1|LED_PIN2;
        PORTB = LED_PIN1;
      }

      if(countRed == 6){              //if 6, locked password was inputted
        if(equal(pass, passcodeRed) == false){    //compares is passwords match, if false blinks yellow
          blink(pass);
          return;
        }

        if(equal(pass, passcodeRed) == true){         //compares passwords, if true goes back to unlocked/programming state
          unlocked();
        }

      }
      else{
        DDRB = LED_PIN3|LED_PIN2;
        PORTB = LED_PIN2;

      }
    }
  }

//-------------------------------------------------
int main(void) {
  unlocked();

 }
