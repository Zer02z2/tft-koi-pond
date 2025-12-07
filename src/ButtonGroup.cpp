#include "ButtonGroup.h"

// GCC builtin for population count
#define popcount(x) __builtin_popcountl(x)

ButtonGroup *ButtonGroup::instance_ = nullptr;

ButtonGroup::ButtonGroup(unsigned long debounce_ms, unsigned long long_ms)
    : debounceMs_(debounce_ms), longMs_(long_ms)
{
    for (uint8_t i = 0; i < MAX_BUTTONS; ++i) {
        pins_[i] = 0;
        lastRaw_[i] = true; // assume pullup wiring
        lastRawChangeMs_[i] = 0;
        pressStartAt_[i] = 0;
        longRegistered_[i] = false;
    }
}

void ButtonGroup::setLeftPin(uint8_t pin) { leftPin_ = pin; addIfMissing(pin); }
void ButtonGroup::setRightPin(uint8_t pin) { rightPin_ = pin; addIfMissing(pin); }
void ButtonGroup::setBottomPin(uint8_t pin) { bottomPin_ = pin; addIfMissing(pin); }

int8_t ButtonGroup::findIndex(uint8_t pin) const {
    for (uint8_t i = 0; i < count_; ++i) if (pins_[i] == pin) return i;
    return -1;
}

void ButtonGroup::addIfMissing(uint8_t pin) {
    if (pin == 0) return;
    if (findIndex(pin) >= 0) return;
    if (count_ >= MAX_BUTTONS) return;
    pins_[count_] = pin;
    lastRaw_[count_] = true;
    lastRawChangeMs_[count_] = 0;
    pressStartAt_[count_] = 0;
    longRegistered_[count_] = false;
    ++count_;
}

void ButtonGroup::begin() {
    instance_ = this;
    for (uint8_t i = 0; i < count_; ++i) {
        pinMode(pins_[i], INPUT_PULLUP);
        int irq = digitalPinToInterrupt(pins_[i]);
        if (irq != NOT_AN_INTERRUPT) attachInterrupt(irq, ButtonGroup::isr_handler, CHANGE);
    }
}

void ButtonGroup::end() {
    if (instance_ == this) instance_ = nullptr;
    for (uint8_t i = 0; i < count_; ++i) {
        int irq = digitalPinToInterrupt(pins_[i]);
        if (irq != NOT_AN_INTERRUPT) detachInterrupt(irq);
    }
}

// simple ISR: read all pins -> set mask and flag
void IRAM_ATTR ButtonGroup::isr_handler() {
    if (!instance_) return;
    uint32_t mask = 0;
    for (uint8_t i = 0; i < instance_->count_; ++i) {
        if (instance_->pins_[i] == 0) continue;
        if (digitalRead(instance_->pins_[i]) == LOW) mask |= (1UL << i);
    }
    instance_->isrMask_ = mask;
    instance_->isrChanged_ = true;
}

void ButtonGroup::service() {
    noInterrupts();
    bool changed = isrChanged_;
    uint32_t rawMask = isrMask_;
    isrChanged_ = false;
    interrupts();

    unsigned long now = millis();

    // 1. Update raw states
    for (uint8_t i = 0; i < count_; ++i) {
        bool raw = (rawMask & (1UL << i)) != 0;
        if (raw != lastRaw_[i]) {
            lastRaw_[i] = raw;
            lastRawChangeMs_[i] = now;
        }
    }

    bool stableChanged = false;

    // 2. Debounce Logic & Edge Detection
    for (uint8_t i = 0; i < count_; ++i) {
        bool raw = lastRaw_[i];
        bool stable = (stableMask_ & (1UL << i)) != 0;
        unsigned long changedAt = lastRawChangeMs_[i];

        // If raw state differs from stable, and enough time passed
        if (raw != stable && changedAt != 0 && (now - changedAt) >= debounceMs_) {
            
            // Generate Immediate Edge Reports (In/Out)
            // This happens EXACTLY when the button state is confirmed changed
            generateEdgeReport(i, raw);

            // Existing Logic
            if (raw) { // Pressed
                stableMask_ |= (1UL << i);
                pressStartAt_[i] = now;
                longRegistered_[i] = false;
                latchedMask_ |= (1UL << i);
                sessionActive_ = true;
            } else { // Released
                stableMask_ &= ~(1UL << i);
                pressStartAt_[i] = 0;
            }
            lastRawChangeMs_[i] = 0;
            stableChanged = true;
        }
    }

    // 3. Long Press Detection
    if (sessionActive_) {
        uint8_t sc = popcount(stableMask_);
        if (sc == 1) {
            for (uint8_t i = 0; i < count_; ++i) {
                if ((stableMask_ & (1UL << i)) == 0) continue;

                if (!longRegistered_[i]) {
                    unsigned long start = pressStartAt_[i];
                    if (start != 0 && (now - start) >= longMs_) {
                        longRegistered_[i] = true;
                        sessionHasLong_ = true;
                        emittedLongMask_ |= (1UL << i);
                        
                        Report r;
                        r.mask = (1UL << i);
                        r.count = 1;
                        r.longPress = true;
                        r.text = buildLabel((1UL << i), true);
                        queueReport(r);
                    }
                }
            }
        }
    }

    // 4. Session End (Existing Logic)
    if (stableChanged && stableMask_ == 0 && sessionActive_) { 
        // if all latched bits were already emitted as immediate longs, skip final report
        if (!(sessionHasLong_ && emittedLongMask_ != 0 && emittedLongMask_ == latchedMask_)) {
            Report r;
            r.mask = latchedMask_;
            r.count = popcount(latchedMask_);
            r.longPress = sessionHasLong_;
            r.text = buildLabel(latchedMask_, sessionHasLong_);
            queueReport(r);
        }
        
        // Reset session
        latchedMask_ = 0;
        sessionActive_ = false;
        sessionHasLong_ = false;
        emittedLongMask_ = 0;
        for (uint8_t i = 0; i < count_; ++i) {
            pressStartAt_[i] = 0;
            longRegistered_[i] = false;
        }
    }
}

bool ButtonGroup::poll(Report &out) {
    noInterrupts();
    bool empty = (qHead_ == qTail_);
    if (!empty) {
        out = reportQueue_[qTail_];
        qTail_ = (qTail_ + 1) % REPORT_QUEUE_SIZE;
    }
    interrupts();
    return !empty;
}

void ButtonGroup::queueReport(const Report &r) {
    noInterrupts();
    uint8_t next = (qHead_ + 1) % REPORT_QUEUE_SIZE;
    if (next != qTail_) {
        reportQueue_[qHead_] = r;
        qHead_ = next;
    }
    interrupts();
}

// Generate Immediate Edge Reports (In/Out)
void ButtonGroup::generateEdgeReport(uint8_t pinIndex, bool isPressed) {
    uint8_t pin = pins_[pinIndex];
    String evtText = "";

    if (leftPin_ && pin == leftPin_) {
        evtText = isPressed? "left-in" : "left-out";
    }
    else if (rightPin_ && pin == rightPin_) {
        evtText = isPressed? "right-in" : "right-out";
    }
    else if (bottomPin_ && pin == bottomPin_) {
        evtText = isPressed? "bottom-in" : "bottom-out";
    }
    
    if (evtText != "") {
        Report r;
        r.mask = (1UL << pinIndex);
        r.count = 1;
        r.longPress = false;
        r.text = evtText;
        queueReport(r);
    }
}

String ButtonGroup::buildLabel(uint32_t mask, bool isLong) const {
    int8_t left = leftPin_ ? findIndex(leftPin_) : -1;
    int8_t right = rightPin_ ? findIndex(rightPin_) : -1;
    int8_t bottom = bottomPin_ ? findIndex(bottomPin_) : -1;

    uint8_t btnState = 0; // Renamed from 'mask' to 'btnState'
    
    // We check the input 'mask' (uint32_t) to see which buttons are active
    if (left >= 0 && (mask & (1UL << left))) btnState |= 1;
    if (right >= 0 && (mask & (1UL << right))) btnState |= 2;
    if (bottom >= 0 && (mask & (1UL << bottom))) btnState |= 4;

    switch (btnState) {
        case 1: return String("left-") + (isLong ? "long" : "short");
        case 2: return String("right-") + (isLong ? "long" : "short");
        case 4: return String("bottom-") + (isLong ? "long" : "short");
        case 1|2: return String("left-right-") + (isLong ? "long" : "short");
        case 1|4: return String("left-bottom-") + (isLong ? "long" : "short");
        case 2|4: return String("right-bottom-") + (isLong ? "long" : "short");
        case 1|2|4: return String("left-right-bottom-") + (isLong ? "long" : "short");
        default: return String("unknown");
    }
}