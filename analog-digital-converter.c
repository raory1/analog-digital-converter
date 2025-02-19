#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "include/ssd1306.h"
#include "hardware/pwm.h"

// Definições dos pinos
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12
#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define JOYSTICK_SW 22
#define BUTTON_A 5
#define BUTTON_B 6
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define MIN_X 4
#define MAX_X 52
#define MIN_Y 4
#define MAX_Y 116
#define STEP 10
#define INITIAL_X 28
#define INITIAL_Y 60
#define PWM_WRAP 20000

// Variáveis globais
PIO pio;
uint sm;
ssd1306_t ssd; // Inicializa a estrutura do display
bool cor = true;
bool borda = true;
volatile absolute_time_t last_press_time = 0;

int x = INITIAL_X;
int y = INITIAL_Y;
int width = 8;
int high = 8;
bool sw_value = 0;

bool pwm_habilitado = true;
ssd1306_t oled_display;
uint slice_led_blue;
uint slice_led_red;

int clamp(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

void atualizar_display()
{
    if (borda)
    {
        ssd1306_fill(&ssd, !cor);
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
    }
    else
    {
        ssd1306_fill(&ssd, cor);
        ssd1306_rect(&ssd, 3, 3, 122, 58, !cor, cor);
    }
    ssd1306_rect(&ssd, x, y, width, high, cor, cor);
    ssd1306_send_data(&ssd);
}

// Funçao callback dos botões
void gpio_irq_handler(uint gpio, uint32_t events)
{
    printf("Configurado\n");

    bool btn_last_state = false;
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    bool btn_pressed = !gpio_get(gpio);

    if (btn_pressed && !btn_last_state &&
        (absolute_time_diff_us(last_press_time, get_absolute_time()) > 200000))
    {
        last_press_time = get_absolute_time();
        btn_last_state = true;
        if (gpio == BUTTON_A)
        {
            printf("Botão A pressionado\n");
            pwm_habilitado = !pwm_habilitado;
            pwm_set_gpio_level(LED_BLUE, 0);
            pwm_set_gpio_level(LED_RED, 0);
            gpio_put(LED_GREEN, 0);
        }
        else if (gpio == JOYSTICK_SW)
        {
            printf("Botão SW pressionado\n");
            gpio_put(LED_GREEN, (!gpio_get(LED_GREEN)));
            borda = !borda;
        }

        // Se o botão A for pressionado, decrementa o padrão (limite mínimo 0)
    }
    else if (!btn_pressed)
    {
        btn_last_state = false;
    }
}

// Inicializa hardware
void setup_all()
{
    stdio_init_all();
    // Joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    gpio_init(JOYSTICK_SW);
    gpio_set_dir(JOYSTICK_SW, GPIO_IN);
    gpio_pull_up(JOYSTICK_SW);
    // Botão
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    // Display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    ssd1306_fill(&ssd, false);                                    // Limpa o display
    ssd1306_send_data(&ssd);
    // LEDs PWM
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);

    slice_led_blue = pwm_gpio_to_slice_num(LED_BLUE);
    slice_led_red = pwm_gpio_to_slice_num(LED_RED);

    pwm_set_clkdiv(slice_led_blue, 125.0);
    pwm_set_clkdiv(slice_led_red, 125.0);

    pwm_set_wrap(slice_led_blue, PWM_WRAP);
    pwm_set_wrap(slice_led_red, PWM_WRAP);

    pwm_set_enabled(slice_led_blue, true);
    pwm_set_enabled(slice_led_red, true);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, false);
}

int main()
{
    setup_all();
    atualizar_display();

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(JOYSTICK_SW, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Loop principal
    while (true)
    {
        adc_select_input(0);
        uint16_t vrx_value = adc_read(); // Eixo X
        adc_select_input(1);
        uint16_t vry_value = adc_read(); // Eixo Y
        sw_value = gpio_get(JOYSTICK_SW);

        x += (vrx_value < 1900) ? STEP : (vrx_value > 2100 ? -STEP : 0);
        y += (vry_value < 1900) ? -STEP : (vry_value > 2100 ? STEP : 0);

        x = clamp(x, MIN_X, MAX_X);
        y = clamp(y, MIN_Y, MAX_Y);

        atualizar_display();

        if((vrx_value > 1900 && vrx_value < 2100 && vry_value > 1900 && vry_value < 2100)){
            vrx_value = 2048;
            vry_value = 2048;
            x = INITIAL_X;
            y = INITIAL_Y;
        }

        // Atualiza os LEDs com base nos valores de VRX e VRY
        if(pwm_habilitado){
            pwm_set_gpio_level(LED_BLUE,  abs(vrx_value - 2048));
            pwm_set_gpio_level(LED_RED, abs(vry_value - 2048));
        }

        sleep_ms(100);
    }

    return 0;
}