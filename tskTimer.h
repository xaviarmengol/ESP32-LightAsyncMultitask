#pragma once

#include <Arduino.h>
#include <vector>
#include "eventsMgmt.h"

#define MAX_TIMERS 20

TaskHandle_t handTskTimer;

void tskTimer(void *pvParameters){

    vTaskDelay(pdMS_TO_TICKS(2000));
    Serial.println("Task Timer on core: " + String(xPortGetCoreID()));
    
    static int32_t totalTimers=0;

    int32_t tskIdTimer = (int32_t)tskNames::NODE_TIMER;

    struct timeAndOrigin_t {
        unsigned long time;
        tskNames tskOrigin;
    };

    timeAndOrigin_t times[MAX_TIMERS];

    // SETUP the timer blocking

    TickType_t lastTime=0;
    TickType_t period = pdMS_TO_TICKS(10);

    while (1) {

        // Send the TIMEOUTs messages when the timmer reach the set time
        // TODO: Refactor with a sorted list

        for (int numTimer=0; numTimer<totalTimers; numTimer++) {
            if (millis() >= times[numTimer].time) {
                
                sendEventToTask(events::TIMEOUT, times[numTimer].tskOrigin, tskNames::NODE_TIMER);
                
                for (int i=numTimer; i<totalTimers; i++) {
                    if ( (i+1) < totalTimers) times[i]=times[i+1];
                }
                totalTimers--;
            }             
        }

        // Event dispatcher. In this case, the block is in a task timer to allow the time management

        event_t receivedEvent = event_t(); // Always the same
        
        if (receiveEvent(tskDef[tskIdTimer].queue, &receivedEvent, 0)) {

            if (receivedEvent.name == events::SET_TIMEOUT) {
                //Serial.println("Timer - Event: SET_TIMEOUT");

                timeAndOrigin_t time;
                time.time = millis() + receivedEvent.value; // Value is a relative time to wait
                time.tskOrigin = receivedEvent.tskFrom;

                times[totalTimers] = time;
                
                if (totalTimers<MAX_TIMERS) {
                    totalTimers++;
                } else {
                    Serial.println("Error in Timer: Not enough slots. Review MAX_TIMERS");
                }

            } 

        } 

        vTaskDelayUntil(&lastTime, period);
    }

}

