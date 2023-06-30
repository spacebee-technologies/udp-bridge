#include "communication_sequence.h"

#include <Arduino.h>
#include <Wire.h>

typedef enum StartingByte {
  STARTING_BYTE_RECEIVE = 'P',
  STARTING_BYTE_SEND = 'S'
} StartingByte_t;

CommunicationSequence::CommunicationSequence() {
  _state = COMMUNICATION_SEQUENCE_STATE_IDLE;
}

uint8_t CommunicationSequence::handleReceive(char *buffer, size_t bufferSize) {
  switch (_state) {
    case COMMUNICATION_SEQUENCE_STATE_IDLE:
      return executeStateIdle(buffer, bufferSize);
    case COMMUNICATION_SEQUENCE_STATE_TX_WAIT_PACKET:
      return executeStateTxWaitPacket(buffer);
    default:
      return 1;
  }
}

void CommunicationSequence::handleRequest(CircularQueue *queue) {
  switch (_state) {
    case COMMUNICATION_SEQUENCE_STATE_RX_WAIT_LENGTH_REQUEST:
      executeStateRxWaitLengthRequest(queue);
      break;
    case COMMUNICATION_SEQUENCE_STATE_RX_WAIT_PACKET_REQUEST:
      executeStateRxWaitPacketRequest();
      break;
    default:
      break;
  }
}

void CommunicationSequence::transitionToState(CommunicationSequenceState_t state) {
  _state = state;
}

uint8_t CommunicationSequence::executeStateIdle(char *buffer, size_t bufferSize) {
  if (bufferSize != 1 && bufferSize != 3) { return 1; }
  switch (buffer[0]) {
    case STARTING_BYTE_RECEIVE:
      transitionToState(COMMUNICATION_SEQUENCE_STATE_RX_WAIT_LENGTH_REQUEST);
      return 0;
    case STARTING_BYTE_SEND:
      _destinationPort = buffer[1] << 8 | buffer[2];
      transitionToState(COMMUNICATION_SEQUENCE_STATE_TX_WAIT_PACKET);
      return 0;
    default:
      return 1;
  }
}

void CommunicationSequence::executeStateRxWaitLengthRequest(CircularQueue *queue) {
  if (!queue->isEmpty()) {
    queue->denqueue(_packet, _packetSize);
    Serial.print("Sending to MCU packet size: ");
    Serial.println(_packetSize);
    Wire.write(_packetSize);  // Send message to master
    transitionToState(COMMUNICATION_SEQUENCE_STATE_RX_WAIT_PACKET_REQUEST);
  } else {  // If queue is empty we should check also if there was a request and answer length = 0
    Serial.println("Sending to MCU packet size: 0 (queue empty)");
    Wire.write(0);
    transitionToState(COMMUNICATION_SEQUENCE_STATE_IDLE);
  }
}

void CommunicationSequence::executeStateRxWaitPacketRequest() {
  Wire.write(reinterpret_cast<byte*>(_packet), _packetSize);  // Send the array to the master
  transitionToState(COMMUNICATION_SEQUENCE_STATE_IDLE);
}

uint8_t CommunicationSequence::executeStateTxWaitPacket(char *buffer) {
  Serial.print("Sending UDP packet to PC: ");
  Serial.println(buffer);
  _udp.begin(_destinationPort);
  _udp.parsePacket();
  _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
  _udp.print(buffer);
  _udp.endPacket();
  _udp.stop();
  transitionToState(COMMUNICATION_SEQUENCE_STATE_IDLE);
  return 0;
}
