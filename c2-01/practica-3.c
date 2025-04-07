/*
Jean Garcia y Jesus Hernandez

Desarrollar un programa donde se toque un pin táctil LUEGO de
mostrar un mensaje por serial. La idea es que se toque lo más rápido
posible, por lo cual se debe imprimir el tiempo transcurrido.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/touch_pad.h"

#define TOUCH_UMBRAL 300 
#define RETARDO 100 

void app_main(void) {
   
    touch_pad_init();
    touch_pad_config(TOUCH_PAD_NUM0, 0);

    while (1) {
        
        printf("Presione\n\n");  //Mostrar mensaje para tomar tiempo transcurrido
        uint64_t tiempo_inicial = esp_timer_get_time();
        uint16_t touch_value;

        while (1) {
            touch_pad_read(TOUCH_PAD_NUM0, &touch_value);

            if (touch_value < TOUCH_UMBRAL) { //Contacto 
                uint64_t tiempo_final = esp_timer_get_time();
                uint64_t tiempo_transcurrido = tiempo_final - tiempo_inicial;

                printf("Tiempo ocupado de presion: %llu seg\n", tiempo_transcurrido / 1000000);
                printf("Tiempo ocupado de presion: %llu ms\n", tiempo_transcurrido / 1000);

                while (touch_value < TOUCH_UMBRAL) {  //No hay contacto
                    touch_pad_read(TOUCH_PAD_NUM0, &touch_value);
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                break;
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        printf("Reposo de %d segundos para repeticion\n", RETARDO / 1000); //Retardo para repetir procedimiento
        vTaskDelay(pdMS_TO_TICKS(RETARDO));
    }
}
