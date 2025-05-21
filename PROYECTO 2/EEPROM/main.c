#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)
#define SERVO_COUNT 4  // Definición añadida aquí

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>  // Incluido para atoi()

// Estructura para almacenar posiciones
typedef struct {
	uint8_t angle[SERVO_COUNT];
} ServoPosition;

// Direcciones EEPROM
#define MAX_POSITIONS 10
#define EEPROM_BASE_ADDR 0

// Prototipos de funciones
void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char* str);
void UART_SendNumber(uint8_t num);
void PWM_Init(void);
void SetServo(uint8_t servo, uint8_t angle);
void SavePosition(uint8_t pos_index);
void PlayPosition(uint8_t pos_index);
void ShowHelp(void);

int main(void) {
	UART_Init();
	PWM_Init();
	
	UART_SendString("\r\nSistema de servos con EEPROM\r\n");
	ShowHelp();
	
	char buffer[16];
	uint8_t index = 0;
	
	while(1) {
		if (UCSR0A & (1 << RXC0)) {
			char c = UDR0;
			
			// Eco para verificar recepción
			UART_SendChar(c);
			
			if (c == '\r' || c == '\n') {
				buffer[index] = '\0';
				index = 0;
				
				// Procesar comandos
				if (strcmp(buffer, "HELP") == 0) {
					ShowHelp();
				}
				else if (strncmp(buffer, "S", 1) == 0 && strlen(buffer) >= 3 && buffer[2] == '=') {
					// Comando servo: S1=90
					uint8_t servo = buffer[1] - '1';
					uint8_t angle = atoi(buffer + 3);
					if (servo < SERVO_COUNT) {
						SetServo(servo, angle);
						UART_SendString("\r\nPosicion actualizada");
					}
				}
				else if (strncmp(buffer, "SAVE", 4) == 0 && strlen(buffer) == 5) {
					// Guardar posición: SAVE1
					uint8_t pos_index = buffer[4] - '0';
					if (pos_index < MAX_POSITIONS) {
						SavePosition(pos_index);
						UART_SendString("\r\nPosicion guardada en slot ");
						UART_SendNumber(pos_index);
					}
				}
				else if (strncmp(buffer, "PLAY", 4) == 0 && strlen(buffer) == 5) {
					// Reproducir posición: PLAY1
					uint8_t pos_index = buffer[4] - '0';
					if (pos_index < MAX_POSITIONS) {
						PlayPosition(pos_index);
						UART_SendString("\r\nReproduciendo posicion ");
						UART_SendNumber(pos_index);
					}
				}
				else if (strlen(buffer) > 0) {
					UART_SendString("\r\nComando no reconocido");
				}
			}
			else if (index < sizeof(buffer) - 1) {
				buffer[index++] = c;
			}
		}
	}
}

void ShowHelp(void) {
	UART_SendString("\r\nComandos disponibles:");
	UART_SendString("\r\nS1=90 - Mover servo 1 a 90 grados");
	UART_SendString("\r\nSAVE1 - Guardar posiciones en slot 1 (0-9)");
	UART_SendString("\r\nPLAY1 - Reproducir posiciones del slot 1");
	UART_SendString("\r\nHELP - Mostrar esta ayuda\r\n");
}

void SavePosition(uint8_t pos_index) {
	ServoPosition current_pos;
	
	// Obtener posiciones actuales
	for (uint8_t i = 0; i < SERVO_COUNT; i++) {
		switch(i) {
			case 0: current_pos.angle[i] = (OCR1A - 2000) * 180 / 3250; break;
			case 1: current_pos.angle[i] = (OCR1B - 2000) * 180 / 3250; break;
			case 2: current_pos.angle[i] = (OCR0B - 20) * 180 / 35; break;
			case 3: current_pos.angle[i] = (OCR2B - 20) * 180 / 35; break;
		}
	}
	
	// Guardar en EEPROM
	eeprom_write_block(&current_pos, (void*)(EEPROM_BASE_ADDR + pos_index * sizeof(ServoPosition)), sizeof(ServoPosition));
}

void PlayPosition(uint8_t pos_index) {
	ServoPosition saved_pos;
	
	// Leer de EEPROM
	eeprom_read_block(&saved_pos, (void*)(EEPROM_BASE_ADDR + pos_index * sizeof(ServoPosition)), sizeof(ServoPosition));
	
	// Aplicar posiciones
	for (uint8_t i = 0; i < SERVO_COUNT; i++) {
		SetServo(i, saved_pos.angle[i]);
	}
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

void UART_SendNumber(uint8_t num) {
	char buffer[4];
	itoa(num, buffer, 10);
	UART_SendString(buffer);
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