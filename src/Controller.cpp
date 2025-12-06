#include "Controller.h"

Controller::Controller(LGFX &lcd,
                       LGFX_Sprite *sp0,
                       LGFX_Sprite *sp1,
                       // Hardware
                       ButtonGroup &buttons,
                       Adafruit_NeoPixel &pixels)
    : lcd_(lcd),
      buttons_(buttons),
      pixels_(pixels)
{
    // Initialize the internal array for double buffering
    sprites_[0] = sp0;
    sprites_[1] = sp1;
}

void Controller::begin() {
    // LCD initialization
    lcd_.begin();
    lcd_.setColorDepth(8);
    if (lcd_.width() < lcd_.height()) lcd_.setRotation(lcd_.getRotation() ^ 1);
    // Sprite initialization
    for (auto& sprite : sprites_) {
        sprite->setColorDepth(8);
        sprite->createSprite(lcd_.width(), lcd_.height());
        sprite->fillScreen(TFT_BLACK);
    }

    buttons_.begin();
    pixels_.begin();
}

void Controller::handleReport(const ButtonGroup::Report &rep) {
    // Handle button report here
}

void Controller::diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1) {
    union {
        std::uint32_t* s32;
        std::uint8_t* s;
    };
    union {
        std::uint32_t* p32;
        std::uint8_t* p;
    };
    
    // Safety check
    if (!sp0 || !sp1) return;

    s32 = (std::uint32_t*)sp0->getBuffer();
    p32 = (std::uint32_t*)sp1->getBuffer();

    auto width  = sp0->width();
    auto height = sp0->height();

    // 32-bit width calc
    auto w32 = (width + 3) >> 2; 
    std::int32_t y = 0;
    
    do {
        std::int32_t x32 = 0;
        do {
            // Fast skip matching blocks
            while (s32[x32] == p32[x32] && ++x32 < w32);
            if (x32 == w32) break;

            // Find exact start byte
            std::int32_t xs = x32 << 2;
            while (s[xs] == p[xs]) ++xs;

            // Find block where difference ends
            while (++x32 < w32 && s32[x32] != p32[x32]);

            // Find exact end byte
            std::int32_t xe = (x32 << 2) - 1;
            if (xe >= width) xe = width - 1;
            while (s[xe] == p[xe]) --xe;

            // Push only the changed pixels to the actual LCD
            lcd_.pushImage(xs, y, xe - xs + 1, 1, &s[xs]);
            
        } while (x32 < w32);
        
        s32 += w32;
        p32 += w32;
    } while (++y < height);
}

void Controller::drawfunc(void) {
    // Drawing function code here
    if (!sprites_[0] || !sprites_[1]) return;

    LGFX_Sprite* sprite;

    auto width = sprites_[0]->width();
    auto height = sprites_[0]->height();

    // Flip between the two sprites
    std::size_t flip = _draw_count & 1;
    sprite = sprites_[flip];

    sprite->clear();

    // Draw something on the sprite

    // Compare and draw differences
    diffDraw(sprites_[flip], sprites_[!flip]);
    ++_draw_count;
}

void Controller::service() {
    // Service routine code here
    buttons_.service();

    ButtonGroup::Report rep;
    if (buttons_.poll(rep)) {
        handleReport(rep);
    }
    
    drawfunc();
}