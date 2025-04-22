# 👾 PixelTracker - BitDogLab 👾

Este projeto é a atividade de revisão da Fase 2 do EmbarcaTech. Consiste em um jogo onde o objetivo é perseguir um frame aleatório que aparece no Display I2C da BitDogLab, utilizando o analógico como ferramenta para perseguir esse frame.

---

## 📌 **Funcionalidades Implementadas**

✅ Surgimento aleatório do pixel que o jogador deve perseguir\
✅ Rastreamento do analógico para controlar o pixel do jogador\
✅ Visualização da pontuação atual do usuário no display\
✅ Botões de reset/pause à disposição do jogador\
✅ Alertas visuais/sonoros toda vez que o jogador pontua

---

## 🛠 **Hardware Utilizado**

- **Placa BitDogLab**
- **Analógicos** para captura do movimento do jogador
- **Botões** para pausar/resetar o jogo, tendo tratamento de debounce no pressionamento
- **Matriz de LEDs endereçáveis** guia que indica a direção do pixel no display, e alerta visual quando ocorre pontuação
- **LED RGB** alerta visual quando ocorre pontuação
- **Buzzer** alerta sonoro quando ocorre pontuação
- **Display I2C** visualização do jogo e informações úteis para o jogador

---

## 📂 **Estrutura do Código**

```
📂 PixelTracker/
├── 📄 main.c                          # Código principal do projeto
├──── 📂libs
├───── 📄 font.h                       # Fonte utilizada no Display I2C
├───── 📄 led_matrix.c                 # Funções para manipulação da matriz de LEDs endereçáveis
├───── 📄 led_matrix.h                 # Cabeçalho para o led_matrix.c
├───── 📄 ssd1306.c                    # Funções que controlam o Display I2C
├───── 📄 ssd1306.h                    # Cabeçalho para o ssd1306.c
├───── 📄 structs.h                    # Structs utilizadas no código principal
├──── 📂generated
├───── 📄 ws2812.pio.h                 # Arquivo gerado para utilização da máquina de estados
├── 📄 ws2812.pio                      # Máquina de estados para controle da matriz de LEDs
├── 📄 CMakeLists.txt                  # Configurações para compilar o código corretamente
└── 📄 README.md                       # Documentação do projeto
```

---

## 📽️ **Vídeo no YouTube**
[Link](https://drive.google.com/file/d/1CBcRcgt1h5qgU2nHKVm_cDSkCQPpzC1Z/view?usp=sharing)
