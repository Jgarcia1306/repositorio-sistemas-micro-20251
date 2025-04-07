/*
Jean García y Jesús Hernandez

En un proyecto de telemetría se tiene un caudalímetro que envía por puerto
serial cada periodo de tiempo la cantidad de caudal detecta, el cual es un
número entero no mayor de 2 dígitos (es decir, se enviará un número entre
00 y 99)
Se recibirá el número por puerto serial, se debe imprimir por serial la
siguiente información:
1. El número mínimo recibido.
2. El número mayor recibido.
3. El último número recibido.
4. El promedio de todos los números recibidos.

Último: 10. Minimo: 10. Máximo: 10. Promedio: 10.00
Último: 1. Minimo: 1. Máximo: 10. Promedio: 5.50
Último: 27. Minimo: 1. Máximo: 27. Promedio: 12.67
En el anterior ejemplo, se han enviado por serial 3 números en orden que
son 10, 1 y 27. Donde al recibir el tercer dato (27), el número menor en ese
momento es 1, el número mayor es 10, el último número recibido es 27 y el
promedio de la sumatoria de todos los números recibidos es (10 + 1 + 27)/3
= 12.667
Si se recibe algo diferente entre 00 y 99 (sea número o letra) se debe
ignorar.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "string.h"
#include "ctype.h"
#include "stdlib.h"

#define UART_PORT UART_NUM_0 //UART 
#define BUF_SIZE 1024 //Tamaño del buffer
#define MAX_NUMEROS 30 //Maximo de numeros que puede ingresar el usuario

int min_num = INT_MAX; //Variable entera numero minimo 
int max_num = INT_MIN; //Variable entera numero maximo 
int last_num = 0; //Variable para determinar el ultimo numero
int total_nums = 0; //Variable para determinar el total de numeros ingresados
long sum_nums = 0; //Sumatoria de los numeros 
int numeros_ingresados[MAX_NUMEROS]; 

bool es_numero_valido(char *str) { //Determinar entero positivo
    if (*str == '\0') return false;

    if (*str == '+') { //Signo + antes del entero como opcion de ingreso
        str++;
    }

    int digit_count = 0; // Verificar que todos sean dígitos
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        digit_count++;
        if (digit_count > 2) return false; //Condición maxima de 2 dígitos
        str++;
    }
    return (digit_count > 0); //Condicion minima menos 1 dígito
}

bool rango_enteros(int num) { //Rango de enteros
    return (num >= 0 && num <= 99);
}

void reset_info() { //Permitir reiniciar la estadistica 
    min_num = INT_MAX;
    max_num = INT_MIN;
    last_num = 0;
    total_nums = 0;
    sum_nums = 0;
}

void info_enviar(int num) {  //Comprobar que no se supere el limite de enteros a ingresar
    if (total_nums < MAX_NUMEROS) {
        numeros_ingresados[total_nums] = num;
    }

    if (num < min_num) min_num = num; //Determinar menor entero
    if (num > max_num) max_num = num; //Determinar mayor entero
    last_num = num; //Determinar ultimo entero ingresado
    sum_nums += num; //Determinar sumatoria de los numeros
    total_nums++; //Determinar el total de enteros ingresados
}

void mostrar_info() { //Imprimir los resultados obtenidos en base a la entrada
    printf("\n--- Procesado ---\n");
    printf("Enteros ingresados: ["); //Mostrar enteros ingresados
    for (int i = 0; i < total_nums; i++) {
        printf("%d", numeros_ingresados[i]);
        if (i < total_nums - 1) printf(", ");
    }
    printf("]\n");
    
    printf("Mínimo: %d\n", min_num);
    printf("Máximo: %d\n", max_num);
    printf("Último: %d\n", last_num);
    
    if (total_nums > 0) {
        float promedio = (float)sum_nums / total_nums;
        printf("Promedio: %.2f\n", promedio);
    }
    printf("------------------\n");
}

void leer_entrada(char *input) {
    char *token = strtok(input, " ,"); // Separar por espacios o comas
    
    while (token != NULL && total_nums < MAX_NUMEROS) {
        if (es_numero_valido(token)) {
            int num = atoi(token);
            if (rango_enteros(num)) {
                info_enviar(num);
            } else {
                printf("Número supera el rango: %s (0-99)\n", token);
            }
        } else {
            printf("Entrada incorrecta: %s\n", token);
        }
        token = strtok(NULL, " ,");
    }
}

void app_main() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_PORT, &uart_config); //Configuración puerto UART 0
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    char buffer[BUF_SIZE];

    printf("Enteros: Maximo, Minimo, Último ingresado y promedio\n");
    printf("Ingrese varios números separados por espacios o comas\n");

    while (1) {
        int len = uart_read_bytes(UART_PORT, (uint8_t *)buffer, BUF_SIZE - 1, pdMS_TO_TICKS(100));

        //Condicionamiento de caracteres en la entrada
        
        if (len > 0) { 
            buffer[len] = '\0';
            
            char *input = buffer;
            while (*input == '\n' || *input == '\r' || *input == ' ') {
                input++;
            }
            
            char *end = input + strlen(input) - 1;
            while (end >= input && (*end == '\n' || *end == '\r' || *end == ' ')) {
                *end = '\0';
                end--;
            }
            
            if (strlen(input) > 0) {
                reset_info(); // Reiniciamos para nueva entrada
                leer_entrada(input);
                
                if (total_nums > 0) {
                    mostrar_info();
                } else {
                    printf("No se ingresaron enteros correctos\n");
                }
                
                printf("\nIngrese nuevos enteros (0-99) separados entre si por espacios/comas:\n");
            }
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}