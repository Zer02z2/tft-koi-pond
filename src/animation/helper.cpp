#include "helper.h"

float findAngleBetween(const Point &pointCenter, const Point &pointA, const Point &pointB) {
    float vectorAx = pointA.x - pointCenter.x;
    float vectorAy = pointA.y - pointCenter.y;
    float vectorBx = pointB.x - pointCenter.x;
    float vectorBy = pointB.y - pointCenter.y;

    float vectorDotProduct = vectorAx * vectorBx + vectorAy * vectorBy;
    float vectorLengthProduct = sqrt(pow(vectorAx, 2) + pow(vectorAy, 2)) * sqrt(pow(vectorBx, 2) + pow(vectorBy, 2));
    
    if (vectorLengthProduct == 0) {
        return 0.0f;
    } else {
        // --- FIX: Clamp the value to the valid range for acos [-1, 1] ---
        float value = vectorDotProduct / vectorLengthProduct;
        if (value > 1.0f) value = 1.0f;
        if (value < -1.0f) value = -1.0f;
        
        return acos(value);
    }
}

float findTangent(const Point &pointA, const Point &pointB) {
    return atan2(pointB.y - pointA.y, pointB.x - pointA.x);
}

bool isOnLeft(const Point &pointA, const Point &pointB, const Point &pointC) {
    return ((pointB.x - pointA.x) * (pointC.y - pointA.y) - (pointB.y - pointA.y) * (pointC.x - pointA.x)) > 0;
}

Point findPosition(const Point &point, float radian, float length){
    return { point.x + length * cos(radian), point.y + length * sin(radian) };
}

Point normalizeVector(const Point &vector, float magnitude) {
    float radian = atan2(vector.y, vector.x);
    return { magnitude * cos(radian), magnitude * sin(radian) };
}

float lerp(float start, float end, float t) {
    return start + t * (end - start);
}

float map(float value, float inMin, float inMax, float outMin, float outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

float dist(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void drawQuadraticBezier(LGFX_Sprite* sprite, float x0, float y0, float x1, float y1, float x2, float y2, uint16_t color) {
    float oldX = x0;
    float oldY = y0;
    // Lower step = smoother but slower. 0.1 is a good balance.
    for (float t = 0.1f; t <= 1.0f; t += 0.1f) {
        float invT = 1.0f - t;
        float x = invT * invT * x0 + 2 * invT * t * x1 + t * t * x2;
        float y = invT * invT * y0 + 2 * invT * t * y1 + t * t * y2;
        sprite->drawLine((int)oldX, (int)oldY, (int)x, (int)y, color);
        oldX = x;
        oldY = y;
    }
}

void fillQuadraticBezier(LGFX_Sprite* sprite, Point anchor, float x0, float y0, float x1, float y1, float x2, float y2, uint16_t color) {
    float oldX = x0;
    float oldY = y0;
    // Step 0.1 gives 10 triangles per curve. Decrease for higher quality.
    for (float t = 0.1f; t <= 1.0f; t += 0.1f) {
        float invT = 1.0f - t;
        float x = invT * invT * x0 + 2 * invT * t * x1 + t * t * x2;
        float y = invT * invT * y0 + 2 * invT * t * y1 + t * t * y2;
        
        // Draw a filled triangle from the anchor to the current line segment
        sprite->fillTriangle((int)anchor.x, (int)anchor.y, (int)oldX, (int)oldY, (int)x, (int)y, color);
        
        oldX = x;
        oldY = y;
    }
}

float randomFloat(float minValue, float maxValue) {
    return minValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxValue - minValue)));
}