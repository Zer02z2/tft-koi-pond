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
    frameCount_ += 25.0f * log(0.3f * acceleration + 1.0f);

    float oscillateScale = (PI / 5.0f) * log(2.0f * acceleration + 1.0f);

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

void Chain::draw(LGFX_Sprite* sprite, uint32_t fillColor, uint32_t strokeColor) {
    std::vector<Point> leftPoints;
    std::vector<Point> rightPoints;
    
    if(length_ < 2) return;

    // --- 1. Calculate Geometry ---
    for (int i = 0; i < length_; i++) {
        float radian = 0;
        if (i != 0 && i != length_ - 1) {
            float radDelta = findAngleBetween(circles_[i].getPosition(), circles_[i + 1].getPosition(), circles_[i - 1].getPosition());
            float radAlpha = findTangent(circles_[i].getPosition(), circles_[i - 1].getPosition());
            if (isOnLeft(circles_[i].getPosition(), circles_[i + 1].getPosition(), circles_[i - 1].getPosition())) {
                radian = radAlpha - radDelta / 2.0f;
            } else {
                radian = radAlpha - (2.0f * PI - radDelta) / 2.0f;
            }
        } else if (i == 0) {
            radian = findTangent(circles_[i].getPosition(), circles_[i + 1].getPosition()) + 0.5f * PI;
        } else if (i == length_ - 1) {
            radian = findTangent(circles_[i].getPosition(), circles_[i - 1].getPosition()) - 0.5f * PI;
        }

        leftPoints.push_back(calculatePoint(circles_[i], radian));
        rightPoints.push_back(calculatePoint(circles_[i], radian + PI)); // Opposite side
    }

    // --- 2. Draw Fill (Triangle Strip) ---
    for (int i = 0; i < length_ - 1; i++) {
        Point l1 = leftPoints[i];
        Point r1 = rightPoints[i];
        Point l2 = leftPoints[i+1];
        Point r2 = rightPoints[i+1];

        // Fill two triangles to form the quad between segments
        sprite->fillTriangle((int)l1.x, (int)l1.y, (int)r1.x, (int)r1.y, (int)l2.x, (int)l2.y, fillColor);
        sprite->fillTriangle((int)r1.x, (int)r1.y, (int)l2.x, (int)l2.y, (int)r2.x, (int)r2.y, fillColor);
    }


    // --- 3. Draw Outline (Bezier Loop) ---
    std::vector<Point> outlinePoints;
    
    // Add Left side points (Head -> Tail)
    // Optional: Add Head cap point logic from TS if needed, but simple loop is usually fine
    if(length_ > 0) {
        float headRad = findTangent(circles_[0].getPosition(), circles_[1].getPosition()) - 0.5f * PI;
        outlinePoints.push_back(calculatePoint(circles_[0], headRad)); // Nose
    }
    for(const auto& p : leftPoints) outlinePoints.push_back(p);

    // Add Right side points (Tail -> Head)
    for (int i = length_ - 1; i >= 0; i--) {
        outlinePoints.push_back(rightPoints[i]);
    }
    // Close the loop
    outlinePoints.push_back(outlinePoints[0]);

    // Draw Smooth Curve through points
    int len = outlinePoints.size();
    if(len < 2) return;
    
    Point pStart = { (outlinePoints[0].x + outlinePoints[1].x)/2.0f, (outlinePoints[0].y + outlinePoints[1].y)/2.0f };
    
    for (int i = 1; i < len - 1; i++) {
        Point pControl = outlinePoints[i];
        Point pEnd = { (outlinePoints[i].x + outlinePoints[i+1].x)/2.0f, (outlinePoints[i].y + outlinePoints[i+1].y)/2.0f };
        
        drawQuadraticBezier(sprite, pStart.x, pStart.y, pControl.x, pControl.y, pEnd.x, pEnd.y, strokeColor);
        pStart = pEnd;
    }
    // Close final segment
    Point lastP = outlinePoints[len-1];
    Point firstMid = { (outlinePoints[0].x + outlinePoints[1].x)/2.0f, (outlinePoints[0].y + outlinePoints[1].y)/2.0f };
    drawQuadraticBezier(sprite, pStart.x, pStart.y, lastP.x, lastP.y, firstMid.x, firstMid.y, strokeColor);
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