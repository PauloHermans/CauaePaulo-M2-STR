#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "driver/gptimer.h"
#include "esp_log.h"

#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define TOUCH_THRESHOLD 100
#define RESPONSE_TIME_MS 100 // Tolerância de 1ms para o atraso do fio
#define READ_DELAY_MS 200    // Tempo de atraso entre leituras do sensor (200 ms)

// Variáveis para medir o tempo
static uint64_t start_time_us = 0;
static uint64_t end_time_us = 0;

// Função de callback do GPTimer
static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    end_time_us = edata->count_value;  // Captura o tempo final em microsegundos
    return true;  // Retornar true para reiniciar o timer
}

// Função para inicializar e configurar o GPTimer para benchmark
static gptimer_handle_t init_gptimer(void)
{
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,   // Fonte de clock padrão
        .direction = GPTIMER_COUNT_UP,        // Timer contando para cima
        .resolution_hz = 1000000,             // 1 MHz, ou seja, precisão de microsegundos
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,  // Callback quando o tempo atingir o limite
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    // Configurar o timer para contar sem alarme fixo
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 0,   // Não usamos um alarme fixo neste caso
        .flags.auto_reload_on_alarm = false,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    return gptimer;  // Retorna o handle do timer
}

static void tp_airbag(void *pvParameter)
{
    uint16_t touch_value;
    gptimer_handle_t gptimer = init_gptimer();  // Inicializar o GPTimer

#if TOUCH_FILTER_MODE_EN
    printf("Touch Sensor filter mode read, the output format is: \nTouchpad num:[raw data, filtered data]\n\n");
#else
    printf("Touch Sensor normal mode read, the output format is: \nTouchpad num:[raw data]\n\n");
#endif

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        // Ler o valor do pino touch 0
        touch_pad_read(0, &touch_value);
        printf("T0:[%4d] ", touch_value);

        // Verificar se o valor de toque está abaixo do limite
        if (touch_value < TOUCH_THRESHOLD) {
            // Captura o tempo de início
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &start_time_us));
            printf("\nSistema de Airbag ativado");
            // Simular o tempo de resposta esperado
            vTaskDelay(RESPONSE_TIME_MS / portTICK_PERIOD_MS);
            // Captura o tempo final e calcula o tempo de resposta
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &end_time_us));
            uint64_t elapsed_time_us = end_time_us - start_time_us;
            printf("\nTempo de resposta: %.2f ms\n", elapsed_time_us / 1000.0);  // Exibe o tempo em milissegundos
        }

        // Pequeno atraso para manter a leitura constante
        vTaskDelayUntil(&xLastWakeTime, READ_DELAY_MS / portTICK_PERIOD_MS);
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
