#include "Chain.h"

Chain::Chain(uint8_t x, uint8_t y, float gap, float angle, uint8_t *sizes[MAX_CHAIN_LENGTH])
    : x_(x), y_(y), gap_(gap)
{
    smallestAngle_ = angle;
    for (uint8_t i = 0; i < MAX_CHAIN_LENGTH; ++i) {
        Circle newCircle = Circle(x_, y_, sizes[i]);
        circles_[i] = newCircle;
        x_ += gap;
    }
}