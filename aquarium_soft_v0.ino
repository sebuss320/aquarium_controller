// Dodanie bibliotek
#include <EEPROM.h>
#include <Stepper.h>
#include <RealTimeClockDS1307.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>              // standardowa biblioteka Arduino
#include <LiquidCrystal_I2C.h> // biblioteka I2C dla LCD
#include <BH1750.h>

// Deklaracja wyjść
#define Ln1 4       //Pin sterowania silnikiem krokowym
#define Ln2 2       //Pin sterowania silnikiem krokowym
#define Ln3 3       //Pin sterowania silnikiem krokowym
#define Ln4 5       //Pin sterowania silnikiem krokowym
OneWire oneWire(6); //Podłączenie 6
#define przyciskGpin 7
#define przyciskMpin 8
#define przyciskSpin 9
#define przyciskUSpin 11
#define przyciskUpin 12
#define przekaznik A0
#define czujnik_zw A1
#define sondaPHpin A2

//#define

Stepper silnik(2048, Ln1, Ln2, Ln3, Ln4);
DallasTemperature czujnik(&oneWire); //Przekazania informacji do biblioteki
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Ustawienie adresu ukladu na 0x27
BH1750 lightMeter;

//Tworzenie zmiennych pomocniczych
const int Kroki = 2048;
int godzina = 0; // dodawanie
int minuta = 0;
int sekunda = 0;
int godzinaK = 0; // ustalanie godziny karmienia
int minutaK = 0;
int sekundaK = 0;
int hours = 0; // Przechowywanie czasu rzeczywistego z rtc
int minutes = 0;
int seconds = 0;
int godzinaW = 0; // ustawianie godziny wschodu, zachodu
int minutaW = 0;
int godzinaZ = 0;
int minutaZ = 0;
int pora = 0;

int pomiarZ = 0;
float voltage = 0; /////////////////////////////////
unsigned int pomiarPH;
float pH = 0;
float lux = 0;

long s_time = 0;
int aa = 0;
int Tkarmienia = 0;
int wynik = 0;

int US = 0;
int ustawienia = 0;
int a, b, c, d, e, f;

void setup()
{
    Serial.begin(9600);
    // konfiguracja pinów
    pinMode(Ln1, OUTPUT);
    pinMode(Ln2, OUTPUT);
    pinMode(Ln3, OUTPUT);
    pinMode(Ln4, OUTPUT);
    pinMode(przyciskGpin, INPUT_PULLUP);
    pinMode(przyciskMpin, INPUT_PULLUP);
    pinMode(przyciskSpin, INPUT_PULLUP);
    pinMode(przyciskUpin, INPUT_PULLUP);
    pinMode(przyciskUSpin, INPUT_PULLUP);

    lcd.begin();        // Inicjalizacja LCD
    lcd.backlight();    // załączenie podświetlenia
    lightMeter.begin(); //inicjalizacja czujnika natężenia oświetlania
}

void loop()
{
    // odczytywanie danych z modułu RTC
    RTC.readClock();
    hours = RTC.getHours();
    minutes = RTC.getMinutes();
    seconds = RTC.getSeconds();

    //Pobranie temperatury z czujnika temperatury
    czujnik.requestTemperatures();

    // Jeśli przycisk S1 zostanie wciśnięty przęłącz tryb
    if (isButtonPressed(przyciskUpin))
    {
        ustawienia++;
        if (ustawienia == 6)
            ustawienia = 0;
        delay(200);
    }

    switch (ustawienia)
    {
        // Wyświetlanie dokonywanych stale pomiarów oraz godziny, po wciśnięciu odpowiednich przycisków pomiar pH wody oraz natężenia oświetlenia
    case 0:

        if (a == 1 || (hours == 0 && minutes == 0 && seconds == 0) || (minutes == 0 && seconds == 0))
            lcd.clear();
        lcd.setCursor(0, 0); // wyświetlanie temperatury
        lcd.print("T:");
        lcd.setCursor(2, 0);
        lcd.print(czujnik.getTempCByIndex(0));

        lcd.setCursor(11, 0); // wyświetlanie czasu
        lcd.print(RTC.getHours());
        lcd.setCursor(13, 0);
        lcd.print(":");
        lcd.setCursor(14, 0);
        lcd.print(RTC.getMinutes());
        d = c = b = f = e = 1; // Zmienne pomocnicze do wyczyszczenia wyswietlacza
        a = 0;

        ustawieniaSet();
        dodacSekunda();
        if (US == 1 && a == 0)
        { // pomiar pH
            pomiarPH = analogRead(sondaPHpin);
            pH = pomiarPH * 5.0 / 1024;
            pH = 3.5 * pH;
            lcd.setCursor(9, 1);
            lcd.print("pH:") + lcd.print(pH);

            delay(10000);
            lcd.clear();
            US = 0;
        }

        if (sekunda == 1 && a == 0)
        { // pomiar nat natężenia
            lux = lightMeter.readLightLevel();
            lcd.setCursor(7, 1);
            lcd.print("lX:") + lcd.print(lux);

            delay(10000);
            lcd.clear();
            sekunda = 0;
        }

        pomiarZ = analogRead(czujnik_zw); // pomiar zmętnienia
        voltage = pomiarZ * (5.0 / 1024.0);
        voltage = (voltage / 5) * 100;
        voltage = 100.0 - voltage;
        wynik = round(voltage);
        lcd.setCursor(0, 1);
        //  lcd.print(wynik) + lcd.print("%");
        lcd.print("43") + lcd.print("%");

        break;
        // Możliwosć modyikacji ustawień godziny karmienia ryb.
    case 1:
        if (b == 1)
        {
            lcd.clear();
            US = 0;
        }
        dodacGodzina(); // funkcje odpowiedzialne za działanie przycisków
        dodacMinuta();
        ustawieniaSet();
        lcd.setCursor(0, 0);
        lcd.print("USTAWIENIA");
        lcd.setCursor(0, 1);
        lcd.print("godz karm");

        if (US == 1)
        { // konfikuracja czasów
            godzinaK = godzina;
            minutaK = minuta;
            EEPROM.write(0, godzinaK);
            EEPROM.write(1, minutaK);
            lcd.setCursor(13, 1);
            lcd.print(":");
            lcd.setCursor(11, 1);
            lcd.print(EEPROM.read(0));
            lcd.setCursor(14, 1);
            lcd.print(EEPROM.read(1));
        }
        lcd.setCursor(13, 1);
        lcd.print(":");
        lcd.setCursor(11, 1);
        lcd.print(EEPROM.read(0));
        lcd.setCursor(14, 1);
        lcd.print(EEPROM.read(1));
        a = 1;
        b = 0;
        c = 1;
        d = 1;
        f = 1;
        e = 1;
        break;
        // Możliwość ustawienia czasu w module RTC
    case 2:

        if (c == 1)
        {
            lcd.clear();
            US = 0;
        }
        dodacGodzina(); // funkcje odpowiedzialne za działanie przycisków
        dodacMinuta();
        dodacSekunda();
        ustawieniaSet();
        lcd.setCursor(0, 0);
        lcd.print("USTAWIENIA");
        lcd.setCursor(0, 1);
        lcd.print("godz");
        if (US == 1)
        { // konfiguracja godziny
            RTC.setHours(godzina);
            RTC.setClock();
            RTC.setMinutes(minuta);
            RTC.setClock();
            RTC.setSeconds(sekunda);
            RTC.setClock();
            lcd.setCursor(6, 1);
            lcd.print(hours);
            lcd.setCursor(9, 1);
            lcd.print(minutes);
            lcd.setCursor(12, 1);
            lcd.print(seconds);
        }
        b = 1;
        a = 1;
        c = 0;
        d = 1;
        f = 1;
        e = 1;

        break;
        // Możliwosć ustawienia długości czasu karmienia ryb.
    case 3:
        if (d == 1)
        {
            lcd.clear();
            US = 0;
        }
        ustawieniaSet();
        dodacSekunda();
        lcd.setCursor(0, 0);
        lcd.print("USTAWIENIA");
        lcd.setCursor(0, 1);
        lcd.print("czas karm");
        if (US == 1)
        {
            Tkarmienia = sekunda;
            EEPROM.write(2, Tkarmienia);
            lcd.setCursor(14, 1);
            lcd.print(EEPROM.read(2));
        }
        lcd.setCursor(14, 1);
        lcd.print(EEPROM.read(2));
        b = 1;
        a = 1;
        c = 1;
        d = 0;
        f = 1;
        e = 1;
        break;
        // Możliwość modyfikacji oraz zapamiętanie godziny wschodu.
    case 4:

        if (e == 1)
        {
            lcd.clear();
            US = 0;
        }
        ustawieniaSet();
        dodacGodzina();
        dodacMinuta();
        lcd.setCursor(0, 0);
        lcd.print("USTAWIENIA");
        lcd.setCursor(0, 1);
        lcd.print("wschodu");
        if (US == 1)
        {
            godzinaW = godzina;
            minutaW = minuta;
            EEPROM.write(3, godzinaW);
            EEPROM.write(4, minutaW);
            lcd.setCursor(13, 1);
            lcd.print(":");
            lcd.setCursor(11, 1);
            lcd.print(EEPROM.read(3));
            lcd.setCursor(14, 1);
            lcd.print(EEPROM.read(4));
        }
        lcd.setCursor(13, 1);
        lcd.print(":");
        lcd.setCursor(11, 1);
        lcd.print(EEPROM.read(3));
        lcd.setCursor(14, 1);
        lcd.print(EEPROM.read(4));
        b = 1;
        a = 1;
        c = 1;
        d = 1;
        f = 1;
        e = 0;
        break;
        // Możliwość modyfikacji oraz zapamiętanie godziny zachodu.
    case 5:

        if (f == 1)
        {
            lcd.clear();
            US = 0;
        }
        ustawieniaSet();
        dodacGodzina();
        dodacMinuta();
        lcd.setCursor(0, 0);
        lcd.print("USTAWIENIA");
        lcd.setCursor(0, 1);
        lcd.print("zachodu");
        if (US == 1)
        {
            godzinaZ = godzina;
            minutaZ = minuta;
            EEPROM.write(5, godzinaZ);
            EEPROM.write(6, minutaZ);
            lcd.setCursor(13, 1);
            lcd.print(":");
            lcd.setCursor(11, 1);
            lcd.print(EEPROM.read(5));
            lcd.setCursor(14, 1);
            lcd.print(EEPROM.read(6));
        }
        lcd.setCursor(13, 1);
        lcd.print(":");
        lcd.setCursor(11, 1);
        lcd.print(EEPROM.read(5));
        lcd.setCursor(14, 1); // Ustawienie kursora
        lcd.print(EEPROM.read(6));
        b = 1;
        a = 1;
        c = 1;
        d = 1;
        f = 0;
        e = 1;
        break;
    } //switch

    //Realizacja pracy silnika krokowego.
    if (EEPROM.read(0) == hours && EEPROM.read(1) == minutes && seconds == 1)
        aa = 1;

    if (aa == 1)
    {
        if (s_time == 0)
        {
            s_time = millis();
        }
        else
        {
            if (millis() - s_time >= 0)
            {
                silnik.step(Kroki);
            }
            if (millis() - s_time >= EEPROM.read(2) * 1000)
            {
                digitalWrite(2, LOW);
                digitalWrite(3, LOW);
                digitalWrite(4, LOW);
                digitalWrite(5, LOW);
                s_time = 0;
                aa = 0;
            }
        }
    }

    // Realizacja załączania przekaźnika
    if (EEPROM.read(3) >= hours && EEPROM.read(4) >= minutes && seconds >= 1)
        pora = 1;

    if (EEPROM.read(5) == hours && EEPROM.read(6) == minutes && seconds == 1)
        pora = 0;

    if (pora == 1)
        analogWrite(przekaznik, 0);
    else
        analogWrite(przekaznik, 255);

} // loop

// Funkcja, odpowiedzialna za modyfikacje godzin.
int dodacGodzina()
{
    if (isButtonPressed(przyciskGpin))
    {
        godzina++;
        if (godzina > 23)
        {
            godzina = 0;
            lcd.clear();
        }
        delay(200);
    }
    return godzina;
}

// Funkcja, odpowiedzialna za modyfikacje minut.
int dodacMinuta()
{
    if (isButtonPressed(przyciskMpin))
    {
        minuta++;
        if (minuta > 59)
        {
            minuta = 0;
            lcd.clear();
        }
        delay(200);
    }
    return minuta;
}

// Funkcja, odpowiedzialna za modyfikacje sekund lub dokonanie pomiaru natężenia oświetlenia.
int dodacSekunda()
{
    if (isButtonPressed(przyciskSpin))
    {
        sekunda++;
        if (sekunda > 59)
        {
            sekunda = 0;
            lcd.clear();
        }
        delay(200);
    }
    return sekunda;
}

// Funkcja, odpowiedzialna za potwierdzenie chęci modyfikacji ustawień lub dokonanie pomiaru pH
int ustawieniaSet()
{
    if (isButtonPressed(przyciskUSpin))
    {
        US++;
        if (US == 2)
            US = 0;
        delay(200);
    }
    return US;
}

//Funkcja, odpowiedzialna za prawidłową pracę przycisków
bool isButtonPressed(int button)
{
    if (digitalRead(button) == LOW)
    {
        delay(20);
        if (digitalRead(button) == LOW)
        {
            return true;
        }
    }
    return false;
}
