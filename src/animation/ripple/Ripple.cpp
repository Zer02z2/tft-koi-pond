#include "Ripple.h"

Ripple::Ripple(float x, float y, float intensity, float initialRadius)
    : x_(x), y_(y), maxIntensity_(intensity)
{
    speed_ = intensity / 50.0f; // TS logic
    startMillis_ = millis();
    
    // Map intensity to number of ripples (0 to 3)
    remainingRipples_ = floor(map(intensity, 0, 255, 0, 3));

    // Add first ring
    RippleRing firstRing;
    firstRing.currentIntensity = intensity;
    firstRing.targetRadius = initialRadius;
    firstRing.currentRadius = initialRadius;
    firstRing.edgeL = x; firstRing.edgeR = x;
    firstRing.edgeT = y; firstRing.edgeB = y;
    rings_.push_back(firstRing);
}

bool Ripple::update() {
    // 1. Spawn new rings (Double Trigger logic)
    if (remainingRipples_ > 0 && (millis() - startMillis_ > interval_)) {
        RippleRing newRing;
        newRing.currentIntensity = maxIntensity_; // Use stored max intensity
        newRing.currentRadius = 0;
        newRing.targetRadius = 0;
        newRing.edgeL = x_; newRing.edgeR = x_;
        newRing.edgeT = y_; newRing.edgeB = y_;
        
        rings_.push_back(newRing);
        startMillis_ = millis();
        remainingRipples_--;
    }

    // 2. Update existing rings
    for (int i = rings_.size() - 1; i >= 0; i--) {
        RippleRing& r = rings_[i];
        
        r.currentIntensity -= speed_;
        r.targetRadius += speed_ * 2.0f;
        // Smooth expansion
        r.currentRadius = lerp(r.currentRadius, r.targetRadius, 0.1f);

        if (r.currentIntensity <= 0) {
            rings_.erase(rings_.begin() + i);
        }
    }

    return !rings_.empty();
}

void Ripple::draw(LGFX_Sprite* sprite) {
    for (const auto& r : rings_) {
        if (r.currentIntensity <= 0) continue;

        uint8_t b = (uint8_t)map(r.currentIntensity, 0, 100, 0, 255);
        
        // 16-bit Grayscale
        uint32_t color = sprite->color565(b, b, b);
        
        sprite->drawCircle((int)x_, (int)y_, (int)r.currentRadius, color);
    }
}

std::vector<Ripple> Ripple::detectBouncing(int width, int height) {
    std::vector<Ripple> newRipples;

    for (auto& r : rings_) {
        float curL = x_ - r.currentRadius;
        float curR = x_ + r.currentRadius;
        float curT = y_ - r.currentRadius;
        float curB = y_ + r.currentRadius;

        float bounceIntensity = r.currentIntensity * 0.6f;
        
        // Left Wall
        if (r.edgeL > 0 && curL <= 0) {
            newRipples.emplace_back(-x_, y_, bounceIntensity, r.currentRadius);
        }
        // Right Wall
        if (r.edgeR < width && curR >= width) {
            newRipples.emplace_back(width * 2 - x_, y_, bounceIntensity, r.currentRadius);
        }
        // Top Wall
        if (r.edgeT > 0 && curT <= 0) {
            newRipples.emplace_back(x_, -y_, bounceIntensity, r.currentRadius);
        }
        // Bottom Wall
        if (r.edgeB < height && curB >= height) {
            newRipples.emplace_back(x_, height * 2 - y_, bounceIntensity, r.currentRadius);
        }

        // Update checked edges
        r.edgeL = curL; r.edgeR = curR;
        r.edgeT = curT; r.edgeB = curB;
    }
    return newRipples;
}