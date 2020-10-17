#pragma once

// Events and Task Names definition
// To be modified

enum struct events {
    VOID,
    INIT,
    TIMEOUT,
    SET_TIMEOUT,
    VALUE_A // First event
};

enum tskNames {
    NODE_PROXY,
    CLIENT,
    SERVER,
    CLIENT2,
    TSK_TOTAL, // All tasks behind this point are special and should be created manually
    NODE_TIMER 
};

enum struct eventModes {
    SIGNAL,
    READ,
    WRITE  
};


typedef uint8_t address_t[6];

typedef struct nodeWithId_t {
    address_t address;
    int32_t nodeId;
} nodeWithId_t;


nodeWithId_t nodeZero = {{0x84, 0x0D, 0x8E, 0xE6, 0x52, 0x74}, 0};
nodeWithId_t nodeOne = {{0x24, 0x62, 0xAB, 0xFF, 0x35, 0x50}, 1};

nodeWithId_t nodes[] = {nodeZero, nodeOne};

#define TOTAL_NODES sizeof(nodes) / sizeof(nodes[0])

