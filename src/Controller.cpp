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
    // 1. Initialize Hardware
    lcd_.begin();
    
    // [16-BIT MODE]
    lcd_.setColorDepth(16);
    
    // [RANDOM SEED]
    srand(esp_random()); 

    if (lcd_.width() < lcd_.height()) lcd_.setRotation(lcd_.getRotation() ^ 1);

    for (auto& sprite : sprites_) {
        // [16-BIT SPRITES]
        sprite->setColorDepth(16); 
        sprite->createSprite(lcd_.width(), lcd_.height());
        // [SWAP BYTES] Match LCD setting
        sprite->setSwapBytes(true);
        sprite->fillScreen(0); 
    }

    buttons_.begin();
    pixels_.begin();

    // 2. Initialize Entities
    int w = lcd_.width();
    int h = lcd_.height();

    // Fish
    float fishSize = sqrt(pow(w, 2) + pow(h, 2)) * 0.015f * randomFloat(0.8f, 1.2f);
    float fishLength = fishSize * randomFloat(6.0f, 8.5f); 
    float fishWidth = fishLength * randomFloat(0.2f, 0.24f);
    
    // [COLORS] 16-bit RGB565
    // Note: color565 returns a uint16_t packed into uint32_t.
    uint32_t fishFill = sprites_[0]->color565(29, 29, 29); 
    uint32_t fishStroke = sprites_[0]->color565(155, 155, 155);
    
    if (fish_) delete fish_;
    fish_ = new Fish(w/2.0f, h/2.0f, fishLength, fishWidth, w, h, fishFill, fishStroke);

    // Leaves
    int numLeaves = 15;
    leaves_.clear();
    uint32_t leafFill = sprites_[0]->color565(62, 145, 60); 
    uint32_t leafStroke = sprites_[0]->color565(0, 0, 0); 

    for(int i=0; i<numLeaves; i++) {
        float size = sqrt(pow(w, 2) + pow(h, 2));
        float radius = randomFloat(size * 0.02f, size * 0.05f);
        leaves_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, 12, leafFill, leafStroke);
    }

    // DuckWeeds
    int numDuckWeeds = 50;
    duckWeeds_.clear();
    for(int i=0; i<numDuckWeeds; i++) {
        float size = sqrt(pow(w, 2) + pow(h, 2));
        float radius = randomFloat(size * 0.001f, size * 0.01f);
        duckWeeds_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, 4);
    }
    
    rippleCooldown_ = (unsigned long)randomFloat(2000, 7000);
    lastRippleTime_ = millis();
}

void Controller::handleReport(const ButtonGroup::Report &rep) {
    if (fish_ && !rep.longPress) {
        fish_->triggerDash();
        Point p = fish_->getPosition();
        // Spawning ripple with 100 intensity
        ripples_.emplace_back(p.x, p.y, rippleIntensity_); 
    }
}

// --- Collision Detection ---

void Controller::detectFishLeafCollision() {
    if(!fish_) return;
    Point fishP = fish_->getPosition(); 
    float fishVel = fish_->getVelocity(); 
    float fishWidth = fish_->getWidth();

    for (auto& leaf : leaves_) {
        Point leafP = leaf.getPosition();
        float d = dist(fishP.x, fishP.y, leafP.x, leafP.y);
        if (d >= fishWidth * 2.0f || d == 0) continue; 
        leaf.applyOscillation(fishP.x, fishP.y, fishVel / d);
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
        if (d >= fishWidth * 2.0f || d == 0) continue;
        dw.applyVector(fishP.x, fishP.y, (0.2f * fishVel) / d);
    }
}

void Controller::detectRippleLeafCollision() {
    for (const auto& r : ripples_) {
        for (auto& leaf : leaves_) {
            Point lPos = leaf.getPosition();
            float d = dist(r.getX(), r.getY(), lPos.x, lPos.y);
            
            for (const auto& ring : r.getRings()) {
                float radius = ring.currentRadius;
                if (d > radius + leaf.getRadius()) continue;
                if (d < radius - leaf.getRadius()) continue;

                // Push leaf - intensity map 0-100 -> 0.0-5.0
                float mag = map(ring.currentIntensity, 0, 100, 0, 5.0f);
                leaf.applyOscillation(r.getX(), r.getY(), mag);
            }
        }
    }
}

void Controller::detectRippleDuckWeedCollision() {
    for (const auto& r : ripples_) {
        for (auto& dw : duckWeeds_) {
            Point dwPos = dw.getPosition();
            float d = dist(r.getX(), r.getY(), dwPos.x, dwPos.y);
            
            for (const auto& ring : r.getRings()) {
                float radius = ring.currentRadius;
                if (d > radius + dw.getRadius()) continue;
                if (d < radius - dw.getRadius()) continue;

                // Push weed - intensity map 0-100 -> 0.0-0.1
                float mag = map(ring.currentIntensity, 0, 100, 0, 0.1f);
                dw.applyVector(r.getX(), r.getY(), mag);
            }
        }
    }
}

// [16-bit DiffDraw]
void Controller::diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1) {
    if (!sp0 || !sp1) return;

    // Use uint16_t pointers for 16-bit color depth
    uint16_t* s16 = (uint16_t*)sp0->getBuffer();
    uint16_t* p16 = (uint16_t*)sp1->getBuffer();

    // Use uint32_t pointers to compare 2 pixels at once
    uint32_t* s32 = (uint32_t*)s16;
    uint32_t* p32 = (uint32_t*)p16;

    int width = sp0->width();
    int height = sp0->height();
    
    int w32 = width >> 1; 

    for (int y = 0; y < height; y++) {
        int x32 = 0;
        while (x32 < w32) {
            while (x32 < w32 && s32[x32] == p32[x32]) x32++;
            if (x32 >= w32) break; 

            int x_start = x32 << 1; 
            if (s16[x_start] == p16[x_start]) x_start++;

            while (x32 < w32 && s32[x32] != p32[x32]) x32++;
            
            int x_end = (x32 << 1) - 1; 
            if (x_end >= width) x_end = width - 1;
            if (x_end > x_start && s16[x_end] == p16[x_end]) x_end--;

            int len = x_end - x_start + 1;
            if (len > 0) {
                lcd_.pushImage(x_start, y, len, 1, &s16[x_start]);
            }
        }
        s16 += width;
        p16 += width;
        s32 = (uint32_t*)s16;
        p32 = (uint32_t*)p16;
    }
}

void Controller::drawfunc(void) {
    if (!sprites_[0] || !sprites_[1] || !fish_) return;

    std::size_t flip = _draw_count & 1;
    LGFX_Sprite* currentSprite = sprites_[flip];
    LGFX_Sprite* prevSprite = sprites_[!flip];

    // Background 0 (Black)
    currentSprite->fillScreen(0);

    // --- Ripple Spawning ---
    unsigned long now = millis();
    if (now - lastRippleTime_ >= rippleCooldown_) {
        float rx = randomFloat(0, lcd_.width());
        float ry = randomFloat(0, lcd_.height());
        // Random Ripple with intensity 100
        ripples_.emplace_back(rx, ry, rippleIntensity_); 
        
        lastRippleTime_ = now;
        rippleCooldown_ = (unsigned long)randomFloat(2000, 7000);
    }

    // --- Update & Bounce ---
    std::vector<Ripple> bouncedRipples;
    for (int i = ripples_.size() - 1; i >= 0; i--) {
        bool alive = ripples_[i].update();
        
        std::vector<Ripple> newR = ripples_[i].detectBouncing(lcd_.width(), lcd_.height());
        bouncedRipples.insert(bouncedRipples.end(), newR.begin(), newR.end());

        if (!alive) {
            ripples_.erase(ripples_.begin() + i);
        }
    }
    ripples_.insert(ripples_.end(), bouncedRipples.begin(), bouncedRipples.end());

    // --- Physics ---
    fish_->update(lcd_.width(), lcd_.height());
    for(auto& l : leaves_) l.update();
    for(auto& d : duckWeeds_) d.update(lcd_.width(), lcd_.height());

    // --- Collisions ---
    detectFishLeafCollision();
    detectFishDuckWeedCollision();
    detectRippleLeafCollision();
    detectRippleDuckWeedCollision();

    // --- Draw Layers ---
    for(auto& d : duckWeeds_) d.draw(currentSprite);
    for(auto& r : ripples_) r.draw(currentSprite);
    fish_->draw(currentSprite);
    for(auto& l : leaves_) l.draw(currentSprite);

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