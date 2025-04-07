/*
Jean García y Jesús Hernadez

Por serial imprimir si el toque a un pin táctil fue de corta o larga
duración. Por grupo definir que se considera de corta o larga duración.
Ejemplo: pueden decir que todo toque menor a 3 segundos es de corta 
duración y más de 3 segundos es de corta duración.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/touch_pad.h"

#define TOUCH_PIN TOUCH_PAD_NUM0   // Pin táctil a usar
#define TOUCH_THRESHOLD 300       
#define RETARDO 100              
#define TIEMPO_SEGUNDO 5           // Valor establecido para determinar la duración del toque (en segundos)

void app_main() {
    
    touch_pad_init();  // Iniciar subsistema de pines táctiles
    touch_pad_config(TOUCH_PIN, 0); // Configurar para el GPIO4 (Touch 0)

    bool touch = false; // Estado del toque
    uint64_t tiempo_inicial = 0;
    
    while (1) {
        uint16_t touch_value;
        touch_pad_read(TOUCH_PIN, &touch_value); 

        if (touch_value < TOUCH_THRESHOLD) { //Condición donde se detecta contacto
            if (!touch) {  //Registrar el tiempo al inicio del toque
                tiempo_inicial = esp_timer_get_time();
                touch = true;
            }
        } else { // Si no hay contacto
            if (touch) {  // Calcular tiempo transcurrido (duración)
                uint64_t tiempo_final = esp_timer_get_time();
                uint64_t tiempo_transcurrido = (tiempo_final - tiempo_inicial) / 1000000; // Tiempo de contacto en segundos

                printf("Tiempo ocupado en presionar: %llu seg\n", tiempo_transcurrido);

                if (tiempo_transcurrido < TIEMPO_SEGUNDO) {
                    printf("El toque es de corta duración\n");
                } else {
                    printf("El toque es de larga duración\n");
                }

                touch = false; 
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(RETARDO));  
    }
}
