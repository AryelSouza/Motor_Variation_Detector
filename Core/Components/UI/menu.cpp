#include "menu.h"
#include "st7789.h"

extern uint8_t g_start_sampling;
extern uint8_t g_run_inference; // NOVO: Flag para ativar a IA
extern volatile uint8_t g_motor_state;

static MenuState_t current_state = STATE_MAIN_MENU;
static int menu_index = 0;

void Menu_Init(void) {
    ST7789_Fill_Color(BLUE);
    ST7789_WriteString(10, 10, "PREDICTIVE MAIN", Font_11x18, WHITE, BLUE);
    ST7789_WriteString(10, 50, (menu_index == 0) ? "> 1. Coletar Dados" : "  1. Coletar Dados", Font_7x10, (menu_index == 0) ? GREEN : WHITE, BLUE);
    ST7789_WriteString(10, 75, (menu_index == 1) ? "> 2. Calibrar" : "  2. Calibrar", Font_7x10, (menu_index == 1) ? GREEN : WHITE, BLUE);
    ST7789_WriteString(10, 100, (menu_index == 2) ? "> 3. Monitorar IA" : "  3. Monitorar IA", Font_7x10, (menu_index == 2) ? GREEN : WHITE, BLUE);
}

void Menu_ProcessButton(unsigned short pin) {
    if (current_state == STATE_MAIN_MENU) {
        if (pin == GPIO_PIN_3) { // K1 Desce no menu
            menu_index++;
            if(menu_index > 2) menu_index = 0;
            Menu_Init();
        } else if (pin == GPIO_PIN_0) { // K_UP Confirma
            if (menu_index == 0) current_state = STATE_DATA_COLLECTION;
            else if (menu_index == 1) current_state = STATE_CALIBRATION;
            else if (menu_index == 2) current_state = STATE_MONITORING;

            ST7789_Fill_Color(BLACK);

            if (current_state == STATE_DATA_COLLECTION) {
                ST7789_WriteString(10, 10, "COLETANDO...", Font_11x18, YELLOW, BLACK);
                g_start_sampling = 1;
            } else if (current_state == STATE_CALIBRATION) {
                ST7789_WriteString(10, 10, "CALIBRANDO", Font_11x18, YELLOW, BLACK);
            } else if (current_state == STATE_MONITORING) {
                ST7789_WriteString(10, 10, "MONITOR DE IA", Font_11x18, MAGENTA, BLACK);
                ST7789_WriteString(10, 130, "Lendo Sensores...", Font_7x10, WHITE, BLACK);
                g_run_inference = 1; // Liga a IA
            }
        }
    } else {
        if (pin == GPIO_PIN_4) { // K0 volta
            g_start_sampling = 0;
            g_run_inference = 0;
            g_motor_state = 0;
            current_state = STATE_MAIN_MENU;
            Menu_Init();
        } else if (current_state == STATE_DATA_COLLECTION && pin == GPIO_PIN_3) {
            g_motor_state++;
            if (g_motor_state == 6){
                g_motor_state = 0;
            }
        }
    }
}
