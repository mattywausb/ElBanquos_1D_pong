/* Functions to handle all input elements */

#include "MainSettings.h"

// Activate general trace output

#ifdef TRACE_ON
#define TRACE_INPUT 
//#define TRACE_INPUT_HIGH 
#endif




/* Port constants --> check the IDS Function */

#define PORT_MAIN_SWITCH 8

const byte switch_pin_list[] = {8,    // Button A
                                6,    // Button B
                               };

const byte button_for_timer[] = {1,3,5,7}; // Mapping of timers to buttons on the board


  
                                

const unsigned int debounce_mask[] = { /* every bit is 2 ms */
  0x0007,    // Button A
  0x0007,    // Button B
};
#define INPUT_PORT_COUNT sizeof(switch_pin_list)


volatile bool setupComplete = false;

volatile unsigned int raw_state_register[INPUT_PORT_COUNT];
volatile unsigned int raw_state;
volatile unsigned int debounced_state = 0; /* Debounced state with history to last cycle managed by the ISR */
volatile unsigned long input_last_change_time = 0;

unsigned int tick_state = 0;              /* State provided in the actual tick, with change indication to last tick */

#ifdef TRACE_INPUT_traceValue_acceleration
volatile byte traceValue_acceleration=0;
volatile int traceValue_turn_interval=0;
#endif


#define INPUT_BITIDX_BUTTON_A 0
#define INPUT_BITIDX_BUTTON_B 2
/*                                         76543210 */


#define INPUT_BUTTON_A_BITS              0x0003
#define INPUT_BUTTON_A_IS_PRESSED_PATTERN   0x0003
#define INPUT_BUTTON_A_GOT_PRESSED_PATTERN   0x0001
#define INPUT_BUTTON_A_GOT_RELEASED_PATTERN  0x0002
#define INPUT_BUTTON_B_BITS              0x0030
#define INPUT_BUTTON_B_IS_PRESSED_PATTERN   0x0003
#define INPUT_BUTTON_B_GOT_PRESSED_PATTERN   0x0001
#define INPUT_BUTTON_B_GOT_RELEASED_PATTERN  0x0002


/* Masks for debounce handling */

#define INPUT_CURRENT_BITS 0x5555
#define INPUT_PREVIOUS_BITS 0xaaaa

/* Timing tracker */
unsigned long last_press_start_time=0;
unsigned long last_press_end_time=0;
bool input_enabled=true;






/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

/* ------------- General Information --------------- */

int input_getMillisSinceLastEvent() {
  return (millis() - input_last_change_time);
}

long input_getCurrentPressDuration()
{
  #ifdef TRACE_INPUT
    Serial.print(F("input CurrentPressDuration:"));
    Serial.println(millis()-last_press_start_time);
  #endif
    
  return millis()-last_press_start_time;
}

long input_getLastPressDuration()
{
  return  last_press_end_time-last_press_start_time;
}

/* ------------- Button events --------------- */

bool input_button_A_gotPressed()
{

  return input_enabled && ((tick_state & INPUT_BUTTON_A_BITS) == INPUT_BUTTON_A_GOT_PRESSED_PATTERN);
}

bool input_button_A_isPressed()
{
  return input_enabled && ((tick_state & INPUT_BUTTON_A_BITS) == INPUT_BUTTON_A_IS_PRESSED_PATTERN); 
}

byte input_button_A_gotReleased()
{
  return input_enabled && ((tick_state & INPUT_BUTTON_A_BITS) == INPUT_BUTTON_A_GOT_RELEASED_PATTERN); 
}


bool input_button_B_gotPressed()
{

  return input_enabled && ((tick_state & INPUT_BUTTON_B_BITS) == INPUT_BUTTON_B_GOT_PRESSED_PATTERN);
}

bool input_button_B_isPressed()
{
  return input_enabled && ((tick_state & INPUT_BUTTON_B_BITS) == INPUT_BUTTON_B_IS_PRESSED_PATTERN); 
}

byte input_button_B_gotReleased()
{
  return input_enabled && ((tick_state & INPUT_BUTTON_B_BITS) == INPUT_BUTTON_B_GOT_RELEASED_PATTERN); 
}


/* ------------- Operations ----------------- */


/* Disable input until all buttons have been released */

void input_pauseUntilRelease()
{
  input_enabled=false;
}

/* *************************** internal implementation *************************** */

 
/* **** Timer 1 interrupt function to track the state changes direct attached buttons and switches  
*/

ISR(TIMER1_COMPA_vect)
{
  static unsigned long last_turn_event=0;
  unsigned long turn_event_interval=0;
  TCNT1 = 0;             // reset the counter register

  if (!setupComplete) return; 

  /* copy debounce of state of last cycle  to history bits */
  debounced_state = (debounced_state & INPUT_CURRENT_BITS) << 1
                    | (debounced_state & INPUT_CURRENT_BITS);

  /* Get state of all switches */
  for (byte i = 0; i < INPUT_PORT_COUNT; i++) { // for all input ports configured

    // Push  actual reading into the raw state registers
    raw_state_register[i] <<= 1;
    raw_state_register[i] |= !digitalRead(switch_pin_list[i]);
    bitWrite(raw_state, i * 2, raw_state_register[i] & 0x0001); /* and the current status bits */

    // if raw state is stable  copy it to debounced state
    if ((raw_state_register[i] & 0x001f) == 0x0000) bitClear(debounced_state, i << 1);
    else if ((raw_state_register[i]&debounce_mask[i]) == debounce_mask[i]) bitSet(debounced_state, i << 1);
  }

  /* Reset last change timer if anything has changed */
  if ((debounced_state & INPUT_CURRENT_BITS) != (debounced_state & INPUT_PREVIOUS_BITS) >> 1) {
  
    input_last_change_time = millis();
  }

}


/* ************************************* TICK ***********************************************
   translate the state of buttons into the ticks of the master loop
   Must be called by the master loop for every cycle to provide valid event states of
   all input devices.
   Also transfers state changes, tracked with the timer interrupt into a tick state
*/

void input_foreward_tick() {


  /* copy processed tick  state to history bits  and take debounced as new value */
  tick_state = (tick_state & INPUT_CURRENT_BITS) << 1
               | (debounced_state & INPUT_CURRENT_BITS);

  /* Track pressing time */
  /* 01 = Press  */
  if((debounced_state & INPUT_CURRENT_BITS) ^( (debounced_state & INPUT_PREVIOUS_BITS) >> 1)&(debounced_state & INPUT_CURRENT_BITS)) last_press_end_time =last_press_start_time=input_last_change_time;
  /* 10 = release */ 
  if(((debounced_state & INPUT_CURRENT_BITS)) ^( (debounced_state & INPUT_PREVIOUS_BITS) >> 1)&(~debounced_state & INPUT_CURRENT_BITS)) last_press_end_time=input_last_change_time;


  /* enable input when all is release */
  if(debounced_state==0) input_enabled=true;
  
} // void input_switches_tick()




/* ***************************       S E T U P           ******************************
*/

void input_setup() {


  /* Initialize switch pins and raw_state_register array */
  for (byte switchIndex = 0; switchIndex < INPUT_PORT_COUNT ; switchIndex++) {
    pinMode(switch_pin_list[switchIndex], INPUT_PULLUP);
    raw_state_register[switchIndex] = 0;
  }

  // Establish timer1 interrupt for regular input scanning

  noInterrupts();           // Alle Interrupts temporär abschalten
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;                // Register mit 0 initialisieren
//  OCR1A = 62  ;             // Interrupt call for every 1 ms https://timer-interrupt-calculator.simsso.de/
//  OCR1A = 124  ;             // Interrupt call for every 2 ms https://timer-interrupt-calculator.simsso.de/
  OCR1A = 186  ;             // Interrupt call for every 3 ms https://timer-interrupt-calculator.simsso.de/
  TCCR1B |= (1 << CS12);    // 256 als Prescale-Wert spezifizieren
  TIMSK1 |= (1 << OCIE1A);  // Timer Compare Interrupt aktivieren
  interrupts();             // alle Interrupts scharf schalten

  setupComplete = true;
}
