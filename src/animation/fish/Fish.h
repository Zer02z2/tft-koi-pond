#pragma once
#include "Chain.h"
#include "Cube.h"
#include <vector>
#include <LovyanGFX.hpp>

struct FinConfig {
    Chain fin;
    float radian;
    int position;
};

struct FishBounds {
    float left, right, top, bottom;
};

class Fish {
    public:
        Fish(float x, float y, float length, float width, int canvasWidth, int canvasHeight, uint16_t fillColor = TFT_BLACK, uint16_t strokeColor = TFT_WHITE);
        
        void update(int width, int height);
        void draw(LGFX_Sprite* sprite);
        
        void triggerDash();               
        void triggerDash(float radian);   
        
        void swim(float dx, float dy);    

        Point getPosition() const;
        float getVelocity() const;
        float getWidth() const;
        float getSwimSpeed() const { return swimSpeed_; } // Getter
        
        bool getIsDashing() const;
        FishBounds getBounds() const;

    private:
        float gap_;
        Chain body_;
        Cube cube_;
        uint16_t fillColor_ = TFT_BLACK;
        uint16_t strokeColor_ = TFT_WHITE;
        float swimSpeed_; // New variable
        
        std::vector<FinConfig> fins_;
        std::vector<FinConfig> tails_;
        std::vector<FinConfig> backFins_;

        void drawBackFin(LGFX_Sprite* ctx);
        void drawEyes(LGFX_Sprite* ctx);

        const std::vector<float> bodyPoints = {
            0.326, 0.641, 0.817, 0.9, 0.97, 0.957, 0.872, 0.787, 0.702, 0.618, 0.516, 0.414, 0.316, 0.219
        };
        const std::vector<float> finPoints = {
            0.226, 0.217, 0.334, 0.476, 0.424, 0.355, 0.11
        };
        const std::vector<float> tailPoints = {
            0.326, 0.321, 0.32, 0.294, 0.283, 0.216, 0.155, 0.09
        };
        const std::vector<float> backFinPoints = {0.5, 0.5, 0.5};
};