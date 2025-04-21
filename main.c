#include <stdio.h>
#include "pico/stdlib.h"
#include "libs/structs.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "libs/font.h"
#include "libs/ssd1306.h"
#include "hardware/i2c.h"
#include "pico/rand.h"
#include "libs/led_matrix.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

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

// Definindo a máscara para ativar a input GPIO
#define INPUT_MASK ((1 << button.a) | (1 << button.b) | (1 << joystick.button))

// Variáveis da PIO declaradas no escopo global
PIO pio;
uint sm;
// Constantes para a matriz de leds
#define IS_RGBW false
#define LED_MATRIX_PIN 7

// Vetor com todos LEDs apagados para a matriz 
bool matrix_off[25] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
};

// Vetor com todos LEDs acesos para a matriz 
bool matrix_on[25] = {
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
};

// Vetor apontando para superior direito
bool matrix_upper_right[25] = {
    0, 0, 1, 1, 1,
    0, 0, 0, 1, 1,
    0, 0, 1, 0, 1,
    0, 1, 0, 0, 0,
    1, 0, 0, 0, 0,
};
// Vetor apontando para superior esquerdo
bool matrix_upper_left[25] = {
    1, 1, 1, 0, 0,
    1, 1, 0, 0, 0,
    1, 0, 1, 0, 0,
    0, 0, 0, 1, 0,
    0, 0, 0, 0, 1,
};
// Vetor apontando para bottom direito
bool matrix_bottom_right[25] = {
    1, 0, 0, 0, 0,
    0, 1, 0, 0, 0,
    0, 0, 1, 0, 1,
    0, 0, 0, 1, 1,
    0, 0, 1, 1, 1,
};
// Vetor apontando para superior esquerdo
bool matrix_bottom_left[25] = {
    0, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    1, 0, 1, 0, 0,
    1, 1, 0, 0, 0,
    1, 1, 1, 0, 0,
};

// Variáveis do PWM
uint wrap = 2047;
uint clkdiv = 125;

// Variável para o debounce (armazena tempo)
uint32_t last_time = 0; 

// Posições aleatórias do quadrado do jogo
uint8_t pos_x, pos_y;
// Posições do joystick no display
uint8_t joystick_displayx, joystick_displayy;

// Variáveis de controle do game
bool pause = false;
uint score = 0;
char converted_num; // Variável que armazena o número convertido em char
char converted_string[3]; // String que armazena o número convertido, no formato a ser exibido no display
int callback_count = 0;

// Definições para o I2C e display 
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDRESS 0x3C
ssd1306_t ssd; // Inicializa a estrutura do display no escopo global
bool cor = true; // Booleano que indica a cor branca do pixel



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

// Função de interrupção da GPIO
void gpio_irq_handler(uint gpio, uint32_t events){
    // Obtendo tempo atual (em us)
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if(current_time - last_time > 200000){
        last_time = current_time; 

        if(gpio == button.a){
            score = 0;
            // Sorteia a nova posição do quadrado aleatório do game
            pos_x = 35 + ((uint8_t)get_rand_32() % 51); // Aleatório no range [35,81] (range curto por conta do limite do joystick)
            pos_y = 4 + ((uint8_t)get_rand_32() % 51); // Aleatório no range [4,51] (range curto por conta do limite do joystick)
        }

        else if(gpio == button.b){
            pause = !pause;
        }
    }
}


// Interrupção do repeating timer
bool repeating_timer_callback(struct repeating_timer *t){
    printf("(JOYSTICK) X: %u | Y: %u\n", joystick.vrx_value, joystick.vry_value);
    printf("(SCORE) %d\n", score);
    if(pause){
        printf("(STATUS) PAUSADO\n");
    }
    else{
        printf("(STATUS) JOGANDO\n");
    }
    printf("\n");
    return true; // Retorna true para repetir a interrupção
}

// Função de callback para alertar que pontuou 
// Ativa buzzer, led RGB e matriz
int64_t turn_off_callback(alarm_id_t id, void *user_data){
    // Condição de parada
    if(callback_count == 10){
        // Reseta o contador
        callback_count = 0;
        // Para o buzzer
        
        
        return 0; // Retorno da função
    }

    if(callback_count%2 == 0){
        // Atualiza os alertas que dependem de PWM
        pwm_set_gpio_level(rgb.blue, 300);
        pwm_set_gpio_level(rgb.green, 300);
        pwm_set_gpio_level(rgb.red, 300);
        pwm_set_gpio_level(buzzer.a, 300);
        pwm_set_gpio_level(buzzer.b, 300);
        // Atualiza a matriz de leds endereçáveis (ON)
        update_matrix(matrix_on);
    }
    else{
        pwm_set_gpio_level(rgb.blue, 0);
        pwm_set_gpio_level(rgb.green, 0);
        pwm_set_gpio_level(rgb.red, 0);
        pwm_set_gpio_level(buzzer.a, 0);
        pwm_set_gpio_level(buzzer.b, 0);
        // Atualiza a matriz de leds endereçáveis (OFF)
        update_matrix(matrix_off);
    }

    callback_count++;
    
    add_alarm_in_ms(50, turn_off_callback, NULL, false);
    return 0;
}


// Função que inicializa o PWM
void buzzer_init(uint gpio, uint wrap){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice_num, clkdiv);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); 
}

// Função que converte int para char
void int_2_char(int num, char *out){
    *out = '0' + num;
}

void int_2_string(int num){
    if(num<9){ // Gera string para as menores que 10
        int_2_char(num, &converted_num); // Converte o dígito à direita do número para char
        converted_string[0] = '0'; // Char para melhorar o visual
        converted_string[1] = converted_num; // Int convertido para char
        converted_string[2] = '\0'; // Terminador nulo da String 
    }
    else{ // Gera a string para as maiores/iguais que 10
        int divider = num/10; // Obtém as dezenas
        int_2_char(divider, &converted_num);
        converted_string[0] = converted_num;

        int_2_char(num%10, &converted_num); // Obtém a parte das unidades
        converted_string[1] = converted_num; // Int convertido para char
        converted_string[2] = '\0'; // Terminador nulo da String
    }
}

// Função para gerar o frame principal do game
bool game_frame(){
    // Desenhando o frame do jogo
    ssd1306_rect(&ssd, 0, 31, 64, 64, cor, !cor);
    // Desenha o quadrado aleatório do game
    ssd1306_draw_char(&ssd, '*', pos_x, pos_y, false); // Quadrado preenchido
    // Desenha o quadrado do Joystick
    ssd1306_draw_char(&ssd, '*', joystick_displayx, joystick_displayy, false); // Quadrado preenchido
}

bool left_frame(){
    ssd1306_draw_string(&ssd, "SCR", 4, 4, false);
    int_2_string(score); // Converte a pontuação int em string
    ssd1306_draw_string(&ssd, converted_string, 4, 14, false);

    ssd1306_draw_string(&ssd, "RST", 4, 43, false);
    ssd1306_draw_string(&ssd, "<-", 8, 53, false);
}

bool right_frame(){
    ssd1306_draw_string(&ssd, "STS", 99, 4, false);
    if(pause){
        ssd1306_draw_char(&ssd, ',', 107, 14, false);
    }
    else{ssd1306_draw_char(&ssd, '+', 107, 14, false);

    }

    ssd1306_draw_string(&ssd, "PSE", 99, 43, false);
    ssd1306_draw_string(&ssd, "->", 103, 53, false);
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

    // Inicializando o PWM dos buzzers zerado
    buzzer_init(buzzer.a, wrap);
    buzzer_init(buzzer.b, wrap);
    pwm_set_gpio_level(buzzer.a, 0);
    pwm_set_gpio_level(buzzer.b, 0);

    // Inicializando o PWM dos LEDs RGB zerado
    buzzer_init(rgb.blue, wrap);
    buzzer_init(rgb.green, wrap);
    buzzer_init(rgb.red, wrap);
    pwm_set_gpio_level(rgb.blue, 0);
    pwm_set_gpio_level(rgb.green, 0);
    pwm_set_gpio_level(rgb.red, 0);

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

    // Inicializando a PIO
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);

    // Sorteia a posição inicial do quadrado aleatório do game
    pos_x = 35 + ((uint8_t)get_rand_32() % 51); // Aleatório no range [35,81] (range curto por conta do limite do joystick)
    pos_y = 4 + ((uint8_t)get_rand_32() % 51); // Aleatório no range [4,51] (range curto por conta do limite do joystick)
    

    while (true) {
        // Condição para só atualizar o game quando não estiver pausado
        if(!pause){
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

            // Calcula a posição do Joystick no display
            joystick_displayx = 59+choice_display_x(joystick.vrx_value);
            joystick_displayy = 27+choice_display_y(joystick.vry_value);

            // Verifica se acertou o quadrado aleatório do game
            if(joystick_displayx == pos_x && joystick_displayy == pos_y){
                // Aumenta o score
                score++;

                // Ativa os alertas sonoros/visuais da pontuação
                add_alarm_in_ms(1, turn_off_callback, NULL, false);

                // Sorteia a nova posição do quadrado aleatório do game
                pos_x = 35 + ((uint8_t)get_rand_32() % 51); // Aleatório no range [35,81] (range curto por conta do limite do joystick)
                pos_y = 4 + ((uint8_t)get_rand_32() % 51); // Aleatório no range [4,51] (range curto por conta do limite do joystick)
            }
        }

        // MATRIZ DE LEDS (INDICATIVO DIREÇÃO)
        // Lado esquerdo
        if(pos_x<=63){
            // Lado superior
            if(pos_y<=31){
                update_matrix(matrix_upper_left);
            }
            // Lado inferior
            else{
                update_matrix(matrix_bottom_left);
            }
        }
        // Canto direito
        else{
            // Lado superior
            if(pos_y<=31){
                update_matrix(matrix_upper_right);
            }
            // Lado inferior
            else{
                update_matrix(matrix_bottom_right);
            }
        }


        // DISPLAY I2C
        // Limpa o display
        ssd1306_fill(&ssd, false);
        
        // Gera o frame atualizado do game
        game_frame();
        // Parte esquerda do display
        left_frame();
        // Parte direita do display
        right_frame();

        ssd1306_send_data(&ssd); // Atualiza o display
        sleep_ms(1);
    }
}
