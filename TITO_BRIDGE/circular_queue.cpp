#include "circular_queue.h"

#include <string.h>

CircularQueue::CircularQueue() {
  _front = _rear = -1;
}

bool CircularQueue::isFull() {
  return (_front == 0 && _rear == CIRCULAR_QUEUE_FIFO_SIZE - 1) || (_rear == _front - 1);
}

bool CircularQueue::isEmpty() {
  return _front == -1;
}

bool CircularQueue::enqueue(char packet[]) {
  if (isFull()) {
      return false;
  }
  if (_front == -1) {
    _front = 0;
  }
  _rear++;
  strncpy(_packets[_rear], packet, CIRCULAR_QUEUE_ELEMENT_MAX_SIZE);
  return true;
}

bool CircularQueue::denqueue(char packet[]) {
  if (isEmpty()) { return false; }
  strncpy(packet, _packets[_front], CIRCULAR_QUEUE_ELEMENT_MAX_SIZE);
  if (_rear == _front) {
    _rear = -1;
    _front = -1;
  } else {
    _front = _front + 1;
  }
  return true;
}
