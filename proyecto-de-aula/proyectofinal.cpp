#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

#define OLED_ANCHO 128
#define OLED_ALTO 64
#define DIRECCION_I2C 0x3C

#define PIN_SUBIR 4
#define PIN_BAJAR 15

RTC_DS3231 rtc;
Adafruit_SSD1306 oled(OLED_ANCHO, OLED_ALTO, &Wire, -1);
MAX30105 particleSensor;

const byte RATE_SIZE = 5;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute = 0;
int beatAvg = 0;

unsigned long lastOledUpdate = 0;
const unsigned long oledUpdateInterval = 500;

int edadSeleccionada = 25;
bool edadConfirmada = false;

unsigned long tiempoPresionadoSubir = 0;
unsigned long tiempoPresionadoBajar = 0;
const unsigned long tiempoConfirmacion = 3000; // 3 segundos de confirmación
unsigned long tiempoSinPresion = 0;            // Tiempo sin presión para confirmar la edad

// Para reinicio de selección de edad
unsigned long tiempoInicioReinicio = 0;
const unsigned long tiempoReinicioEdad = 10000; // 10 segundos

void obtenerRangoBPM(int edad, int &limiteInferior, int &limiteSuperior)
{
    if (edad <= 17)
    {
        limiteInferior = 70;
        limiteSuperior = 100;
    }
    else if (edad <= 40)
    {
        limiteInferior = 60;
        limiteSuperior = 100;
    }
    else if (edad <= 60)
    {
        limiteInferior = 60;
        limiteSuperior = 95;
    }
    else
    {
        limiteInferior = 50;
        limiteSuperior = 90;
    }
}

void seleccionarEdad()
{
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.println("Selecciona tu edad");
    oled.setTextSize(2);
    int edadAncho = edadSeleccionada < 10 ? 24 : (edadSeleccionada < 100 ? 16 : 8);
    oled.setCursor((OLED_ANCHO - edadAncho) / 2, 25);
    oled.print(edadSeleccionada);
    oled.display();

    int edadAnterior = -1;

    while (!edadConfirmada)
    {
        bool subirPresionado = digitalRead(PIN_SUBIR) == LOW;
        bool bajarPresionado = digitalRead(PIN_BAJAR) == LOW;

        if (!subirPresionado && !bajarPresionado)
        {
            if (tiempoSinPresion == 0)
            {
                tiempoSinPresion = millis();
            }
            if (millis() - tiempoSinPresion >= tiempoConfirmacion)
            {
                edadConfirmada = true;
                oled.clearDisplay();
                oled.setTextSize(1);
                oled.setCursor(0, 0);
                oled.println("Edad confirmada");
                oled.setTextSize(2);
                oled.setCursor((OLED_ANCHO - 16) / 2, 30);
                oled.print(edadSeleccionada);
                oled.display();
                delay(2000);
                break;
            }
        }
        else
        {
            tiempoSinPresion = 0;
        }

        if (subirPresionado && !bajarPresionado)
        {
            if (tiempoPresionadoSubir == 0)
                tiempoPresionadoSubir = millis();
            if (millis() - tiempoPresionadoSubir >= tiempoConfirmacion)
            {
                edadSeleccionada++;
                tiempoPresionadoSubir = 0;
            }
        }
        else
        {
            tiempoPresionadoSubir = 0;
        }

        if (bajarPresionado && !subirPresionado)
        {
            if (tiempoPresionadoBajar == 0)
                tiempoPresionadoBajar = millis();
            if (millis() - tiempoPresionadoBajar >= tiempoConfirmacion)
            {
                if (edadSeleccionada > 1)
                    edadSeleccionada--;
                tiempoPresionadoBajar = 0;
            }
        }
        else
        {
            tiempoPresionadoBajar = 0;
        }

        if (edadSeleccionada != edadAnterior)
        {
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(0, 0);
            oled.println("Selecciona tu edad");
            oled.setTextSize(2);
            edadAncho = edadSeleccionada < 10 ? 24 : (edadSeleccionada < 100 ? 16 : 8);
            oled.setCursor((OLED_ANCHO - edadAncho) / 2, 25);
            oled.print(edadSeleccionada);
            oled.display();
            edadAnterior = edadSeleccionada;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_SUBIR, INPUT_PULLUP);
    pinMode(PIN_BAJAR, INPUT_PULLUP);

    Wire.begin();
    delay(500);

    if (!oled.begin(SSD1306_SWITCHCAPVCC, DIRECCION_I2C))
    {
        Serial.println("No se encontró el OLED");
        while (1)
            ;
    }

    oled.setTextColor(SSD1306_WHITE);

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println("Iniciando...");
    oled.display();
    delay(500);

    seleccionarEdad();

    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setCursor(0, 25);
    oled.println("Detectando");
    oled.display();
    delay(1000);

    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
    {
        Serial.println("MAX30102 no encontrado");
        while (1)
            ;
    }

    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x4F);
    particleSensor.setPulseAmplitudeGreen(0);
    if (!rtc.begin())
    {
        Serial.println("No se encuentra el RTC");
        while (1)
            ;
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC sin hora. Estableciendo hora por defecto...");
        rtc.adjust(DateTime(2025, 5, 26, 14, 30, 0));
    }
}

void loop()
{
    DateTime now = rtc.now();
    // Mostrar hora arriba a la derecha

    long irValue = particleSensor.getIR();

    if (irValue < 50000)
    {
        if (millis() - lastOledUpdate >= oledUpdateInterval)
        {
            lastOledUpdate = millis();
            oled.clearDisplay();
            oled.setTextSize(2);
            oled.setCursor(0, 25);
            oled.println("Detectando");
            oled.display();
        }
    }
    else
    {
        if (checkForBeat(irValue))
        {
            long delta = millis() - lastBeat;
            lastBeat = millis();
            beatsPerMinute = 60 / (delta / 1000.0);

            if (beatsPerMinute < 255 && beatsPerMinute > 20)
            {
                rates[rateSpot++] = (byte)beatsPerMinute;
                rateSpot %= RATE_SIZE;

                beatAvg = 0;
                for (byte x = 0; x < RATE_SIZE; x++)
                {
                    beatAvg += rates[x];
                }
                beatAvg /= RATE_SIZE;
            }
        }

        if (millis() - lastOledUpdate >= oledUpdateInterval)
        {
            lastOledUpdate = millis();
            oled.clearDisplay();

            oled.setTextSize(1);
            oled.setCursor(80, 0);
            oled.print(now.hour());
            oled.print(":");
            if (now.minute() < 10)
                oled.print("0");
            oled.print(now.minute());

            oled.setTextSize(1);
            oled.setCursor(0, 0);
            oled.println("BPM:");

            oled.setTextSize(2);
            oled.setCursor(0, 20);
            if (beatAvg > 0)
            {
                oled.print(beatAvg);
            }
            else
            {
                oled.print("--");
            }

            int limInf, limSup;
            obtenerRangoBPM(edadSeleccionada, limInf, limSup);
            oled.setTextSize(1);
            oled.setCursor(0, 55);

            if (beatAvg == 0)
            {
                oled.print("Sin lectura");
            }
            else if (beatAvg < limInf)
            {
                oled.print("Ritmo bajo");
            }
            else if (beatAvg > limSup)
            {
                oled.print("Ritmo alto");
            }
            else
            {
                oled.print("Ritmo normal");
            }

            oled.display();
        }
    }

    // === Verificación para reiniciar la edad si ambos botones están presionados por 10 segundos ===
    if (digitalRead(PIN_SUBIR) == HIGH && digitalRead(PIN_BAJAR) == HIGH)
    {
        if (tiempoInicioReinicio == 0)
        {
            tiempoInicioReinicio = millis();
        }
        else if (millis() - tiempoInicioReinicio >= tiempoReinicioEdad)
        {
            // Mostrar mensaje
            oled.clearDisplay();
            oled.setTextSize(1);
            oled.setCursor(0, 0);
            oled.println("Reiniciando edad...");
            oled.display();
            delay(1500);

            edadConfirmada = false;
            seleccionarEdad();
            tiempoInicioReinicio = 0;
        }
    }
    else
    {
        tiempoInicioReinicio = 0;
    }
}
