/**
 * driving_mode.h — Driving Mode Definitions
 * 
 * Definiuje tryby jazdy robota BugC2:
 * - MANUAL: pełna kontrola przez użytkownika
 * - SEMI_AUTO: użytkownik steruje, ale auto-hamowanie przy kolizji
 * - AUTONOMOUS: pełna autonomia z omijaniem przeszkód
 */

#ifndef DRIVING_MODE_H
#define DRIVING_MODE_H

enum DrivingMode {
    MODE_MANUAL = 0,
    MODE_SEMI_AUTO = 1,
    MODE_AUTONOMOUS = 2,
    MODE_COUNT = 3          // Ilość trybów (do cyklicznego przełączania)
};

/**
 * Zwraca nazwę trybu jako string (do wyświetlania na LCD / Serial).
 */
inline const char* modeToString(DrivingMode mode) {
    switch (mode) {
        case MODE_MANUAL:     return "MANUAL";
        case MODE_SEMI_AUTO:  return "SEMI-AUTO";
        case MODE_AUTONOMOUS: return "AUTONOMOUS";
        default:              return "UNKNOWN";
    }
}

/**
 * Zwraca kolor trybu jako wartość 16-bit (RGB565) do wyświetlania na LCD.
 */
inline uint16_t modeToColor(DrivingMode mode) {
    switch (mode) {
        case MODE_MANUAL:     return 0x001F;  // Blue (TFT_BLUE)
        case MODE_SEMI_AUTO:  return 0xFFE0;  // Yellow (TFT_YELLOW)
        case MODE_AUTONOMOUS: return 0x07E0;  // Green (TFT_GREEN)
        default:              return 0xFFFF;  // White
    }
}

/**
 * Przełącz na następny tryb (cyklicznie).
 */
inline DrivingMode nextMode(DrivingMode current) {
    return (DrivingMode)((current + 1) % MODE_COUNT);
}

#endif // DRIVING_MODE_H
