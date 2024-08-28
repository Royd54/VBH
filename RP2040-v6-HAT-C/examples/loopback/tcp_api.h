#ifndef TCP_API_H
#define TCP_API_H

#include <stdint.h>
#include <stdbool.h>
#include "cJSON.h"

#define NUM_BUTTONS 18
#define NUM_COMMANDS 17
#define DATA_BUF_SIZE 2048 // Max size of the data buffer
#define NAME_SIZE 50

extern int32_t init_api_socket(uint8_t sn, uint8_t *buf, uint16_t port);
extern void api_socket_behaviour(uint8_t *buf, datasize_t len, char *item);
extern void fade_button_group(cJSON *object);
extern void init_button_settings();
extern void getButtonState(char command[], int uart_index);
void api_command(const char *command, cJSON *object);
void handle_unknown_command();
void button_fade_command(cJSON *object);
void fade_gui(cJSON *object);
void fade_menu(cJSON *object);
void press_button(cJSON *object);
void set_button_brightness(cJSON *object);
void set_button_brightness_group(cJSON *object);
void system_restart(cJSON *object);
void disconnect_socket(cJSON *object);
void calibrate_buttons(cJSON *object);
void set_button_color(cJSON *object);
void set_button_color_group(cJSON *object);
void play_sound(cJSON *object);
void set_haptic_intensity(cJSON *object);
void update_system_ip(cJSON *object);
void update_system_baudrate(cJSON *object);
void update_socket_inactive_timer(cJSON *object);
void set_button_function(cJSON *object);
void updateButtonStates(int buttons[], bool state, int buttonAmount);
void setButtonActivity();
void playSoundEffect(int sound, int volume);

// Define the state of each button
extern bool buttonState[NUM_BUTTONS];
// Button release commands
extern char *buttonReleaseCommands[];
// Button press commands  
extern char *buttonPressCommands[];
// Assigned button functions (index of array stands for button number. So buttonBehaviour[3] = button 3)
extern char *buttonBehaviour[];

// Struct for applying settings/functions to the buttons.
typedef struct {
    char setting[20];
} ButtonSettings;
extern ButtonSettings buttonSettings[NUM_BUTTONS]; // Array index (+1) corresponds with physical button index (button [0] is physical 1)

// Structure to represent a command and its handler function
typedef struct {
    const char *command_name;
    void (*handler)();
} CommandHandler;

// Array of command handlers
extern CommandHandler command_handlers[];

typedef struct {
    char name[NAME_SIZE];
    char panelType[NAME_SIZE];
    int sfx_volume;
    int max_brightness;
    int time_out;
    int radar_enable_delay;
    int number_of_knocks;
    uint16_t radar_sensor;
    uint16_t sfx_actuator;
    uint16_t knock_sensor;
    uint16_t module_A;
    uint16_t module_B;
    uint16_t module_C;
} PanelSettings;

extern PanelSettings* retrieved;

typedef struct {
    char pageName[NAME_SIZE];
    int logo;
    int power;
    int service;
    int tv_on_off;
    int hvac;
    int shutters;
    int audio;
    int lights;
    int up;
    int down;
    int play;
    int pause;
    int forward;
    int back;
    int day;
    int sunset;
    int evening;
    int night;
    int seventeenC;
    int eightteenC;
    int nineteenC;
    int twentyC;
    int twentyoneC;
    int twentytwoC;
    int twentythreeC;
    int twentyfourC;
} GuiSettingsPage;

typedef struct {
    char panelType[NAME_SIZE];
    GuiSettingsPage** pages;
    int pageCount;
    int pageCapacity;  // To keep track of allocated capacity
} GuiSettings;

GuiSettings* createGuiSettings(const char* panelType);
GuiSettingsPage* createGuiSettingsPage(cJSON *object);
void addGuiSettingsPage(GuiSettings* settings, GuiSettingsPage* page);
GuiSettingsPage* getGuiSettingsPage(GuiSettings* settings, const char* pageName);
void freeGuiSettings(GuiSettings* settings);

void printGuiSettingsPages(GuiSettings* settings);

typedef struct Node {
    PanelSettings* settings;
    struct Node* next;
} Node;

extern Node* head;

typedef struct NodeGUI {
    GuiSettings* settings;
    struct NodeGUI* next;
} NodeGUI;

extern NodeGUI* headGUI;

PanelSettings* createPanelSettings(
    const char* name, const char* panelType, int sfx_volume, int max_brightness,
    int time_out, int radar_enable_delay, int number_of_knocks,
    uint16_t radar_sensor, uint16_t sfx_actuator, uint16_t knock_sensor,
    uint16_t module_A, uint16_t module_B, uint16_t module_C
);
void addPanelSettings(Node** head, PanelSettings* settings);
PanelSettings* getPanelSettings(Node* head, const char* name);
void api_socket_behaviour_Settings(uint8_t *buf, size_t len, char *item);
void updatePanelData(cJSON *object);
void updatePanelConfig(cJSON *object);

typedef struct {
    char buttonType[NAME_SIZE];
    int released[2];
    int pressed[2];
    int blink[2];
    int active[2];
    int blink_hi_low_time[2];
    char pressEvent[NAME_SIZE];

    int cleared[2];
    int selected[2];
    int unselected[2];

    int held_time;
    char heldEvent[NAME_SIZE];  // Renamed to indicate this is from PowerButton

    int fade_out[2];
    int fade_in[2];
    int fade_high_low_time[2];
} ButtonProperties;

typedef struct {
    char panelType[NAME_SIZE];
    ButtonProperties** buttonProperties;
    int buttonCount;
    int buttonCapacity;  // To keep track of allocated capacity
} PanelProperties;

typedef struct NodeProperties {
    PanelProperties* settings;
    struct NodeProperties* next;
} NodeProperties;

extern NodeProperties* headProperties;

PanelProperties* createPanelProperties(const char* panelType);
ButtonProperties* createButtonProperties(cJSON *object);
void addButtonProperties(PanelProperties* settings, ButtonProperties* button);
ButtonProperties* getButtonPorperties(PanelProperties* settings, const char* buttonType);
void freePanelProperties(PanelProperties* settings);
void printButtonProperties(PanelProperties* settings);
#endif