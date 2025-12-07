#include "Fish.h"

Fish::Fish(float x, float y, float length, float width, int canvasWidth, int canvasHeight) {
    gap_ = length / (float)bodyPoints.size();
    float smallestAngle = 165.0f;
    
    std::vector<float> sizes;
    for(float p : bodyPoints) sizes.push_back(p * width);
    
    body_ = Chain(x, y, gap_, smallestAngle, sizes);
    cube_ = Cube(x, y, width * 0.15f);

    // Fins
    int finPos[] = {2, 2, 6, 6};
    float finRadianBase = PI / 1.8f;
    for (int i = 0; i < 4; i++) {
        int pos = finPos[i];
        float finFactor = bodyPoints[pos] * 0.8f;
        std::vector<float> fSizes;
        for(float p : finPoints) fSizes.push_back(p * width * (i <= 1 ? 1.5f : 1.0f) * finFactor);

        float angle = (i <= 1 ? 175.0f : 155.0f) + 20.0f * (width / length);
        Chain newFin(x, y, gap_ * 2.5f * (width / length), angle, fSizes);
        
        float radian = finRadianBase * (i % 2 == 0 ? 1 : -1) * finFactor;
        fins_.push_back({newFin, radian, pos});
    }

    // Tails
    float tailRadian = PI / 5.0f;
    int tailPos[] = {12, 12};
    for (int i = 0; i < 2; i++) {
        int pos = tailPos[i];
        std::vector<float> tSizes;
        for(float p : tailPoints) tSizes.push_back(width * p);

        Chain newTail(x, y, gap_ * 0.5f * random(0.7f, 0.9f), 120.0f, tSizes);
        float radian = random(0.0f, tailRadian) * (i % 2 == 0 ? 1 : -1);
        tails_.push_back({newTail, radian, pos});
    }

    // Back Fin
    int backFinPos = 3;
    std::vector<float> bfSizes = {0,0,0}; 
    Chain newBackFin(x, y, gap_ * 1.5f, 120.0f, bfSizes);
    backFins_.push_back({newBackFin, 0.0f, backFinPos});
}

void Fish::update(int width, int height) {
    cube_.update(width, height);
    Point pos = cube_.getPosition();
    
    body_.freeMove(pos.x, pos.y, width, height);

    for (auto& bf : backFins_) {
        Point start = body_.getCircle(bf.position).getPosition();
        Point next = body_.getCircle(bf.position + 1).getPosition();
        float radian = findTangent(start, next) + bf.radian;
        bf.fin.constrainMove(start.x, start.y, radian, 0.0f);
    }

    for (int i = 0; i < fins_.size(); i++) {
        auto& f = fins_[i];
        Point start = body_.getCircle(f.position).getPosition();
        Point next = body_.getCircle(f.position + 1).getPosition();
        float radian = findTangent(start, next) + f.radian;
        f.fin.constrainMove(start.x, start.y, radian, (i <= 1 ? 0.3f : 0.8f));
    }

    for (auto& t : tails_) {
        Point start = body_.getCircle(t.position).getPosition();
        Point next = body_.getCircle(t.position + 1).getPosition();
        float radian = findTangent(start, next) + t.radian;
        t.fin.constrainMove(start.x, start.y, radian, 0.3f);
    }
}

void Fish::triggerDash() {
    float angle = atan2(cube_.vY * cube_.directionY, cube_.vX * cube_.directionX);
    cube_.dash(angle);
}

// --- Getters for Collision Logic ---

Point Fish::getPosition() const {
    return cube_.getPosition();
}

float Fish::getVelocity() const {
    return sqrt(pow(cube_.vX, 2) + pow(cube_.vY, 2));
}

float Fish::getWidth() const {
    // Return approximate width based on the middle body segment (index ~4)
    // radius * 2 = diameter
    if (body_.getLength() > 4) {
        return ((Circle&)body_.circles_[4]).getRadius() * 2.0f;
    }
    return 10.0f; // Fallback
}

// --- Drawing ---

void Fish::draw(LGFX_Sprite* sprite) {
    uint32_t bodyColor   = sprite->color565(29, 29, 29);
    uint32_t finColor    = sprite->color565(30, 30, 30);
    uint32_t tailColor   = sprite->color565(30, 30, 30);
    
    for (auto& f : fins_) f.fin.drawOutline(sprite, finColor);
    for (auto& t : tails_) t.fin.drawOutline(sprite, tailColor);

    body_.drawOutline(sprite, bodyColor);

    drawBackFin(sprite);
    drawEyes(sprite);
}

void Fish::drawBackFin(LGFX_Sprite* ctx) {
    for (auto& bf : backFins_) {
        int endPosition = bf.position + backFinPoints.size() + 1;
        if(endPosition >= body_.getLength()) continue;

        Point finPoint = bf.fin.getCircle(backFinPoints.size() - 1).getPosition();
        Point startPoint = body_.getCircle(bf.position + 1).getPosition();
        Point endPoint = body_.getCircle(endPosition).getPosition();

        drawQuadraticBezier(ctx, startPoint.x, startPoint.y, finPoint.x, finPoint.y, endPoint.x, endPoint.y, TFT_DARKGREY);

        for (int i = endPosition; i >= bf.position + 2; i--) {
            Point pCurr = body_.getCircle(i).getPosition();
            Point pPrev = body_.getCircle(i-1).getPosition();
            Point mid = {(pCurr.x + pPrev.x)/2.0f, (pCurr.y + pPrev.y)/2.0f};
            ctx->drawLine((int)pCurr.x, (int)pCurr.y, (int)mid.x, (int)mid.y, TFT_DARKGREY);
        }
    }
}

void Fish::drawEyes(LGFX_Sprite* ctx) {
    Point p0 = body_.getCircle(0).getPosition();
    Point p1 = body_.getCircle(1).getPosition();
    float radian = findTangent(p0, p1);
    float eyeDist = body_.getCircle(2).getRadius(); // Approx dist
    float eyeSize = gap_ * 0.4f;

    auto drawEye = [&](float rad) {
        float dx = eyeDist * cos(rad);
        float dy = eyeDist * sin(rad);
        ctx->fillCircle((int)(p0.x + dx), (int)(p0.y + dy), (int)eyeSize, TFT_WHITE);
    };

    drawEye(radian + PI / 4.0f);
    drawEye(radian - PI / 4.0f);
}