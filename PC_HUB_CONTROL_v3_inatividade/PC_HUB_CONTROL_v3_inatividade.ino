/*  
    ---------------PC Shortcut Central Display---------------------------------------------------------------------------------------------------
    Create By Jonathan Rafael - Reservado2312/Arroz23 
    This code uses the lvgl library in conjunction with others to allow control of a computer through a server running in python on the computer.
    First configure your esp32 libraries so that the project works 100% follow the instructions below
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
// Seu endereço MAC 00-11-22-33-44-55, só escrever cada conjundo de numero na sequencia a baixo
byte macAddress[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 }; 
// Porta para Wake-on-LAN
const int wolPort = 9;
WiFiUDP udp;

TFT_eSPI tft = TFT_eSPI();

#define TFT_BL 21  // Pino da retroiluminação (ajuste conforme seu display)
#define TOUCH_CS 33 // Pino Chip Select do Touchscreen
#define TEMPO_DESCANSO 15000  // 15 segundos
unsigned long ultimoToque = 0;
bool telaLigada = true;
static bool isPlaying = false;  // Estado do botão (false = pausado, true = tocando)


// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];


// Linha 200 terá uma função com para determinar o que cada botão fara, usei alguns simbolos da lib LVGL, ex: LV_SYMBOL_PAUSE" Pause", na documentação da lib lvgl tem todos os dispomiveis
// Atenção!! ao definir os nomes dos botões , Favor manter o mesmo nome escrito do mesmo jeito na função que determina o que é feito por cada bota~
// Mapas de botões para cada aba (9 botões por aba), Fique  A vontade Para Alterar
static const char *btnm_map_tab1[] = {"Chrome", "GitHub", "ChatGPT", "\n",
                                      "Discord", "Steam", "Xbox", "\n",
                                      "Youtube", "TikTok", "Whatsapp", ""};

static const char *btnm_map_tab2[] = {"ArdIDE", "VScode", "WAMP", "\n",
                                      LV_SYMBOL_SETTINGS" Config", "Calc", "NotePad", "\n",                                      
                                      LV_SYMBOL_POWER" PwOn", "Lock", LV_SYMBOL_POWER" PwOff", ""};                                    

static const char *btnm_map_tab3[] = {"YTMusic", "AppleMusic", "Groove", "\n",
                                      LV_SYMBOL_PREV" Prev", LV_SYMBOL_PLAY" Play/Pause", "Next "LV_SYMBOL_NEXT, "\n",
                                      LV_SYMBOL_VOLUME_MID" Vol-", LV_SYMBOL_AUDIO" mute", LV_SYMBOL_VOLUME_MAX" Vol+", ""};

static const char *btnm_map_tab4[] = {"Docs", "Fotos", "Musicas", "\n",
                                      "Python", "PHP", "Arduino", "\n",
                                      "Downloads", "GDrive", "Videos", ""};                                      

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(ts.tirqTouched() && ts.touched()) {
    // Get Touchscreen points
    TS_Point p = ts.getPoint();
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
    // Essa função verifica qual botão foi precionado pelo texto que esta escrito no botão, você pode altera-lo na linha 67
    // Só informar o caminho para a função sendCommand que envia para o servidor qual atalho que executar sendCommand("/open/chrome");  
    // Lembre-se de mater um padrão de nomes no código Python do servidor
    if (strcmp(txt, "Chrome") == 0) 
    {
      Serial.println("Abrindo Chrome...");
      sendCommand("/open/chrome");  
    }
    else if (strcmp(txt, "GitHub") == 0) 
    {
      Serial.println("Abrindo GitHub...");
      sendCommand("/open/github");   
    }
    else if (strcmp(txt, "Steam") == 0) 
    {
      Serial.println("Abrindo Steam...");
      sendCommand("/open/steam");   
    }
    else if (strcmp(txt, "Xbox") == 0) 
    {
      Serial.println("Abrindo Xbox...");
      sendCommand("/open/xbox");   
    }
    else if (strcmp(txt, "Whatsapp") == 0) 
    {
      Serial.println("Abrindo Whatsapp...");
      sendCommand("/open/whatsapp");   
    }
    else if (strcmp(txt, "WAMP") == 0) 
    {
      Serial.println("Abrindo WAMP...");
      sendCommand("/open/wamp");   
    }
    else if (strcmp(txt, "VScode") == 0) 
    {
      Serial.println("Abrindo VS Code...");
      sendCommand("/open/vscode");   
    }
    else if (strcmp(txt, "ArdIDE") == 0)
    {
      Serial.println("Abrindo ArduinoIDE...");
      sendCommand("/open/arduide");   
    }
    else if (strcmp(txt, "YTMusic") == 0)
    {
      Serial.println("Abrindo Youtube Music...");
      sendCommand("/open/ytMusic");   
    }
    else if (strcmp(txt, "AppleMusic") == 0) 
    {
      Serial.println("Abrindo Apple Music...");
      sendCommand("/open/apMusic");   
    }    
    else if (strcmp(txt, "Groove") == 0) 
    {
      Serial.println("Abrindo Groove Music...");
      sendCommand("/open/groove");   
    }                                                               
    else if (strcmp(txt, "Docs") == 0) 
    {
      Serial.println("Abrindo Documentos...");
      sendCommand("/open/Documents");   
    }
    else if (strcmp(txt, "Fotos") == 0) 
    {
      Serial.println("Abrindo Fotos...");
      sendCommand("/open/images");   
    }
    else if (strcmp(txt, "Musicas") == 0) 
    {
      Serial.println("Abrindo Musicas Locais...");
      sendCommand("/open/Localmusics");   
    } 
    else if (strcmp(txt, "Python") == 0) 
    {
      Serial.println("Abrindo Projetos Python...");
      sendCommand("/open/python");   
    } 
    else if (strcmp(txt, "PHP") == 0) 
    {
      Serial.println("Abrindo Projetos PHP...");
      sendCommand("/open/php");   
    } 
    else if (strcmp(txt, "Arduino") == 0) 
    {
      Serial.println("Abrindo Projetos Arduino...");
      sendCommand("/open/arduino");   
    } 
    else if (strcmp(txt, "Videos") == 0) 
    {
      Serial.println("Abrindo Pasta Videos...");
      sendCommand("/open/videos");   
    } 
    else if (strcmp(txt, "GDrive") == 0) 
    {
      Serial.println("Abrindo Pasta Google Drive...");
      sendCommand("/open/gDrive");   
    } 
    else if (strcmp(txt, "Downloads") == 0) 
    {
      Serial.println("Abrindo Pasta Downloads...");
      sendCommand("/open/downloads");   
    }        
    else if (strcmp(txt, "ChatGPT") == 0) 
    {
      Serial.println("Abrindo ChatGPT...");
      sendCommand("/open/chatgpt");   
    } 
    else if (strcmp(txt, "Youtube") == 0) 
    {
      Serial.println("Abrindo YouTube...");
      sendCommand("/open/youtube");   
    } 
    else if (strcmp(txt, "Discord") == 0) 
    {
      Serial.println("Abrindo Discord...");
      sendCommand("/open/discord");   
    } 
    else if (strcmp(txt, "TikTok") == 0) 
    {
      Serial.println("Abrindo TikTok...");
      sendCommand("/open/tiktok");               
    } 
    else if (strcmp(txt, LV_SYMBOL_SETTINGS" Config") == 0) {
      Serial.println("Abrindo Configurações...");
      sendCommand("/open/settings");
    } 
    else if (strcmp(txt, "Lock") == 0) 
    {
      Serial.println("Bloqueando PC...");
      sendCommand("/lock");
    } 
    else if (strcmp(txt, LV_SYMBOL_POWER" PwOff") == 0) 
    {
      Serial.println("Desligando PC...");
      sendCommand("/shutdown");   
    } 
    else if (strcmp(txt,LV_SYMBOL_POWER" PwOn") == 0) 
    {
      Serial.println("Ligando PC...");
      sendWOLPacket();
    } 
    else if (strcmp(txt, "Calc") == 0) 
    {
      Serial.println("Abrindo Calculadora...");
      sendCommand("/open/calc");            
    } 
    else if (strcmp(txt, "NotePad") == 0) 
    {
      Serial.println("Abrindo NotePad...");
      sendCommand("/open/notepad");
    } 
    else if (strcmp(txt, LV_SYMBOL_PREV" Prev") == 0) 
    {
      Serial.println("Música anterior...");
      sendCommand("/music/previous");   
    } 
    else if (strcmp(txt, LV_SYMBOL_AUDIO" mute") == 0)
    {
      Serial.println("Mutando Audio...");
      sendCommand("/music/mute");   
    } 
    else if (strcmp(txt, "Next "LV_SYMBOL_NEXT) == 0) 
    {
      Serial.println("Próxima música...");
      sendCommand("/music/next");   
    } 
    else if (strcmp(txt, LV_SYMBOL_VOLUME_MID" Vol-") == 0) 
    {
      Serial.println("Diminuindo volume...");
      sendCommand("/decrease_volume");   
    } 
    else if (strcmp(txt, LV_SYMBOL_VOLUME_MAX" Vol+") == 0) 
    {
      Serial.println("Aumentando volume...");
      sendCommand("/increase_volume");               
    }
      else if (strcmp(txt, LV_SYMBOL_PLAY" Play/Pause") == 0 || strcmp(txt, LV_SYMBOL_PAUSE" Play/Pause") == 0) {
      // Alternar entre Play e Pause
      isPlaying = !isPlaying;

      if (isPlaying) {
        Serial.println("Reproduzindo música...");
        sendCommand("/music/play");
      } else {
        Serial.println("Pausando música...");
        sendCommand("/music/pause");
      }
    }
  }
}

void create_tab_with_buttons(lv_obj_t * parent, const char *btn_map[]) {
    lv_obj_t *btnm = lv_btnmatrix_create(parent);
    lv_btnmatrix_set_map(btnm, btn_map);
    lv_obj_center(btnm);
    lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_CLICKED, NULL);  // Adiciona evento de clique
    // Definir o tamanho para ocupar a tela toda (opcionalmente, você pode ajustar se quiser bordas menores)
    lv_obj_set_size(btnm, 290, 160);
}

void lv_example_tabview_with_buttons(void) {
  // Corrigido: Cria um objeto Tabview com apenas 1 argumento (parent)
  lv_obj_t *tabview = lv_tabview_create(lv_scr_act());

  // Adiciona 3 abas
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Apps");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Atalhos");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Midia");
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "Locais");

  // Ajusta o tamanho total do Tabview (opcional)
  lv_obj_set_size(tabview, LV_HOR_RES, LV_VER_RES - 5); // Ajusta o tamanho para a tela

  // Cria matriz de botões em cada aba com seus respectivos botões
  create_tab_with_buttons(tab1, btnm_map_tab1);
  create_tab_with_buttons(tab2, btnm_map_tab2);
  create_tab_with_buttons(tab3, btnm_map_tab3);
  create_tab_with_buttons(tab4, btnm_map_tab4);
  lv_obj_set_style_pad_top(tab1, 20, LV_PART_MAIN); // Reduz o padding no topo
  lv_obj_set_style_pad_top(tab2, 20, LV_PART_MAIN); // Reduz o padding no topo
  lv_obj_set_style_pad_top(tab3, 20, LV_PART_MAIN); // Reduz o padding no topo
  lv_obj_set_style_pad_top(tab4, 20, LV_PART_MAIN); // Reduz o padding no topo
  lv_obj_set_style_pad_top(tabview, 0, LV_PART_MAIN); // Reduz o padding no topo
  lv_obj_set_style_text_font(tabview, &lv_font_montserrat_12, LV_PART_MAIN); // Reduz a fonte dos nomes das abas
}


void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // Liga a retroiluminação
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
  
  tft.begin();
  tft.setRotation(2);

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  ts.setRotation(2);

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
  if (ts.touched()) { // Se houver toque na tela
    ultimoToque = millis(); // Atualiza o tempo do último toque
    if (!telaLigada) {
        digitalWrite(TFT_BL, HIGH); // Religa a tela
        telaLigada = true;
    }
  }

  // Verifica se passaram 15 segundos sem toque
  if (millis() - ultimoToque > TEMPO_DESCANSO && telaLigada) {
      digitalWrite(TFT_BL, LOW); // Desliga a retroiluminação
      telaLigada = false;
  }

}