#ifndef particle_h
#define particle_h

#include <Adafruit_NeoPixel.h>



class Particle
{
  public:
        Particle(void);
        
        /* operations */
        void awake(int r,int g, int b, int pos, int velocity, byte decayrate);
        void tick(void);
        void draw(Adafruit_NeoPixel &strip);

        /* information */
        bool isAlive(void) {return life!=0;}

  protected:
        byte movetick=0;
        byte life=0;
        int position=0;
        int velocity=0;
        byte decayrate=0;
        byte colorChannel[3]={0,0,0};
};

#endif
