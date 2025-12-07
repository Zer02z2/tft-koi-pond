#include "Leaf.h"

Leaf::Leaf(float x, float y, float radius, int segments) 
    : radius_(radius), xOrg_(x), yOrg_(y), xCur_(x), yCur_(y), xTar_(x), yTar_(y)
{
    float firstPointRadian = random(0, 2 * PI);
    float segmentRadian = (2 * PI) / segments;

    for (int i = 0; i < segments; i++) {
        float len = (i == 0) ? random(radius * 0.1f, radius * 0.2f) : random(radius * 0.98f, radius * 1.02f);
        float radian = firstPointRadian + segmentRadian * i;
        points_.push_back({len, radian});
    }
    
    oscillateMax_ = radius * 0.4f;
}

void Leaf::update() {
    float acc = sqrt(pow(oscillateVector_.x, 2) + pow(oscillateVector_.y, 2));
    frameCount_ += 0.1f * log(0.01f * acc + 1.0f);
    
    float xOffset = sin(frameCount_) * oscillateVector_.x;
    float yOffset = sin(frameCount_) * oscillateVector_.y;
    
    xTar_ = xOrg_ + xOffset;
    yTar_ = yOrg_ + yOffset;

    oscillateVector_.x *= 0.99f;
    oscillateVector_.y *= 0.99f;
}

void Leaf::applyOscillation(float x, float y, float strength) {
    Point distVec = { xCur_ - x, yCur_ - y };
    Point newVec = normalizeVector(distVec, strength);
    
    Point resultVec = { newVec.x + oscillateVector_.x, newVec.y + oscillateVector_.y };
    float mag = sqrt(pow(resultVec.x, 2) + pow(resultVec.y, 2));
    
    if (mag > oscillateMax_) {
        resultVec = normalizeVector(resultVec, oscillateMax_);
    }
    oscillateVector_ = resultVec;
}

void Leaf::draw(LGFX_Sprite* sprite) {
    xCur_ += (xTar_ - xCur_) * 0.1f;
    yCur_ += (yTar_ - yCur_) * 0.1f;

    if (points_.empty()) return;

    std::vector<Point> renderPoints;
    for (const auto& p : points_) {
        renderPoints.push_back(findPosition({xCur_, yCur_}, p.radian, p.length));
    }

    // Colors (Hardcoded based on TS style)
    uint32_t leafColor = sprite->color565(62, 145, 60); 
    // Outline color in TS was background, here we skip or use dark
    
    int len = renderPoints.size();
    Point pLast = renderPoints[len - 1];
    Point pStart = { (renderPoints[0].x + pLast.x)/2.0f, (renderPoints[0].y + pLast.y)/2.0f };
    
    Point currentP = pStart;

    // Drawing closed loop
    for (int i = 0; i < len - 1; i++) {
        Point p1 = renderPoints[i];
        Point p2 = renderPoints[i+1];
        Point mid = { (p1.x + p2.x)/2.0f, (p1.y + p2.y)/2.0f };
        
        drawQuadraticBezier(sprite, currentP.x, currentP.y, p1.x, p1.y, mid.x, mid.y, leafColor);
        currentP = mid;
    }
    
    // Close the loop
    Point pEnd = renderPoints[len-1];
    drawQuadraticBezier(sprite, currentP.x, currentP.y, pEnd.x, pEnd.y, pStart.x, pStart.y, leafColor);
}

Point Leaf::getPosition() const {
    return {xCur_, yCur_};
}