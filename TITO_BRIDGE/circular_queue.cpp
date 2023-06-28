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

bool CircularQueue::enqueue(char packet[CIRCULAR_QUEUE_ELEMENT_MAX_SIZE], int size_packet) {
  if (isFull()) { return false; }
  if (_front == -1) {
    _front = 0;
  }
  if (_rear == CIRCULAR_QUEUE_FIFO_SIZE - 1) {
    _rear = 0;
  } else {
    _rear++;
  }
  strncpy(_packets[_rear], packet, CIRCULAR_QUEUE_ELEMENT_MAX_SIZE);
  _size_packets[_rear] = size_packet;
  return true;
}

bool CircularQueue::denqueue(char packet[], int& size_packet) {
  if (isEmpty()) { return false; }
  strncpy(packet, _packets[_front], CIRCULAR_QUEUE_ELEMENT_MAX_SIZE);
  size_packet = _size_packets[_front];
  if (_rear == _front){
    _rear = -1;
    _front = -1;
  } else {
    _front = _front + 1;
  }
  return true;
}
