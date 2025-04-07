/*
Jean García y Jesús Hernandez

1. Se puede calcular el cuadrado de un número N al sumar los N primeros
números impares, así 7 al cuadrado es igual a 1+3+5+7+9+11+13=49,
donde la serie del 1 al 13 son los primeros 7 números impares.
2. Describir una solución para el microcontrolador que calcule el cuadrado
de un número recibido por puerto serial e imprima el resultado por
serial.
3. Si se recibe algo diferente a un número entero se debe ignorar.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "string.h"
#include "ctype.h"

#define UART_PORT UART_NUM_0  //UART integrado en el ESP32
#define BUF_SIZE 1024         // Tamaño del buffer de recepción

long calcular_cuadrado(int n) { // Función para calcular el cuadrado sumando N primeros impares
    long resultado = 0;
    int impar_actual = 1;
    
    if (n < 0) { //Tomar entero negativo como positivo (cuadrado igual)
        n = -n;
    }
    
    for (int i = 0; i < n; i++) {
        resultado += impar_actual;
        impar_actual += 2;
    }
    return resultado;
}

bool es_numero_valido(char *str) { //Determinar que la cadena sean enteros 
    if (*str == '\0') return false;  

    if (*str == '-' || *str == '+') { //Validar para enteros negativos o positivos con signo
        str++;  
    }
    
    while (*str) {  // Verifica que los caracteres sean dígitos 
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}

void app_main() {
    
    uart_config_t uart_config = {  //Configuración UART
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_PORT, &uart_config);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    char buffer[BUF_SIZE];  // Buffer para almacenar datos recibidos
    
    printf("Ingrese un número entero:\n");

    while (1) {
        
        int len = uart_read_bytes(UART_PORT, (uint8_t *)buffer, BUF_SIZE - 1, pdMS_TO_TICKS(100)); //Interpretación del mensaje recibido por UART
        
        if (len > 0) {
            buffer[len] = '\0';  //Adaptamos a cadena
            
            char *input = buffer;
            while (*input == '\n' || *input == '\r' || *input == ' ') {
                input++;
            }
            
            char *end = input + strlen(input) - 1;
            while (end >= input && (*end == '\n' || *end == '\r' || *end == ' ')) {
                *end = '\0';
                end--;
            }
            
            if (strlen(input) > 0 && es_numero_valido(input)) {    //Determinar que sea entero
                int n = atoi(input);  // Convertimos a entero
                long cuadrado = calcular_cuadrado(n);
                
                printf("\n%d² = ", n); //Calculo por el método de números impares
                int impar = 1;
                for (int i = 0; i < abs(n); i++) {  //abs(n) para permitir enteros negativos 
                    printf("%d", impar);
                    if (i < abs(n) - 1) {
                        printf(" + ");
                    }
                    impar += 2;
                }
                printf(" = %ld\n", cuadrado);
            } else {
                printf("\nNo valido, ingrese un entero\n");
            }
            
            printf("\nIngrese un entero:\n");
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}