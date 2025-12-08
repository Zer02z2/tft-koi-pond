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
    // Default is usually 16-bit, so we don't strictly need to call lcd_.setColorDepth(16)
    // unless it was changed elsewhere.
    
    if (lcd_.width() < lcd_.height()) lcd_.setRotation(lcd_.getRotation() ^ 1);

    for (auto& sprite : sprites_) {
        // [FIX] Set to 16-bit color depth (RGB565)
        sprite->setColorDepth(16);
        sprite->createSprite(lcd_.width(), lcd_.height());
        // [FIX] Use color565 for 16-bit
        sprite->fillScreen(0); 
    }

    buttons_.begin();
    pixels_.begin();

    int w = lcd_.width();
    int h = lcd_.height();

    // --- 1. Initialize Fish ---
    float fishSize = sqrt(pow(w, 2) + pow(h, 2)) * 0.015f * randomFloat(0.8f, 1.2f);
    float fishLength = fishSize * randomFloat(6.0f, 8.5f); 
    float fishWidth = fishLength * randomFloat(0.2f, 0.24f);
    
    // [FIX] Use color565 standard colors
    uint32_t fishFill = sprites_[0]->color565(29, 29, 29); 
    uint32_t fishStroke = sprites_[0]->color565(155, 155, 155);
    
    if (fish_) delete fish_;
    fish_ = new Fish(w/2.0f, h/2.0f, fishLength, fishWidth, w, h, fishFill, fishStroke);

    // --- 2. Initialize Leaves ---
    int numLeaves = 15;
    leaves_.clear();
    uint32_t leafFill = sprites_[0]->color565(62, 145, 60); 
    uint32_t leafStroke = sprites_[0]->color565(10, 10, 10); 

    for(int i=0; i<numLeaves; i++) {
        float size = sqrt(pow(w, 2) + pow(h, 2));
        float radius = randomFloat(size * 0.02f, size * 0.05f);
        leaves_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, 12, leafFill, leafStroke);
    }

    // --- 3. Initialize DuckWeeds ---
    int numDuckWeeds = 50;
    duckWeeds_.clear();
    for(int i=0; i<numDuckWeeds; i++) {
        float size = sqrt(pow(w, 2) + pow(h, 2));
        float radius = randomFloat(size * 0.001f, size * 0.01f);
        duckWeeds_.emplace_back(randomFloat(0, w), randomFloat(0, h), radius, 4);
    }
    
    // Initialize Ripple Timer
    rippleCooldown_ = (unsigned long)randomFloat(2000, 7000);
    lastRippleTime_ = millis();
}

void Controller::handleReport(const ButtonGroup::Report &rep) {
    if (fish_ && !rep.longPress) {
        fish_->triggerDash();
        Point p = fish_->getPosition();
        ripples_.emplace_back(p.x, p.y, 100); 
    }
}

void Controller::detectFishLeafCollision() {
    if(!fish_) return;
    Point fishP = fish_->getPosition(); 
    float fishVel = fish_->getVelocity(); 
    float fishWidth = fish_->getWidth();

    for (auto& leaf : leaves_) {
        Point leafP = leaf.getPosition();
        float d = dist(fishP.x, fishP.y, leafP.x, leafP.y);
        if (d >= fishWidth * 2.0f || d == 0) continue; 
        leaf.applyOscillation(fishP.x, fishP.y, fishVel);
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

// [FIX] New DiffDraw for 16-bit buffers
void Controller::diffDraw(LGFX_Sprite* sp0, LGFX_Sprite* sp1) {
    if (!sp0 || !sp1) return;

    // Cast to uint16_t pointer for 16-bit pixel access
    uint16_t* s16 = (uint16_t*)sp0->getBuffer();
    uint16_t* p16 = (uint16_t*)sp1->getBuffer();

    // Cast to uint32_t to compare 2 pixels at a time
    uint32_t* s32 = (uint32_t*)s16;
    uint32_t* p32 = (uint32_t*)p16;

    int width = sp0->width();
    int height = sp0->height();
    
    // Number of 32-bit blocks per line (width / 2)
    int w32 = width >> 1; 

    for (int y = 0; y < height; y++) {
        int x32 = 0;
        while (x32 < w32) {
            // Fast skip matching blocks
            while (x32 < w32 && s32[x32] == p32[x32]) x32++;
            if (x32 >= w32) break; // Line done

            // Found a difference. Calculate exact start pixel.
            int x_start = x32 << 1; 
            // Check if the difference is actually in the second pixel of the pair
            if (s16[x_start] == p16[x_start]) x_start++;

            // Find end of the block difference
            while (x32 < w32 && s32[x32] != p32[x32]) x32++;
            
            // Calculate exact end pixel
            int x_end = (x32 << 1) - 1; 
            if (x_end >= width) x_end = width - 1;
            // Backtrack if the very last pixel actually matched
            if (x_end > x_start && s16[x_end] == p16[x_end]) x_end--;

            // Push to LCD
            int len = x_end - x_start + 1;
            if (len > 0) {
                lcd_.pushImage(x_start, y, len, 1, &s16[x_start]);
            }
        }
        // Advance pointers to the next line
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

    // [FIX] Use color565 for 16-bit background
    currentSprite->fillScreen(0);

    // --- Ripple Logic ---
    unsigned long now = millis();
    if (now - lastRippleTime_ >= rippleCooldown_) {
        float rx = randomFloat(0, lcd_.width());
        float ry = randomFloat(0, lcd_.height());
        ripples_.emplace_back(rx, ry, 100); 
        
        lastRippleTime_ = now;
        rippleCooldown_ = (unsigned long)randomFloat(2000, 7000);
    }

    for (int i = ripples_.size() - 1; i >= 0; i--) {
        if (!ripples_[i].update()) {
            ripples_.erase(ripples_.begin() + i);
        }
    }

    // --- Entity Updates ---
    fish_->update(lcd_.width(), lcd_.height());
    for(auto& l : leaves_) l.update();
    for(auto& d : duckWeeds_) d.update(lcd_.width(), lcd_.height());

    detectFishLeafCollision();
    detectFishDuckWeedCollision();

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