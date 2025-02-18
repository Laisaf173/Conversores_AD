#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h" //biblioteca para controlar o hardware de PWM
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14 
#define I2C_SCL 15 
#define endereco 0x3C // Endereço do display
#define JOYSTICK_X_PIN 26  // GPIO para eixo X
#define JOYSTICK_Y_PIN 27  // GPIO para eixo Y
#define JOYSTICK_PB 22 // GPIO para botão do Joystick

int16_t adc_value_x;  // Valor do ADC do eixo X
uint16_t adc_value_y;  // Valor do ADC do eixo Y

// Define os valores centrais e as zonas mortas para cada eixo
const uint16_t centroX = 1919;           // Centro do Joystick para o eixo X
const uint16_t centroY = 2190;           // Centro do Joystick para o eixo Y
const uint16_t zonaDesligamentoX = 250;    // Zona morta para o eixo X
const uint16_t zonaDesligamentoY = 320;    // Zona morta para o eixo Y

static volatile bool borda_extra_eLED = false;
bool cor = true; // Variável para alternar a cor do pixel
bool Led_status = true; // Variável para alternar o estado dos leds PWM

static volatile uint last_time = 0;
ssd1306_t ssd; // Inicializa a estrutura do display

const uint button_A = 5; // Botão A
const uint LED_VERDE = 11; // Pino do LED Verde
const uint LED_AZUL = 12; // Pino do LED Azul
const uint LED_VERMELHO = 13; // Pino do LED Vermelho

//Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"


void gpio_irq_handler(uint gpio, uint32_t events); // Protótipo da função de tratamento de interrupção
int main(){
    stdio_init_all();

    // Config do PWM
    gpio_set_function(LED_AZUL, GPIO_FUNC_PWM); //habilitar o pino GPIO como PWM
    gpio_set_function(LED_VERMELHO, GPIO_FUNC_PWM); //habilitar o pino GPIO como PWM

    // Configuração do LED Azul
    uint sliceBlue = pwm_gpio_to_slice_num(LED_AZUL); //obter o canal PWM da GPIO
    uint sliceRed = pwm_gpio_to_slice_num(LED_VERMELHO); //obter o canal PWM da GPIO


    pwm_set_clkdiv(sliceBlue, 125.0f); //define o divisor de clock do PWM
    pwm_set_wrap(sliceBlue, 255); //definir o valor de wrap
    pwm_set_enabled(sliceBlue, true); //habilita o pwm no slice correspondente


    pwm_set_clkdiv(sliceRed, 125.0f); //define o divisor de clock do PWM
    pwm_set_wrap(sliceRed, 255); //definir o valor de wrap
    pwm_set_enabled(sliceRed, true); //habilita o pwm no slice correspondente

    //Configuração do LED Verde
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);

    //Configuração dos botões
    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);

    // Configuração do botão do Joystick
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB); 

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); 

    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL);
    
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa o ADC e configura os pinos do Joystick
    adc_init();// Inicializa o ADC
    adc_gpio_init(JOYSTICK_X_PIN);// Inicializa o pino do eixo X
    adc_gpio_init(JOYSTICK_Y_PIN);  // Inicializa o pino do eixo Y

    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita a interrupção do botão do Joystick

    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita a interrupção do botão A

    while (true){
        // Leitura dos ADCs:
        // Observação: conforme o código original, o canal 0 é utilizado para o eixo Y
        // e o canal 1 para o eixo X.
        adc_select_input(0); // Seleciona o ADC para o eixo Y (pino 27)
        adc_value_y = adc_read();
        adc_select_input(1); // Seleciona o ADC para o eixo X (pino 26)
        adc_value_x = adc_read();    

        // Calcula os offsets (valores negativos são possíveis)
        int16_t offset_x = adc_value_x - centroX;
        int16_t offset_y = adc_value_y - centroY;
        
        printf("offset x: %d\n", offset_x);
        printf("offset y: %d\n", offset_y);

        uint16_t brilho_red = 0;
        uint16_t brilho_blue = 0;
        
        // Cálculo do brilho para o LED Vermelho (eixo X)
        if (abs(offset_x) >= zonaDesligamentoX) {
            if (offset_x > 0) {
                // Deslocamento máximo para a direita: de centro até 4095
                uint16_t max_offset = 4095 - centroX;
                brilho_red = ((offset_x - zonaDesligamentoX) * 255) / (max_offset - zonaDesligamentoX);
            } else {
                // Deslocamento máximo para a esquerda: de centro até 0
                uint16_t max_offset = centroX;
                brilho_red = ((abs(offset_x) - zonaDesligamentoX) * 255) / (max_offset - zonaDesligamentoX);
            }
            if (brilho_red > 255)
                brilho_red = 255;
        } else {
            brilho_red = 0;
        }
        
        // Cálculo do brilho para o LED Azul (eixo Y)
        if (abs(offset_y) >= zonaDesligamentoY) {
            if (offset_y > 0) {
                // Deslocamento máximo para baixo: de centro até 4095
                uint16_t max_offset = 4095 - centroY;
                brilho_blue = ((offset_y - zonaDesligamentoY) * 255) / (max_offset - zonaDesligamentoY);
            } else {
                // Deslocamento máximo para cima: de centro até 0
                uint16_t max_offset = centroY;
                brilho_blue = ((abs(offset_y) - zonaDesligamentoY) * 255) / (max_offset - zonaDesligamentoY);
            }
            if (brilho_blue > 255)
                brilho_blue = 255;
        } else {
            brilho_blue = 0;
        }

        // Se a diferença ultrapassar a zona de desligamento, mapeia para 0 a 255
        

        // Atualiza o PWM dos LEDs
        pwm_set_gpio_level(LED_VERMELHO, brilho_red);
        pwm_set_gpio_level(LED_AZUL, brilho_blue);
        
        // Atualiza o conteúdo do display com animações
        ssd1306_fill(&ssd, !cor); // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo

        if(borda_extra_eLED){
            ssd1306_rect(&ssd, 4, 4, 120, 58, cor, !cor); // Desenha um retângulo menor duplicando a borda
        }
        // Calcula a posição do quadrado no display
        uint8_t pos_X = (adc_value_x *(128-8))/ 4095;
        uint8_t pos_Y = ((4095 - adc_value_y) *(64-8))/ 4095;

        // Desenha o quadrado no display
        ssd1306_rect(&ssd, pos_Y, pos_X, 8, 8, cor, cor);
        ssd1306_send_data(&ssd); // Atualiza o display
       
        sleep_ms(100);
   
    }
}

// Função de tratamento de interrupção
void gpio_irq_handler(uint gpio, uint32_t events){

    // Verifica se o tempo desde a última interrupção é maior que 200ms
    uint32_t current_time = to_us_since_boot(get_absolute_time()); 
    
    if (current_time - last_time > 200000) {
        last_time = current_time; 
        if(gpio == JOYSTICK_PB) {
            // Alternar estado do LED Verde
            gpio_put(LED_VERDE, (borda_extra_eLED = !borda_extra_eLED));
        }
        if(gpio == button_A)
        {
           // Alternar estado dos LEDs PWM 
            pwm_set_enabled(pwm_gpio_to_slice_num(LED_VERMELHO), (Led_status = !Led_status));
            pwm_set_enabled(pwm_gpio_to_slice_num(LED_AZUL), Led_status);
            
        }
    }
}