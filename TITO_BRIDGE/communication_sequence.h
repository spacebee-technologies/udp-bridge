#ifndef COMMUNICATION_SEQUENCE_H_
#define COMMUNICATION_SEQUENCE_H_

#include <stddef.h>
#include <stdint.h>
#include <WiFi.h>

#include "circular_queue.h"

typedef enum CommunicationSequenceState {
  COMMUNICATION_SEQUENCE_STATE_IDLE,
  COMMUNICATION_SEQUENCE_STATE_RX_WAIT_LENGTH_REQUEST,
  COMMUNICATION_SEQUENCE_STATE_RX_WAIT_PACKET_REQUEST,
  COMMUNICATION_SEQUENCE_STATE_TX_WAIT_PACKET,
} CommunicationSequenceState_t;

class CommunicationSequence {
  public:
    CommunicationSequence();
    uint8_t handleReceive(char *buffer, size_t bufferSize);
    void handleRequest(CircularQueue *queue);

  private:
    CommunicationSequenceState_t _state;
    char _packet[CIRCULAR_QUEUE_ELEMENT_MAX_SIZE];
    int _packetSize;
    uint16_t _destinationPort;
    WiFiUDP _udp;
    void transitionToState(CommunicationSequenceState_t state);
    uint8_t executeStateIdle(char *buffer, size_t bufferSize);
    void executeStateRxWaitLengthRequest(CircularQueue *queue);
    void executeStateRxWaitPacketRequest();
    uint8_t executeStateTxWaitPacket(char *buffer);
};

#endif  // COMMUNICATION_SEQUENCE_H_
