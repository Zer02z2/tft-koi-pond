#pragma once
#include <Arduino.h>
#include <cmath>
#include "../helper.h"

class Circle {
    public:
        Circle(uint32_t x, uint32_t y, uint32_t radius);

        float followPoint(float targetX, float targetY, uint32_t width, uint32_t height);
        void followBody(const Circle &target,
                        const Circle *targetOfTarget,
                        float gap, float smallestAngle,
                        float *oscillateRadian = nullptr);
        
        void teleport(float x, float y);
        void applyPullingForce(const Circle &target, float gap, float *oscillateRadian = nullptr);
        void applyAngleConstrain(const Point &center,
                                const Point &theOtherPoint,
                                float gap, float smallestAngle);

        Point getPosition() const;

    private:
        float x_;
        float y_;
        float radius_;
};