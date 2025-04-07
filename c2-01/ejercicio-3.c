/*
Jean García y Jesús Hernandez

1. Desarrolla un sistema de autenticación basado en un patrón táctil.
2. El usuario debe tocar la secuencia en un pin táctil:
(a) 3 toques largos.
(b) 3 toques cortos.
(c) 3 toques largos.
3. Luego de hacer esa secuencia, se debe tocar otro pin táctil para validar
la secuencia.
4. Imprimir por serial "APROBADO" o "NO APROBADO" si la secuencia ingresada es correcta o no.
5. Por grupo definir el tiempo a su criterio para determinar que es "toque
largo" y por "toque corto".
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/touch_pad.h"

#define TOUCH_PIN TOUCH_PAD_NUM0   // Pin táctil a usar GPIO4
#define TOUCH_PIN_VALIDO TOUCH_PAD_NUM3 // Pin táctil para validar GPIO15
#define TOUCH_THRESHOLD 300       
#define RETARDO 100              
#define TIEMPO_SEGUNDO 5           // Valor establecido para determinar la duración del toque en segundos

void app_main() {
    touch_pad_init();  // Iniciar subsistema de pines táctiles
    touch_pad_config(TOUCH_PIN, 0); // Configurar pin táctil principal
    touch_pad_config(TOUCH_PIN_VALIDO, 0); // Configurar pin táctil de validación

    bool touch = false;
    uint64_t tiempo_inicial = 0;
    int secuencia[9];  // Para almacenar la secuencia de toques
    int index = 0;
    bool validacion_disponible = false;  // Solo permite validar después de completar la secuencia
    bool validacion_tocada = false; // Detectar flanco de bajada en validación

    while (1) {
        uint16_t touch_value;
        touch_pad_read(TOUCH_PIN, &touch_value);

        if (!validacion_disponible) { // Solo registrar toques si no se ha completado la secuencia
            if (touch_value < TOUCH_THRESHOLD) {  //Condicion para determinar contacto o no 
                if (!touch) {
                    tiempo_inicial = esp_timer_get_time();
                    touch = true;
                }
            } else {
                if (touch) {
                    uint64_t tiempo_final = esp_timer_get_time();
                    uint64_t tiempo_transcurrido = (tiempo_final - tiempo_inicial) / 1000000;
                    printf("Toque %d: %llu seg\n", index + 1, tiempo_transcurrido);
                    
                    if (index < 9) {    //Para determinar la secuencia de toques largos y cortos 
                        secuencia[index++] = (tiempo_transcurrido >= TIEMPO_SEGUNDO) ? 1 : 0;  
                    }
                    if (index == 9) {
                        validacion_disponible = true; // Activar validación solo después de la secuencia completa
                        printf("Secuencia completa. Toque el pin de validación.\n");
                    }
                    touch = false;
                }
            }
        }
        
        uint16_t touch_valido; //Para validar la secuencia de toques
        touch_pad_read(TOUCH_PIN_VALIDO, &touch_valido);
        
        if (validacion_disponible) {
            if (touch_valido < TOUCH_THRESHOLD) {
                validacion_tocada = true; // Se detecta contacto en el pin de validación
            } else if (validacion_tocada) { // Esperar a que se suelte el contacto para validar
                int secuencia_correcta[9] = {1, 1, 1, 0, 0, 0, 1, 1, 1};
                bool aprobada = true;
                for (int i = 0; i < 9; i++) {
                    if (secuencia[i] != secuencia_correcta[i]) {
                        aprobada = false;
                        break;
                    }
                }
                printf(aprobada ? "APROBADO\n" : "NO APROBADO\n");
                index = 0;
                validacion_disponible = false;
                validacion_tocada = false;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(RETARDO));  
    }
}
