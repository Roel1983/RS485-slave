#ifndef FAKEAVR_AVR_INTERRUPT_H_
#define FAKEAVR_AVR_INTERRUPT_H_

void FakeIoReset();

#define cli()
#define sei()

#define ISR(vec) void isr_vect_##vec()

#endif
