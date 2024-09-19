#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#include "esp_rom_sys.h"  // Para usar esp_rom_delay_us()

#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define TOUCH_THRESHOLD 100
#define DELAY_MICROSECONDS 499

static void tp_injecao_eletronica(void *pvParameter)
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

        // Verificar se o valor de toque está abaixo do limite
        if (touch_value < TOUCH_THRESHOLD) {
            printf("\nInjeção Eletrônica Funcionando\n");
        }

        // Esperar 499 microsegundos antes da próxima verificação
        esp_rom_delay_us(DELAY_MICROSECONDS);
    }
}

static void tp_injecao_eletronica_init(void)
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
    tp_injecao_eletronica_init();
#if TOUCH_FILTER_MODE_EN
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
#endif
    // Iniciar a tarefa para verificar o touch a cada 500 microsegundos
    xTaskCreate(&tp_injecao_eletronica, "touch_pad_injecao_task", 2048, NULL, 5, NULL);
}
