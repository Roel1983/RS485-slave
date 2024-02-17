#include <avr/interrupt.h>
#include <avr/io.h>

namespace timestamp {

#if F_CPU == 14745600
constexpr uint32_t steps     = 14745;
constexpr uint8_t  devisor   =  6;
constexpr uint8_t  nominator = 10;
#else
#error "F_CPU not supported"
#endif

volatile uint32_t counter;

void setup() {
	
    constexpr uint8_t compare_output_mode_a       = 0; // Normal port operation
    constexpr uint8_t compare_output_mode_b       = 0; // Normal port operation
    constexpr uint8_t waveform_generation_mode    = 4; // CRC
    constexpr uint8_t clock_select                = 1; // clk/1
    constexpr uint8_t input_capture_noise_cancler = 0;
    constexpr uint8_t input_capture_edge_select   = 0;
    
    TCCR1A = (((waveform_generation_mode    & 0b011) >> 0) << WGM10)
           | (((compare_output_mode_a       &  0b11) >> 0) << COM1A0)
		   | (((compare_output_mode_b       &  0b11) >> 0) << COM1B0);
	
	TCCR1B = (((clock_select                & 0b111) >> 0) << CS10)
           | (((waveform_generation_mode    & 0b100) >> 2) << WGM12)
           | (((input_capture_edge_select   &   0b1) >> 0) << ICES1)
           | (((input_capture_noise_cancler &   0b1) >> 0) << ICNC1);
    
    constexpr uint8_t overflow_interrupt_enable               = 0;
    constexpr uint8_t output_compare_a_match_interrupt_enable = 1;
    constexpr uint8_t output_compare_b_match_interrupt_enable = 0;
    constexpr uint8_t input_capture_interrupt_enable          = 0;
    
    TIMSK1 = (((overflow_interrupt_enable               & 0b1) >> 0) << TOIE1)
           | (((output_compare_a_match_interrupt_enable & 0b1) >> 0) << OCIE1A)
           | (((output_compare_b_match_interrupt_enable & 0b1) >> 0) << OCIE1B)
           | (((input_capture_interrupt_enable          & 0b1) >> 0) << ICIE1);
    
    OCR1A  = steps - 1;
}

uint32_t getMsTimestamp() {
	uint32_t val1 = counter;
	uint32_t val2 = counter;
	while(val1 != val2) {
		val1 = val2;
		val2 = counter;
	}
	return val2;
}

ISR(TIMER1_COMPA_vect) {
	static uint8_t    error     =  0;
	
	if(error > nominator) {
		error -= (nominator - devisor);
		OCR1A = steps;
	} else {
		error += devisor;
		OCR1A = steps - 1;
	}
	counter++;
}

} // End of: namespace timestamp
