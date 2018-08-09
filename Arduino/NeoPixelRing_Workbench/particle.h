#ifndef particle_h
#define particle_h

#include <Adafruit_NeoPixel.h>

#define PARTICLE_NORM_DISTANCE 15876     // Full distance the norm pixel will fly

#define PARTICLE_NORM_DURATION 64
#define PARTICLE_FRAMES_PER_TICK 1        // at 30 fps the norm partice will live 4 Seconds

#define PARTICLE_PIXEL_COUNT 12
#define POSITION_TO_PIXEL_FACTOR 1323    // Full distance divided to 12 pixel strand

class Particle
{
  public:
        Particle(void);
        
        /* operations */

        /* Ignite a particle at a position with a Base for the velocity */

        void igniteRandom(int pos, int velocityBase=0); 

        void ignite(int r,int g, int b, int pos, int vlct, byte dcr);

        /* Calculate progress of the particle (must be called for every frame) */
        void frameTick(unsigned long frameNumber);

        /* draw the particle on the Neopixel strip */
        void draw(Adafruit_NeoPixel &strip);

        /* information */
        bool isAlive(void) {return life!=0;}



  protected:
        byte colorChannel[3]={0,0,0};

        /* 
         *  The folowing defaults are calculated for a "norm" particle
         *  is alive for 64 Steps and travels a distance of 15876
         */
        
        // due to loss in integer arithmetic we define 1000 as 1
        int life=0;        // startvalue will be 1000, 0 = dead
        byte decayrate=93;  // % life remains every  step  
        int position=0;    // internal position (
        int velocity=1000;    // "norm" Velocity
        int acceleration=-15; // will stop movment with life decay rate 93


        static const int max_position=PARTICLE_PIXEL_COUNT*POSITION_TO_PIXEL_FACTOR;

};

#endif
