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
#define RESPONSE_TIME_MS 100
#define SEATBELT_CHECK_INTERVAL_MS 1000

static const char *TAG = "Benchmark_Example";

// Variáveis para medir o tempo
static uint64_t touch_start_time_us = 0;
static uint64_t check_start_time_us = 0;
static uint64_t touch_end_time_us = 0;

// Variável para monitorar se o toque foi detectado
static bool touch_detected = false;

// Variável para armazenar o handle do GPTimer
static gptimer_handle_t gptimer = NULL;

// Função de callback do GPTimer para medir o tempo de resposta
static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    touch_end_time_us = edata->count_value;  // Captura o tempo final em microsegundos
    return true;  // Retornar true para reiniciar o timer
}

// Função para inicializar e configurar o GPTimer para benchmark
static void init_gptimer(void)
{
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
}

static void tp_seatbelt(void *pvParameter)
{
    uint16_t touch_value;
    static TickType_t last_wake_time = 0;
    init_gptimer();  // Inicializar o GPTimer

#if TOUCH_FILTER_MODE_EN
    printf("Touch Sensor filter mode read, the output format is: \nTouchpad num:[raw data, filtered data]\n\n");
#else
    printf("Touch Sensor normal mode read, the output format is: \nTouchpad num:[raw data]\n\n");
#endif

    while (1) {
        // Ler o valor do pino touch 0
        touch_pad_read(0, &touch_value);
        printf("T0:[%4d] ", touch_value);

        // Captura o tempo de início e verifica o toque
        if (touch_value < TOUCH_THRESHOLD) {
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_start_time_us));
            printf("\nCinto de Segurança Ativado");
            touch_detected = true;  // O toque foi detectado
            // Esperar RESPONSE_TIME_MS milissegundos para garantir a resposta dentro do tempo limite
            vTaskDelay(RESPONSE_TIME_MS / portTICK_PERIOD_MS);
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_end_time_us));
            uint64_t elapsed_time_us = touch_end_time_us - touch_start_time_us;
            printf("\nTempo de resposta: %.2f ms\n", elapsed_time_us / 1000.0);  // Exibe o tempo em milissegundos
        }

        // Aguardar até o próximo ciclo fixo
        vTaskDelayUntil(&last_wake_time, SEATBELT_CHECK_INTERVAL_MS / portTICK_PERIOD_MS);
    }
}

static void seatbelt_check_task(void *pvParameter)
{
    static TickType_t last_check_time = 0;

    while (1) {
        if (!touch_detected) {
            printf("\nSem Cinto de Segurança\n");
        }
        // Resetar a variável para monitorar o próximo toque
        touch_detected = false;

        // Captura o tempo de início da verificação do cinto de segurança
        ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &check_start_time_us));
        // Esperar SEATBELT_CHECK_INTERVAL_MS antes da próxima verificação
        vTaskDelayUntil(&last_check_time, SEATBELT_CHECK_INTERVAL_MS / portTICK_PERIOD_MS);

        // Captura o tempo final e calcula o tempo entre verificações
        uint64_t check_end_time_us;
        ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &check_end_time_us));
        uint64_t check_elapsed_time_us = check_end_time_us - check_start_time_us;
        printf("Tempo entre verificações do cinto: %.2f ms\n", check_elapsed_time_us / 1000.0);  // Exibe o tempo em milissegundos
    }
}

static void tp_seatbelt_init(void)
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
    tp_seatbelt_init();
#if TOUCH_FILTER_MODE_EN
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
#endif
    // Iniciar a tarefa para ler valores dos pads de toque
    xTaskCreate(&tp_seatbelt, "touch_pad_read_task", 2048, NULL, 5, NULL);

    // Iniciar a tarefa para verificar o cinto de segurança a cada SEATBELT_CHECK_INTERVAL_MS
    xTaskCreate(&seatbelt_check_task, "seatbelt_check_task", 2048, NULL, 5, NULL);
}