#ifndef CIRCULAR_QUEUE_H_
#define CIRCULAR_QUEUE_H_

#include <stdbool.h>

#define CIRCULAR_QUEUE_FIFO_SIZE          10    // Number of elements in FIFO
#define CIRCULAR_QUEUE_ELEMENT_MAX_SIZE   228   // Max size of each element

class CircularQueue {
  public:
    CircularQueue();
    bool isFull();
    bool isEmpty();
    bool enqueue(char packet[]);
    bool denqueue(char packet[]);

  private:
    char _packets[CIRCULAR_QUEUE_FIFO_SIZE][CIRCULAR_QUEUE_ELEMENT_MAX_SIZE];
    int _front, _rear;
};

#endif  // CIRCULAR_QUEUE_H_
