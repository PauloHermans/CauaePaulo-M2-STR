#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_flash.h"
#include "esp_chip_info.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_timer.h"
#include <inttypes.h>
#include <stdio.h>

// Definição dos pinos GPIO para cada sensor.
#define SENSOR_AIRBAG_PIN 35
#define SENSOR_ABS_PIN 27
#define SENSOR_INJECAO_PIN 32
#define SENSOR_TEMPERATURA_PIN 33
#define SENSOR_CINTO_PIN 22

// Definição dos intervalos de monitoramento em milissegundos (ms).
#define TEMPO_AIRBAG_MS 99
#define TEMPO_ABS_MS 99
#define TEMPO_INJECAO_US 490
#define TEMPO_TEMPERATURA_MS 19
#define TEMPO_CINTO_MS 999

QueueHandle_t queue_sensor_data;
gptimer_handle_t gptimer = NULL;

uint64_t wcet_injecao = 0, wcet_temperatura = 0, wcet_abs = 0, wcet_airbag = 0, wcet_cinto = 0;
uint64_t wcrt_injecao = 0, wcrt_temperatura = 0, wcrt_abs = 0, wcrt_airbag = 0, wcrt_cinto = 0;

void configurar_sensores();
void tempo_airbag(void *pvParameter);
void tempo_abs(void *pvParameter);
void tempo_injecao(void *pvParameter);
void tempo_temperatura(void *pvParameter);
void tempo_cinto(void *pvParameter);

void configurar_sensores() {
    esp_rom_gpio_pad_select_gpio(SENSOR_INJECAO_PIN);
    gpio_set_direction(SENSOR_INJECAO_PIN, GPIO_MODE_INPUT);

    esp_rom_gpio_pad_select_gpio(SENSOR_TEMPERATURA_PIN);
    gpio_set_direction(SENSOR_TEMPERATURA_PIN, GPIO_MODE_INPUT);

    esp_rom_gpio_pad_select_gpio(SENSOR_ABS_PIN);
    gpio_set_direction(SENSOR_ABS_PIN, GPIO_MODE_INPUT);

    esp_rom_gpio_pad_select_gpio(SENSOR_AIRBAG_PIN);
    gpio_set_direction(SENSOR_AIRBAG_PIN, GPIO_MODE_INPUT);

    esp_rom_gpio_pad_select_gpio(SENSOR_CINTO_PIN);
    gpio_set_direction(SENSOR_CINTO_PIN, GPIO_MODE_INPUT);
}

void tempo_airbag(void *pvParameter) {
    uint64_t tempo_inicio, tempo_fim, tempo_execucao, tempo_resposta;

    while (1) {
        gptimer_get_raw_count(gptimer, &tempo_inicio);

        if (gpio_get_level(SENSOR_AIRBAG_PIN)) {
            printf("Airbag acionado!\n");

            gptimer_get_raw_count(gptimer, &tempo_fim);
            tempo_execucao = tempo_fim - tempo_inicio;

            // Atualiza WCET para Airbag
            if (tempo_execucao > wcet_airbag) {
                wcet_airbag = tempo_execucao;
            }

            // Cálculo do WCRT para Airbag
            tempo_resposta = tempo_execucao + TEMPO_AIRBAG_MS * 1000;
            if (tempo_resposta > wcrt_airbag) {
                wcrt_airbag = tempo_resposta;
            }

            printf("WCET Airbag: %" PRIu64 " us, WCRT Airbag: %" PRIu64 " us\n", wcet_airbag, wcrt_airbag);

        }

        vTaskDelay(pdMS_TO_TICKS(TEMPO_AIRBAG_MS));
    }
}

void tempo_abs(void *pvParameter) {
    uint64_t tempo_inicio, tempo_fim, tempo_execucao, tempo_resposta;

    while (1) {
        gptimer_get_raw_count(gptimer, &tempo_inicio);

        if (gpio_get_level(SENSOR_ABS_PIN)) {
            printf("ABS acionado!\n");

            gptimer_get_raw_count(gptimer, &tempo_fim);
            tempo_execucao = tempo_fim - tempo_inicio;

            // Atualiza WCET para ABS
            if (tempo_execucao > wcet_abs) {
                wcet_abs = tempo_execucao;
            }

            // Cálculo do WCRT para ABS
            tempo_resposta = tempo_execucao + TEMPO_ABS_MS * 1000;
            if (tempo_resposta > wcrt_abs) {
                wcrt_abs = tempo_resposta;
            }

            printf("WCET ABS: %" PRIu64 " us, WCRT ABS: %" PRIu64 " us\n", wcet_abs, wcrt_abs);

        }

        vTaskDelay(pdMS_TO_TICKS(TEMPO_ABS_MS));
    }
}

void tempo_injecao(void *pvParameter) {
    uint64_t tempo_inicio, tempo_fim, tempo_execucao, tempo_resposta;

    while (1) {
        gptimer_get_raw_count(gptimer, &tempo_inicio); // Início da medição do tempo de resposta

        // Medição de execução real da tarefa
        if (gpio_get_level(SENSOR_INJECAO_PIN)) {
            printf("Injeção eletrônica acionada!\n");

            gptimer_get_raw_count(gptimer, &tempo_fim); // Fim da medição
            tempo_execucao = tempo_fim - tempo_inicio;

            // Atualiza WCET, se necessário
            if (tempo_execucao > wcet_injecao) {
                wcet_injecao = tempo_execucao;
            }

            // Cálculo do WCRT com intervalo entre execuções
            tempo_resposta = tempo_execucao + TEMPO_INJECAO_US; // Adiciona 500 microssegundos
            if (tempo_resposta > wcrt_injecao) {
                wcrt_injecao = tempo_resposta;
            }

            printf("WCET injeção: %" PRIu64 " us, WCRT injeção: %" PRIu64 " us\n", wcet_injecao, wcrt_injecao);
            
            esp_rom_delay_us(TEMPO_INJECAO_US); // Delay de 500 microssegundos
        
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    }
}

void tempo_temperatura(void *pvParameter) {
    uint64_t tempo_inicio, tempo_fim, tempo_execucao, tempo_resposta;

    while (1) {
        gptimer_get_raw_count(gptimer, &tempo_inicio);

        if (gpio_get_level(SENSOR_TEMPERATURA_PIN)) {
            printf("Temperatura do motor acima do limite!\n");

            gptimer_get_raw_count(gptimer, &tempo_fim);
            tempo_execucao = tempo_fim - tempo_inicio;

            // Atualiza WCET
            if (tempo_execucao > wcet_temperatura) {
                wcet_temperatura = tempo_execucao;
            }

            // Cálculo do WCRT
            tempo_resposta = tempo_execucao + TEMPO_TEMPERATURA_MS * 1000;
            if (tempo_resposta > wcrt_temperatura) {
                wcrt_temperatura = tempo_resposta;
            }

            printf("WCET temperatura: %" PRIu64 " us, WCRT temperatura: %" PRIu64 " us\n", wcet_temperatura, wcrt_temperatura);

        }

        vTaskDelay(pdMS_TO_TICKS(TEMPO_TEMPERATURA_MS));
    }
}

void tempo_cinto(void *pvParameter) {
    uint64_t tempo_inicio, tempo_fim, tempo_execucao, tempo_resposta;

    while (1) {
        gptimer_get_raw_count(gptimer, &tempo_inicio);

        if (gpio_get_level(SENSOR_CINTO_PIN)) {
            printf("Cinto de segurança acionado!\n");

            gptimer_get_raw_count(gptimer, &tempo_fim);
            tempo_execucao = tempo_fim - tempo_inicio;

            // Atualiza WCET para Cinto
            if (tempo_execucao > wcet_cinto) {
                wcet_cinto = tempo_execucao;
            }

            // Cálculo do WCRT para Cinto
            tempo_resposta = tempo_execucao + TEMPO_CINTO_MS * 1000;
            if (tempo_resposta > wcrt_cinto) {
                wcrt_cinto = tempo_resposta;
            }

            printf("WCET Cinto: %" PRIu64 " us, WCRT Cinto: %" PRIu64 " us\n", wcet_cinto, wcrt_cinto);

        }

        vTaskDelay(pdMS_TO_TICKS(TEMPO_CINTO_MS));
    }
}

void app_main() {
    configurar_sensores();

    gptimer_config_t config = {
        .clk_src = GPTIMER_CLK_SRC_APB,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000  // Contador em microsegundos
    };
    gptimer_new_timer(&config, &gptimer);
    gptimer_enable(gptimer);  // Habilita o temporizador antes de iniciar
    gptimer_start(gptimer);   // Inicia o temporizador

    xTaskCreate(tempo_airbag, "airbag", 2048, NULL, 6, NULL);
    xTaskCreate(tempo_abs, "abs", 2048, NULL, 5, NULL);
    xTaskCreate(tempo_injecao, "injecao", 2048, NULL, 4, NULL);
    xTaskCreate(tempo_temperatura, "temperatura", 2048, NULL, 3, NULL);
    xTaskCreate(tempo_cinto, "cinto", 2048, NULL, 2, NULL);
}