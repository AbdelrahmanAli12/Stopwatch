#define F_CPU 1000000UL // MCU Clock Frequency
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
// Declare global variables
volatile uint8_t sec1 = 0, sec2 = 0, min1 = 0, min2 = 0, hour1 = 0, hour2 = 0;
volatile uint8_t stopwatch_paused = 0;
volatile uint16_t pause_time;
uint16_t seg[6];

// Function prototypes
void setup_timer1();
void init_INT0();
void init_INT1();
void init_INT2();
int main() {
	//7 segment display activation
	DDRC|=0x0F;
	//
	setup_timer1();               // Configure Timer1
	init_INT0();                  //reset
	init_INT1();             //pause
	init_INT2();             //resume
	sei();                        // Enable global interrupts

	while (1) {
		seg[0]=sec1;
		seg[1]=sec2;
		seg[2]=min1;
		seg[3]=min2;
		seg[4]=hour1;
		seg[5]=hour2;
		// Display stop watch time on six 7-segment displays
		// Loop over the six 7-segments
		for(int i=0; i<6; i++)
		{
			// Select the current 7-segment
			PORTA = (1<<i);

			// Insert the value of the corresponding element from the seg array
			PORTC = (PORTC & 0xF0) | (seg[i] & 0x0F);

			// Delay for 7-segment processing
			_delay_ms(2);
		}
	}
}
//timer 1 compare mode setup
void setup_timer1() {
	TCCR1B |= (1 << WGM12);        // CTC mode, TOP = OCR1A
	TIMSK |= (1 << OCIE1A);        // Enable Timer1 output compare match A interrupt
	OCR1A = 977;                 // Set output compare value for 1 second at 1 MHz and 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10);   // Start Timer1 with 1024 prescaler
}
//reset setup
void init_INT0() {
	// Set PD2 as input with internal pull-up resistor
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);
	// Configure INT0 for falling edge trigger
	MCUCR |= (1 << ISC01);
	// Enable INT0
	GICR |= (1 << INT0);
}
//pause
void init_INT1() {
	DDRD &= ~(1 << PD3); // Set PD3 as input
	PORTD |= (1 << PD3); // Enable internal pull-up resistor for PD3
	MCUCR |= (1 << ISC11) | (1 << ISC10); // Set INT1 to trigger on raising edge
	GICR |= (1 << INT1);
}
//resume
void init_INT2() {
	// Configure INT0, INT1, INT2 as external interrupts
	DDRB &= ~(1 << PB2);
	PORTB |= (1 << PB2);
	GICR |= (1 << INT2);
}

ISR(TIMER1_COMPA_vect) {
	// Update time
	if (++sec1 == 10) {
		sec1 = 0;
		if (++sec2 == 6) {
			sec2 = 0;
			if (++min1 == 10) {
				min1 = 0;
				if (++min2 == 6) {
					min2 = 0;
					if (++hour1 == 10) {
						hour1 = 0;
						if (++hour2 == 10) {
							hour2 = 0;
						}
					}
				}
			}
		}
	}}
ISR(INT0_vect) {
	// Reset Stop Watch time
	sec1= 0;
	sec2= 0;
	min1= 0;
	min1= 0;
	hour1= 0;
	hour2= 0;
}
ISR(INT1_vect) {
	// Disable Timer1 clock source (stop counting)
	TCCR1B &= ~(1 << CS10) & ~(1 << CS12);
	// Store current Timer1 value as pause time
	pause_time = TCNT1;
	// Update flag to indicate stop watch is paused
	stopwatch_paused = 1;
}
ISR(INT2_vect) {
	stopwatch_paused = 0;
	//resume
	TCCR1B |= (1 << CS10) | (1 << CS12);
}
