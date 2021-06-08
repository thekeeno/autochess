#ifndef ALIBESteer_h
	#define ALIBESteer_h
	#include <Arduino.h>
	#include <AccelStepper.h>
    #include "ramps.h"


    class ChessAxis: public AccelStepper {
        public:
            ChessAxis(int , int, int, int, int, bool);
            AccelStepper motor;
            bool axis;
        
            int stepPin;
            int dirPin;
            int enablePin;
            int minPin;
            int maxPin;

            bool homed;

            int boardOffset = 0;
            int endstopLow = 0;

            void init();
            void home();
            void run();
            void printStatus();
        private:

    };
#endif