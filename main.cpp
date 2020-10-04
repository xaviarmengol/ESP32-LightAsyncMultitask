#include <Arduino.h>
#include <string>

#include "eventsMgmt.h"
#include "tskTimer.h"


// Tasks and Queue definition

TaskHandle_t handTskClient;
void tskClient (void *pvParameters);
QueueHandle_t queueClient;

TaskHandle_t handTskServer;
void tskServer (void *pvParameters);
QueueHandle_t queueServer;


void setup() {

    Serial.begin(115200);

    // Tasks timer creation

    xTaskCreateUniversal(tskTimer, "TaskTimer", 10000, NULL, 1, &handTskTimer, 0);
    delay(100);
    
    // Application tasks creation

    xTaskCreateUniversal(tskServer, "TaskServer", 10000, NULL, 1, &handTskServer, 0);
    delay(100);

    xTaskCreateUniversal(tskClient, "TaskClient", 10000, NULL, 1, &handTskClient, 1);
    delay(100);

}

void loop() {

    delay(100000);
}


void tskClient(void *pvParameters){

    // Task init

    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("Task Client on core: " + String(xPortGetCoreID()));
    queueClient = xQueueCreate( MAX_INP_QUEUE_LENTH , sizeof(event_t));

    // Client number - Just for debugging purpouses

    int clientNum = 1;

    // States declaration 

    enum struct state_t {
        READY_TO_GET, WAITING
    }; 
    state_t stateTsk = state_t::READY_TO_GET;

    // Prime the timer (if exist)

    sendTimerEvent(queueClient, 1000);

    while (true) {

        // Internal state machine

        switch (stateTsk) {
            case state_t::READY_TO_GET: {

                sendEvent(events::GET_VALUE_A, queueServer, queueClient);

                stateTsk = state_t::WAITING;
                break;
            }

            case state_t::WAITING: {
                break;
            }
            
            default: {
                Serial.println("Error with state");
                break;
            }
        }

        // Event dispatcher

        event_t receivedEvent = event_t();
        if (receiveEvent(queueClient, &receivedEvent)) {
            
            // Move states and do actions based on events received

            switch (receivedEvent.name) {

                case (events::VALUE_A): { 
                    Serial.print("Client ");
                    Serial.print(clientNum);
                    Serial.println(" - Event: VALUE_A");
                    
                    // DO something with value A (ex: Print in HMI)
                    Serial.print("Client ");
                    Serial.print(clientNum);
                    Serial.print(" - Value received: ");
                    Serial.println(receivedEvent.value); 
                    break;
                }

                case (events::TIMEOUT): {
                    Serial.print("Client ");
                    Serial.print(clientNum);
                    Serial.println(" - Event: TIMEOUT");

                    if (stateTsk == state_t::WAITING) {
                        stateTsk = state_t::READY_TO_GET;

                        // Arm again the timer
                        // The timer will sendback a TIMEOUT message in x ms
                        sendTimerEvent(queueClient, 1000);
                    }
                    break;
                }
                
                default: {
                    Serial.print("Client ");
                    Serial.print(clientNum);
                    Serial.print(" - Event: ");
                    Serial.print((int)receivedEvent.name);
                    Serial.println(" was not expected, and is discharged.");

                    break;
                }
            }
        }
    }
}


void tskServer(void *pvParameters){

    // Task Init

    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("Task Server on core: " + String(xPortGetCoreID()));
    queueServer = xQueueCreate( MAX_INP_QUEUE_LENTH , sizeof(event_t));

    // States declaration 

    enum struct state_t {
        UPDATING_VALUE, READY_TO_ANSWER
    }; 
    state_t stateTsk = state_t::UPDATING_VALUE;

    // Local variables

    int32_t valueA=0;

    // Prime the timer

    sendTimerEvent(queueServer, 3000);

    while (true) {

        // Internal state machine

        switch (stateTsk) {
            case state_t::UPDATING_VALUE: {

                valueA = valueA + random(-2,2);
                stateTsk = state_t::READY_TO_ANSWER;

                break;
            }
            
            case state_t::READY_TO_ANSWER: {
                // ...
                break;
            }

            default:{
                Serial.println("Error with state");
                break;
            }
        }

        // Event dispatcher. Here blocks waiting for a message.
        // Optionally we can block every x ms, setting receiveEvent msWait=0 and using vTaskDelay (or similiar)

        event_t receivedEvent = event_t();
        if (receiveEvent(queueServer, &receivedEvent)) { 
            
            // Case to manage the received events and move state machine

            switch (receivedEvent.name) {

                case (events::TIMEOUT): {
                    Serial.println("Server - Event: TIMEOUT");

                    if (stateTsk == state_t::READY_TO_ANSWER) {
                        stateTsk = state_t::UPDATING_VALUE;
                    }

                    sendTimerEvent(queueServer, 3000);
                    
                    break;
                }

                case (events::GET_VALUE_A): {
                    Serial.println("Server - Event: GET_VALUE_A");

                    sendEvent(events::VALUE_A, receivedEvent.queueOrigin, queueServer, valueA);
                    break;
                }
                
                default: {

                    Serial.print("Server - Event: ");
                    Serial.print((int)receivedEvent.name);
                    Serial.println(" was not expected, and is discharged.");

                    break;
                }
            }
        }
    }
}


