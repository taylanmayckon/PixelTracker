#include <stdio.h>
#include "pico/stdlib.h"
#include "libs/structs.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "libs/font.h"
#include "libs/ssd1306.h"
#include "hardware/i2c.h"
#include "pico/rand.h"

// Definindo os pinos do Led RGB
LEDs rgb = { 
    .red = 13,
    .green = 11,
    .blue = 12
};

// Definindo os pinos dos botões
BUTTONS button = {
    .a = 5,
    .b = 6,
};

// Definindo os pinos dos buzzers
BUZZERS buzzer = {
    .a = 21,
    .b = 10,
};

JOY joystick = {
    .x_pin = 27,
    .y_pin = 26,
    .button = 22
};

// Definindo a máscara para ativar a output GPIO 
#define OUTPUT_MASK ((1 << rgb.blue) | (1 << rgb.green) | (1 << rgb.red))
// Definindo a máscara para ativar a input GPIO
#define INPUT_MASK ((1 << button.a) | (1 << button.b) | (1 << joystick.button))

// Variáveis do PWM
uint wrap = 2047;
uint clkdiv = 125;

// Variável para o debounce (armazena tempo)
uint32_t last_time = 0; 

// Posições aleatórias do quadrado do jogo
uint8_t pos_x=31, pos_y=0;

// Definições para o I2C e display 
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDRESS 0x3C
ssd1306_t ssd; // Inicializa a estrutura do display no escopo global
bool cor = true; // Booleano que indica a cor branca do pixel


// Função de interrupção da GPIO
void gpio_irq_handler(uint gpio, uint32_t events){
    // Obtendo tempo atual (em us)
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if(current_time - last_time > 200000){
        last_time = current_time; 

        if(gpio == button.a){
            pwm_set_gpio_level(buzzer.a, 300);
        }

        else if(gpio == button.b){
            pwm_set_gpio_level(buzzer.a, 600);
        }

        else if(gpio == joystick.button){
            pwm_set_gpio_level(buzzer.a, 0);
            // Sorteia a posição do quadrado aleatório do game
            pos_x = 31 + ((uint8_t)get_rand_32() % 56); // Aleatório no range [31,86] (86 por conta do limite do frame central)
            pos_y = (uint8_t)get_rand_32() % 56; // Aleatório no range [0,55] (56 por conta do limite do frame central)
            
        }
    }
}


// Interrupção do repeating timer
bool repeating_timer_callback(struct repeating_timer *t){
    printf("\n-> Nova transmissão\n");

    printf("(JOYSTICK) X: %u | Y: %u", joystick.vrx_value, joystick.vry_value);
    printf("\n");
    return true; // Retorna true para repetir a interrupção
}


// Função que inicializa o PWM
void buzzer_init(uint gpio, uint wrap){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice_num, clkdiv);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); 
}

// Função que interpreta a posição do eixo X no display
int choice_display_x(int joy_input){
    if(joy_input < 2048){
        return -(2047-joy_input)/(2047/27);
    }
    else{
        return (joy_input-2048)/(2047/27);
    }
}

// Função que interpreta a posição do eixo Y no display
int choice_display_y(int joy_input){
    if(joy_input < 2048){
        return (2047-joy_input)/(2047/27);
    }
    else{
        return -(joy_input-2048)/(2047/27);
    }
}

int main(){
    stdio_init_all();

    // Configurações do I2C e Display 
    // Inicializando o I2C
    i2c_init(I2C_PORT, 400*1000);
    // Setando as funções dos pinos do I@C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // Garantindo o Pull up do I2C
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Configuração do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDRESS, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializando a máscara da GPIO dos Leds
    gpio_init_mask(OUTPUT_MASK);
    // Definindo como saída
    gpio_set_dir_out_masked(OUTPUT_MASK);

    // Inicializando o PWM dos buzzers zerado
    buzzer_init(buzzer.a, wrap);
    buzzer_init(buzzer.b, wrap);
    pwm_set_gpio_level(buzzer.a, 0);
    pwm_set_gpio_level(buzzer.b, 0);

    // Iniciando os pinos dos botões
    gpio_init_mask(INPUT_MASK);
    // Habilitando os pull ups internos
    gpio_pull_up(button.a);
    gpio_pull_up(button.b);
    gpio_pull_up(joystick.button);
    
    // Configurando a interrupção de GPIO
    gpio_set_irq_enabled_with_callback(button.a, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button.b, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(joystick.button, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    // Configurando o ADC
    adc_init();
    adc_gpio_init(joystick.x_pin); // Canal 1
    adc_gpio_init(joystick.y_pin); // Canal 0

    // Configurando a interrupção do repeating timer para envio via UART
    // Configurando um temporizador de repetição
    struct repeating_timer timer;
    // Configurnado o repeating timer
    uint16_t atraso = 1000;
    add_repeating_timer_ms(atraso, repeating_timer_callback, NULL, &timer);
    

    while (true) {
        // Leitura do Eixo X (Canal 1)
        adc_select_input(1);
        joystick.vrx_value = adc_read();
        // Leitura do Eixo Y (Canal 0)
        adc_select_input(0);
        joystick.vry_value = adc_read();

        // Deadzone para os joysticks
        if (joystick.vry_value>=1900 && joystick.vry_value<=2194){
            joystick.vry_value=2048;
        }
        if (joystick.vrx_value>=1900 && joystick.vrx_value<=2194){
            joystick.vrx_value=2048;
        }


        // DISPLAY I2C
        // Limpa o display
        ssd1306_fill(&ssd, false);
        // Desenhando o frame do jogo
        ssd1306_rect(&ssd, 0, 31, 64, 64, cor, !cor);

        // Desenha o quadrado aleatório do game
        ssd1306_draw_char(&ssd, '*', pos_x, pos_y, false); // Quadrado preenchido

        // Desenha o quadrado do Joystick
        ssd1306_draw_char(&ssd, '*', 59+choice_display_x(joystick.vrx_value), 27+choice_display_y(joystick.vry_value), false); // Quadrado preenchido
 
        //printf("Display x: %d | Display Y: %d\n", choice_display_x(vrx_value), choice_display_y(vry_value));
 
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(1);
    }
}
