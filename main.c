#include <stdio.h>
#include "pico/stdlib.h"
#include "include/structs.h"
#include "hardware/pwm.h"

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
    .joystick = 22
};

// Definindo os pinos dos buzzers
BUZZERS buzzer = {
    .a = 21,
    .b = 10,
};

// Definindo a máscara para ativar a output GPIO 
#define OUTPUT_MASK ((1 << rgb.blue) | (1 << rgb.green) | (1 << rgb.red))
// Definindo a máscara para ativar a input GPIO
#define INPUT_MASK ((1 << button.a) | (1 << button.b))

// Variáveis do PWM
uint wrap = 2047;
uint clkdiv = 125;

uint32_t last_time = 0;

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

        else if(gpio == button.joystick){
            pwm_set_gpio_level(buzzer.a, 0);
        }
    }
}


// Função que inicializa o PWM
void buzzer_init(uint gpio, uint wrap){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice_num, clkdiv);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); 
}

int main(){
    stdio_init_all();

    // Inicializando a máscara da GPIO dos Leds
    gpio_init_mask(OUTPUT_MASK);
    // Definindo como saída
    gpio_set_dir_out_masked(OUTPUT_MASK);

    // Inicializando o PWM dos buzzers
    buzzer_init(buzzer.a, wrap);
    buzzer_init(buzzer.b, wrap);

    pwm_set_gpio_level(buzzer.a, 0);
    pwm_set_gpio_level(buzzer.b, 0);

    // Iniciando os pinos dos botões
    gpio_init_mask(INPUT_MASK);
    // Habilitando os pull ups internos
    gpio_pull_up(button.a);
    gpio_pull_up(button.b);
    gpio_pull_up(button.joystick);
    
    // Configurando a interrupção de GPIO
    gpio_set_irq_enabled_with_callback(button.a, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button.b, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button.joystick, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    

    while (true) {
        /*
        gpio_put(rgb.blue, 1);
        gpio_put(rgb.red, 1);
        gpio_put(rgb.green, 1);
        */
    }
}
