#include <Arduino.h>
#include <AccelStepper.h>
#include "ChessAxis.h"



    ChessAxis::ChessAxis(int stepPin, int dirPin, int enablePin, int maxPin, int minPin, bool axis){
        this->axis  = axis;
        this->dirPin = dirPin;
        this->stepPin = stepPin;
        this->enablePin = enablePin;
        this->minPin = minPin;
        this->maxPin = maxPin;
        this->homed = false;
        this->motor = AccelStepper(motorInterfaceType,stepPin, dirPin);
    }

    void ChessAxis::home(){
        this->homed = true;
        Serial.println("Homing not implemented yet");
    
    }

    void ChessAxis::printStatus(){
        Serial.print("D: ");
        Serial.println(motor.distanceToGo());

    }

    void ChessAxis::init(){
    
        
    
        pinMode(this->minPin, INPUT);
        pinMode(this->maxPin, INPUT);
        pinMode(this->enablePin, OUTPUT);
        digitalWrite(this->enablePin, LOW);

        this->motor.setAcceleration(ACCELERATION);

        this->motor.setMaxSpeed(MAX_SPEED);





    }

    void ChessAxis::run(){

            this->motor.run();

    }

    void ChessAxis::moveToSquare(int square){
        wake();
        this->motor.runToNewPosition(square*SQUARE_SIZE);
        sleep();
    }

    void ChessAxis::sleep(){
        digitalWrite(this->enablePin, HIGH);
    }


    void ChessAxis::wake(){
        digitalWrite(this->enablePin, LOW);
    }