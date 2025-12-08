#ifndef BUTTON_GROUP_H
#define BUTTON_GROUP_H

#include <Arduino.h>

#define MAX_BUTTONS 12
#define REPORT_QUEUE_SIZE 8 

class ButtonGroup {
    public:
        struct Report {
            uint32_t mask;
            uint8_t count;
            bool longPress;
            String text;
        };
        ButtonGroup(unsigned long debounce_ms = 50, unsigned long long_ms = 1000);

        void setLeftPin(uint8_t pin);
        void setRightPin(uint8_t pin);
        void setBottomPin(uint8_t pin);
        void setSpreadPin(uint8_t pin); // New setter

        void begin();
        void end();
        void service();
        bool poll(Report &out);

        static void isr_handler();

    private:
        static ButtonGroup *instance_;

        unsigned long debounceMs_;
        unsigned long longMs_;

        uint8_t pins_[MAX_BUTTONS];
        uint8_t count_ = 0;

        // Role Pins
        uint8_t leftPin_ = 0;
        uint8_t rightPin_ = 0;
        uint8_t bottomPin_ = 0;
        uint8_t spreadPin_ = 0; // New role

        volatile uint32_t isrMask_ = 0;
        volatile bool isrChanged_ = false;

        bool lastRaw_[MAX_BUTTONS];
        unsigned long lastRawChangeMs_[MAX_BUTTONS];
        unsigned long pressStartAt_[MAX_BUTTONS];
        bool longRegistered_[MAX_BUTTONS];

        uint32_t stableMask_ = 0;
        uint32_t latchedMask_ = 0;

        bool sessionActive_ = false;
        bool sessionHasLong_ = false;
        uint32_t emittedLongMask_ = 0;

        Report reportQueue_[REPORT_QUEUE_SIZE];
        uint8_t qHead_ = 0;
        uint8_t qTail_ = 0;

        int8_t findIndex(uint8_t pin) const;
        void addIfMissing(uint8_t pin);
        String buildLabel(uint32_t mask, bool isLong) const;

        void queueReport(const Report &r);
        void generateEdgeReport(uint8_t pinIndex, bool isPressed);
};

#endif