#include "Circle.h"

Circle::Circle(float x, float y, float radius)
    : x_(x), y_(y), radius_(radius)
{
}

float Circle::followPoint(float targetX, float targetY, uint32_t width, uint32_t height) {
    float x = lerp(x_, targetX, 0.1f);
    float y = lerp(y_, targetY, 0.1f);

    float radian = findTangent({x_, y_}, {targetX, targetY}) + 0.5f * PI;
    float factor = map(
        dist(x_, y_, targetX, targetY),
        0,
        sqrt(pow(width, 2) + pow(height, 2)),
        0.0f, 1.0f
    );
    
    float xOffset = cos(radian) * factor;
    float yOffset = sin(radian) * factor;
    
    x_ = x + xOffset;
    y_ = y + yOffset;
    
    // Calculate acceleration (approximate)
    float acceleration = sqrt(pow(xOffset, 2) + pow(yOffset, 2));
    return acceleration;
}

void Circle::followBody(const Circle &target,
                        const Circle *targetOfTarget,
                        float gap, float smallestAngle,
                        float *oscillateRadian)
{
    applyPullingForce(target, gap, oscillateRadian);
    if (targetOfTarget) {
       applyAngleConstrain(target.getPosition(), targetOfTarget->getPosition(), gap, smallestAngle);
    }
}

void Circle::teleport(float x, float y) {
    x_ = x;
    y_ = y;
}

void Circle::applyPullingForce(const Circle &target, float gap, float *oscillateRadian) {
    float radian = findTangent(target.getPosition(), getPosition());
    if (oscillateRadian) radian += *oscillateRadian;

    float xOffset = gap * cos(radian);
    float yOffset = gap * sin(radian);
    x_ = target.x_ + xOffset;
    y_ = target.y_ + yOffset;
}

void Circle::applyAngleConstrain(const Point &center,
                                const Point &theOtherPoint,
                                float gap, float smallestAngle)
{
    float radianDelta = findAngleBetween(center, getPosition(), theOtherPoint);

    if (radianDelta < smallestAngle) {
        float theOtherPointRadian = findTangent(center, theOtherPoint);
        float radian;
        if (isOnLeft(center, theOtherPoint, getPosition())) {
            radian = theOtherPointRadian + smallestAngle;
        } else {
            radian = theOtherPointRadian - smallestAngle;
        }
        x_ = center.x + gap * cos(radian);
        y_ = center.y + gap * sin(radian);
    }

    if (radianDelta != PI) {
        float targetRadian = findTangent(theOtherPoint, center) + PI;
        float currentRadian = findTangent(center, getPosition());
        float radianDiff = abs(targetRadian - currentRadian);
        float radian;
        
        // Simple easing/correction
        if (isOnLeft(center, theOtherPoint, getPosition())) {
            radian = currentRadian + radianDiff * 0.001f;
        } else {
            radian = currentRadian - radianDiff * 0.001f;
        }
        x_ = center.x + gap * cos(radian);
        y_ = center.y + gap * sin(radian);
    }
}

Point Circle::getPosition() const {
    return {x_, y_};
}