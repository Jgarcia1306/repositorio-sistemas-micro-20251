/*
Jean Garcia y Jesus Hernandez

Por serial recibir uno o varios carácteres (que puede ser un número,
letra o símbolo). El ESP32 debe enviar por serial si lo recibido fue un
número entero o no.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "string.h"
#include <ctype.h>
#include <stdbool.h>

#define UART_NUM UART_NUM_0
#define BUF_TAMAÑO 1024

bool tiene_mayusculas_o_tildes(const uint8_t *str, int len) {   //Estructura para detectar mayúsculas/tildes
    for (int i = 0; i < len; i++) {
        // Detectar mayúsculas (A-Z)
        if (str[i] >= 'A' && str[i] <= 'Z') {
            return true;
        }
        // Detectar tildes en UTF-8 (Á, É, Í, Ó, Ú)
        if (str[i] == 0xC3 && i+1 < len && 
            (str[i+1] >= 0x81 && str[i+1] <= 0x89)) {
            return true;
        }
    }
    return false;
}

void app_main() {
    uart_config_t uart_config = {  //UART
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(UART_NUM, &uart_config); //Configuración de la UART integrada en el ESP32
    uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_TAMAÑO * 2, 0, 0, NULL, 0);

    uint8_t data[BUF_TAMAÑO];
    char mensaje[] = "\nIngrese un valor: "; //Entrada
    uart_write_bytes(UART_NUM, mensaje, strlen(mensaje));

    while (1) {
        int len = uart_read_bytes(UART_NUM, data, BUF_TAMAÑO - 1, 0);
        
        if (len > 0) {
            data[len] = '\0';
            
            // Limpiar saltos de línea
            if (len > 0 && (data[len-1] == '\n' || data[len-1] == '\r')) {
                data[len-1] = '\0';
                len--;
            }

            char salida[BUF_TAMAÑO + 50]; // Mostrar entrada del usuario
            snprintf(salida, sizeof(salida), "\nInput: '%s'", data);
            uart_write_bytes(UART_NUM, salida, strlen(salida));

            bool es_entero = true;  // Verificar si es entero (sin tener en cuenta mayúsculas o tildes)
            int digit_count = 0;
            int i = 0;
            
            if (tiene_mayusculas_o_tildes(data, len)) { //Definir para mayúscula y tildes como no enteros
                es_entero = false;
            } else {
                
                if (len > 0 && data[0] == '-') { // Permitir signo negativo
                    i = 1;
                }
                
                for (; i < len; i++) {   // Verificar dígitos ingresados (en caso tal)
                    if (!isdigit(data[i])) {
                        es_entero = false;
                        break;
                    }
                    digit_count++;
                }
            }

            snprintf(salida, sizeof(salida),  //Determinar si es entero o no 
                "\nResultado: %s\n",
                (es_entero && digit_count > 0) ? "Es entero" : "No es entero");
            uart_write_bytes(UART_NUM, salida, strlen(salida));

            uart_write_bytes(UART_NUM, mensaje, strlen(mensaje));  //Retardo para repetición de la entrada 
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}