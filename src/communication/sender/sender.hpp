#ifndef COMMUNICATION_SENDER_SENDER_H_
#define COMMUNICATION_SENDER_SENDER_H_

namespace communitation {
namespace sender {

struct Command {
	uint8_t  command_id;
	uint8_t  payload_length;
	uint8_t *payload_buffer;
};

#ifdef UNITTEST
void teardown();
#endif

void setup();

bool send(Command& command);
extern void onSendComplete();

}}


#endif // COMMUNICATION_SENDER_SENDER_H_
