Conversor_AD é um código implementado para cumprir com a atividade proposta no capítulo 8 - Unidade 4 do Embarcatech - CEPEDI. Trata-se de um experimento para consolidar o aprendizado em ADC (Conversores Analógico-Digital).

O código foi desenvolvido com a intenção de ser executado na ferramenta educacional BitDogLab, que possui como microcontrolador um Raspberry Pi Pico W e todos os periféricos necessários para o experimento (Joystick, pushbutton, display SSD1306 e LED RGB).

Ele utiliza as bibliotecas do pico SDK e uma desenvolvida especificamente para o display citado acima, implementando o protocolo de comunicação i2c, interrupção e debouncing via software para o pressionamento dos botões, além de utilizar PWM em dois pinos.


**Os requisitos implementados são**
- Ajuste de intensidade do LED azul através da movimentação do joystick pelo eixo Y;
- Ajuste de intensidade do LED vermelho através da movimentação do joystick pelo eixo X;
- Quando o joystick estiver centralizado, ambos os LEDs devem estar apagados;
- Exibição de um quadrado 8x8 pixels no display, que se moverá conforme o movimento do joystick;
- Alternância do estado do LED verde e das bordas do display quando o botão do joystick for pressionado;
- Ativação ou desativação dos LEDs pwm a cada acionamento do botão A.


**LEDs PWM e sua interação com o Joystick**

Para os LEDs cuja intensidade variam, utilizou-se pwm neles, sendo que seus dutycycle são definidos pelo valor que é lido a partir do potenciômetros do eixo X e Y do joystick (0 a 4095).
Como há variações na fabricação de cada joystick, a centralização de todos nem sempre retorna 2048. Logo, na inicialização do programa, um offset de cada eixo é capturado a partir da posição inicial do joystick.
Isso é feito para que o dutycycle subtraia o valor atualizado de cada movimentação por esses offsets, aproximando-o de 0 quando ele estiver parado (oscilações impedem valores absolutos e constantes).

É **vital** que nenhuma interação seja feita com o joystick durante a inicialização, pois isso irá fazer toda leitura subsequente retornar um dutycycle errôneo.


**Rotina de Interrupção**

O pressionamento dos botões dispara uma rotina de interrupção, onde o botão A apenas ativa ou desativa os pinos pwm. **Isso não corrige o uso indevido do joystick durante a inicialização**.
Enquanto isso, o botão do joystick prepara uma mudança nas bordas exibidas no display, porém para evitar conflito no protocolo i2c, apenas a função main atualiza o display.
A borda interior é um retângulo que já estava presente na biblioteca ssd1306. A borda exterior é um retângulo tracejado, função implementada unicamente para este código e presente na mesma biblioteca.


**Movimentação do Quadrado 8x8**

O quadrado movimentado pelo display pelo joystick é a consequência de uma constante atualização do próprio display, onde é printada uma nova posição do objeto na mesma medida em que a anterior é apagada.
Dadas as dimensões de 128x64 pixels do display, o quadrado é limitado exclusivamente entre 8 ~ 116 no eixo X e 7 ~ 54 no eixo Y, o limite máximo para não sobescrever a borda interior.
No código, há uma normalização que garante que o X e Y do quadrado nunca saiam desse intervalo, sendo que a do eixo Y é invertida (faz-se 4096 - y) para que a movimentação do quadrado seja feita corretamente.

É necessário inverter a variável y pois o display enumera seus pixels de cima para baixo (0 a 63), enquanto o joystick da BitDogLab varia o valor do eixo Y de baixo para cima (0 a 4096).
Sem essa inverção, ao movimentar o joystick para cima, o quadrado iria para baixo e vice-versa.

A posição anterior do quadrado é sempre gravada para que, quando ele se movimentar, ela seja utilizada paga apagar a instância anterior dele, impedindo que o display seja "pintado" de branco por ele.


**Vídeo Demonstrativo**

Para visualizar uma breve explicação e um experimento na BitDogLab, basta acessar o link a seguir:
[https://drive.google.com/file/d/1hMwHucPFrBXfuPv1z7PKfjRq-aheIbCn/view?usp=sharing]
