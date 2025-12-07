#include "Controller.h"
#include "animation/helper.h"

Controller::Controller(LGFX &lcd,
                       LGFX_Sprite *sp0,
                       LGFX_Sprite *sp1,
                       ButtonGroup &buttons,
                       Adafruit_NeoPixel &pixels)
    : lcd_(lcd),
      buttons_(buttons),
      pixels_(pixels)
{
    sprites_[0] = sp0;
    sprites_[1] = sp1;
}

void Controller::begin() {
    lcd_.begin();
    // Use 8-bit color to save RAM on ESP32-S3 (No PSRAM)
    lcd_.setColorDepth(8);
    if (lcd_.width() < lcd_.height()) lcd_.setRotation(lcd_.getRotation() ^ 1);

    for (auto& sprite : sprites_) {
        sprite->setColorDepth(8);
        sprite->createSprite(lcd_.width(), lcd_.height());
        sprite->fillScreen(0); 
    }

    buttons_.begin();
    pixels_.begin();

    // 1. Initialize Fish
    int w = lcd_.width();
    int h = lcd_.height();
    float fishSize = sqrt(pow(w, 2) + pow(h, 2)) * 0.015 * randomFloat(0.8f, 1.2f);
    float fishLength = fishSize * randomFloat(6.0f, 8.5f); 
    float fishWidth = fishLength * randomFloat(0.2f, 0.24f);
    uint32_t fishFillColor = lcd_.color565(0, 0, 0);
    uint32_t fishStrokeColor = lcd_.color565(155, 155, 155);
    
    if (fish_) delete fish_;
    // Fish is centered initially
    fish_ = new Fish(w/2.0f, h/2.0f, fishLength, fishWidth, w, h, fishFillColor, fishStrokeColor);
    // 2. Initialize Leaves
    int numLeaves = 10;
    leaves_.clear();
    uint32_t leafFillColor = lcd_.color565(62, 145, 60); // Green
    for(int i=0; i<numLeaves; i++) {
        float size = sqrt(pow(w, 2) + pow(h, 2));
        float radius = randomFloat(size * 0.02f, size * 0.05f);
        leaves_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, 32, leafFillColor, TFT_BLACK);
    }

    // 3. Initialize DuckWeeds
    int numDuckWeeds = 50;
    duckWeeds_.clear();
    for(int i=0; i<numDuckWeeds; i++) {
        float size = sqrt(pow(w, 2) + pow(h, 2));
        float radius = randomFloat(size * 0.001f, size * 0.01f);
        duckWeeds_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, 4);
    }
}

void Controller::handleReport(const ButtonGroup::Report &rep) {
    if (fish_ && !rep.longPress) {
        fish_->triggerDash();
    }
}

void Controller::detectFishLeafCollision() {
    if(!fish_) return;
    
    // Uses the Cube/Head position as the interaction point
    Point fishP = fish_->getPosition(); 
    float fishVel = fish_->getVelocity(); 
    float fishWidth = fish_->getWidth();

    for (auto& leaf : leaves_) {
        Point leafP = leaf.getPosition();
        float d = dist(fishP.x, fishP.y, leafP.x, leafP.y);
        
        // Interaction threshold: approx 2x fish width
        if (d >= fishWidth * 2.0f) continue; 
        if (d == 0) continue;

        // Apply force based on velocity and distance
        float magnitude = fishVel / d;
        leaf.applyOscillation(fishP.x, fishP.y, magnitude);
    }
}

void Controller::detectFishDuckWeedCollision() {
    if(!fish_) return;
    
    Point fishP = fish_->getPosition(); 
    float fishVel = fish_->getVelocity(); 
    float fishWidth = fish_->getWidth();

    for (auto& dw : duckWeeds_) {
        Point dwP = dw.getPosition();
        float d = dist(fishP.x, fishP.y, dwP.x, dwP.y);
        
        if (d >= fishWidth * 2.0f) continue;
        if (d == 0) continue;

        float magnitude = (0.2f * fishVel) / d;
        dw.applyVector(fishP.x, fishP.y, magnitude);
    }
}

void Controller::diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1) {
    union { std::uint32_t* s32; std::uint8_t* s; };
    union { std::uint32_t* p32; std::uint8_t* p; };
    
    if (!sp0 || !sp1) return;

    s32 = (std::uint32_t*)sp0->getBuffer();
    p32 = (std::uint32_t*)sp1->getBuffer();

    auto width  = sp0->width();
    auto height = sp0->height();
    auto w32 = (width + 3) >> 2; 
    std::int32_t y = 0;
    
    do {
        std::int32_t x32 = 0;
        do {
            while (s32[x32] == p32[x32] && ++x32 < w32);
            if (x32 == w32) break;

            std::int32_t xs = x32 << 2;
            while (s[xs] == p[xs]) ++xs;

            while (++x32 < w32 && s32[x32] != p32[x32]);

            std::int32_t xe = (x32 << 2) - 1;
            if (xe >= width) xe = width - 1;
            while (s[xe] == p[xe]) --xe;

            lcd_.pushImage(xs, y, xe - xs + 1, 1, &s[xs]);
            
        } while (x32 < w32);
        
        s32 += w32;
        p32 += w32;
    } while (++y < height);
}

void Controller::drawfunc(void) {
    if (!sprites_[0] || !sprites_[1] || !fish_) return;

    // Double buffer swap
    std::size_t flip = _draw_count & 1;
    LGFX_Sprite* currentSprite = sprites_[flip];
    LGFX_Sprite* prevSprite = sprites_[!flip];

    // Clear background
    currentSprite->fillScreen(currentSprite->color565(10, 10, 10));

    // 1. Update Physics
    fish_->update(lcd_.width(), lcd_.height());
    for(auto& l : leaves_) l.update();
    for(auto& d : duckWeeds_) d.update(lcd_.width(), lcd_.height());

    // 2. Collision Checks
    detectFishLeafCollision();
    detectFishDuckWeedCollision();

    // 3. Draw Layers (Bottom to Top)
    for(auto& d : duckWeeds_) d.draw(currentSprite); // Duckweed bottom
    fish_->draw(currentSprite);                      // Fish middle
    for(auto& l : leaves_) l.draw(currentSprite);    // Leaves top

    // 4. Render difference to screen
    diffDraw(currentSprite, prevSprite);
    ++_draw_count;
}

void Controller::service() {
    buttons_.service();

    ButtonGroup::Report rep;
    if (buttons_.poll(rep)) {
        handleReport(rep);
    }
    
    drawfunc();
}