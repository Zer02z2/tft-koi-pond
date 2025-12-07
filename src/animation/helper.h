#pragma once
#include <cmath>
#include <LovyanGFX.hpp>

struct Point {
    float x;
    float y;
};

float findAngleBetween(const Point &pointCenter, const Point &pointA, const Point &pointB);
float findTangent(const Point &pointA, const Point &pointB);
bool isOnLeft(const Point &pointA, const Point &pointB, const Point &pointC);
Point findPosition(const Point &point, float radian, float length);
Point normalizeVector(const Point &vector, float magnitude);

float lerp(float start, float end, float t);
float map(float value, float inMin, float inMax, float outMin, float outMax);
float dist(float &x1, float &y1, float &x2, float &y2);

void drawCircle(LGFX_Sprite* sprite, int x, int y, int radius, uint32_t fillColor, uint32_t strokeColor);
void line(LGFX_Sprite* sprite, int x1, int y1, int x2, int y2, uint32_t color);
void rect(LGFX_Sprite* sprite, int x, int y, int w, int h, uint32_t strokeColor);

float random(float min, float max);
template<typename T, size_t N>
int getRandom(T (&array)[N]);