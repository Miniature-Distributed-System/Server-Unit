# Sink

These are used to push and pop objects from queue. The sinks are used by senders and receivers. The data that needs to be queued for processing uses the global sink. The global
Sink provides some basic queue operations. The data is queued based on task priority. 

## Operations

### Push Object
#### `pushObject(void *object, TaskPriority priority)`

- This method can be invoked to push object of any type into queue with valid task priority

## Pop Object
#### `popObject()`

- This method pops object from queue. Have to be careful while popping as we are not using templates here therfore use caution while accessing objects.

## Sink Size
#### `getCurrentSinkSpace()`

- This method returns the sink size currently. It shows how much is free in the queue.

## Sink Bools
#### `isSinkFull()`

- This method returns true if sink has no empty slots.

#### `isSinkEmpty()`

- This method returns true if sink is empty. If no slots have any data in them.

## Top task priority
#### `getTopTaskPriority()`

- This method returns the top most task's priority feild.
