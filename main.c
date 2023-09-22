#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#define RS PC0
#define RW PC1
#define EN PC2



void LCD_cmd(char cmd);
void LCD_write(char data);
void LCD_clear();
void move_cursor(unsigned char x ,unsigned char y);
void LCD_print(const char *str);
void LCD_init();
void ADC_init();
uint16_t ADC_read();
char* itoa(int num, char* str, int base);
void reverse(char str[], int length);

void main(void) {
	DDRD = 0xFF; // PORTD as input
	DDRC = 0xFF; // PORTC as output
	DDRB = 0xFF; // PORTB as output

	LCD_init();
	ADC_init();

	char adcValueStr[10]; // String buffer to store ADC value

	while (1) {

		uint16_t temp = ADC_read();
		itoa(temp, adcValueStr, 10);
		// Display ADC value on the LCD
		move_cursor(0, 0);
		LCD_print("Temperature: ");
		LCD_print(adcValueStr);

		_delay_ms(100); // Delay for a short interval (adjust as needed)

		if (temp > 25) {
			PORTD |= (1 << PIND0);
			move_cursor(1, 4);
			LCD_print("* FAN ON *");
			} else {
			PORTD &= ~(1 << PIND0);
			move_cursor(1, 4);
			LCD_print("* FAN OFF *");
		}
	}
}

void ADC_init(void) {
	ADMUX |= (1 << REFS0) | (1 << REFS1);
	ADCSRA |= (1 << ADEN);
	ADCSRA |= ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
	ADMUX |= (1 << ADLAR);
}

uint16_t ADC_read() {
    // Select the LM35 input channel
    ADMUX = (ADMUX & 0xF8) | (6 & 0x07);

    // Start the conversion
    ADCSRA |= (1 << ADSC);

    // Wait for the conversion to complete
    while (ADCSRA & (1 << ADSC));

    // Read the ADC result (10-bit resolution)
    uint16_t adc_result = ADCH;

    // Convert ADC value to temperature in degrees Celsius
    // LM35 outputs 10 mV per degree Celsius, and the ADC is 10-bit
    // So, Temperature (in Celsius) = (ADC_value / 1024) * 500
    uint16_t temperature = (adc_result * 500UL) / 1024UL;

    return temperature;
}

void LCD_cmd(char cmd){
	PORTB = cmd;
	PORTC &= ~(1 << RS);
	PORTC &= ~(1 << RW);
	PORTC |= (1 << EN);
	_delay_ms(10);
	PORTC &= ~(1 << EN);
}

void LCD_write(char data){
	PORTB = data;
	PORTC |= (1 << RS);
	PORTC &= ~(1 << RW);
	PORTC |= (1 << EN);
	_delay_ms(10);
	PORTC &= ~(1 << EN);

}
void LCD_clear(){
	LCD_cmd(0x01);
	_delay_ms(10);
}
void LCD_print(const char *str){
	int i=0;
	while(str[i] != 0){
		LCD_write(str[i]);
		i++;
	}
}

void move_cursor(unsigned char x_pos, unsigned char y_pos) {
	uint8_t address = 0;
	if (x_pos == 0) {
		address = 0x80;
		} else if (x_pos == 1) {
		address = 0xC0;
		} if (y_pos < 16) {
		address += y_pos;
		LCD_cmd(address);
	}
}

void LCD_init(){
	LCD_cmd(0x38);  // 8-bit mode
	LCD_cmd(0x0C);
	LCD_cmd(0x06);
	LCD_cmd(0x80);
	_delay_ms(10);
}

void reverse(char str[], int length) {
	int start = 0;
	int end = length - 1;
	while (start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
}

char* itoa(int num, char* str, int base) {
	int i = 0;
	int isNegative = 0;

	// Handle 0 explicitly, otherwise empty string is printed
	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	// Handle negative numbers for bases other than 10
	if (num < 0 && base != 10) {
		isNegative = 1;
		num = -num;
	}

	// Convert the number to a string in reverse order
	while (num != 0) {
		int rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}

	// Append '-' for negative numbers in base 10
	if (isNegative && base == 10) {
		str[i++] = '-';
	}

	str[i] = '\0'; // Null-terminate the string

	// Reverse the string
	reverse(str, i);

	return str;
}

