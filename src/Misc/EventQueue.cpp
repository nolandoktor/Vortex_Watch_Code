#include <FreeRTOS_SAMD21.h>
#include "EventQueue.h"

QueueHandle_t xEventQueue = NULL;

int init_event_queue()
{
    xEventQueue =  xQueueCreate(MAX_EVENTS, sizeof(struct event_message) );
    if (xEventQueue == NULL) {
        return -1;
    }
    return 0;
}
QueueHandle_t get_event_queue()
{
    return xEventQueue;
}