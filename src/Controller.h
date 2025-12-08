#pragma once

#include <Arduino.h>
#include "ButtonGroup.h"
#include <Adafruit_NeoPixel.h>
#include <LovyanGFX.hpp>
#include <config.hpp>
#include <vector>

#include "animation/fish/Fish.h"
#include "animation/leaf/Leaf.h"
#include "animation/leaf/DuckWeed.h"
#include "animation/ripple/Ripple.h"

class Controller{
    public:
        Controller(LGFX &lcd,
                   LGFX_Sprite *sp0,
                   LGFX_Sprite *sp1,
                   ButtonGroup &buttons,
                   Adafruit_NeoPixel &pixels
                );

        void begin();
        void handleReport(const ButtonGroup::Report &rep);
        void service();

    private:
        LGFX &lcd_;
        LGFX_Sprite *sprites_[2];
        ButtonGroup &buttons_;
        Adafruit_NeoPixel &pixels_;

        std::vector<Fish> fishes_;
        std::vector<Leaf> leaves_;
        std::vector<DuckWeed> duckWeeds_;
        std::vector<Ripple> ripples_;

        volatile std::uint32_t _draw_count = 0;
        void diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1);
        void drawfunc(void);
        
        // Collisions
        void detectFishLeafCollision();
        void detectFishDuckWeedCollision();
        void detectRippleLeafCollision();
        void detectRippleDuckWeedCollision();
        void detectFishFishCollision(); 

        unsigned long lastRippleTime_ = 0;
        unsigned long rippleCooldown_ = 0;
        float rippleIntensity_ = 60.0f;

        // Button Interaction Flags
        bool swimTopLeft_ = false;
        bool swimTopRight_ = false;
        bool swimBottomCenter_ = false;
        bool spreadHolding_ = false;
};