#pragma once
#include "../helper.h"
#include <vector>

struct DuckWeedPoint {
    float length;
    float radian;
};

class DuckWeed {
    public:
        DuckWeed(float x, float y, float radius, int segments);
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
};