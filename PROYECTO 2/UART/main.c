#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>  // Para atoi()

// Prototipos
void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char* str);
void PWM_Init(void);
void SetServo(uint8_t servo, uint8_t angle);
void ProcessCommand(char* command);

int main(void) {
	// 1. Inicialización
	UART_Init();
	PWM_Init();
	
	UART_SendString("\r\nSistema de servos UART\r\n");
	UART_SendString("Comandos: S1=90, S2=45, etc (0-180)\r\n");
	
	char buffer[16];
	uint8_t index = 0;
	
	while(1) {
		if (UCSR0A & (1 << RXC0)) {
			char c = UDR0;
			
			// Eco para verificación
			if(c != '\r' && c != '\n') {
				UART_SendChar(c);
			}
			
			if (c == '\r' || c == '\n') {
				buffer[index] = '\0';
				if(index > 0) {
					ProcessCommand(buffer);
				}
				index = 0;
			}
			else if (index < sizeof(buffer) - 1) {
				// Convertir a mayúsculas si es letra
				if(c >= 'a' && c <= 'z') c -= 32;
				buffer[index++] = c;
			}
		}
	}
}

void ProcessCommand(char* command) {
	// Comando HELP
	if(strcmp(command, "HELP") == 0) {
		UART_SendString("\r\nComandos validos:\r\n");
		UART_SendString("S1=90 - Mueve servo 1 a 90°\r\n");
		UART_SendString("S2=45 - Mueve servo 2 a 45°\r\n");
		UART_SendString("S3=180 - Mueve servo 3 a 180°\r\n");
		UART_SendString("S4=0 - Mueve servo 4 a 0°\r\n");
		UART_SendString("HELP - Muestra ayuda\r\n");
		return;
	}
	
	// Comandos de servo (S1=90)
	if(command[0] == 'S' &&
	(command[1] >= '1' && command[1] <= '4') &&
	command[2] == '=' &&
	strlen(command) >= 4) {
		
		uint8_t servo = command[1] - '1';
		uint8_t angle = atoi(command + 3);
		
		SetServo(servo, angle);
		UART_SendString("\r\nOK - Servo ");
		UART_SendChar(command[1]);
		UART_SendString(" a ");
		UART_SendString(command + 3);
		UART_SendString("°\r\n");
		return;
	}
	
	// Comando no reconocido
	UART_SendString("\r\nERROR: Comando no reconocido: ");
	UART_SendString(command);
	UART_SendString("\r\n");
}

void UART_Init(void) {
	UBRR0H = (MYUBRR >> 8);
	UBRR0L = MYUBRR;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendChar(char c) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

void UART_SendString(const char* str) {
	while (*str) UART_SendChar(*str++);
}

void PWM_Init(void) {
	DDRB |= (1 << PB1) | (1 << PB2);
	DDRD |= (1 << PD3) | (1 << PD5);
	
	// Timer1 (PB1 y PB2)
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	ICR1 = 39999;
	OCR1A = 3000;
	OCR1B = 3000;
	
	// Timer0 (PD5)
	TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
	TCCR0B = (1 << CS02) | (1 << CS00);
	OCR0B = 38;
	
	// Timer2 (PD3)
	TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
	OCR2B = 38;
}

void SetServo(uint8_t servo, uint8_t angle) {
	if (angle > 180) angle = 180;
	
	switch(servo) {
		case 0: OCR1A = 2000 + (angle * 3250UL / 180); break; // PB1
		case 1: OCR1B = 2000 + (angle * 3250UL / 180); break; // PB2
		case 2: OCR0B = 20 + (angle * 35 / 180); break;       // PD5
		case 3: OCR2B = 20 + (angle * 35 / 180); break;       // PD3
	}
}