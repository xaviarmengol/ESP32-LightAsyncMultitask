#pragma once
#include <Arduino.h>
#include "eventsDef.h"

#include <Wifi.h>

const int MAX_INP_QUEUE_LENTH = 16;

const int MAX_TASKS = 10;

int32_t localNode=0;
 
// Event Definition

class event_t {
    public: 
        event_t () {
            name = events::VOID;
            tskTo = tskNames::TSK_TOTAL; // To fill with something
            tskFrom = tskNames::TSK_TOTAL;
            eventMode = eventModes::SIGNAL;
            value = 0;

        }

        event_t (events nameInp, tskNames tskToInp, tskNames tskFromInp, eventModes eventModeInp, int32_t valueInp = 0) {
            name = nameInp;
            tskTo = tskToInp;
            tskFrom = tskFromInp;
            eventMode = eventModeInp;
            value = valueInp;
        }

        void println(String init, int tskId) {
            String msg = init + String (" - Tsk ") + String(tskId) + String(" - ") + String(" From:") + String((int32_t) tskFrom)+ String(" To:") + String((int) tskTo) + String(" Event:") + String((int) name)+ String(" Mode:") + String((int) eventMode)+ String(" Val:") + String(value);
            Serial.println(msg);
        }

        events name;
        tskNames tskTo;
        tskNames tskFrom;
        eventModes eventMode;
        int32_t value;
        
};

typedef void (*ptrFunc)(event_t&, int);

void tskVoid(event_t& event, int tskId) {
    Serial.println("Task VOID launched");
}

struct tskDef_t {

    tskDef_t() {
        name = tskNames::TSK_TOTAL;
        func = tskVoid;
        handler = nullptr;
        queue = nullptr;
        nodeId = -1;
    }

    tskNames name;
    ptrFunc func;
    TaskHandle_t handler;
    QueueHandle_t queue;
    int32_t nodeId;
};

tskDef_t tskDef[MAX_TASKS];


// Hadware abstraction Functions helpers

QueueHandle_t queueTimer; // Needed for helper function

void sendLocalEvent(events eventName, tskNames tskDest, tskNames tskFrom, eventModes eventMode=eventModes::SIGNAL, int32_t value=0, int32_t msWait = 0) {
    int32_t tskIdDest = (int32_t) tskDest;
    event_t event = event_t (eventName, tskDest, tskFrom, eventMode, value);
    event.println("S",(int32_t) tskFrom);
    xQueueSend(tskDef[tskIdDest].queue, &event, pdMS_TO_TICKS(msWait));

}

void sendProxyEvent(events eventName, tskNames tskDest, tskNames tskFrom, eventModes eventMode=eventModes::SIGNAL, int32_t value=0, int32_t msWait = 0) {

    int32_t tskIdDest = (int32_t) tskNames::NODE_PROXY; // Use the proxy tsk to route the event

    event_t event = event_t (eventName, tskDest, tskFrom, eventMode, value);
    event.println("X",(int32_t) tskFrom);
    xQueueSend(tskDef[tskIdDest].queue, &event, pdMS_TO_TICKS(msWait));

}

bool isLocalNode(tskNames task) {

    int tskId = (int) task;
    return(tskDef[tskId].nodeId == localNode);

}

void sendEventToTask(events eventName, tskNames tskDestination, tskNames tskFrom, eventModes eventMode=eventModes::SIGNAL, int32_t value=0, int32_t msWait=0) {

    if (isLocalNode(tskDestination)) {      
        sendLocalEvent(eventName, tskDestination, tskFrom, eventMode, value, msWait);

    } else {
        // In case of task in other node, send it to the proxy to route it
        sendProxyEvent(eventName, tskDestination, tskFrom, eventMode, value, msWait);
    }
}

void sendEventToTask(const event_t& event) {
    sendEventToTask(event.name, event.tskTo, event.tskFrom, event.eventMode, event.value);
}


void sendTimerEvent (tskNames tskFrom, int32_t valueMs=0, int32_t msWait = 0) {
    sendLocalEvent(events::SET_TIMEOUT, tskNames::NODE_TIMER, tskFrom, eventModes::SIGNAL, valueMs, msWait);
}

bool receiveEvent(QueueHandle_t queue, void* pvBuffer, int32_t msWait = 10000) {
    bool allOk = (xQueueReceive(queue, pvBuffer, pdMS_TO_TICKS(msWait)) == pdPASS);

    if (allOk) {
        event_t receivedEvent = event_t();
        memcpy(&receivedEvent, pvBuffer, sizeof(event_t) );
        receivedEvent.println("R", (int)receivedEvent.tskTo);
    }
    return (allOk);
}

bool getAddressFromNodeId(int nodeId, address_t& address) {

    for (int i=0; i < TOTAL_NODES; i++) {
        if (nodes[i].nodeId == nodeId) { 
            memcpy(address, nodes[i].address, sizeof(address_t)); 
            return(true);
        }
    }
    return (false);
}

int32_t getLocalNode() {

    address_t mac;
    WiFi.macAddress(mac);
    
    for (int i=0; i < TOTAL_NODES; i++) {
        bool macEqual = true;
        //Serial.print("Node tested: "); Serial.println(i);    
        for (int j=0; j<6; j++) {
            //Serial.print(nodes[i].address[j]); Serial.print("-"); Serial.println(mac[j]);
            if (nodes[i].address[j] != mac[j]) {
                macEqual &= false;
            }
        }
        if (macEqual) return(nodes[i].nodeId);
    }

    Serial.println ("MAC not found");
    return(-1);
}



void valueReadWrite(events eventName, event_t& event, int32_t& tskVar, bool readEnabled = true, bool writeEnabled = true) {

    if (event.eventMode == eventModes::READ && readEnabled) {
        sendEventToTask(events::VALUE_A, event.tskFrom, event.tskTo, eventModes::WRITE, tskVar);
    }

    if (event.eventMode == eventModes::WRITE && writeEnabled) {
        tskVar = event.value;
        Serial.print("W - Tsk "); Serial.print(event.tskTo); Serial.print(" Value Write:"); Serial.println(event.value);
    }
}

