#pragma once
#include "../helper.h"
#include "Circle.h"
#include <vector>

#define MAX_CHAIN_LENGTH 20

class Chain {
    public:
        // Adjusted constructor to take a vector or array pointer + length
        Chain(float x, float y, float gap, float angle, const std::vector<float>& sizes);
        Chain() = default; // Default constructor for vectors

        void freeMove(float x, float y, int width, int height);
        void constrainMove(float x, float y, float idealRadian, float constrainStrength = 0.7f);
        void simpleMove(float x, float y, int width, int height);

        void draw(LGFX_Sprite* sprite, uint32_t fillColor, uint32_t strokeColor);
        void drawRig(LGFX_Sprite* sprite, uint32_t color);
        
        Point calculatePoint(const Circle& circle, float radian);
        Circle& getCircle(int index);
        int getLength() const { return length_; }

        // Expose circles for Fish class access
        Circle circles_[MAX_CHAIN_LENGTH];

    private:
        float x_;
        float y_;
        float gap_;
        float smallestAngle_;
        float frameCount_ = 0;
        int length_ = 0;
};