#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
// Controle PWM do servo
#include "driver/ledc.h"
// Delay em microssegundos
#include "esp_rom_sys.h"
// Timer da ESP32
#include "esp_timer.h"
// DEFINIÇÃO DOS PINOS


// Pino TRIG do HC-SR04 - Ele vai enviar o ultrassom.
#define TRIG_PIN GPIO_NUM_12

// Pino ECHO do HC-SR04 - Ele vai receber o retorno do ultrassom.
#define ECHO_PIN GPIO_NUM_13

// Pino do servo motor - Ele vai controlar o servo motor.
#define SERVO_PIN GPIO_NUM_14

// FUNÇÃO PARA MEDIR DISTÂNCIA e retorna valor decimal.

float medir_distancia()
{
    // Variáveis do tempo 
    // vai criar um tempo inicial e um tempo final.
    int64_t inicio = 0;
    int64_t fim = 0;

    // Garante TRIG desligado
    gpio_set_level(TRIG_PIN, 0);

    // Espera 2 microssegundos
    esp_rom_delay_us(2);

    // Liga TRIG e envia ultrassom.
    gpio_set_level(TRIG_PIN, 1);

    // Mantém ligado por 10 microssegundos (Esse é o tempo exigido pelo HC-SR04).
    esp_rom_delay_us(10);

    // Desliga TRIG - O pulso terminou.
    gpio_set_level(TRIG_PIN, 0);

    
    while (gpio_get_level(ECHO_PIN) == 0)
    /* ESP espera:  ECHO ficar HIGH, Enquanto estiver LOW:o som 
    ainda não voltou */
    {
    }

    // o instante exato que o ECHO começou.
    inicio = esp_timer_get_time();

    //Enquanto ECHO estiver HIGH: o som ainda está retornando.
    while (gpio_get_level(ECHO_PIN) == 1)
    {
    }

    // O instante que o retorno terminou.
    fim = esp_timer_get_time();

/* Vai calcular o: tempo final - tempo inicial. Vai sair
o Resultado: tempo do ultrassom*/
    float tempo = fim - inicio;

/* Vamos criar uma variável  float para calcular a distância
detectada pelo sensor ultrassônico. Usamos float porque o valor
da distância pode possuir casas decimais.
O valor 0.034 representa a velocidade do som em cm por microssegundo.
A divisão é por 2 porque o ultrassom percorre o caminho de ida
até o objeto e depois retorna ao sensor. Ai dividimos por 2 por que
prescisamos do resultado para obter apenas a distância entre o
sensor e o objeto.*/
    float distancia = tempo * 0.034 / 2;

    // Retorna a distância medida.
    return distancia;
}
void app_main()
{  // CONFIGURA GPIOs

    // TRIG como saída, envia o sinal
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);

    // ECHO como entrada, recebe o sinal
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

  // CONFIGURA PWM DO SERVO, a parte mais dificil por não conhecer nada.
    
      ledc_timer_config_t timer = { //Cria estrutura do timer PWM
        .speed_mode = LEDC_HIGH_SPEED_MODE, //modo rápido
        .timer_num = LEDC_TIMER_0, //timer 0
        .duty_resolution = LEDC_TIMER_13_BIT, /*Define resolução do PWM 13
        bits: maior precisão. */
        .freq_hz = 50, // Mesma Frequência do servo motor
        .clk_cfg = LEDC_AUTO_CLK}; //ESP32 escolhe automaticamente o clock do PWM

    // Aplica configuração do timer
    ledc_timer_config(&timer);

    // Configuração do canal PWM
    ledc_channel_config_t canal = {
        .gpio_num = SERVO_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};

    // Aplica configuração do canal
    ledc_channel_config(&canal);

    // ==========================
    // LOOP PRINCIPAL
    // ==========================

    while (1)
    {
        // Mede distância
        float distancia = medir_distancia();

        // Mostra no terminal
        printf("Distancia: %.2f cm\n", distancia);

        // Verifica aproximação
        if (distancia < 20)
        {
            // Mensagem
            printf("Objeto detectado\n");

            // ==========================
            // ABRIR PORTA
            // ==========================

            // PWM para abrir
            ledc_set_duty(
                LEDC_HIGH_SPEED_MODE,
                LEDC_CHANNEL_0,
                600);

            // Atualiza PWM
            ledc_update_duty(
                LEDC_HIGH_SPEED_MODE,
                LEDC_CHANNEL_0);

            // Mostra mensagem
            printf("Porta aberta\n");

            // Espera 3 segundos
            vTaskDelay(pdMS_TO_TICKS(3000));

            // ==========================
            // FECHAR PORTA
            // ==========================

            // PWM para fechar
            ledc_set_duty(
                LEDC_HIGH_SPEED_MODE,
                LEDC_CHANNEL_0,
                200);

            // Atualiza PWM
            ledc_update_duty(
                LEDC_HIGH_SPEED_MODE,
                LEDC_CHANNEL_0);

            // Mostra mensagem
            printf("Porta fechada\n");
        }

        // Espera 500 ms
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}