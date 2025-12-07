#include "helper.h"

float findAngleBetween(const Point &pointCenter, const Point &pointA, const Point &pointB) {
    float vectorAx = pointA.x - pointCenter.x;
    float vectorAy = pointA.y - pointCenter.y;
    float vectorBx = pointB.x - pointCenter.x;
    float vectorBy = pointB.y - pointCenter.y;

    float vectorDotProduct = vectorAx * vectorBx + vectorAy * vectorBy;
    float vectorLengthProduct = sqrt(pow(vectorAx, 2) + pow(vectorAy, 2)) * sqrt(pow(vectorBx, 2) + pow(vectorBy, 2));
    if (vectorLengthProduct == 0) {
        return 0.0f; // Avoid division by zero
    }
    else {
        float radianDelta = acos(vectorDotProduct / vectorLengthProduct);
        return radianDelta;
    }
}

float findTangent(const Point &pointA, const Point &pointB) {
    float deltaX = pointB.x - pointA.x;
    float deltaY = pointB.y - pointA.y;

    float radian = atan2(deltaY, deltaX);
    return radian;
}

bool isOnLeft(const Point &pointA, const Point &pointB, const Point &pointC) {
    // Return if Point C is left of Point A to Point B line
    float crossProduct = (pointB.x - pointA.x) * (pointC.y - pointA.y) - (pointB.y - pointA.y) * (pointC.x - pointA.x);
    return crossProduct > 0;
}

Point findPosition(const Point &point, float radian, float length){
    auto& [x, y] = point;
    Point result;
    result.x = x + length * cos(radian);
    result.y = y + length * sin(radian);
    return result;
}

float lerp(float start, float end, float t) {
    return start + t * (end - start);
}

float map(float value, float inMin, float inMax, float outMin, float outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

float dist(float &x1, float &y1, float &x2, float &y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void drawCircle(LGFX_Sprite* sprite, int x, int y, int radius, uint32_t fillColor, uint32_t strokeColor) {
    sprite->startWrite();
    sprite->fillCircle(x, y, radius, fillColor);
    sprite->drawCircle(x, y, radius, strokeColor);
    sprite->endWrite();
}

void line(LGFX_Sprite* sprite, int x1, int y1, int x2, int y2, uint32_t color){
    sprite->startWrite();
    sprite->drawLine(x1, y1, x2, y2, color);
    sprite->endWrite();
}

void rect(LGFX_Sprite* sprite, int x, int y, int w, int h, uint32_t strokeColor){
    sprite->startWrite();
    sprite->drawRect(x, y, w, h, strokeColor);
    sprite->endWrite();
}

float random(float min, float max) {
    return min + (max - min) * static_cast<float>(rand()) / RAND_MAX;
}

template<typename T, size_t N>
int getRandom(T (&array)[N]) {
    return rand() % N;
}


Point normalizeVector(const Point &vector, float magnitude) {
    float radian = findTangent({0,0}, vector);
    float vX = magnitude * cos(radian);
    float vY = magnitude * sin(radian);
    return {vX, vY};
}