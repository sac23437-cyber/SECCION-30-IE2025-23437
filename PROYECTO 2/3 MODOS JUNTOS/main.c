#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD-1)
#define SERVO_COUNT 4
#define FILTER_SAMPLES 3
#define MAX_POSITIONS 10
#define EEPROM_BASE_ADDR 0

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

// Definición de modos
typedef enum {
	MODO_MANUAL = 0,
	MODO_EEPROM = 1,
	MODO_UART = 2
} ModoOperacion;

// Variables globales
volatile uint16_t adc_values[4];
volatile uint8_t current_channel = 0;
volatile ModoOperacion modo_actual = MODO_MANUAL;

char uart_buffer[32];
uint8_t uart_index = 0;
uint8_t uart_ready = 0;

// Estructura para EEPROM
typedef struct {
	uint8_t angle[SERVO_COUNT];
} ServoPosition;

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
void ADC_Init(void);
uint16_t filtered_adc(uint8_t channel);
void cambiarModo(void);
void actualizarLEDs(void);


// Para el modo EEPROM--------------------------------------------------------------------
void ProcessEEPROMCommand(char* command) {
	if (strcmp(command, "HELP") == 0) {
		ShowHelp();
	}
	else if (strncmp(command, "S", 1) == 0 && strlen(command) >= 3 && command[2] == '=') {
		uint8_t servo = command[1] - '1';
		uint8_t angle = atoi(command + 3);
		if (servo < SERVO_COUNT) {
			SetServo(servo, angle);
			UART_SendString("\r\nPosicion actualizada");
		}
	}
	else if (strncmp(command, "SAVE", 4) == 0 && strlen(command) == 5) {
		uint8_t pos_index = command[4] - '0';
		if (pos_index < MAX_POSITIONS) {
			SavePosition(pos_index);
			UART_SendString("\r\nPosicion guardada en slot ");
			UART_SendNumber(pos_index);
		}
	}
	else if (strncmp(command, "PLAY", 4) == 0 && strlen(command) == 5) {
		uint8_t pos_index = command[4] - '0';
		if (pos_index < MAX_POSITIONS) {
			PlayPosition(pos_index);
			UART_SendString("\r\nReproduciendo posicion ");
			UART_SendNumber(pos_index);
		}
	}
	else if (strlen(command) > 0) {
		UART_SendString("\r\nComando no reconocido");
	}
}

// Para el modo EEPROM--------------------------------------------------------------------



int main(void) {
	// Configurar PB0, PD6 y PD7 como salidas (LEDs de modo)
	DDRB |= (1 << PB0);    // LED Modo Manual (PB0)
	DDRD |= (1 << PD6) | (1 << PD7);  // LEDs Modo EEPROM (PD6) y UART (PD7)
	
	// Configurar PB5 como entrada (botón para cambiar modo)
	DDRB &= ~(1 << PB5);
	PORTB &= ~(1 << PB5);  // Sin pull-up inicial
	
	// Inicializaciones
	UART_Init();
	PWM_Init();
	ADC_Init();
	
	// Mostrar mensaje inicial
	UART_SendString("\r\nSistema de control de servos multifuncion\r\n");
	UART_SendString("Modo actual: MANUAL\r\n");
	UART_SendString("Presione el boton para cambiar de modo\r\n");
	ShowHelp();
	
	// Estado inicial de LEDs
	actualizarLEDs();
	
	// Habilitar interrupciones globales
	sei();
	
	while(1) {
		// Verificación del botón para cambiar modo
		if (!(PINB & (1 << PB5))) {
			_delay_ms(50);
			while (!(PINB & (1 << PB5)));
			_delay_ms(50);
			
			cambiarModo();
			actualizarLEDs();
			
			// Limpiar buffer UART
			memset(uart_buffer, 0, sizeof(uart_buffer));
			uart_index = 0;
			
			// Mostrar mensaje del modo actual
			UART_SendString("\r\nModo cambiado a: ");
			switch(modo_actual) {
				case MODO_MANUAL:
				UART_SendString("MANUAL\r\n");
				break;
				case MODO_EEPROM:
				UART_SendString("EEPROM\r\n> ");
				break;
				case MODO_UART:
				UART_SendString("UART (Control desde Adafruit)\r\n");  // Cambiado
				break;
			}
		}
		
		// Procesamiento adicional para modo MANUAL
		if (modo_actual == MODO_MANUAL) {
			// Código del ADC para el modo manual
		}
	}
}

void cambiarModo(void) {
	modo_actual = (modo_actual + 1) % 3;
}

void actualizarLEDs(void) {
	// Apagar todos los LEDs primero
	PORTB &= ~(1 << PB0);
	PORTD &= ~((1 << PD6) | (1 << PD7));
	
	// Encender solo el LED correspondiente al modo actual
	switch(modo_actual) {
		case MODO_MANUAL:
		PORTB |= (1 << PB0);  // LED PB0 para Manual
		break;
		case MODO_EEPROM:
		PORTD |= (1 << PD7);  // LED PD7 para EEPROM
		break;
		case MODO_UART:
		PORTD |= (1 << PD6);  // LED PD6 para UART
		break;
	}
}

// Funciones para el modo MANUAL (ADC)
void ADC_Init(void) {
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	DIDR0 = 0x0F; // Deshabilitar entradas digitales en A0-A3
	ADMUX = (1 << REFS0);
	ADCSRB = 0;
	PORTC |= 0xF0; // Pull-ups en A4-A7
	ADCSRA |= (1 << ADSC);
}

uint16_t filtered_adc(uint8_t channel) {
	static uint16_t samples[4][FILTER_SAMPLES];
	static uint8_t index = 0;
	samples[channel][index] = ADC;
	index = (index + 1) % FILTER_SAMPLES;
	uint32_t sum = 0;
	for(uint8_t i=0; i<FILTER_SAMPLES; i++) sum += samples[channel][i];
	return sum / FILTER_SAMPLES;
}


ISR(USART_RX_vect) {
	char c = UDR0;
	
	if (modo_actual == MODO_EEPROM) {
		// Hacer eco del carácter solo en modo EEPROM
		if(c != '\r' && c != '\n') {
			UART_SendChar(c);
		}
		
		// Procesar comando completo al presionar Enter (solo modo EEPROM)
		if (c == '\r' || c == '\n') {
			uart_buffer[uart_index] = '\0';
			if(uart_index > 0) {
				ProcessEEPROMCommand(uart_buffer);
				UART_SendString("\r\n> ");
			}
			uart_index = 0;
		}
		else if (uart_index < sizeof(uart_buffer) - 1) {
			// Convertir a mayúsculas si es letra
			if(c >= 'a' && c <= 'z') {
				c -= 32;
			}
			uart_buffer[uart_index++] = c;
		}
	}
	else if (modo_actual == MODO_UART) {
		// Solo procesar comandos en formato "Sx=valor\n" sin eco
		if (c == '\r' || c == '\n') {
			uart_buffer[uart_index] = '\0';
			if(uart_index > 0) {
				// Verificar si es un comando de servo válido
				if(uart_buffer[0] == 'S' &&
				(uart_buffer[1] >= '1' && uart_buffer[1] <= '4') &&
				uart_buffer[2] == '=') {
					uint8_t servo = uart_buffer[1] - '1';
					uint8_t angle = atoi(uart_buffer + 3);
					SetServo(servo, angle);
					
					// Enviar confirmación al dashboard
					UART_SendString("OK S");
					UART_SendChar(uart_buffer[1]);
					UART_SendString("=");
					UART_SendString(uart_buffer + 3);
					UART_SendString("\n");
				}
			}
			uart_index = 0;
		}
		else if (uart_index < sizeof(uart_buffer) - 1) {
			uart_buffer[uart_index++] = c;
		}
	}
}


ISR(ADC_vect) {
	if(modo_actual == MODO_MANUAL) {
		adc_values[current_channel] = filtered_adc(current_channel);
		
		current_channel = (current_channel + 1) % 4;
		ADMUX = (1 << REFS0) | (current_channel + 4); // A4-A7
		
		// Asignación con rangos correctos
		OCR1A = 1000 + (adc_values[3] * 4000UL / 1023); // A7 ? PB1 (1-2ms)
		OCR1B = 2000 + (adc_values[2] * 3250UL / 1023); // A6 ? PB2 (1-2ms)
		OCR0B = 26 + (adc_values[1] * 35 / 1023);       // A5 ? PD5 (1-2ms)
		OCR2B = 26 + (adc_values[0] * 35 / 1023);       // A4 ? PD3 (1-2ms)
	}
	ADCSRA |= (1 << ADSC);
}

// Funciones para el modo EEPROM
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

// Funciones comunes
void ShowHelp(void) {
	UART_SendString("\r\nComandos disponibles por modo:");
	UART_SendString("\r\n[MODO MANUAL]: Control por potenciómetros");
	UART_SendString("\r\n[MODO EEPROM]:");
	UART_SendString("\r\n  S1=90 - Mover servo 1 a 90 grados");
	UART_SendString("\r\n  SAVE1 - Guardar posiciones en slot 1 (0-9)");
	UART_SendString("\r\n  PLAY1 - Reproducir posiciones del slot 1");
	UART_SendString("\r\n[MODO UART]: Control exclusivo desde dashboard");
	UART_SendString("\r\nHELP - Mostrar esta ayuda\r\n");
}

void UART_Init(void) {
	UBRR0H = (MYUBRR >> 8);
	UBRR0L = MYUBRR;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // Añadir RXCIE0
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
	// Configurar pines como salidas
	DDRB |= (1 << PB1) | (1 << PB2);  // PB1 (OC1A) y PB2 (OC1B)
	DDRD |= (1 << PD3) | (1 << PD5);  // PD3 (OC2B) y PD5 (OC0B)
	
	// Timer1 (16-bit) para servos 1 y 2 (PB1 y PB2)
	// Modo Fast PWM con ICR1 como TOP (Modo 14)
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8
	ICR1 = 39999; // Periodo de 20ms (50Hz) para servos
	
	// Valores iniciales (1.5ms - posición central)
	OCR1A = 3000; // Servo 1 (PB1)
	OCR1B = 3000; // Servo 2 (PB2)
	
	    // Timer0 (8-bit) para servo 3 (PD5)
	    // Modo Phase Correct PWM, TOP=0xFF, Prescaler 256
	    TCCR0A = (1 << COM0B1) | (1 << WGM00);
	    TCCR0B = (1 << CS02); // Prescaler 256
	    
	    // Timer2 (8-bit) para servo 4 (PD3)
	    // Modo Phase Correct PWM, TOP=0xFF, Prescaler 256
	    TCCR2A = (1 << COM2B1) | (1 << WGM20);
	    TCCR2B = (1 << CS22) | (1 << CS21); // Prescaler 256
	    
	    // Valores iniciales (1.5ms - posición central)
	    OCR1A = 3000; // Servo 1 (PB1) - 1.5ms
	    OCR1B = 3000; // Servo 2 (PB2) - 1.5ms
	    OCR0B = 38;   // Servo 3 (PD5) - 1.5ms (256/20ms*1.5ms ? 19.2 ? 19)
	    OCR2B = 38;   // Servo 4 (PD3) - 1.5ms
}


void SetServo(uint8_t servo, uint8_t angle) {
	if (angle > 180) angle = 180;
	
	switch(servo) {
		case 0: // Servo 1 (PB1) - Timer1A
		OCR1A = 2000 + (angle * 2000UL / 180); // 1-2ms (2000-4000)
		break;
		case 1: // Servo 2 (PB2) - Timer1B
		OCR1B = 2000 + (angle * 2000UL / 180); // 1-2ms (2000-4000)
		break;
		case 2: // Servo 3 (PD5) - Timer0B
		OCR0B = 13 + (angle * 13 / 180); // 1-2ms (13-26)
		break;
		case 3: // Servo 4 (PD3) - Timer2B
		OCR2B = 13 + (angle * 13 / 180); // 1-2ms (13-26)
		break;
	}
}
