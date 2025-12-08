#pragma once
#include "../helper.h"
#include <vector>
#include <LovyanGFX.hpp>

struct DuckWeedPoint {
    float length;
    float radian;
};

class DuckWeed {
    public:
        // Changed constructor to accept fillColor and strokeColor
        DuckWeed(float x, float y, float radius, int segments, uint16_t fillColor, uint16_t strokeColor);
        void update(int width, int height);
        void applyVector(float x, float y, float strength);
        void draw(LGFX_Sprite* sprite);
        Point getPosition() const;
        float getRadius() const { return radius_; }

    private:
        float radius_;
        float xCur_, yCur_;
        float xTar_, yTar_;
        
        std::vector<DuckWeedPoint> points_;
        Point moveVector_ = {0, 0};
        float vectorMax_;

        uint16_t fillColor_;
        uint16_t strokeColor_;
};