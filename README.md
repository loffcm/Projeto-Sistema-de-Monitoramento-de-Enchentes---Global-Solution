# 🌊 Projeto: Sistema de Monitoramento de Enchentes - Global Solution
📌 Descrição
Este projeto tem como objetivo monitorar em tempo real os níveis de água em regiões vulneráveis a enchentes, emitindo alertas progressivos que possibilitam evacuação segura e ação preventiva, protegendo vidas e patrimônios.

A solução foi desenvolvida utilizando um sensor ultrassônico HC-SR04 conectado a um Arduino UNO, que ativa LEDs de status, um buzzer sonoro e mensagens em um display LCD conforme o nível da água detectado.

⚙️ Componentes Utilizados
1x Arduino UNO (ou similar)

1x Sensor Ultrassônico HC-SR04

1x Display LCD 16x2 com módulo I2C

1x LED Verde (nível seguro)

1x LED Amarelo (nível de atenção)

1x LED Vermelho (nível crítico)

1x Buzzer (alarme sonoro)

Jumpers e Protoboard

📐 Circuito
O sensor HC-SR04 é responsável por medir a distância da água até o sensor. O Arduino interpreta esses dados e ativa os sistemas de alerta. O display LCD exibe mensagens de status ou erro, conforme o caso.

Componente	Pino Arduino
HC-SR04 (Trig)	9
HC-SR04 (Echo)	8
LCD (via I2C)	A4 (SDA), A5 (SCL)
LED Verde	2
LED Amarelo	3
LED Vermelho	4
Buzzer	5

🧠 Lógica de Funcionamento
O sistema classifica o risco conforme a distância medida até a água:

🟢 NORMAL (>150 cm)
→ LED verde acende
→ Display mostra “Nível Seguro”

🟡 ATENÇÃO (70 cm a 150 cm)
→ LED amarelo acende
→ Display mostra “Nível de Atenção”
→ Monitoramento intensificado

🔴 CRÍTICO (≤70 cm)
→ LED vermelho acende
→ Buzzer intermitente dispara
→ Display mostra “Nível Crítico - Evacuar!”

Tratamento de Erros:
Se o sensor falhar, o sistema ativa LED vermelho piscando e o display mostra “Erro de leitura”, diferenciando falha técnica de risco real.

💻 Código
O código está disponível no arquivo monitoramento_enchentes.ino.
Comentários explicativos acompanham cada parte para facilitar a manutenção e entendimento.

📹 Demonstração
📽️ Link para vídeo explicativo do projeto no [YouTube ou Google Drive] (inserir link aqui)
📸 Foto do projeto 

🛠️ Como Reproduzir
Monte o circuito conforme o esquema acima.

Conecte o Arduino ao computador.

Faça o upload do código via IDE Arduino.

Abra o Monitor Serial para acompanhamento.

Simule níveis d’água variando a distância de objetos ao sensor.

🔗 Projeto disponível no Tinkercad: (inserir link aqui)

🧪 Testes Realizados
Simulação com distância > 150 cm: LED verde ativo + LCD normal

Simulação entre 70 cm e 150 cm: LED amarelo + alerta no LCD

Simulação ≤ 70 cm: LED vermelho + buzzer + mensagem de evacuação

Desconexão ou falha do sensor: LED vermelho piscando + “Erro de leitura” no display

👨‍💻 Autores

Murilo Gonzalez Bez Chleba | RM: 556199 | GitHub

Caio Marques Lins | RM: 559805 | GitHub

Guilherme Augusto Ferreira Fernandes | RM: 562107  | GitHub
