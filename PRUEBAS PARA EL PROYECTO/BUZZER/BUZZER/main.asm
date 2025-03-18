; Configuraci�n de registros
    .include "m328Pdef.inc"  ; Incluir definiciones para el ATmega328P

    .cseg
    .org 0x00                ; Inicio del programa
    rjmp main                ; Saltar a la etiqueta main

main:
    ; Configuraci�n de puertos
    sbi DDRD, PD7            ; Configurar PD7 (D7) como salida (buzzer)
    cbi DDRC, PC3            ; Configurar PC3 (A3) como entrada (bot�n)
    cbi PORTC, PC3           ; Asegurar que el pull-down est� habilitado (sin resistencia interna pull-up)

loop:
    sbic PINC, PC3           ; Verificar si el bot�n est� presionado (PC3 = 1)
    rjmp buzzer_on           ; Si est� presionado, activar buzzer
    cbi PORTD, PD7           ; Si no est� presionado, apagar buzzer
    rjmp loop                ; Repetir el ciclo

buzzer_on:
    sbi PORTD, PD7           ; Encender el buzzer
    rjmp loop                ; Volver al inicio del loop