#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "matriz_led.h"

//Define os pinos do LED vermelho (GPIO 13) e os botões A (GPIO 5) e B (GPIO 6)
#define LED_RED 13 
#define BUTTON_A 5
#define BUTTON_B 6

volatile int numero_atual = 0; //Variável que armazena o número exibido na matriz de LEDs.
PIO pio = pio0;
uint sm;
bool led_state = false; //Variável usada para alternar o estado do LED verde.

// Debounce via software por botão
bool debounce(uint gpio) { //impede que o botão seja lido várias vezes devido ao efeito de "bounce" (falsas leituras)
    static uint32_t last_time_a = 0;
    static uint32_t last_time_b = 0;

    uint32_t *last_time = (gpio == BUTTON_A) ? &last_time_a : &last_time_b;
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (now - *last_time < 200) return false;
    *last_time = now;
    return gpio_get(gpio) == 0;
}

// Callback para interrupções dos botões (para ocorrer a interrupção dos botoes quando pressionado.)
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A && debounce(BUTTON_A)) {
        numero_atual = (numero_atual + 1) % 10;
        exibir_numero(numero_atual, pio, sm, 1.0, 0.0, 0.0); // Verde
    } 
    else if (gpio == BUTTON_B && debounce(BUTTON_B)) {
        numero_atual = (numero_atual - 1 + 10) % 10;
        exibir_numero(numero_atual, pio, sm, 0.0, 0.0, 1.0); // Azul
    }
}

// Configuração dos botões
void setup_buttons() {
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

// Configuração do WS2812 via PIO
void setup_ws2812() {
    set_sys_clock_khz(128000, false);
    uint offset = pio_add_program(pio, &pio_matrix_program);
    sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);
}

int main() { //inicia os conceitos anteriores, deixando em lupe infinito.
    stdio_init_all();
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    setup_buttons();
    setup_ws2812();

    while (true) {
        sleep_ms(200);  // 200 ms para piscar 5 vezes por segundo
        led_state = !led_state;
        gpio_put(LED_RED, led_state);
    }
}
