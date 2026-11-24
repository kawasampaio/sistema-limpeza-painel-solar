#include <Arduino.h>

// =====================================================
// PINOS
// =====================================================

// Sensor infravermelho
const uint8_t SENSOR_PIN = A0;

// Relés
const uint8_t RELE_BOMBA_1 = 7;  // IN1 / K1
const uint8_t RELE_BOMBA_2 = 8;  // IN2 / K2


// =====================================================
// CONFIGURAÇÃO DO RELÉ
// =====================================================

// A maioria dos módulos de relé para Arduino é ACTIVE LOW:
//
// LOW  = relé ligado
// HIGH = relé desligado
//
// Se o seu funcionar ao contrário, troque os dois valores.

const uint8_t RELE_LIGADO = LOW;
const uint8_t RELE_DESLIGADO = HIGH;


// =====================================================
// CONFIGURAÇÃO DO SENSOR
// =====================================================

// Se aumentar a sujeira fizer a leitura do A0 DIMINUIR:
// deixe false.
//
// Se aumentar a sujeira fizer a leitura do A0 AUMENTAR:
// coloque true.

const bool SENSOR_INVERTED = false;


// =====================================================
// LIMITE DE SUJIDADE
// =====================================================

// Painel limpo = 100%
//
// O sistema será acionado quando o sinal cair 5%.
//
// 100% - 5% = 95%

const float TRIGGER_PERCENT = 95.0;


// =====================================================
// CONFIRMAÇÃO
// =====================================================

// Número de leituras seguidas abaixo de 95% antes de
// considerar que realmente existe sujeira.

const uint8_t CONFIRMATION_READS = 8;


// =====================================================
// TEMPOS DAS BOMBAS
// =====================================================

// Tempo que as bombas ficarão ligadas.
//
// 5000 ms = 5 segundos.
//
// PARA O PRIMEIRO TESTE:
// deixe apenas 5 segundos.

const unsigned long TEMPO_BOMBAS = 5000;


// Depois da limpeza espera 10 segundos antes
// de começar uma nova análise.

const unsigned long TEMPO_ESPERA_APOS_LIMPEZA = 10000;


// =====================================================
// LEITURAS
// =====================================================

const uint16_t CALIBRATION_SAMPLES = 150;
const uint8_t READ_SAMPLES = 25;


// =====================================================
// VARIÁVEIS
// =====================================================

float baselineSignal = 0.0;

uint8_t contadorSujeira = 0;


// =====================================================
// FUNÇÃO: MÉDIA DO SENSOR
// =====================================================

float lerSensorMedia(uint16_t quantidade) {

    unsigned long soma = 0;

    for (uint16_t i = 0; i < quantidade; i++) {

        soma += analogRead(SENSOR_PIN);

        delay(5);
    }

    return (float)soma / quantidade;
}


// =====================================================
// FUNÇÃO: CORRIGIR SENTIDO DO SENSOR
// =====================================================

float converterSinal(float leitura) {

    if (SENSOR_INVERTED) {

        return 1023.0 - leitura;
    }

    return leitura;
}


// =====================================================
// FUNÇÃO: DESLIGAR BOMBAS
// =====================================================

void desligarBombas() {

    digitalWrite(RELE_BOMBA_1, RELE_DESLIGADO);

    digitalWrite(RELE_BOMBA_2, RELE_DESLIGADO);
}


// =====================================================
// FUNÇÃO: LIGAR BOMBAS
// =====================================================

void ligarBombas() {

    digitalWrite(RELE_BOMBA_1, RELE_LIGADO);

    digitalWrite(RELE_BOMBA_2, RELE_LIGADO);
}


// =====================================================
// CICLO DE LIMPEZA
// =====================================================

void executarLimpeza() {

    Serial.println();
    Serial.println(F("======================================"));
    Serial.println(F("SUJIDADE DETECTADA"));
    Serial.println(F("INICIANDO LIMPEZA"));
    Serial.println(F("======================================"));


    // Liga os dois relés
    ligarBombas();


    Serial.println(F("K1 / BOMBA 1: LIGADA"));
    Serial.println(F("K2 / BOMBA 2: LIGADA"));


    // Mantém as bombas ligadas
    delay(TEMPO_BOMBAS);


    // Desliga
    desligarBombas();


    Serial.println();
    Serial.println(F("BOMBA 1: DESLIGADA"));
    Serial.println(F("BOMBA 2: DESLIGADA"));

    Serial.println(F("LIMPEZA FINALIZADA"));
    Serial.println();


    // Espera antes de voltar a medir
    Serial.println(F("Aguardando estabilizacao do sensor..."));

    delay(TEMPO_ESPERA_APOS_LIMPEZA);


    contadorSujeira = 0;
}


// =====================================================
// SETUP
// =====================================================

void setup() {

    Serial.begin(115200);


    // -----------------------------
    // Sensor
    // -----------------------------

    pinMode(SENSOR_PIN, INPUT);


    // -----------------------------
    // Relés
    // -----------------------------

    pinMode(RELE_BOMBA_1, OUTPUT);

    pinMode(RELE_BOMBA_2, OUTPUT);


    // IMPORTANTE:
    // Arduino sempre inicia com as bombas desligadas.

    desligarBombas();


    delay(2000);


    Serial.println();
    Serial.println(F("======================================"));
    Serial.println(F(" SISTEMA DE LIMPEZA DE PAINEL SOLAR"));
    Serial.println(F("======================================"));

    Serial.println();

    Serial.println(F("Sensor: A0"));

    Serial.println(F("Bomba 1: D7 / K1"));

    Serial.println(F("Bomba 2: D8 / K2"));

    Serial.println();


    // =================================================
    // CALIBRAÇÃO
    // =================================================

    Serial.println(F("CALIBRANDO SENSOR..."));

    Serial.println(F("Mantenha o sensor apontado para"));
    Serial.println(F("o painel LIMPO durante a calibracao."));

    Serial.println();


    delay(3000);


    float leituraInicial = lerSensorMedia(CALIBRATION_SAMPLES);


    baselineSignal = converterSinal(leituraInicial);


    Serial.println(F("CALIBRACAO CONCLUIDA"));

    Serial.print(F("Leitura A0: "));

    Serial.println(leituraInicial, 1);


    Serial.print(F("Referencia: "));

    Serial.print(baselineSignal, 1);

    Serial.println(F(" = 100%"));


    Serial.print(F("Limite de limpeza: "));

    Serial.print(TRIGGER_PERCENT);

    Serial.println(F("%"));


    Serial.println();

    Serial.println(F("--------------------------------------"));

    Serial.println(F("A0 | Sinal (%) | Queda (%) | Estado"));

    Serial.println(F("--------------------------------------"));
}


// =====================================================
// LOOP
// =====================================================

void loop() {

    // =================================================
    // LÊ SENSOR
    // =================================================

    float leituraBruta = lerSensorMedia(READ_SAMPLES);


    float sinal = converterSinal(leituraBruta);


    // =================================================
    // CALCULA PORCENTAGEM
    // =================================================

    float porcentagem = 100.0;


    if (baselineSignal > 0) {

        porcentagem =
            (sinal / baselineSignal) * 100.0;
    }


    float queda = 100.0 - porcentagem;


    if (queda < 0) {

        queda = 0;
    }


    // =================================================
    // MOSTRA NO SERIAL
    // =================================================

    Serial.print(leituraBruta, 1);

    Serial.print(F(" | "));

    Serial.print(porcentagem, 1);

    Serial.print(F("% | "));

    Serial.print(queda, 1);

    Serial.print(F("% | "));


    // =================================================
    // VERIFICA SUJEIRA
    // =================================================

    if (porcentagem <= TRIGGER_PERCENT) {

        contadorSujeira++;


        Serial.print(F("SUJEIRA "));

        Serial.print(contadorSujeira);

        Serial.print(F("/"));

        Serial.println(CONFIRMATION_READS);


        // =============================================
        // CONFIRMOU SUJEIRA
        // =============================================

        if (contadorSujeira >= CONFIRMATION_READS) {

            executarLimpeza();
        }
    }

    else {

        contadorSujeira = 0;

        Serial.println(F("NORMAL"));
    }


    delay(250);
}