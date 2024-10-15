/*  Rui Santos & Sara Santos - Random Nerd Tutorials
    THIS EXAMPLE WAS TESTED WITH THE FOLLOWING HARDWARE:
    1) ESP32-2432S028R 2.8 inch 240×320 also known as the Cheap Yellow Display (CYD): https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/cyd-lvgl/
    2) REGULAR ESP32 Dev Board + 2.8 inch 240x320 TFT Display: https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/ and https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/esp32-tft-lvgl/
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

/*  Install the "lvgl" library version 9.2 by kisvegabor to interface with the TFT Display - https://lvgl.io/
    *** IMPORTANT: lv_conf.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE lv_conf.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <lvgl.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <TFT_eSPI.h>

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen - Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Configurações Wi-Fi
const char* ssid = "Rede Casa";      // Substituir pelo nome da sua rede
const char* password = "gerlianexavier23"; // Substituir pela senha da rede

// IP e porta do servidor Flask (ajuste conforme necessário)
const char* serverIP = "192.168.1.29";  // IP do computador que roda o Flask
const int serverPort = 5100;

// Endereço MAC do computador para Wake-on-LAN
byte macAddress[] = { 0x08, 0x97, 0x98, 0x62, 0x32, 0x48 }; // Exemplo de MAC address
// Porta para Wake-on-LAN
const int wolPort = 9;
WiFiUDP udp;

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];


// Mapas de botões para cada aba (6 botões por aba)
static const char *btnm_map_tab1[] = {"Chrome", "GitHub", "ChatGPT", "\n",
                                      "Youtube", "Gmail", "TikTok", ""};

static const char *btnm_map_tab2[] = {LV_SYMBOL_SETTINGS" Config", "Lock", LV_SYMBOL_POWER" PwOff", "\n",
                                      LV_SYMBOL_POWER" PwOn", "Projets", "NotePad", ""};

static const char *btnm_map_tab3[] = {LV_SYMBOL_PREV" Prev", LV_SYMBOL_PLAY" Play", "Next "LV_SYMBOL_NEXT, "\n",
                                      LV_SYMBOL_VOLUME_MID" Vol-", LV_SYMBOL_PAUSE" Pause", LV_SYMBOL_VOLUME_MAX" Vol+", ""};

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;

    // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
    /* Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Pressure = ");
    Serial.print(z);
    Serial.println();*/
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}


// Função para enviar comandos ao servidor Flask
void sendCommand(String command) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + String(serverIP) + ":" + String(serverPort) + command;
    http.begin(url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Resposta do servidor: " + response);
    } else {
      Serial.println("Erro na requisição: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("Wi-Fi desconectado");
  }
}


// Função para enviar o pacote WOL
void sendWOLPacket() 
{
  byte packet[102];  // O pacote mágico tem 102 bytes
  
  // Preenche os primeiros 6 bytes com 0xFF
  for (int i = 0; i < 6; i++) {
    packet[i] = 0xFF;
  }
  
  // Preenche os próximos 16 blocos de 6 bytes com o endereço MAC do computador
  for (int i = 1; i <= 16; i++) {
    for (int j = 0; j < 6; j++) {
      packet[i * 6 + j] = macAddress[j];
    }
  }

  // Calcula o endereço de broadcast e envia o pacote WOL
  IPAddress broadcastIP = getBroadcastAddress();
  udp.beginPacket(broadcastIP, wolPort);  // Enviar para o endereço de broadcast
  udp.write(packet, sizeof(packet));
  udp.endPacket();

  Serial.print("Pacote WOL enviado para o endereço de broadcast: ");
  Serial.println(broadcastIP);
}

// Função para calcular o endereço de broadcast
IPAddress getBroadcastAddress() {
  IPAddress ip = WiFi.localIP();
  IPAddress subnet = WiFi.subnetMask();
  IPAddress broadcast;

  for (int i = 0; i < 4; i++) {
    broadcast[i] = ip[i] | ~subnet[i];
  }
  
  return broadcast;
}


// Evento do botão
void btnm_event_handler(lv_event_t * e) {
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);  // Corrige o cast de void* para lv_obj_t*
    uint16_t btn_id = lv_btnmatrix_get_selected_btn(obj);  // Obtém o ID do botão

    if (btn_id != LV_BTNMATRIX_BTN_NONE) {
        const char *txt = lv_btnmatrix_get_btn_text(obj, btn_id);  // Texto do botão
        //Debug qual botão foi precionado
        //Serial.print("Botão pressionado: ");
        //Serial.println(txt);

        // Comandos separados para cada botão
        if (strcmp(txt, "Chrome") == 0) {
            // Comando para o botão "Chrome"
            Serial.println("Abrindo Chrome...");
            // Adicione seu código aqui
            sendCommand("/open/chrome");  
        } else if (strcmp(txt, "GitHub") == 0) {
            // Comando para o botão "GitHub"
            Serial.println("Abrindo GitHub...");
            // Adicione seu código aqui
            sendCommand("/open/github");   
        } else if (strcmp(txt, "ChatGPT") == 0) {
            // Comando para o botão "ChatGPT"
            Serial.println("Abrindo ChatGPT...");
            // Adicione seu código aqui
            sendCommand("/open/chatgpt");   
        } else if (strcmp(txt, "Youtube") == 0) {
            // Comando para o botão "Youtube"
            Serial.println("Abrindo YouTube...");
            // Adicione seu código aqui
            sendCommand("/open/youtube");   
        } else if (strcmp(txt, "Gmail") == 0) {
            // Comando para o botão "Gmail"
            Serial.println("Abrindo Gmail...");
            // Adicione seu código aqui
            sendCommand("/open/gmail");   
        } else if (strcmp(txt, "TikTok") == 0) {
            // Comando para o botão "TikTok"
            Serial.println("Abrindo TikTok...");
            // Adicione seu código aqui
            sendCommand("/open/tiktok");               
        } else if (strcmp(txt, LV_SYMBOL_SETTINGS" Config") == 0) {
            // Comando para o botão "Settings"
            Serial.println("Abrindo Configurações...");
            // Adicione seu código aqui
            sendCommand("/open/settings");
        } else if (strcmp(txt, "Lock") == 0) {
            // Comando para o botão "Lock"
            Serial.println("Trancando...");
            // Adicione seu código aqui
            sendCommand("/lock");
        } else if (strcmp(txt, LV_SYMBOL_POWER" PwOff") == 0) {
            // Comando para o botão "Power Off"
            Serial.println("Desligando...");
            // Adicione seu código aqui
            sendCommand("/shutdown");   
        } else if (strcmp(txt,LV_SYMBOL_POWER" PwOn") == 0) {
            // Comando para o botão "Power On"
            Serial.println("Ligando...");
            // Adicione seu código aqui
            sendWOLPacket();
        } else if (strcmp(txt, "Projets") == 0) {
            // Comando para o botão "Projets"
            Serial.println("Abrindo Projetos...");
            // Adicione seu código aqui
            sendCommand("/open/projets");            
        } else if (strcmp(txt, "NotePad") == 0) {
            // Comando para o botão "Downloads"
            Serial.println("Abrindo NotePad...");
            // Adicione seu código aqui
            sendCommand("/open/notepad");
        } else if (strcmp(txt, LV_SYMBOL_PREV" Prev") == 0) {
            // Comando para o botão "Prev"
            Serial.println("Música anterior...");
            // Adicione seu código aqui
            sendCommand("/music/previous");   
        } else if (strcmp(txt, LV_SYMBOL_PLAY" Play") == 0) {
            // Comando para o botão "Play"
            Serial.println("Reproduzindo música...");
            // Adicione seu código aqui
            sendCommand("/music/play");   
        } else if (strcmp(txt, "Next "LV_SYMBOL_NEXT) == 0) {
            // Comando para o botão "Next"
            Serial.println("Próxima música...");
            // Adicione seu código aqui
            sendCommand("/music/next");   
        } else if (strcmp(txt, LV_SYMBOL_VOLUME_MID" Vol-") == 0) {
            // Comando para o botão "Vol-"
            Serial.println("Diminuindo volume...");
            // Adicione seu código aqui
            sendCommand("/decrease_volume");   
        } else if (strcmp(txt, LV_SYMBOL_PAUSE" Pause") == 0) {
            // Comando para o botão "Pause"
            Serial.println("Pausando música...");
            // Adicione seu código aqui
            sendCommand("/music/play");   
        } else if (strcmp(txt, LV_SYMBOL_VOLUME_MAX" Vol+") == 0) {
            // Comando para o botão "Vol+"
            Serial.println("Aumentando volume...");
            // Adicione seu código aqui
            sendCommand("/increase_volume");               
        }
    }
}

void create_tab_with_buttons(lv_obj_t * parent, const char *btn_map[]) {
    lv_obj_t *btnm = lv_btnmatrix_create(parent);
    lv_btnmatrix_set_map(btnm, btn_map);
    lv_obj_center(btnm);
    lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_CLICKED, NULL);  // Adiciona evento de clique
    // Definir o tamanho para ocupar a tela toda (opcionalmente, você pode ajustar se quiser bordas menores)
    lv_obj_set_size(btnm, 290, 140);
}

void lv_example_tabview_with_buttons(void) {
    // Corrigido: Cria um objeto Tabview com apenas 1 argumento (parent)
    lv_obj_t *tabview = lv_tabview_create(lv_scr_act());

    // Adiciona 3 abas
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Apps");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Atalhos");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Midia");

    // Cria matriz de botões em cada aba com seus respectivos botões
    create_tab_with_buttons(tab1, btnm_map_tab1);
    create_tab_with_buttons(tab2, btnm_map_tab2);
    create_tab_with_buttons(tab3, btnm_map_tab3);


}


void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.begin(115200);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");

  Serial.println(LVGL_Arduino);
  
  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Function to draw the GUI (text, buttons and sliders)
  lv_example_tabview_with_buttons();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}