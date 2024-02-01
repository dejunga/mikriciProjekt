#include <avr/io.h>
#include "AVR VUB/avrvub.h"
#include <util/delay.h>
#include "LCD/lcd.h"
#include "ADC/adc.h"

#define F_CPU 16000000UL // Assuming a clock speed of 16 MHz for ATmega32U4
#define BAUD 9600
#define BRC ((F_CPU/16/BAUD) - 1)
const int SAMPLES_PER_SECOND = 10; // Number of samples to take in one second

void uart_init() {
    // Set baud rate
    UBRR1H = (BRC >> 8);
    UBRR1L = BRC;

    // Enable receiver and transmitter
    UCSR1B = (1 << RXEN1) | (1 << TXEN1);
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);
}

void uart_transmit(char data) {
    // Wait for empty transmit buffer
    while (!(UCSR1A & (1 << UDRE1)));
    // Put data into buffer, sends the data
    UDR1 = data;
}

void uart_print(const char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

void inicijalizacija() {
	// konfigurirajte ADC i LCD
	lcd_init();
	adc_init();
	uart_init(); // Initialize UART
}

int main(void) {
	inicijalizacija();
	uint16_t ADC_5; // vrijednost AD pretvorbe na pinu ADC5
	float U_ADC5; // napon na pinu ADC5
	const float V_REF = 5.0; // AVCC je referentni napon
	float T; // temperatura u okolini senzora LM35
	char buffer[20]; // Buffer for temperature string
	
	while (1) {
		float sumT = 0;
		
		for (int i = 0; i < SAMPLES_PER_SECOND; i++) {
        ADC_5 = adc_read(ADC5);
        U_ADC5 = ADC_5 * V_REF / 1023.0;
        T = U_ADC5 * 100; // Temperature in Celsius from sensor LM35

        sumT += T; // Add the temperature reading to the sum

        // Wait a fraction of a second before next reading
        _delay_ms(1000 / SAMPLES_PER_SECOND);
    }
	
	// Calculate the average temperature
    float avgT = sumT / SAMPLES_PER_SECOND;

    // Convert average temperature to string and send over UART
    snprintf(buffer, sizeof(buffer), "Avg T = %0.2fC\r\n", avgT);
    uart_print(buffer);

    // Display on LCD
    lcd_clrscr();
    lcd_home();
    lcd_print("Avg %.2fC", avgT);

    // Wait for a while before taking the next set of readings
    _delay_ms(1000 - (1000 / SAMPLES_PER_SECOND * SAMPLES_PER_SECOND));
}
}