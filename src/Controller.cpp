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
    lcd_.setColorDepth(16);
    srand(esp_random()); 

    if (lcd_.width() < lcd_.height()) lcd_.setRotation(lcd_.getRotation() ^ 1);

    for (auto& sprite : sprites_) {
        sprite->setColorDepth(16); 
        sprite->createSprite(lcd_.width(), lcd_.height());
        sprite->setSwapBytes(true);
        sprite->fillScreen(0); 
    }

    buttons_.begin();

    pixels_.begin();
    pixels_.setBrightness(60);
    pixels_.show();

    int w = lcd_.width();
    int h = lcd_.height();

    int numFish = 5;
    fishes_.clear();
    uint16_t fishFill = lcd_.color565(29, 29, 29); 
    uint16_t fishStroke = lcd_.color565(155, 155, 155);

    for (int i=0; i<numFish; i++) {
        float fishSize = sqrt(pow(w, 2) + pow(h, 2)) * 0.015f * randomFloat(0.8f, 1.2f);
        float fishLength = fishSize * randomFloat(6.0f, 8.5f); 
        float fishWidth = fishLength * randomFloat(0.24f, 0.28f);
        int posX = random(0, w);
        int posY = random(0, h);
        fishes_.emplace_back(posX, posY, fishLength, fishWidth, w, h, fishFill, fishStroke);
    }

    int numLeaves = 15;
    leaves_.clear();
    uint16_t leafFill = lcd_.color565(62, 145, 60); 
    uint16_t leafStroke = lcd_.color565(0, 0, 0); 
    int segments = 16;

    for(int i=0; i<numLeaves; i++) {
        float size = sqrt(pow(w, 2) + pow(h, 2)) * 1.2f;
        float radius = randomFloat(size * 0.02f, size * 0.05f);
        leaves_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, segments, leafFill, leafStroke);
    }

    int numDuckWeeds = 50;
    duckWeeds_.clear();
    uint16_t weedFill = lcd_.color565(62, 145, 60);
    uint16_t weedStroke = lcd_.color565(0, 0, 0);

    for(int i=0; i<numDuckWeeds; i++) {
        float size = sqrt(pow(w, 2)+ pow(h, 2));
        float radius = randomFloat(size * 0.001f, size * 0.01f);
        duckWeeds_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, 4, weedFill, weedStroke);
    }
    
    rippleCooldown_ = (unsigned long)randomFloat(2000, 7000);
    lastRippleTime_ = millis();
}

void Controller::handleReport(const ButtonGroup::Report &rep) {
    String txt = rep.text;
    
    if (txt == "left-in") {
        swimTopLeft_ = true;
    } else if (txt == "left-out") {
        swimTopLeft_ = false;
    } else if (txt == "right-in") {
        swimTopRight_ = true;
    } else if (txt == "right-out") {
        swimTopRight_ = false;
    } else if (txt == "bottom-in") {
        swimBottomCenter_ = true;
    } else if (txt == "bottom-out") {
        swimBottomCenter_ = false;
    } else if (txt == "spread-in") {
        spreadHolding_ = true;
        // Trigger dashing away
        if (!fishes_.empty()) {
             float sumX = 0, sumY = 0;
             for(auto& f : fishes_) { Point p = f.getPosition(); sumX += p.x; sumY += p.y; }
             float avgX = sumX / fishes_.size();
             float avgY = sumY / fishes_.size();
             
             for(auto& f : fishes_) {
                 Point p = f.getPosition();
                 float angle = atan2(p.y - avgY, p.x - avgX);
                 f.triggerDash(angle);
             }
        }
    } else if (txt == "spread-out") {
        spreadHolding_ = false;
    }
}

// Helper for Fish-Fish collision
static bool isOverlapping(const FishBounds& a, const FishBounds& b) {
    if (a.right <= b.left || b.right <= a.left) return false;
    if (a.bottom <= b.top || b.bottom <= a.top) return false;
    return true;
}

void Controller::detectFishFishCollision() {
    for (size_t i = 0; i < fishes_.size(); i++) {
        if (!fishes_[i].getIsDashing()) continue;
        
        FishBounds b1 = fishes_[i].getBounds();
        
        for (size_t j = 0; j < fishes_.size(); j++) {
            if (i == j) continue;
            FishBounds b2 = fishes_[j].getBounds();
            if (isOverlapping(b1, b2)) {
                fishes_[j].triggerDash();
            }
        }
    }
}

void Controller::detectFishLeafCollision() {
    if(fishes_.empty()) return;
    for (auto& fish : fishes_) {
        Point fishP = fish.getPosition(); 
        float fishVel = fish.getVelocity(); 
        float fishWidth = fish.getWidth();

        for (auto& leaf : leaves_) {
            Point leafP = leaf.getPosition();
            float d = dist(fishP.x, fishP.y, leafP.x, leafP.y);
            if (d >= fishWidth * 2.0f || d == 0) continue; 
            leaf.applyOscillation(fishP.x, fishP.y, fishVel / d * 2.0f);
        }
    }
}

void Controller::detectFishDuckWeedCollision() {
    if(fishes_.empty()) return;
    for (auto& fish : fishes_) {
        Point fishP = fish.getPosition(); 
        float fishVel = fish.getVelocity(); 
        float fishWidth = fish.getWidth();

        for (auto& dw : duckWeeds_) {
            Point dwP = dw.getPosition();
            float d = dist(fishP.x, fishP.y, dwP.x, dwP.y);
            if (d >= fishWidth * 2.0f || d == 0) continue;
            dw.applyVector(fishP.x, fishP.y, (0.2f * fishVel) / d);
        }
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
                float mag = map(ring.currentIntensity, 0, 100, 0, 0.1f);
                dw.applyVector(r.getX(), r.getY(), mag);
            }
        }
    }
}

void Controller::diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1) {
    if (!sp0 || !sp1) return;
    uint16_t* s16 = (uint16_t*)sp0->getBuffer();
    uint16_t* p16 = (uint16_t*)sp1->getBuffer();
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
    if (!sprites_[0] || !sprites_[1] || fishes_.empty()) return;

    std::size_t flip = _draw_count & 1;
    LGFX_Sprite* currentSprite = sprites_[flip];
    LGFX_Sprite* prevSprite = sprites_[!flip];
    currentSprite->fillScreen(0);

    // 1. Update LEDs based on current flags
    uint32_t color = pixels_.Color(62, 145, 60);
    
    // Reset all to 0 first
    uint32_t c0 = 0, c1 = 0, c2 = 0;

    if (spreadHolding_) {
        c0 = color; c1 = color; c2 = color;
    } else {
        if (swimTopLeft_) c0 = color;
        if (swimTopRight_) c2 = color;
        if (swimBottomCenter_) c1 = color;
    }
    pixels_.setPixelColor(0, c0);
    pixels_.setPixelColor(1, c1);
    pixels_.setPixelColor(2, c2);
    pixels_.show();

    // 2. Spawn Ripples
    unsigned long now = millis();
    if (now - lastRippleTime_ >= rippleCooldown_) {
        float rx = randomFloat(0, lcd_.width());
        float ry = randomFloat(0, lcd_.height());
        ripples_.emplace_back(rx, ry, rippleIntensity_); 
        lastRippleTime_ = now;
        rippleCooldown_ = (unsigned long)randomFloat(2000, 7000);
    }

    // 3. Update & Bounce Ripples
    bool bounceEnabled = false;
    std::vector<Ripple> bouncedRipples;
    for (int i = ripples_.size() - 1; i >= 0; i--) {
        bool alive = ripples_[i].update();
        if (bounceEnabled) {
            std::vector<Ripple> newR = ripples_[i].detectBouncing(lcd_.width(), lcd_.height());
            bouncedRipples.insert(bouncedRipples.end(), newR.begin(), newR.end());
        }
        if (!alive) ripples_.erase(ripples_.begin() + i);
    }
    if (bounceEnabled) ripples_.insert(ripples_.end(), bouncedRipples.begin(), bouncedRipples.end());

    // 4. Swimming Logic
    // Pre-calculate directional vectors
    float invSqrt2 = 0.70710678f; // 1 / sqrt(2)

    if (swimTopLeft_) {
        // Target: (0, 0)
        for(auto& f : fishes_) {
            Point p = f.getPosition();
            float dx = 0 - p.x;
            float dy = 0 - p.y;
            float mag = sqrt(dx*dx + dy*dy);
            if (mag > 0.1f) {
                float s = f.getSwimSpeed();
                f.swim((dx / mag) * s, (dy / mag) * s);
            }
        }
    }
    if (swimTopRight_) {
        // Target: (width, 0)
        float targetX = lcd_.width();
        for(auto& f : fishes_) {
            Point p = f.getPosition();
            float dx = targetX - p.x;
            float dy = 0 - p.y;
            float mag = sqrt(dx*dx + dy*dy);
            if (mag > 0.1f) {
                float s = f.getSwimSpeed();
                f.swim((dx / mag) * s, (dy / mag) * s);
            }
        }
    }
    if (swimBottomCenter_) {
        float targetX = lcd_.width() / 2.0f;
        float targetY = lcd_.height();
        for(auto& f : fishes_) {
            Point p = f.getPosition();
            float dx = targetX - p.x;
            float dy = targetY - p.y;
            float mag = sqrt(dx*dx + dy*dy);
            if (mag > 0.1f) {
                float s = f.getSwimSpeed();
                f.swim((dx / mag) * s, (dy / mag) * s);
            }
        }
    }

    // Physics
    for (auto& fish : fishes_) fish.update(lcd_.width(), lcd_.height());
    for(auto& l : leaves_) l.update();
    for(auto& d : duckWeeds_) d.update(lcd_.width(), lcd_.height());

    // Collisions
    detectFishLeafCollision();
    detectFishDuckWeedCollision();
    detectRippleLeafCollision();
    detectRippleDuckWeedCollision();
    detectFishFishCollision(); 

    // Draw
    for (auto& fish : fishes_) fish.draw(currentSprite);
    for(auto& d : duckWeeds_) d.draw(currentSprite);
    for(auto& r : ripples_) r.draw(currentSprite);
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