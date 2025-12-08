#pragma once
#include <LovyanGFX.hpp>
#include "../helper.h"

class Ripple {
    public:
        Ripple(float x, float y, float intensity);
        
        // Returns false if the ripple has faded out and should be removed
        bool update();
        void draw(LGFX_Sprite* sprite);

    private:
        float x_, y_;
        float currentRadius_;
        float currentIntensity_; // 0 - 100 usually
        float speed_;
};