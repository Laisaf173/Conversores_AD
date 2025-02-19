# Controle de LEDs e Display com Joystick Analógico
Este projeto utiliza Raspberry Pi Pico para controlar LEDs RGB e um display OLED através de um joystick analógico.

## Funcionalidades
--> Controle de LEDs via Joystick:

- LED Vermelho: controlado pelo eixo X do joystick
- LED Azul: controlado pelo eixo Y do joystick
- LED Verde: alternado pelo botão do joystick

--> Display OLED SSD1306:
- Mostra um retângulo de borda que pode ser duplicado
- Exibe um quadrado que se move de acordo com a posição do joystick

--> Controles:
- Botão do Joystick: alterna o LED verde e a borda dupla do retângulo
- Botão A: liga/desliga os LEDs PWM (vermelho e azul)

## Modo de uso:
O joystick controla a intensidade dos LEDs:
- Movimentação horizontal afeta o LED vermelho
- Movimentação vertical afeta o LED azul
- Existe uma zona morta central onde os LEDs permanecem apagados (acontece ao soltar o joystick)

Os botões têm funções específicas:
- Pressionar o botão do joystick alterna o LED verde e a borda dupla
- Pressionar o botão A liga/desliga os LEDs PWM

O display OLED mostra:
- Um retângulo de borda
- Um quadrado que se move conforme a posição do joystick
- Borda dupla opcional (ativada pelo botão do joystick)

Vídeo mostrando o funcionamento do projeto: https://drive.google.com/file/d/1WoNXvtLAKVECvAQ86igmws_YM57H_YGf/view?usp=sharing
