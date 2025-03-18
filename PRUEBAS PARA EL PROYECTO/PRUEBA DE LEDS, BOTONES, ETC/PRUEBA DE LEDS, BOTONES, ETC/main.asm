; Definiciones de registros y pines
.include "m328pdef.inc"

; Configuración de los puertos
    sbi DDRB, PB5      ; Configura PB5 como salida (LED)
    cbi PORTB, PB5     ; Apaga el LED en PB5 inicialmente

    sbi DDRC, PC0      ; Configura PC0 como salida (LED)
    cbi PORTC, PC0     ; Apaga el LED en PC0 inicialmente

    //sbi DDRC, PC1      ; Configura PC1 como salida (LED)
    //cbi PORTC, PC1     ; Apaga el LED en PC1 inicialmente

    cbi DDRC, PC1      ; Configura PC2 como entrada (Botón)
    cbi PORTC, PC1     ; Desactiva la resistencia de pull-up interna (pull-down externo)

; Bucle principal
main:
    sbis PINC, PC1     ; Salta la siguiente instrucción si PC2 está en alto (botón presionado)
    rjmp main          ; Si no está presionado, vuelve al inicio del bucle

    ; Encender los LEDs
    sbi PORTB, PB5     ; Enciende el LED en PB5
    sbi PORTC, PC0     ; Enciende el LED en PC0
    //sbi PORTC, PC1     ; Enciende el LED en PC1

    ; Esperar a que el botón sea liberado
debounce:
    sbic PINC, PC1     ; Salta la siguiente instrucción si PC2 está en bajo (botón liberado)
    rjmp debounce      ; Si no está liberado, sigue esperando

    ; Apagar los LEDs
    cbi PORTB, PB5     ; Apaga el LED en PB5
    cbi PORTC, PC0     ; Apaga el LED en PC0
    //cbi PORTC, PC1     ; Apaga el LED en PC1

    rjmp main          ; Vuelve al inicio del bucle principal