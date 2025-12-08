#include "Ripple.h"

Ripple::Ripple(float x, float y, float intensity)
    : x_(x), y_(y), currentIntensity_(intensity)
{
    currentRadius_ = 0;
    speed_ = intensity / 100.0f;
    // Map TS intensity to something visible. 
    // In TS: speed = intensity/100. radius += speed * 5.
}

bool Ripple::update() {
    currentIntensity_ -= speed_;
    currentRadius_ += speed_ * 5.0f;
    
    return (currentIntensity_ > 0);
}

void Ripple::draw(LGFX_Sprite* sprite) {
    if (currentIntensity_ <= 0) return;

    // Simulate opacity by darkening the color (White -> Grey -> Black)
    // Map intensity (0-100) to RGB value (0-255)
    uint8_t brightness = (uint8_t)map(currentIntensity_, 0, 100, 0, 255);
    
    // Use color332 for 8-bit sprites (RRRGGGBB)
    uint32_t color = sprite->color332(brightness, brightness, brightness);
    
    sprite->drawCircle((int)x_, (int)y_, (int)currentRadius_, color);
}