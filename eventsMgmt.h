#pragma once
#include <Arduino.h>

const int MAX_INP_QUEUE_LENTH = 16;
 
enum struct events {
    VOID,
    GET_VALUE_A,
    VALUE_A, 
    TIMEOUT,
    SET_TIMEOUT
};

struct event_t {

    event_t () {
        name = events::VOID;
        queueOrigin = nullptr;
        value = 0;
    }

    event_t (events nameInp, QueueHandle_t queueOriginInp, int32_t valueInp = 0) {
        name = nameInp;
        queueOrigin = queueOriginInp;
        value = valueInp;
    }

    events name;
    QueueHandle_t queueOrigin;
    int32_t value;

};


// Hadware abstraction Functions helpers
QueueHandle_t queueTimer; // Needed for helper function

void sendEvent(events eventName, QueueHandle_t queueDestination, QueueHandle_t queueFrom, int32_t value=0, int32_t msWait = 0) {
    event_t setTimeOutEvent = event_t (eventName, queueFrom, value);
    xQueueSend(queueDestination, &setTimeOutEvent, pdMS_TO_TICKS(msWait));
}

void sendTimerEvent (QueueHandle_t queueFrom, int32_t valueMs=0, int32_t msWait = 0) {
    sendEvent(events::SET_TIMEOUT, queueTimer, queueFrom, valueMs, msWait);
}

bool receiveEvent(QueueHandle_t queue, void* pvBuffer, int32_t msWait = 10000) {
    return (xQueueReceive(queue, pvBuffer, pdMS_TO_TICKS(msWait)) == pdPASS);
}
