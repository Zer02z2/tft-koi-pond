#pragma once
#include "../helper.h"
#include "Circle.h"

#define MAX_CHAIN_LENGTH 20

class Chain{
    public:
        Chain(uint8_t x, uint8_t y, float gap, float angle, uint8_t *sizes[MAX_CHAIN_LENGTH]);

        void freeMove(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
        void constrainMove(uint8_t x, uint8_t y, float idealRadian, float constrainStrength = 0.7f);
        void simpleMove(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

        void drawOutline(LGFX_Sprite* sprite);
        void drawRig(LGFX_Sprite* sprite);
        Point calculatePoint(const Circle& circle, float radian);

    private:
        Circle circles_[MAX_CHAIN_LENGTH];
        uint8_t x_;
        uint8_t y_;
        float gap_;
        float smallestAngle_;
        long frameCount_ = 0;
};