#ifndef UI_MENU_H_
#define UI_MENU_H_

typedef enum {
    STATE_MAIN_MENU,
    STATE_DATA_COLLECTION,
    STATE_CALIBRATION,
	STATE_MONITORING
} MenuState_t;

void Menu_Init(void);
void Menu_Update(void);
void Menu_ProcessButton(unsigned short pin); // Usando primitivo ANSI C Puro

#endif /* UI_MENU_H_ */
