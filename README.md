
# Conversor Analógico/Digital - Controle Joystick no Raspberry Pi Pico

## Descrição

Este projeto implementa um sistema de controle utilizando um joystick para manipular a exibição em um display OLED SSD1306, com LEDs RGB controlados por PWM. A interface permite interação através de botões e exibe informações visuais no display.

## Componentes Utilizados

-   **Microcontrolador:** Raspberry Pi Pico
-   **Display OLED:** SSD1306 (via I2C)
-   **Joystick Analógico** (Eixos X/Y e Botão SW)
-   **LEDs RGB** (Vermelho, Verde e Azul)
-   **Botão de Entrada** (A)

## Funcionalidades

-   Leitura do joystick e movimentação de um elemento no display OLED.
-   Alteranção de borda e fundo do display através do botão SW do joystick.
-   Liga/desliga os LEDs RGB baseando-se nos valores do joystick.
-   Controle de eventos com interrupção GPIO para os botões.
-   Comunicação via UART para depuração.

## Configuração do Ambiente

### Dependências

Para compilar e rodar este projeto, certifique-se de que o ambiente de desenvolvimento do Raspberry Pi Pico esteja configurado.

1.  **Instale o SDK do Raspberry Pi Pico**
2.  **Configure o ambiente de desenvolvimento**
3.  **Compile e carregue o firmware para o Pico**
## Execução

-   Conecte o Raspberry Pi Pico ao computador via USB.
-   Abra um terminal serial (ex: `PuTTY` ou `Serial Monitor (VSCode)`).
-   Execute o código e observe a resposta dos LEDs e do display ao movimento do joystick.

## Video
https://youtu.be/JqB0Bt8s_HI
