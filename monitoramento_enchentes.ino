/*
  Sistema de Monitoramento de Distância com Sensor Ultrassônico HC-SR04
  e Indicadores (LEDs + Buzzer) + Display LCD I2C (16x2)

  Funcionalidades:
  - Mede distância em cm usando sensor ultrassônico HC-SR04.
  - Indica três estados de distância:
      • NORMAL    (distância > 150 cm): LED verde aceso.
      • ATENÇÃO   (70 cm < distância ≤ 150 cm): LED amarelo aceso.
      • CRÍTICO   (distância ≤ 70 cm): LED vermelho e buzzer intermitente.
  - Exibe distância e estado no display LCD I2C.
  - Em caso de erro/timeout na leitura, pisca LED vermelho e exibe mensagem de erro.

  Boas práticas implementadas:
  - Uso de nomes de constantes e variáveis claros e autoexplicativos.
  - Separação lógica em seções: definições, variáveis globais, setup, loop e funções auxiliares.
  - Comentários detalhados em cada bloco de código explicando a lógica.
  - Indentação consistente e formatação legível.
  - Tratamento de erros/timeout no sensor.

*/

// ============================
// ========== INCLUDES ========
// ============================
#include <Wire.h>               // Comunicação I2C
#include <LiquidCrystal_I2C.h>  // Biblioteca para LCD I2C

// ============================
// ======= DEFINIÇÕES =========
// ============================

// Pinos do Sensor Ultrassônico HC-SR04
const uint8_t PIN_TRIGGER = 9;  // Pino TRIG do HC-SR04
const uint8_t PIN_ECHO    = 8;  // Pino ECHO do HC-SR04

// Pinos dos LEDs Indicadores de Estado
const uint8_t PIN_LED_VERDE    = 4;  // Estado NORMAL (distância > 150 cm)
const uint8_t PIN_LED_AMARELO  = 3;  // Estado ATENÇÃO (70 cm < distância ≤ 150 cm)
const uint8_t PIN_LED_VERMELHO = 2;  // Estado CRÍTICO (distância ≤ 70 cm) ou ERRO

// Pino do Buzzer (somente dispara em estado CRÍTICO)
const uint8_t PIN_BUZZER = 6;

// Intervalo entre "beeps" do buzzer em estado CRÍTICO (em milissegundos)
const unsigned long INTERVALO_BUZZER_MS = 800;

// Tempo máximo de espera pelo pulso de eco (timeout) em microssegundos
// Corresponde a distância de até ~500 cm (pulseIn retorna 0 se passar do timeout)
const unsigned long TIMEOUT_US = 30000;

// Limites de distância (em centímetros) para cada estado
const uint16_t LIMITE_CRITICO_CM  = 70;
const uint16_t LIMITE_ATENCAO_CM  = 150;
const uint16_t LIMITE_NORMAL_CM   = 300;  // Opcional, apenas para referência

// Endereço I2C padrão do display LCD 16x2 (ajuste se necessário)
const uint8_t LCD_I2C_ENDERECO = 0x27;

// ============================
// ===== VARIÁVEIS GLOBAIS ====
// ============================

// Objeto para controlar o display LCD via I2C (16 colunas x 2 linhas)
LiquidCrystal_I2C lcd(LCD_I2C_ENDERECO, 16, 2);

// Controle de intermitência do buzzer
unsigned long ultimoTimestampBuzzer = 0;
bool     buzzerLigado              = false;

// ============================
// ==== PROTÓTIPOS DE FUNÇÕES ==
// ============================
void inicializarHardware();
void atualizarDisplay(const String& linha1, const String& linha2);
int  medirDistanciaCm();
void acionarIntermitenciaBuzzer();
void piscarLed(uint8_t pinLed, unsigned long duracaoMs);

// ============================
// ========== SETUP ===========
// ============================
void setup() {
  // Inicializa comunicação serial para debug (opcional)
  Serial.begin(9600);

  // Configura pinos de saída e entrada
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);

  pinMode(PIN_BUZZER, OUTPUT);

  // Garante que LEDs e buzzer comecem desligados
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AMARELO, LOW);
  digitalWrite(PIN_LED_VERMELHO, LOW);
  noTone(PIN_BUZZER);

  // Inicializa o display LCD (I2C)
  lcd.init();        // Inicializa o hardware do LCD
  lcd.backlight();   // Liga o backlight
  lcd.clear();       // Limpa tela

  // Exibe mensagem inicial indicando que o LCD foi iniciado com sucesso
  lcd.setCursor(0, 0);   // Coluna 0, Linha 0
  lcd.print("LCD Ligado");
  lcd.setCursor(0, 1);   // Segunda linha
  lcd.print("Sucesso!");
  delay(1500);  // Pausa para que usuário veja a mensagem

  lcd.clear();  // Limpa tela após mensagem inicial

  // Mensagem no Serial Monitor indicando que o sistema está pronto
  Serial.println("=== SISTEMA INICIADO ===");
}

// ============================
// =========== LOOP ==========
// ============================
void loop() {
  // 1. Medir distância (cm) ou obter código de erro (-1)
  int distanciaCm = medirDistanciaCm();

  // 2. Se leitura inválida (timeout ou distância fora de faixa), indicar ERRO
  if (distanciaCm == -1) {
    Serial.println("ERRO: Timeout ou leitura inválida");
    piscarLed(PIN_LED_VERMELHO, 250);  // Pisca LED vermelho para indicar erro

    // Atualiza LCD com mensagem de erro
    atualizarDisplay("Erro de leitura", "Verificar sensor");
    delay(500);  // Aguarda 500 ms antes de nova leitura
    return;      // Sai do loop para repetir a medição
  }

  // 3. Exibe valor lido no Serial Monitor para monitoramento
  Serial.print("Distancia: ");
  Serial.print(distanciaCm);
  Serial.println(" cm");

  // 4. Reseta indicadores (desliga LEDs e buzzer) antes de definir novo estado
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AMARELO, LOW);
  digitalWrite(PIN_LED_VERMELHO, LOW);
  noTone(PIN_BUZZER);
  buzzerLigado = false;

  // 5. Define estado com base na distância e aciona LEDs, buzzer e LCD
  if (distanciaCm <= LIMITE_CRITICO_CM) {
    // ============================
    // ======== ESTADO CRÍTICO ====
    // ============================
    Serial.print(">>> ESTADO: CRÍTICO – ");
    Serial.print(distanciaCm);
    Serial.println(" cm <<<");

    digitalWrite(PIN_LED_VERMELHO, HIGH);  // Acende LED vermelho
    acionarIntermitenciaBuzzer();          // Intermitir buzzer em 1 kHz

    // Atualiza LCD informando estado crítico
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(distanciaCm);
    lcd.print(" cm");
    lcd.setCursor(0, 1);
    lcd.print("ESTADO: CRITICO");
  }
  else if (distanciaCm <= LIMITE_ATENCAO_CM) {
    // ============================
    // ======== ESTADO ATENÇÃO ====
    // ============================
    Serial.print(">>> ESTADO: ATENCAO – ");
    Serial.print(distanciaCm);
    Serial.println(" cm <<<");

    digitalWrite(PIN_LED_AMARELO, HIGH);  // Acende LED amarelo

    // Atualiza LCD informando estado de atenção
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(distanciaCm);
    lcd.print(" cm");
    lcd.setCursor(0, 1);
    lcd.print("ESTADO: ATENCAO");
  }
  else {
    // ============================
    // ======== ESTADO NORMAL =====
    // ============================
    Serial.print(">>> ESTADO: NORMAL – ");
    Serial.print(distanciaCm);
    Serial.println(" cm <<<");

    digitalWrite(PIN_LED_VERDE, HIGH);  // Acende LED verde

    // Atualiza LCD informando estado normal
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(distanciaCm);
    lcd.print(" cm");
    lcd.setCursor(0, 1);
    lcd.print("ESTADO: NORMAL");
  }

  // 6. Pequena pausa (~200 ms) antes de nova medição
  delay(200);
}

// ============================
// === IMPLEMENTAÇÃO DE FUNÇÕES
// ============================

/*
  Função: medirDistanciaCm
  Descrição:
    - Dispara pulso de trigger no sensor HC-SR04.
    - Lê tempo de subida do sinal ECHO (pulseIn).
    - Converte tempo (microsegundos) em distância (cm).
    - Retorna -1 em caso de erro (timeout ou distância fora de faixa).
  Retorno:
    - int: distância em centímetros (2 a 400 cm) ou -1 em caso de erro.
*/
int medirDistanciaCm() {
  // 1. Gera pulso de trigger de 10 microssegundos
  digitalWrite(PIN_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIGGER, LOW);

  // 2. Lê duração do pulso de echo (em microssegundos) com timeout
  long duracaoPulsoUs = pulseIn(PIN_ECHO, HIGH, TIMEOUT_US);

  // 3. Se pulseIn retornar 0, significa timeout (eco não detectado)
  if (duracaoPulsoUs == 0) {
    return -1;  // Erro de leitura
  }

  // 4. Converte tempo em distância:
  //    velocidade do som ≈ 343 m/s = 0.0343 cm/µs
  //    distância (ida e volta) / 2
  int distanciaCm = (int)((duracaoPulsoUs * 0.0343) / 2.0);

  // 5. Valida faixa de distância (sensor confiável de ~2 a 400 cm)
  if (distanciaCm < 2 || distanciaCm > 400) {
    return -1;  // Leitura fora de faixa → erro
  }

  return distanciaCm;
}

/*
  Função: acionarIntermitenciaBuzzer
  Descrição:
    - Alterna estado do buzzer (ligado/desligado) a cada intervalo definido.
    - Usa millis() para controlar tempo sem bloquear execução.
    - Emite tom de 1 kHz por 300 ms quando ativado.
*/
void acionarIntermitenciaBuzzer() {
  unsigned long agora = millis();

  // Verifica se já passou o intervalo para trocar estado do buzzer
  if (agora - ultimoTimestampBuzzer >= INTERVALO_BUZZER_MS) {
    if (!buzzerLigado) {
      // Liga buzzer em 1 kHz por 300 ms
      tone(PIN_BUZZER, 1000, 300);
      buzzerLigado = true;
    } else {
      // Desliga buzzer
      noTone(PIN_BUZZER);
      buzzerLigado = false;
    }
    // Atualiza timestamp para próxima troca
    ultimoTimestampBuzzer = agora;
  }
}

/*
  Função: piscarLed
  Descrição:
    - Pisca um LED conectado em 'pinLed'.
    - Acende por 'duracaoMs' milissegundos, depois apaga pelo mesmo tempo.
  Parâmetros:
    - pinLed: pino digital onde o LED está conectado.
    - duracaoMs: tempo em milissegundos para cada estado (HIGH/LOW).
*/
void piscarLed(uint8_t pinLed, unsigned long duracaoMs) {
  digitalWrite(pinLed, HIGH);
  delay(duracaoMs);
  digitalWrite(pinLed, LOW);
  delay(duracaoMs);
}

/*
  Função: atualizarDisplay
  Descrição:
    - Limpa o display LCD e exibe duas linhas de texto.
  Parâmetros:
    - linha1: texto a ser exibido na primeira linha (até 16 caracteres).
    - linha2: texto a ser exibido na segunda linha (até 16 caracteres).
*/
void atualizarDisplay(const String& linha1, const String& linha2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
}
