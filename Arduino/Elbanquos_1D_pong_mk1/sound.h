#ifndef sound_h
#define sound_h

void sound_setup();
void sound_tick();

bool sound_isPlaying();
void sound_stop();

void sound_start_GameStartMelody();
void sound_start_GameOverMelody();
void sound_start_Pong(int ball_velocity);
void sound_start_BoostPong(int ball_velocity);
void sound_start_PongService();
void sound_start_Barrier();
void sound_start_Crash();


#endif
