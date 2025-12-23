# Schemat Połączeń - Mutator2000

## Widok z góry - Seeed Xiao BLE nRF52840

```
                    USB-C
                   ┌─────┐
                   │     │
     ┌─────────────┴─────┴──────────────┐
     │                                   │
 D0  │ ○                             ○  │ 5V
 D1  │ ○ ◄── Przycisk                ○  │ GND
 D2  │ ○                             ○  │ 3V3
 D3  │ ○                             ○  │ BAT ◄── Bateria (+) przez przełącznik
 D4  │ ○                             ○  │
 D5  │ ○     [LED]  [LED]  [LED]     ○  │
 D6  │ ○     (RGB wbudowane)         ○  │
 D7  │ ○                             ○  │
 D8  │ ○                             ○  │
 D9  │ ○                             ○  │
 D10 │ ○                             ○  │
     │                                   │
     └───────────────────────────────────┘
                      │
                     GND ◄── Bateria (-) oraz Przycisk
```

## Schemat Elektryczny

```
    ┌─────────┐
    │ Bateria │
    │ Li-Po   │
    │ 3.7V    │
    └────┬────┘
         │
    (+)  │  (-)
         │    │
    ┌────┴────┴─────┐
    │  Przełącznik  │
    │   ON / OFF    │
    └────┬──────────┘
         │          │
         │          │
    BAT  │         GND
         │          │
    ┌────┴──────────┴────┐
    │  Seeed Xiao BLE    │
    │    nRF52840        │
    │                    │
    │  D1 ────┐          │
    │         │          │
    └─────────┼──────────┘
              │
         ┌────┴────┐
         │ Przycisk│ (momentowy, NO - normalnie otwarty)
         └────┬────┘
              │
             GND
```

## Szczegóły Połączeń

### Zasilanie
```
Bateria (+) ──┬── [Przełącznik] ──┬── Xiao BAT pin
              │                    │
           Opcjonalne:             │
           Kondensator 100µF       │
              │                    │
             GND                  GND
```

### Przycisk z Debouncing (opcjonalne)
```
Xiao D1 ──┬── [Rezystor 10kΩ] ──┬── Przycisk ── GND
          │                      │
          └── [Kondensator 100nF]┘
```
**Uwaga**: Rezystor pull-up jest już wbudowany w Xiao (INPUT_PULLUP), więc zewnętrzny nie jest wymagany. Kondensator także opcjonalny - debouncing jest obsłużony w software.

## Uproszczone Połączenie (minimalna wersja)

```
┌─────────┐
│ Bateria │ (+) ──── [Przełącznik] ──── BAT (Xiao)
│ 3.7V    │ (-)  ────────────────────── GND (Xiao)
└─────────┘

┌──────────┐
│ Przycisk │ pin 1 ──── D1 (Xiao)
│          │ pin 2 ──── GND
└──────────┘
```

## Lista Części

| Część | Specyfikacja | Ilość | Uwagi |
|-------|-------------|-------|-------|
| Seeed Xiao BLE nRF52840 | Płytka główna | 1 | Z wbudowanym Bluetooth 5.0 |
| Bateria Li-Po | 3.7V, 100-500mAh | 1 | Im większa pojemność, tym dłuższy czas pracy |
| Przełącznik SPST | Mini toggle/slide | 1 | Włącz/wyłącz zasilanie |
| Przycisk momentowy | 6x6mm tact switch | 1 | Normalnie otwarty (NO) |
| Kable/przewody | 28-22 AWG | - | Do połączeń |
| Opcjonalnie: Obudowa | 3D printed | 1 | Wymiary dostosowane do komponentów |

## Rozmiary i Montaż

### Wymiary Xiao BLE
- Długość: 21mm
- Szerokość: 17.5mm
- Wysokość: ~3mm (bez pinów)

### Sugerowane wymiary obudowy
- Wewnętrzne: 30mm x 25mm x 15mm (minimum)
- Otwór na przycisk: 6mm x 6mm
- Otwór na przełącznik: według wybranego modelu
- Otwór na USB-C: do ładowania

## Montaż Krok po Kroku

1. **Przygotuj Xiao**
   - Nie potrzebujesz lutować pinów jeśli połączenia będą bezpośrednie
   - Dla stabilności można przylutować przewody bezpośrednio do padów

2. **Podłącz baterie**
   - (+) baterii → Przełącznik → BAT pad na Xiao
   - (-) baterii → GND pad na Xiao
   - **UWAGA**: Sprawdź polaryzację!

3. **Podłącz przycisk**
   - Jeden pin → D1 (GPIO 1) na Xiao
   - Drugi pin → GND na Xiao

4. **Testuj przed zamknięciem**
   - Wgraj kod przez USB
   - Włącz przełącznik
   - Sprawdź czy LED miga (tryb parowania)
   - Sparuj z komputerem
   - Testuj przycisk

5. **Montaż w obudowie**
   - Umieść komponenty w obudowie
   - Upewnij się że przycisk jest dostępny
   - Przełącznik powinien być łatwo dostępny
   - Zostaw dostęp do USB-C do ładowania

## Ładowanie

- Podłącz USB-C do Xiao
- Czerwony LED na płytce świeci podczas ładowania
- LED gaśnie gdy bateria naładowana
- Można ładować i używać jednocześnie

## Bezpieczeństwo

⚠️ **Ważne zasady bezpieczeństwa dla baterii Li-Po:**

- Nie dopuść do zwarcia baterii
- Używaj baterii z wbudowaną ochroną (PCB)
- Nie ładuj baterii bez nadzoru
- Nie przebijaj ani nie gnij baterii
- Przechowuj w temperaturze 15-25°C
- Przy uszkodzeniu baterii - usuń bezpiecznie

## Opcjonalne Ulepszenia

### 1. Wskaźnik Baterii
```cpp
// Dodaj pin do pomiaru napięcia baterii
#define VBAT_PIN A0

void checkBattery() {
  int raw = analogRead(VBAT_PIN);
  float voltage = raw * 3.6 / 1024;
  Serial.print("Bateria: ");
  Serial.print(voltage);
  Serial.println("V");
}
```

### 2. Drugi Przycisk (Custom funkcja)
```
D2 ──── Przycisk 2 ──── GND
```

### 3. RGB LED Zewnętrzny (feedback wizualny)
```
D3 ──── [Rezystor 220Ω] ──── LED Red ──── GND
D4 ──── [Rezystor 220Ω] ──── LED Green ──── GND
D5 ──── [Rezystor 220Ω] ──── LED Blue ──── GND
```
