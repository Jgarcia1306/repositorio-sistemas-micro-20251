/*
Jean Garcia y Jesus Hernandez

De los pines táctiles disponibles en su placa, escoger uno e imprimir
cuantas veces se ha tocado el pin táctil. Tener en cuenta que se debe
considerar un "toque" como presionar y soltar el pin táctil.
*/

#include <stdio.h>
#include "driver/touch_pad.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TOUCH_THRESHOLD  300         // Valor fijado para detectar o no contacto del pin
#define RETARDO 100 


static int touch_count = 0;
static bool touch_state = false;  // Indica si el sensor esta en contacto 

void app_main() {
    
    // Inicializar el touch pad
    
    touch_pad_init();  //Iniciar subsistema de pines tactiles
    touch_pad_config(TOUCH_PAD_NUM0, 0 ); //Configurar para el GPIO4 (Touch 0)

    while (1) {
        uint16_t touch_value;
        touch_pad_read(TOUCH_PAD_NUM0 , &touch_value);

        if (touch_value < TOUCH_THRESHOLD) {  // Condición para detectar un toque
            if (!touch_state) {              // Se activa en el primer contacto
                touch_state = true;
            }
        } else {  // Si se suelta el toque
            if (touch_state) {  // Solo contar cuando se libera
                touch_count++;
                printf("Pin táctil tocado %d veces\n", touch_count);
                touch_state = false;
            }
        }
        vTaskDelay(RETARDO / portTICK_PERIOD_MS);  
    }
}