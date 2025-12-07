#pragma once
#include <cmath>
#include <LovyanGFX.hpp>

// Constants
#ifndef PI
#define PI 3.14159265358979323846f
#endif

struct Point {
    float x;
    float y;
};

// Math Helpers
float findAngleBetween(const Point &pointCenter, const Point &pointA, const Point &pointB);
float findTangent(const Point &pointA, const Point &pointB);
bool isOnLeft(const Point &pointA, const Point &pointB, const Point &pointC);
Point findPosition(const Point &point, float radian, float length);
Point normalizeVector(const Point &vector, float magnitude);

float lerp(float start, float end, float t);
float map(float value, float inMin, float inMax, float outMin, float outMax);
float dist(float x1, float y1, float x2, float y2);

// Drawing Helpers
void drawQuadraticBezier(LGFX_Sprite* sprite, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color);
void fillQuadraticBezier(LGFX_Sprite* sprite, Point anchor, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color);
