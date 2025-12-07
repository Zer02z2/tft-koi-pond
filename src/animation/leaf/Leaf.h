#pragma once
#include "../helper.h"
#include <vector>

struct LeafPoint {
    float length;
    float radian;
};

class Leaf {
    public:
        Leaf(float x, float y, float radius, int segments, uint32_t fillColor = TFT_WHITE, uint32_t strokeColor = TFT_BLACK);
        void update();
        void applyOscillation(float x, float y, float strength);
        void draw(LGFX_Sprite* sprite);
        Point getPosition() const;
        float getRadius() const { return radius_; }

    private:
        float radius_;
        // Coordinate system (Original, Current, Target)
        float xOrg_, yOrg_;
        float xCur_, yCur_;
        float xTar_, yTar_;
        
        std::vector<LeafPoint> points_;
        float frameCount_ = 0;
        
        Point oscillateVector_ = {0, 0};
        float oscillateMax_;

        uint32_t fillColor_;
        uint32_t strokeColor_;
};