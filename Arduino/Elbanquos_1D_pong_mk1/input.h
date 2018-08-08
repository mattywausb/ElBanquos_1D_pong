#ifndef input_h
#define input_h

int input_getMillisSinceLastEvent();
long input_getCurrentPressDuration();
long input_getLastPressDuration();

bool input_button_A_gotPressed();
bool input_button_A_isPressed();
byte input_button_A_gotReleased();

bool input_button_B_gotPressed();
bool input_button_B_isPressed();
byte input_button_B_gotReleased();

void input_pauseUntilRelease();
void input_capture_tick();

#endif

