#include "Chain.h"

Chain::Chain(float x, float y, float gap, float angle, const std::vector<float>& sizes)
    : x_(x), y_(y), gap_(gap)
{
    smallestAngle_ = (angle * PI) / 180.0f;
    length_ = sizes.size();
    if(length_ > MAX_CHAIN_LENGTH) length_ = MAX_CHAIN_LENGTH;

    for (int i = 0; i < length_; ++i) {
        // sizes[i] in TS is diameter, Circle takes radius
        circles_[i] = Circle(x_, y_, sizes[i] / 2.0f);
        x_ += gap;
    }
}

void Chain::freeMove(float x, float y, int width, int height) {
    float acceleration = circles_[0].followPoint(x, y, width, height);
    frameCount_ += 25.0f * log(0.15f * acceleration + 1.0f);

    float oscillateScale = (PI / 5.0f) * log(1.0f * acceleration + 1.0f);

    for (int i = 1; i < length_; i++) {
        float oscillateOffset = i * length_ * PI * 1.1368f;
        float mapVal = map((float)i, 0.0f, (float)length_, 0.5f, 3.0f);
        
        float oscillateRadian = sin(frameCount_ + oscillateOffset) * oscillateScale * mapVal;

        Circle* targetOfTarget = (i >= 2) ? &circles_[i - 2] : nullptr;
        
        circles_[i].followBody(circles_[i - 1], targetOfTarget, gap_, smallestAngle_, &oscillateRadian);
    }
}

void Chain::constrainMove(float x, float y, float idealRadian, float constrainStrength) {
    circles_[0].teleport(x, y);
    float idealDisplaceX = gap_ * cos(idealRadian);
    float idealDisplaceY = gap_ * sin(idealRadian);
    Point idealPosition = { x + idealDisplaceX, y + idealDisplaceY };

    Point pos0 = {x, y};
    Point pos1 = circles_[1].getPosition();

    float deltaRadian = findAngleBetween(pos0, pos1, idealPosition);
    float currentRadian = findTangent(pos0, pos1);
    
    int dir = isOnLeft(pos0, idealPosition, pos1) ? -1 : 1;
    float radian = currentRadian + dir * deltaRadian * constrainStrength;

    float displaceX = gap_ * cos(radian);
    float displaceY = gap_ * sin(radian);
    
    circles_[1].teleport(x + displaceX, y + displaceY);

    for (int i = 2; i < length_; i++) {
        Circle* targetOfTarget = (i >= 2) ? &circles_[i - 2] : nullptr;
        circles_[i].followBody(circles_[i - 1], targetOfTarget, gap_, smallestAngle_);
    }
}

void Chain::simpleMove(float x, float y, int width, int height) {
    circles_[0].followPoint(x, y, width, height);
    for (int i = 1; i < length_; i++) {
        Circle* targetOfTarget = (i >= 2) ? &circles_[i - 2] : nullptr;
        circles_[i].followBody(circles_[i - 1], targetOfTarget, gap_, smallestAngle_);
    }
}

Point Chain::calculatePoint(const Circle& circle, float radian) {
    // Circle stores radius, TS code used diameter/2. 
    // radius_ is already d/2.
    Point pos = circle.getPosition();
    float r = circle.getRadius(); 
    return { pos.x + r * cos(radian), pos.y + r * sin(radian) };
}

void Chain::drawOutline(LGFX_Sprite* sprite, uint32_t color) {
    std::vector<Point> points;
    
    // 1. Left Side
    for (int i = 0; i < length_; i++) {
        float radian = 0;
        if (i != 0 && i != length_ - 1) {
            float radianDelta = findAngleBetween(circles_[i].getPosition(), circles_[i + 1].getPosition(), circles_[i - 1].getPosition());
            float radianAlpha = findTangent(circles_[i].getPosition(), circles_[i - 1].getPosition());
            
            if (isOnLeft(circles_[i].getPosition(), circles_[i + 1].getPosition(), circles_[i - 1].getPosition())) {
                radian = radianAlpha - radianDelta / 2.0f;
            } else {
                radian = radianAlpha - (2.0f * PI - radianDelta) / 2.0f;
            }
        } else if (i == 0) {
            radian = findTangent(circles_[i].getPosition(), circles_[i + 1].getPosition()) + 0.5f * PI;
        } else if (i == length_ - 1) {
            radian = findTangent(circles_[i].getPosition(), circles_[i - 1].getPosition()) - 0.5f * PI;
        }

        Point p = calculatePoint(circles_[i], radian);
        if (i == 0) {
            float headRadian = findTangent(circles_[i].getPosition(), circles_[i + 1].getPosition()) - 0.5f * PI;
            points.push_back(calculatePoint(circles_[i], headRadian));
        }
        points.push_back(p);
    }

    // 2. Right Side
    for (int i = length_ - 1; i >= 0; i--) {
        float radian = 0;
        if (i != 0 && i != length_ - 1) {
            float radianDelta = findAngleBetween(circles_[i].getPosition(), circles_[i - 1].getPosition(), circles_[i + 1].getPosition());
            float radianAlpha = findTangent(circles_[i].getPosition(), circles_[i + 1].getPosition());

            if (isOnLeft(circles_[i].getPosition(), circles_[i - 1].getPosition(), circles_[i + 1].getPosition())) {
                radian = radianAlpha - radianDelta / 2.0f;
            } else {
                radian = radianAlpha - (2.0f * PI - radianDelta) / 2.0f;
            }
            if (i == 1) {
                points.insert(points.begin(), calculatePoint(circles_[i], radian));
                continue;
            }
        } else if (i == 0) {
            radian = findTangent(circles_[i].getPosition(), circles_[i + 1].getPosition()) - 0.5f * PI;
        } else if (i == length_ - 1) {
            radian = findTangent(circles_[i].getPosition(), circles_[i - 1].getPosition()) + 0.5f * PI;
        }
        points.push_back(calculatePoint(circles_[i], radian));
    }

    // 3. Draw Loop
    int len = points.size();
    if(len < 2) return;
    
    // Draw using Quadratic Bezier approximation
    // Start from midpoint of first two points
    Point pStart = { (points[0].x + points[len-1].x)/2.0f, (points[0].y + points[len-1].y)/2.0f };
    
    for (int i = 1; i < len - 2; i++) {
        Point pControl = points[i];
        Point pEnd = { (points[i].x + points[i+1].x)/2.0f, (points[i].y + points[i+1].y)/2.0f };
        
        drawQuadraticBezier(sprite, pStart.x, pStart.y, pControl.x, pControl.y, pEnd.x, pEnd.y, color);
        pStart = pEnd;
    }
    // Close the loop roughly (or refine logic)
    drawQuadraticBezier(sprite, pStart.x, pStart.y, points[len-1].x, points[len-1].y, (points[0].x + points[len-1].x)/2.0f, (points[0].y + points[len-1].y)/2.0f, color);
}

void Chain::drawRig(LGFX_Sprite* sprite, uint32_t color) {
    for (int i = 0; i < length_; ++i) {
        Point p = circles_[i].getPosition();
        sprite->drawCircle((int)p.x, (int)p.y, (int)circles_[i].getRadius(), color);
        if(i < length_ - 1) {
            Point pNext = circles_[i+1].getPosition();
            sprite->drawLine((int)p.x, (int)p.y, (int)pNext.x, (int)pNext.y, color);
        }
    }
}

Circle& Chain::getCircle(int index) {
    return circles_[index];
}