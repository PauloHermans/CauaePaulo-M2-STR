#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define TOUCH_THRESHOLD 100
#define RESPONSE_TIME_MS 99 //Uma tolêrancia de 1ms para o atraso do fio

static void tp_airbag(void *pvParameter)
{
    uint16_t touch_value;

#if TOUCH_FILTER_MODE_EN
    printf("Touch Sensor filter mode read, the output format is: \nTouchpad num:[raw data, filtered data]\n\n");
#else
    printf("Touch Sensor normal mode read, the output format is: \nTouchpad num:[raw data]\n\n");
#endif
    while (1) {
        // Ler o valor do pino touch 0
        touch_pad_read(0, &touch_value);
        printf("T0:[%4d] ", touch_value);

        // Verificar se o valor de toque está abaixo do limite
        if (touch_value < TOUCH_THRESHOLD) {
            printf("\nSistema de Airbag ativado");
            // Esperar 100ms para garantir a resposta dentro do tempo limite
            vTaskDelay(RESPONSE_TIME_MS / portTICK_PERIOD_MS);
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);  // Pequeno atraso para leitura constante
    }
}

static void tp_airbag_init(void)
{
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

void app_main(void)
{
    // Inicializar o periférico touch pad
    touch_pad_init();
    // Definir voltagem de referência
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    tp_airbag_init();
#if TOUCH_FILTER_MODE_EN
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
#endif
    // Iniciar a tarefa para ler valores dos pads de toque
    xTaskCreate(&tp_airbag, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
