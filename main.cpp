#include <Arduino.h>
#include "eventFramework.h"
#include "eventsDef.h"

// Declaration of the dispatching functions

// eventsDef.h should be filled
// registerTasks() inside eventFramework.h should be filled


void client(event_t& event, int tskId) {
    tskNames tskName = tskDef[tskId].name;

    enum struct state_t {
        READY_TO_GET
    };

    // Should survive the calls because are states
    static state_t stateTsk;
    static int valueA;

    switch (event.name) {

        case (events::INIT): {

            Serial.println("Init Client");
            stateTsk = state_t::READY_TO_GET;
            sendTimerEvent(tskName, 1000);
            break;

        }

        case (events::TIMEOUT): {

            if (stateTsk == state_t::READY_TO_GET) {
                sendEventToTask(events::VALUE_A, tskNames::SERVER, tskName, eventModes::READ);
            }

            break;
        }

        case (events::VALUE_A): {

            valueReadWrite(events::VALUE_A, event, valueA, false, true);
            if (event.eventMode == eventModes::WRITE) {
                sendTimerEvent(tskName, 1000);
            }
            break;
        }
    
        default: {
            break;
        }
    }
}

void server(event_t& event, int tskId) {
    tskNames tskName = tskDef[tskId].name;
    static int valueAServer=0;

    switch (event.name) {

        case (events::INIT): {
            sendTimerEvent(tskName, 4000);
            break;
        }
        case (events::TIMEOUT): {
            sendEventToTask(events::VALUE_A, tskName, tskName, eventModes::WRITE, valueAServer + random(-5,5));
            sendTimerEvent(tskName, 4000);
            break;
        }

        case (events::VALUE_A): {

            valueReadWrite(events::VALUE_A, event, valueAServer);
            break;
        }

    }
}



void client2(event_t& event, int tskId) {
    tskNames tskName = tskDef[tskId].name;

    enum struct state_t {
        READY_TO_GET
    };

    // Should survive the calls because are states
    static state_t stateTsk;
    static int valueA;

    switch (event.name) {

        case (events::INIT): {

            Serial.println("Init Client 2");
            stateTsk = state_t::READY_TO_GET;
            sendTimerEvent(tskName, 1000);
            break;

        }

        case (events::TIMEOUT): {

            if (stateTsk == state_t::READY_TO_GET) {
                sendEventToTask(events::VALUE_A, tskNames::SERVER, tskName, eventModes::READ);
            }

            break;
        }

        case (events::VALUE_A): {

            valueReadWrite(events::VALUE_A, event, valueA, false, true);
            if (event.eventMode == eventModes::WRITE) {
                sendTimerEvent(tskName, 1000);
            }
            break;
        }
    
        default: {
            break;
        }
    }
}
