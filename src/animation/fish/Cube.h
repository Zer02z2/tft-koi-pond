#pragma once
#include <Arduino.h>
#include "../helper.h"
#include "Chain.h"

class Cube {
    public:
        Cube(float x, float y, float vMax);
        Cube() = default;

        void update(int xBound, int yBound);
        void dash(float radian);
        Point getPosition() const;
        
        float vX, vY;
        float vMax, vMin, vDash;
        int directionX, directionY;

    private:
        float x_, y_;
        float w_, h_;
        float pBoost_ = 0.005f;
        float pDirectionChange_ = 0.001f;
        
        float boostVelocity();
        void preventOverBoarder(int xBound, int yBound);
};