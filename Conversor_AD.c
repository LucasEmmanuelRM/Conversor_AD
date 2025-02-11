#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definição dos pinos
#define BUTTON_A 5
#define LED_G 11
#define LED_B 12
#define LED_R 13
#define I2C_SDA 14
#define I2C_SCL 15
#define JOYSTICK_BUTTON 22
#define JOYSTICK_X 26
#define JOYSTICK_Y 27

// I2C
#define I2C_ID i2c1
#define I2C_ADDR 0x3C   // Endereço do dispositivo i2c
#define I2C_FREQ 100000 // 100kHz

// Variáveis para PWM
const uint16_t wrap_period = 4095;  // Valor máximo do contador - WRAP (Pode ir de 1 a 65535)
const float pwm_div = 255.0;        // Divisor do clock para o PWM (Pode ir de 1,0 a 255,9)
uint16_t duty_cycle = 0;            // Nível inicial do pwm
bool bool_pwm = true;               // Estado dos pwm

// Variáveis globais. Não devem ser alteradas manualmente!
static volatile uint32_t last_time = 0; // Armazena o tempo em microssegundos
ssd1306_t ssd;                          // Inicializa a estrutura do display ssd1306

// Protótipos das funções
static void gpio_irq_handler(uint gpio, uint32_t events);
void inicializar_perifericos();

int main(){
    
    stdio_init_all();
    inicializar_perifericos();

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    ssd1306_dashrect(&ssd, 3, 3, 122, 60, gpio_get(LED_G));  // Borda tracejada vazia
    ssd1306_send_data(&ssd);

    uint16_t x, y;                // Para armazenar o valor atual dos eixos do Joystick (0 ~ 4095)
    uint16_t prev_x, prev_y;      // Armazenar o valor anterior de X e Y para atualizar a posição do quadrado corretamente
    uint16_t x_offset, y_offset;  // Para correção de leitura dos eixos do Joystick, nem sempre centralizado = 2048

    adc_select_input(1);
    x_offset = adc_read();
    prev_x = adc_read();

    adc_select_input(0);
    prev_y = adc_read();
    y_offset = adc_read();

    while (true) {
        adc_select_input(1);
        x = adc_read();
        pwm_set_gpio_level(LED_R, abs(x - x_offset));
        printf("x = %d\n", x);

        adc_select_input(0);
        y = adc_read(); 
        pwm_set_gpio_level(LED_B, abs(y - y_offset));
        printf("y = %d\n", y);
        
        // Normalização dos valores de X e Y para ficarem dentro de ambas as bordas (6 < x < 116 && 6 < y < 56)
        x = x/39 + 8;           // 1 + 4096/(116-8) = 39
        y = (4096 - y)/91 + 7;  // 2 + 4096/(54-8) = 91

        // Apaga o quadrado na posição anterior e desenha na posição atual
        ssd1306_rect(&ssd, prev_y, prev_x, 8, 8, 0, 1);
        ssd1306_rect(&ssd, y, x, 8, 8, 1, 1);
        ssd1306_send_data(&ssd);

        // Atualiza os valores anteriores do quadrado
        prev_x = x;
        prev_y = y;

    }
}


void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Verifica se passou tempo suficiente desde o último evento
    if(current_time - last_time > 300000){ // 300 ms de debouncing

        if(gpio == BUTTON_A){
            uint slice = pwm_gpio_to_slice_num(LED_B);
            pwm_set_enabled(slice, bool_pwm = !bool_pwm);
            slice = pwm_gpio_to_slice_num(LED_R);
            pwm_set_enabled(slice, bool_pwm);
        }

        if(gpio == JOYSTICK_BUTTON){
            gpio_put(LED_G, !gpio_get(LED_G));
            ssd1306_dashrect(&ssd, 3, 3, 122, 60, gpio_get(LED_G)); // Borda tracejada vazia
            ssd1306_rect(&ssd, 6, 6, 116, 54, gpio_get(LED_G), 0);  // Borda retangular sem preenchimento
        }

        last_time = current_time; // Atualiza o tempo do último evento
    }
}


void inicializar_perifericos(){

    // Inicializa o LED verde
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, 0);

    // Inicializa os LEDs Azul e Vermelho como PWM
    gpio_set_function(LED_B, GPIO_FUNC_PWM);      // Habilitar o pino GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(LED_B);    // Obter o canal PWM da GPIO
    pwm_set_clkdiv(slice, pwm_div);               // Define o divisor de clock do PWM
    pwm_set_wrap(slice, wrap_period);             // Definir o valor de wrap
    pwm_set_gpio_level(LED_B, duty_cycle);        // Definir o ciclo de trabalho (duty cycle) do pwm
    pwm_set_enabled(slice, true);                 // Habilita o pwm no slice correspondente

    gpio_set_function(LED_R, GPIO_FUNC_PWM);     
    slice = pwm_gpio_to_slice_num(LED_R); 
    pwm_set_clkdiv(slice, pwm_div);  
    pwm_set_wrap(slice, wrap_period); 
    pwm_set_gpio_level(LED_R, duty_cycle); 
    pwm_set_enabled(slice, true);

    // Inicializa o botão A e do Joystick
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);          // Habilita o pull-up interno

    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON); 

    // Inicializa o ADC no Joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);  

    // Inicializa o I2C
    i2c_init(I2C_ID, I2C_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // Inicializa o SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, I2C_ADDR, I2C_ID); // Inicializa o display
    ssd1306_config(&ssd);                                       // Configura o display
    ssd1306_send_data(&ssd);                                    // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

}