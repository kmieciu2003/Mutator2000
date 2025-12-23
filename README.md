# Mutator2000 - Bluetooth Mikrofon Mute Button

Projekt przycisku do wyciszania mikrofonu przez Bluetooth dla **Seeed Xiao BLE nRF52840**.

## 🔧 Komponenty

- **Seeed Xiao BLE nRF52840** - główna płytka
- **Przycisk momentowy** - podłączony do pinu D1 (GPIO 1)
- **Bateria** (np. Li-Po 3.7V) - zasilanie mobilne
- **Przełącznik** - między baterią a płytką
- Opcjonalnie: **obudowa 3D printed**

## 📐 Schemat Połączeń

```
Bateria (+) ──── [Przełącznik] ──── BAT (pin na Xiao)
Bateria (-)  ────────────────────── GND (pin na Xiao)

Przycisk:
  - Pin 1 ──── D1 (GPIO 1)
  - Pin 2 ──── GND

LED wbudowany (LED_BUILTIN) - wskaźnik statusu
```

### Pinout Seeed Xiao BLE nRF52840
- **D1 (GPIO 1)** - Przycisk mute
- **LED_BUILTIN** - LED statusu (czerwony/niebieski/zielony w zależności od wersji)
- **BAT** - Zasilanie z baterii
- **GND** - Masa

## 🚀 Funkcjonalność

### Uruchamianie
1. **Zwykły start**: Włącz przełącznik - urządzenie próbuje połączyć się z ostatnio sparowanym komputerem
2. **Reset historii**: Przytrzymaj przycisk podczas włączania - czyści historię i wchodzi w tryb parowania

### Działanie
- **Połączony** (LED świeci): Wciśnięcie przycisku wysyła skrót `Ctrl+Shift+M` do komputera
- **Niepołączony** (LED nie świeci): Urządzenie próbuje się połączyć lub jest w trybie parowania

### Tryby
1. **Tryb normalny**: Łączenie z ostatnim urządzeniem (30 sekund próby)
2. **Tryb parowania**: Jeśli brak historii lub nie udało się połączyć z ostatnim urządzeniem

## 📦 Instalacja i Kompilacja

### Opcja 1: VSCode + PlatformIO (ZALECANE) ⭐

1. **Zainstaluj VSCode**
   - Pobierz z https://code.visualstudio.com/

2. **Zainstaluj rozszerzenie PlatformIO**
   - Otwórz VSCode
   - Extensions (Ctrl+Shift+X)
   - Szukaj "PlatformIO IDE"
   - Kliknij Install

3. **Otwórz projekt**
   - File → Open Folder
   - Wybierz folder `Mutator2000`

4. **Kompilacja i wgrywanie**
   - Podłącz Xiao BLE przez USB-C
   - Kliknij ikonę PlatformIO (alien) na lewym pasku
   - **Build** - kompiluje projekt
   - **Upload** - wgrywa na płytkę
   - **Monitor** - otwiera Serial Monitor
   - Lub użyj przycisków na dolnym pasku VSCode

5. **Skróty klawiszowe**
   - `Ctrl+Alt+B` - Build
   - `Ctrl+Alt+U` - Upload
   - `Ctrl+Alt+S` - Serial Monitor

### Opcja 2: Arduino IDE (jeśli wolisz klasycznie)

1. **Zainstaluj Arduino IDE**
   - Pobierz z https://www.arduino.cc/en/software

2. **Dodaj Board Support Package**
   - File → Preferences → Additional Boards Manager URLs:
   - Dodaj: `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
   - Tools → Board → Boards Manager → szukaj "Adafruit nRF52" → Install

3. **Zainstaluj biblioteki**
   - Sketch → Include Library → Manage Libraries
   - Zainstaluj: **Adafruit TinyUSB Library**, **Adafruit LittleFS**

4. **Konfiguracja**
   - **Board**: Tools → Board → Seeed nRF52 Boards → Seeed XIAO BLE - nRF52840
   - **Port**: Wybierz odpowiedni port COM/USB
   - Otwórz plik `Mutator2000.ino`
   - Upload (Ctrl+U)

## 🔑 Konfiguracja Skrótu Klawiszowego

Domyślnie urządzenie wysyła `Ctrl+Shift+M`. Aby zmienić:

```cpp
// W pliku Mutator2000.ino, linie 16-17:
#define MUTE_KEY HID_KEY_M
#define MUTE_MODIFIERS (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT)
```

### Dostępne klawisze HID
- `HID_KEY_M`, `HID_KEY_A`, `HID_KEY_B`, etc.
- `HID_KEY_F13` - `HID_KEY_F24` (rzadko używane, dobre dla custom skrótów)

### Modyfikatory
- `KEYBOARD_MODIFIER_LEFTCTRL`
- `KEYBOARD_MODIFIER_LEFTSHIFT`
- `KEYBOARD_MODIFIER_LEFTALT`
- `KEYBOARD_MODIFIER_LEFTGUI` (Windows key)

## 🖥️ Konfiguracja na Komputerze

### Windows 10/11

1. **Parowanie**:
   - Settings → Bluetooth & devices → Add device
   - Wybierz "Mutator2000"
   
2. **Konfiguracja skrótu mute**:
   - Pobierz aplikację typu AutoHotkey lub użyj Windows PowerToys
   - Skonfiguruj `Ctrl+Shift+M` do przełączania mute mikrofonu

### Przykład AutoHotkey (mute_toggle.ahk):
```autohotkey
^+m::  ; Ctrl+Shift+M
    SoundSet, +1, MASTER, MUTE, 1  ; Toggle microphone mute
return
```

### Alternatywa - Discord/Teams/Zoom
Większość aplikacji komunikacyjnych pozwala ustawić custom skrót dla mute:
- **Discord**: User Settings → Keybinds → Toggle Mute
- **Teams**: Settings → Privacy → Keyboard shortcuts
- **Zoom**: Settings → Keyboard Shortcuts → Mute/Unmute My Audio

## 🔋 Zarządzanie Energią

Seeed Xiao BLE ma wbudowany układ ładowania baterii Li-Po:
- Ładuj przez USB-C podczas użytkowania
- Napięcie baterii: 3.7V nominal (3.0V - 4.2V)
- Prąd spoczynkowy: ~5mA (Bluetooth connected)
- Szacowany czas pracy: 200-500h (zależy od pojemności baterii)

## 🐛 Troubleshooting

### Urządzenie nie łączy się
1. Upewnij się, że Bluetooth jest włączony na komputerze
2. Sprawdź czy urządzenie jest sparowane (Settings → Bluetooth)
3. Usuń urządzenie i sparuj ponownie
4. Przeładuj kod z włączonym Serial Monitor - sprawdź komunikaty

### Przycisk nie działa
1. Sprawdź połączenia (D1 i GND)
2. Upewnij się, że urządzenie jest połączone (LED świeci)
3. Sprawdź Serial Monitor - powinien pokazywać "Mute toggled!"

### LED nie świeci
- LED świeci tylko gdy urządzenie jest połączone przez Bluetooth
- Sprawdź zasilanie baterii

### Reset nie działa
- Upewnij się, że przytrzymujesz przycisk **przed** włączeniem przełącznika
- LED powinien migać 5 razy jako potwierdzenie

## 📝 Modyfikacje

### Zmiana czasu połączenia
```cpp
// W funkcji startAdvertising(), linia ~135:
Bluefruit.Advertising.start(30); // Zmień 30 na inną wartość (sekundy)
```

### Dodanie wibracji/buzzera
```cpp
#define BUZZER_PIN 2

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void toggleMute() {
  blehid.keyboardReport(MUTE_MODIFIERS, MUTE_KEY);
  
  // Krótki beep
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
  
  delay(10);
  blehid.keyRelease();
}
```

## 📄 Licencja

MIT License - projekt otwarty, modyfikuj według potrzeb!

## 🤝 Contributing

Sugestie i pull requesty mile widziane!

---

**Autor**: kmieciu2003  
**Data**: 2025-12-21  
**Wersja**: 1.0
