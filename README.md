# M5Stack BugC2 Advanced Web Controller

Ten projekt to zaawansowany sterownik dla bazy robotycznej **M5Stack BugC2** współpracującej z modułem **M5StickC / Plus / Plus2**. Rozwiązuje on powszechne problemy z komunikacją I2C oraz błędną polaryzacją silników, oferując płynne i intuicyjne sterowanie przez przeglądarkę.

## 🚀 Kluczowe funkcje
- **Precyzyjny Mikser Ruchu (Skid-Steer):** Płynne przechodzenie między jazdą prosto, łukami a obrotami.
- **Poprawna Obsługa I2C:** Wykorzystanie typów `signed char` (int8_t) zapewnia pełną kontrolę nad jazdą do tyłu (rozwiązanie problemu braku wstecznego).
- **Horyzontalny Interfejs Web:** Responsywny joystick do jazdy oraz dedykowane przyciski o obniżonej mocy do precyzyjnych obrotów w miejscu.
- **Telemetria na Ekranie:** M5StickC wyświetla dane sieci WiFi (SSID, Hasło) oraz adres IP serwera sterowania zaraz po starcie.
- **Tryb Diagnostyczny:** Dołączony skrypt do statycznego testowania silników bez użycia sieci.

## 🛠 Konfiguracja Sprzętowa
- **Urządzenie:** M5StickC / Plus / Plus2
- **Baza:** BugC2 (Adres I2C: `0x38`)
- **Piny I2C:** SDA: `0`, SCL: `26`
- **Rejestry Silników:** - `0x00`: Przedni Lewy
  - `0x01`: Przedni Prawy
  - `0x02`: Tylny Lewy
  - `0x03`: Tylny Prawy

## 📦 Instalacja
1. Sklonuj repozytorium na swój dysk.
2. Użyj **Thonny IDE** lub **M5Burner**, aby wgrać firmware MicroPython na swoje urządzenie.
3. Prześlij pliki `.py` oraz folder `static` (jeśli istnieje) do pamięci M5StickC.
4. Zresetuj urządzenie – dane do połączenia pojawią się na wyświetlaczu LCD.

## ⚠️ Rozwiązywanie problemów (Troubleshooting)
Jeśli robot kręci się w kółko przy jeździe do przodu:
- Sprawdź sekcję `config` w pliku głównym. Niektóre partie produkcyjne BugC2 mają odwróconą polaryzację silników. Wystarczy zmienić mnożnik kierunku dla konkretnego silnika z `1` na `-1`.

## 📄 Licencja
MIT License - czuj się swobodnie w modyfikowaniu projektu pod własne potrzeby!

# M5Stack BugC2 Advanced Web Controller

This project is an advanced driver for the **M5Stack BugC2** robotic base, designed to work with the **M5StickC / Plus / Plus2** modules. It solves common I2C communication issues and motor polarity misalignments, offering smooth and intuitive control via a web browser.

## 🚀 Key Features
- **Precision Motion Mixer (Skid-Steer):** Seamless transitions between driving straight, turning in arcs, and spinning in place.
- **Correct I2C Handling:** Utilizes `signed char` (int8_t) types to ensure full control over reverse movement (fixing the "no-reverse" bug).
- **Horizontal Web Interface:** Responsive joystick for driving and dedicated low-power buttons for precise 360° spins.
- **On-Screen Telemetry:** The M5StickC LCD displays WiFi network data (SSID, Password) and the Web Server IP address immediately upon startup.
- **Diagnostic Mode:** Includes a standalone script for static motor testing without the need for a network connection.

## 🛠 Hardware Configuration
- **Device:** M5StickC / Plus / Plus2
- **Base:** BugC2 (I2C Address: `0x38`)
- **I2C Pins:** SDA: `0`, SCL: `26`
- **Motor Registers:** - `0x00`: Front Left
  - `0x01`: Front Right
  - `0x02`: Rear Left
  - `0x03`: Rear Right

## 📦 Installation
1. Clone the repository to your local machine.
2. Use **Thonny IDE** or **M5Burner** to flash the MicroPython firmware onto your device.
3. Upload the `.py` files and the `static` folder (if applicable) to the M5StickC memory.
4. Restart the device – the connection details will appear on the LCD screen.

## ⚠️ Troubleshooting
If the robot spins in circles while driving forward:
- Check the `config` section in the main file. Some BugC2 production batches have inverted motor polarity. Simply change the direction multiplier for the specific motor from `1` to `-1`.

## 📄 License
MIT License - feel free to modify this project for your own needs!
