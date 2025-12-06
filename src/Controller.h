#pragma once

#include <Arduino.h>
#include "ButtonGroup.h"
#include <Adafruit_NeoPixel.h>
#include <LovyanGFX.hpp>
#include <config.hpp>

class Controller{
    public:
        Controller(LGFX &lcd,
                   LGFX_Sprite *sp0,
                   LGFX_Sprite *sp1,

                   // Hardware
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

        // animation handler
        volatile std::uint32_t _draw_count = 0;
        void diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1);
        void drawfunc(void);
};