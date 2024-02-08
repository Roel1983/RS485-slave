#ifndef COMMUNICATION_COMMUNICATION_H_
#define COMMUNICATION_COMMUNICATION_H_

namespace communitation {
	
constexpr uint32_t BAUDRATE       = 115200;
constexpr uint8_t  PREAMBLE_BYTE  = 0x55;
constexpr uint8_t  PREAMBLE_COUNT = 2;
constexpr uint8_t  EXTENDED_PAYLOAD_LENGHT_MASK = 0x80;


#ifdef UNITTEST
void teardown();
#endif

void setup();
void loop();

} // namespace communitation

#endif // COMMUNICATION_COMMUNICATION_H_
