#pragma once

#include <Arduino.h>
#include "ButtonGroup.h"
#include <Adafruit_NeoPixel.h>
#include <LovyanGFX.hpp>
#include <config.hpp>
#include <vector>

// Include Animation Classes
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

        // --- Animation Entities ---
        Fish* fish_ = nullptr;
        std::vector<Leaf> leaves_;
        std::vector<DuckWeed> duckWeeds_;
        std::vector<Ripple> ripples_;

        // --- Physics & Drawing ---
        volatile std::uint32_t _draw_count = 0;
        void diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1);
        void drawfunc(void);
        
        // --- Collision Logic ---
        void detectFishLeafCollision();
        void detectFishDuckWeedCollision();

        // --- Ripple Logic ---
        unsigned long lastRippleTime_ = 0;
        unsigned long rippleCooldown_ = 0;
};