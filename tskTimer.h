#include <Arduino.h>
#include <vector>
#include "eventsMgmt.h"

TaskHandle_t handTskTimer;

void tskTimer(void *pvParameters){

    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("Task Timer on core: " + String(xPortGetCoreID()));
    queueTimer = xQueueCreate( MAX_INP_QUEUE_LENTH , sizeof(event_t));

    struct timeAndOrigin_t {
        unsigned long time;
        QueueHandle_t queueOrigin;
    };

    std::vector <timeAndOrigin_t> times;
    times.reserve(10);
    event_t eventTimeout = event_t(events::TIMEOUT, queueTimer);

    // SETUP the timer blocking

    TickType_t lastTime=0;
    TickType_t period = pdMS_TO_TICKS(10);

    while (1) {

        // Send the TIMEOUTs messages when the timmer reach the set time
        // TODO: Refactor with a sorted list

        int indexVector = 0;
        for (timeAndOrigin_t time : times) {
            if (millis() >= time.time) {
                xQueueSend(time.queueOrigin, &eventTimeout, 0);
                times.erase(times.begin() + indexVector);
            } 
            indexVector++;
        }

        // Event dispatcher. In this case, the block is in a task timer to allow the time management

        event_t receivedEvent = event_t(); // Always the same
        if (xQueueReceive(queueTimer, &receivedEvent, 0) == pdPASS) {

            if (receivedEvent.name == events::SET_TIMEOUT) {
                Serial.println("Timer - Event: SET_TIMEOUT");

                timeAndOrigin_t time;
                time.time = millis() + receivedEvent.value; // Value is a relative time to wait
                time.queueOrigin = receivedEvent.queueOrigin;
                times.push_back(time);
            } else {
                Serial.print("Timer - Event: ");
                Serial.print((int)receivedEvent.name);
                Serial.println(" was not expected, and is discharged.");
            }

        } 

        vTaskDelayUntil(&lastTime, period);
    }

}

