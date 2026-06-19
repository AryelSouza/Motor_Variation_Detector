# Projeto Final: Monitor de Inteligência Artificial com STM32

[![Demonstração no YouTube](https://img.shields.io/badge/YouTube-Video_de_Apresentacao-FF0000?style=for-the-badge&logo=youtube&logoColor=white)](https://youtu.be/ysLtPMXzl_8)

## Autores
* Kevin Ryan
* Aryel Souza

## Descrição do Projeto
Este repositório contém o firmware do nosso projeto final desenvolvido para o ecossistema STM32. O sistema atua como um monitor de estabilidade inteligente, capturando dados inerciais de um sensor MPU9250 via I2C, exibindo informações em tempo real em um display TFT ST7789 via SPI, e processando os dados localmente utilizando uma rede neural.

O núcleo do projeto utiliza o Edge Impulse SDK para rodar inferências diretamente no microcontrolador, classificando o comportamento do sistema (como "Parado", "Estável" ou "Instável") com base nos 6 eixos do sensor (Acelerômetro e Giroscópio). 

## Hardware e Conexões
* **Microcontrolador:** STM32 (Clock configurado para 168MHz)
* **Sensor:** MPU9250 (Acelerômetro, Giroscópio e Magnetômetro) - I2C1
* **Display:** TFT ST7789 - SPI1 (Controle de Backlight no pino PA1)
* **Interface de Usuário:** Botões físicos mapeados nas interrupções externas (EXTI0, EXTI3, EXTI4)

## Modos de Operação e Funcionalidades

O sistema é controlado por um menu interativo desenhado no display e operado por botões, dividindo-se em duas rotinas principais:

### 1. Coleta de Dados (Data Logging)
Neste modo, o timer de hardware (TIM3) aciona a leitura do sensor MPU9250. Os dados brutos de aceleração, giroscópio e magnetômetro são empacotados e transmitidos via UART1 (baud rate de 460800) em formato CSV. Esse modo foi utilizado para coletar o dataset que alimentou o treinamento do modelo na plataforma Edge Impulse.

### 2. Inferência (Edge AI)
A rede neural embarcada coleta continuamente janelas de dados dos 6 eixos do sensor. Ao preencher o buffer definido pelo DSP, a função `run_classifier` é executada. O microcontrolador calcula a probabilidade e exibe a classe vencedora diretamente no display TFT.

### 3. Gerenciamento de Energia (Sleep Mode)
Para otimizar o consumo, foi implementado um ciclo de economia de energia. Imediatamente após exibir o resultado da classificação por 3 segundos, o sistema realiza a seguinte rotina:
* Desativa o timer de amostragem.
* Apaga o backlight do display TFT.
* Envia o comando de Sleep para o registrador de energia do MPU9250.
* Coloca a CPU do STM32 em modo de suspensão profunda (Wait For Interrupt - WFI) por 60 segundos.
* Após o tempo determinado, o sistema acorda, reconfigura os periféricos e reinicia o ciclo de monitoramento.

### 4. Tolerância a Falhas de Hardware
O código inclui um scanner I2C adaptado. Se o módulo físico do MPU9250 for desconectado acidentalmente, o sistema identifica a ausência do sinal de "ACK", trava a execução de forma segura, pinta o display de vermelho alertando sobre o erro crítico e aguarda a reconexão física para se reiniciar automaticamente.

## Dependências e Bibliotecas
* STM32 HAL / CMSIS
* `menu.h` / `st7789.h` / `mpu9250.h` (Drivers locais)
* `edge-impulse-sdk` (Gerada via Edge Impulse)

## Configuração do Ambiente
Para compilar este projeto:
1. Clone este repositório.
2. Abra a pasta do projeto utilizando o STM32CubeIDE.
3. Se necessário, re-gere o código de inicialização através do arquivo `.ioc`.
4. Compile o projeto e realize o flash para a placa.
5. Acesse o link do YouTube no início deste documento para ver o sistema em operação.
