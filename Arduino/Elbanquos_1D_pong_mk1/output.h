#ifndef output_h
#define output_h

bool output_isSequenceRunning();
unsigned long output_sceneDurationMillis();

void output_begin_GAME_SCENE ();
void output_begin_GAME_OVER_SCENE();
void output_begin_PLAYER_SCORE_SEQUENCE(byte player);

#endif
