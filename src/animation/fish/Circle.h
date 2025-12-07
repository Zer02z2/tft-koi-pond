#pragma once
#include <Arduino.h>
#include <cmath>
#include "../helper.h"

class Circle {
    public:
        // Default constructor
        Circle() : x_(0), y_(0), radius_(0) {}
        
        Circle(float x, float y, float radius);

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
        float getRadius() const { return radius_; }

    private:
        float x_;
        float y_;
        float radius_;
};