/*
Jean Garcia y Jesus Hernandez

Crear un programa que imprime por serial un número aleatorio entre 1 y 6.
El número solo se actualiza cuando el usuario toca un pin táctil.
Mientras no se toca el pin táctil no se imprime nada.
*/


#include <stdio.h>
#include <stdlib.h>  
#include <time.h>    
#include "driver/touch_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TOUCH_THRESHOLD 300  
#define RETARDO 100  

void app_main() {
    srand(time(NULL));  //Semilla aleatoria 

    touch_pad_init();
    touch_pad_config(TOUCH_PAD_NUM0, TOUCH_THRESHOLD);

    bool was_touched = false;

    while (1) {
        uint16_t touch_value;
        touch_pad_read(TOUCH_PAD_NUM0, &touch_value);

        if (touch_value < TOUCH_THRESHOLD) {  //Detecta contacto
            if (!was_touched) {  
                int numero_aleatorio = (rand() % 6) + 1;  //Numero aleatorio de 1 a 6
                printf("Número aleatorio: %d\n", numero_aleatorio);
                was_touched = true;
            }
        } else {
            was_touched = false; //No hay contacto
        }

        vTaskDelay(RETARDO / portTICK_PERIOD_MS);
    }
}

