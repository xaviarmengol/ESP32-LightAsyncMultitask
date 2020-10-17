#pragma once

#include "eventsMgmt.h"
#include <esp_now.h>

void nodeESPNOWProxy(event_t& event, int tskId) {
    
    // ESPNOW

    //Serial.println("Inside Proxy");

    int32_t tskIdDest = (int) event.tskTo; // It is an enum
    int32_t nodeId = tskDef[tskIdDest].nodeId;

    Serial.println(tskIdDest);
    Serial.println(nodeId);

    address_t destMacAddress;

    bool addDestOK=false;
    addDestOK = getAddressFromNodeId(nodeId, destMacAddress);
    if (!addDestOK) Serial.println("Node destination unknown");

    esp_err_t result = esp_now_send(destMacAddress, (uint8_t *) &event, sizeof(event));

    if ( result == ESP_OK) {
        Serial.println("Sent with success");
    }
    else {
        Serial.println("Error sending the data");
    }

}


