#pragma once

#include <Arduino.h>
#include <stdio.h>
#include <esp_now.h>
#include <WiFi.h>

#include "eventsMgmt.h"
#include "tskTimer.h"
#include "tskProxy.h"

bool setupFinished = false;

///////////////////////////////////////////////////////////////////


void client(event_t& event, int tskId);
void server(event_t& event, int tskId);
void client2(event_t& event, int tskId);
void nodeESPNOWProxy(event_t& event, int tskId);

void registerTasks (){


    
    // Function Loop registration: Add every function loop

    tskDef[tskNames::CLIENT].name = tskNames::CLIENT;
    tskDef[tskNames::CLIENT].func = client;
    tskDef[tskNames::CLIENT].nodeId = 0; 

    tskDef[tskNames::SERVER].name = tskNames::SERVER;
    tskDef[tskNames::SERVER].func = server;
    tskDef[tskNames::SERVER].nodeId = 0; 

    tskDef[tskNames::CLIENT2].name = tskNames::CLIENT2;
    tskDef[tskNames::CLIENT2].func = client2;
    tskDef[tskNames::CLIENT2].nodeId = 1; 

    // Do not modify. Proxy should be always local

    tskDef[tskNames::NODE_PROXY].name = tskNames::NODE_PROXY;
    tskDef[tskNames::NODE_PROXY].func = nodeESPNOWProxy;
    tskDef[tskNames::NODE_PROXY].nodeId = localNode; 

    tskDef[tskNames::NODE_TIMER].name = tskNames::NODE_TIMER;
    tskDef[tskNames::NODE_TIMER].func = nullptr;
    tskDef[tskNames::NODE_TIMER].nodeId = localNode; 

}

///////////////////////////////////////////////////////////////////

// ESP NOW CALLBACK AND ADDRESS


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t* mac, const uint8_t *incomingData, int len) {
  
    if (setupFinished) {
        event_t receivedEvent;
        memcpy(&receivedEvent, incomingData, sizeof(receivedEvent));

        sendEventToTask(receivedEvent);
    }
}


//////////////////////////////////////////////////////////////////

// Tsk Server (framework)

void eventManagement(void *pvParameters) {

    vTaskDelay(pdMS_TO_TICKS(2000)); // Startup Delay

    int tskId = *((int *)pvParameters);

    Serial.print("Id: "); Serial.print(tskId);
    Serial.println(" client on core: " + String(xPortGetCoreID()));
    
    while (true) {

        event_t receivedEvent = event_t();

        if (receiveEvent(tskDef[tskId].queue, &receivedEvent)) {
           
            ptrFunc fn = tskDef[tskId].func;
            fn(receivedEvent, tskId);
        }
    }
}


void setup() {

    Serial.begin(115200);

    for (int i=0; i<MAX_TASKS; i++) {
        tskDef[i] = tskDef_t();
    }

    /////////////////////////////////////////////////

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    Serial.println(WiFi.macAddress());

    localNode = getLocalNode();

    Serial.println(String("Local Node: ") + String(localNode));

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    // Register peers

    for (int i = 0; i < TOTAL_NODES; i++) {

        //Serial.print("Total Nodes");
        //Serial.println(TOTAL_NODES);

        int32_t nodeId = nodes[i].nodeId;

        if ( nodeId != localNode) { 

            address_t nodeAddress;
            getAddressFromNodeId(nodeId, nodeAddress);
            
            esp_now_peer_info_t peerInfo;
            memcpy(peerInfo.peer_addr, nodeAddress, 6);
            peerInfo.channel = 0;  
            peerInfo.encrypt = false;

            //for (int j=0; j<6; j++) Serial.println(peerInfo.peer_addr[j]);
            
            // Add peer
            if (esp_now_add_peer(&peerInfo) != ESP_OK){
                Serial.println("Failed to add peer");
                return;
            }
        }
    }

    ///////////////////////////////////////////////

    registerTasks();

    // Tasks creation (framework)

    // Special timer task and queue
    tskDef[(int32_t) tskNames::NODE_TIMER].queue = xQueueCreate( MAX_INP_QUEUE_LENTH , sizeof(event_t));
    xTaskCreateUniversal(tskTimer, "TaskTimer", 10000, NULL, 1, &handTskTimer, 0);
    delay(1000);

    // Queue creactions

    for(int tskIdQ = 0; tskIdQ<tskNames::TSK_TOTAL ; tskIdQ++ ) {
        tskDef[tskIdQ].queue = xQueueCreate( MAX_INP_QUEUE_LENTH , sizeof(event_t));
    }

    // Tasks creation
    
    for(int32_t tskId = 0; tskId<tskNames::TSK_TOTAL ; tskId++ ) {

        int32_t tskNode = tskDef[tskId].nodeId; 
        int32_t tskIdParam = tskId;

        // Creates only the tasks belonging to the current node

        if (tskNode == localNode) {

            String tskName = String("Task_") + String(tskIdParam);
            xTaskCreateUniversal(eventManagement, "Task" , 10000, (void *)&tskIdParam, 1, &(tskDef[tskIdParam].handler), 1);
            Serial.print(tskName); Serial.println(" created");
            vTaskDelay(pdMS_TO_TICKS(2000));

            if (tskDef[tskId].name != tskNames::NODE_PROXY) {
                sendLocalEvent(events::INIT, tskDef[tskIdParam].name, tskDef[tskIdParam].name); // Wake up signal
            }
        }

    }

    setupFinished = true;

}

void loop() {

    delay(10000);
}

