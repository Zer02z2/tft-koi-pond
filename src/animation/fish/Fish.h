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

class Fish {
    public:
        Fish(float x, float y, float length, float width, int canvasWidth, int canvasHeight, uint16_t fillColor = TFT_BLACK, uint16_t strokeColor = TFT_WHITE);
        
        void update(int width, int height);
        void draw(LGFX_Sprite* sprite);
        void triggerDash();

        // Getters for external collision detection (Controller)
        Point getPosition() const;   // Returns Head (Cube) position
        float getVelocity() const;   // Returns current velocity magnitude
        float getWidth() const;      // Returns approximate width of the fish body

    private:
        float gap_;
        Chain body_;
        Cube cube_;
        uint16_t fillColor_ = TFT_BLACK;
        uint16_t strokeColor_ = TFT_WHITE;
        
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