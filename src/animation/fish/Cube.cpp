#include "Cube.h"

Cube::Cube(float x, float y, float vMax) : x_(x), y_(y), vMax(vMax) {
    vMin = vMax * 0.1f;
    vDash = vMax * 2.0f;
    vX = random(0.0f, vMax);
    vY = random(0.0f, vMax);
    w_ = 10.0f; // Arbitrary size for bounds
    h_ = 10.0f;
    directionX = (random(0.0f, 1.0f) < 0.5f) ? 1 : -1;
    directionY = (random(0.0f, 1.0f) < 0.5f) ? 1 : -1;
}

void Cube::update(int xBound, int yBound) {
    if (random(0.0f, 1.0f) < pBoost_ || (vX - vMin) <= 0.002f) {
        vX = boostVelocity();
        directionX *= (random(0.0f, 1.0f) < 0.2f) ? -1 : 1;
    }
    if (random(0.0f, 1.0f) < pBoost_ || (vY - vMin) <= 0.002f) {
        vY = boostVelocity();
        directionY *= (random(0.0f, 1.0f) < 0.2f) ? -1 : 1;
    }

    if (vX > vMin) vX -= (vX - vMin) * random(0.01f, 0.02f);
    if (vY > vMin) vY -= (vY - vMin) * random(0.01f, 0.02f);

    if (random(0.0f, 1.0f) < pDirectionChange_) directionX *= -1;
    if (random(0.0f, 1.0f) < pDirectionChange_) directionY *= -1;

    x_ += vX * directionX;
    y_ += vY * directionY;
    preventOverBoarder(xBound, yBound);
}

float Cube::boostVelocity() {
    return random(vMax / 2.0f, vMax);
}

void Cube::preventOverBoarder(int xBound, int yBound) {
    if (x_ + w_ / 2.0f >= xBound) {
        x_ = xBound - w_ / 2.0f;
        directionX *= -1;
    } else if (x_ - w_ / 2.0f <= 0) {
        x_ = w_ / 2.0f;
        directionX *= -1;
    } else if (y_ + h_ / 2.0f >= yBound) {
        y_ = yBound - h_ / 2.0f;
        directionY *= -1;
    } else if (y_ - h_ / 2.0f <= 0) {
        y_ = h_ / 2.0f;
        directionY *= -1;
    }
}

void Cube::dash(float radian) {
    float velX = vDash * cos(radian);
    float velY = vDash * sin(radian);
    vX = abs(velX);
    vY = abs(velY);
    directionX = (velX >= 0) ? 1 : -1;
    directionY = (velY >= 0) ? 1 : -1;
}

Point Cube::getPosition() const {
    return {x_, y_};
}