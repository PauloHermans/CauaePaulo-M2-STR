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
#define RESPONSE_TIME_MS 99 // Uma tolerância de 1ms para o atraso do fio
#define DELAY_MICROSECONDS 499
#define SEATBELT_CHECK_INTERVAL_MS 1000

static const char *TAG = "Benchmark_Example";

// Variáveis para medir o tempo
static uint64_t touch_start_time_us = 0;
static uint64_t touch_end_time_us = 0;

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

static void tp_abs(void *pvParameter)
{
    uint16_t touch_value;
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

        // Verificar se o valor de toque está abaixo do limite
        if (touch_value < TOUCH_THRESHOLD) {
            // Captura o tempo de início
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_start_time_us));
            printf("\nSistema de ABS Ativado");
            // Esperar 99ms para garantir a resposta dentro do tempo limite
            vTaskDelay(RESPONSE_TIME_MS / portTICK_PERIOD_MS);
            // Captura o tempo final e calcula o tempo de resposta
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_end_time_us));
            uint64_t elapsed_time_us = touch_end_time_us - touch_start_time_us;
            printf("Tempo de resposta: %.2f ms\n", elapsed_time_us / 1000.0);  // Exibe o tempo em milissegundos
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);  // Pequeno atraso para leitura constante
    }
}

static void tp_abs_init(void)
{
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
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
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

static void tp_airbag_init(void)
{
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

static void tp_injecao_eletronica(void *pvParameter)
{
    uint16_t touch_value;
    init_gptimer();  // Inicializar o GPTimer

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
            // Captura o tempo de início
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_start_time_us));
            printf("\nInjeção Eletrônica Funcionando\n");
            // Esperar 499 microsegundos antes da próxima verificação
            esp_rom_delay_us(DELAY_MICROSECONDS);
            // Captura o tempo final e calcula o tempo de resposta
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_end_time_us));
            uint64_t elapsed_time_us = touch_end_time_us - touch_start_time_us;
            printf("Tempo de resposta: %.2f ms\n", elapsed_time_us / 1000.0);  // Exibe o tempo em milissegundos
        }
        
        // Atraso antes da próxima verificação do valor do toque
        vTaskDelay(1 / portTICK_PERIOD_MS);  // Ajuste o atraso conforme necessário
    }
}

static void tp_injecao_eletronica_init(void)
{
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

static void tp_seatbelt(void *pvParameter)
{
    uint16_t touch_value;
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

        // Verificar se o valor de toque está abaixo do limite
        if (touch_value < TOUCH_THRESHOLD) {
            // Captura o tempo de início
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_start_time_us));
            printf("\nCinto de Segurança Ativado");
            touch_detected = true;  // O toque foi detectado
            // Esperar RESPONSE_TIME_MS milissegundos para garantir a resposta dentro do tempo limite
            vTaskDelay(RESPONSE_TIME_MS / portTICK_PERIOD_MS);
            // Captura o tempo final e calcula o tempo de resposta
            ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &touch_end_time_us));
            uint64_t elapsed_time_us = touch_end_time_us - touch_start_time_us;
            printf("\nTempo de resposta: %.2f ms\n", elapsed_time_us / 1000.0);  // Exibe o tempo em milissegundos
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);  // Pequeno atraso para leitura constante
    }
}

static void seatbelt_check_task(void *pvParameter)
{
    while (1) {
        if (!touch_detected) {
            printf("\nSem Cinto de Segurança\n");
        }
        // Resetar a variável para monitorar o próximo toque
        touch_detected = false;

        // Captura o tempo de início da verificação do cinto de segurança
        ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &check_start_time_us));
        // Esperar 1 segundo antes da próxima verificação
        vTaskDelay(SEATBELT_CHECK_INTERVAL_MS / portTICK_PERIOD_MS);

        // Captura o tempo final e calcula o tempo de resposta
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
    tp_abs_init();
    tp_airbag_init();
    tp_injecao_eletronica_init();
    tp_seatbelt_init();
#if TOUCH_FILTER_MODE_EN
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
#endif
    // Iniciar a tarefa para ler valores dos pads de toque
    xTaskCreate(&tp_abs, "touch_pad_read_task", 2048, NULL, 5, NULL);
    xTaskCreate(&tp_airbag, "touch_pad_read_task", 2048, NULL, 5, NULL);
    xTaskCreate(&tp_injecao_eletronica, "touch_pad_injecao_task", 2048, NULL, 5, NULL);
    xTaskCreate(&tp_seatbelt, "touch_pad_read_task", 2048, NULL, 5, NULL);
    xTaskCreate(&seatbelt_check_task, "seatbelt_check_task", 2048, NULL, 5, NULL);
}
