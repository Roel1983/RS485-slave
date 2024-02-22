#ifndef COMMUNICATION_SENDER_SENDER_H_
#define COMMUNICATION_SENDER_SENDER_H_

namespace communication {
namespace sender {

struct Command {
	uint8_t  id;
	uint8_t  payload_length;
	uint8_t *payload_buffer;
};

#ifdef UNITTEST
void tearDown();
#endif

void setup();

bool send(Command& command);
bool is_sending();
extern void onSendComplete();

}}

#endif // COMMUNICATION_SENDER_SENDER_H_
