#include "DuckWeed.h"

DuckWeed::DuckWeed(float x, float y, float radius, int segments)
    : radius_(radius), xCur_(x), yCur_(y), xTar_(x), yTar_(y)
{
    float firstPointRadian = random(0, 2 * PI);
    float segmentRadian = (2 * PI) / segments;

    for (int i = 0; i < segments; i++) {
        float len = random(radius * 0.98f, radius * 1.02f);
        float radian = firstPointRadian + segmentRadian * i;
        points_.push_back({len, radian});
    }
    
    vectorMax_ = radius * 0.1f;
}

void DuckWeed::update(int width, int height) {
    xTar_ += moveVector_.x;
    yTar_ += moveVector_.y;

    // Bounds check
    if (xTar_ < 0) xTar_ = 0;
    if (xTar_ > width) xTar_ = width;
    if (yTar_ < 0) yTar_ = 0;
    if (yTar_ > height) yTar_ = height;

    moveVector_.x *= 0.99f;
    moveVector_.y *= 0.99f;
}

void DuckWeed::applyVector(float x, float y, float strength) {
    Point distVec = { xCur_ - x, yCur_ - y };
    Point newVec = normalizeVector(distVec, strength);
    
    Point resultVec = { newVec.x + moveVector_.x, newVec.y + moveVector_.y };
    float mag = sqrt(pow(resultVec.x, 2) + pow(resultVec.y, 2));
    
    if (mag > vectorMax_) {
        resultVec = normalizeVector(resultVec, vectorMax_);
    }
    moveVector_ = resultVec;
}

void DuckWeed::draw(LGFX_Sprite* sprite) {
    xCur_ += (xTar_ - xCur_) * 0.1f;
    yCur_ += (yTar_ - yCur_) * 0.1f;

    if (points_.empty()) return;

    std::vector<Point> renderPoints;
    for (const auto& p : points_) {
        renderPoints.push_back(findPosition({xCur_, yCur_}, p.radian, p.length));
    }

    uint32_t weedColor = sprite->color565(93, 206, 71); 

    int len = renderPoints.size();
    Point pLast = renderPoints[len - 1];
    Point pStart = { (renderPoints[0].x + pLast.x)/2.0f, (renderPoints[0].y + pLast.y)/2.0f };
    Point currentP = pStart;

    for (int i = 0; i < len - 1; i++) {
        Point p1 = renderPoints[i];
        Point p2 = renderPoints[i+1];
        Point mid = { (p1.x + p2.x)/2.0f, (p1.y + p2.y)/2.0f };
        drawQuadraticBezier(sprite, currentP.x, currentP.y, p1.x, p1.y, mid.x, mid.y, weedColor);
        currentP = mid;
    }
    
    Point pEnd = renderPoints[len-1];
    drawQuadraticBezier(sprite, currentP.x, currentP.y, pEnd.x, pEnd.y, pStart.x, pStart.y, weedColor);
}

Point DuckWeed::getPosition() const {
    return {xCur_, yCur_};
}