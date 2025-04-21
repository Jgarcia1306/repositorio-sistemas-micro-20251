#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "string.h"
#include "ctype.h"
#include "esp_timer.h"
#include "driver/touch_pad.h"
#include <driver/adc.h>

#define POT_INP ADC2_CHANNEL_0
#define TOUCH_UMBRAL 200
#define BUF_SIZE 1024

float ADC_VOLT_MAX = 3.3;
int RESOLUCION = 12;
uint16_t touch_value;

// Configuración de UART
static void uart_init(int BAUD_RATE)
{
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, 1, 3, 22, 19));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, ESP_INTR_FLAG_IRAM));
}

void app_main()
{
    // Configurar ADC
    adc2_config_channel_atten(POT_INP, ADC_ATTEN_DB_11);

    // Configurar UART
    uart_init(115200);

    // Configurar táctil
    touch_pad_init();
    touch_pad_config(TOUCH_PAD_NUM4, TOUCH_UMBRAL);

    uint8_t string_txt[BUF_SIZE];

    while (1)
    {
        int len = uart_read_bytes(UART_NUM_0, string_txt, BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (len > 0)
        {
            string_txt[len] = '\0';

            // Solo aceptar si la cadena recibida es exactamente "1\r"
            if (strcmp((char *)string_txt, "1\r") == 0)
            {
                printf("Comprobado, presione el táctil\n");

                while (1)
                {
                    touch_pad_read(TOUCH_PAD_NUM4, &touch_value);

                    if (touch_value < TOUCH_UMBRAL)
                    {
                        int read_raw;
                        adc2_get_raw(POT_INP, ADC_WIDTH_BIT_12, &read_raw);

                        float por = ((float)read_raw / (pow(2, RESOLUCION) - 1)) * 100.0;

                        char salida[100];
                        snprintf(salida, sizeof(salida), "ADC = %d | %.2f%%\r\n", read_raw, por);
                        uart_write_bytes(UART_NUM_0, salida, strlen(salida));

                        vTaskDelay(pdMS_TO_TICKS(1000));
                    }
                }
            }
            else
            {
                printf("Ingresar nuevamente cadena por serial\n");
                char mensaje_error[] = "Ingresar nuevamente cadena por serial\r\n";
                uart_write_bytes(UART_NUM_0, mensaje_error, strlen(mensaje_error));
            }
        }
    }
}
