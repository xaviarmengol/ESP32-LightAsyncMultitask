# ESP32-LightAsyncMultitask
Light Async Multitask template. Based on the study of modern event-driven architecture and active objects

How the light async framework works:

## Events: The events are composed of:
- Event name: Based on the name, the receiver does actions
- Sender: The sender queue can be used by the receiver if it wants to send back an answer to the sender.
- Value: A value can be attached to the event. Can be a parameter of the event, or an answer to a previous event.

## Active Objects: Every active object runs in a FreeRtos task, and follow the "Share nothing" principle. Are composed of:
- Task initialitzation: input Queue, local variables
- State machine (inside loop - with NO blocking allowed)
- Events reader (inside loop - the ONLY bloking point allowed)
- Events dispatcher (inside loop - with NO blocking allowed)

How the example works:

## Client: Every second ask for a value to the server
- Timer: An event is sent to an unique timer task, and the timer task answer one second later with another event.
- Doing requests: The client sends a event to the server, and the server answer with a message with the value to the client.

## Server: Answer the value requests, and every 3 seconds changes the value.
- Timer: An event is sent to the same unique timer task, and the timer task answer three seconds later with another event.
- Answering requests: The server answer to any request at any time. Sends an event with the requested value to the task sender.
