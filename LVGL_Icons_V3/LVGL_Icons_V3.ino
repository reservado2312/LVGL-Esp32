#include <lvgl.h>
#include <TFT_eSPI.h>
#include "src/image.h"
#include <XPT2046_Touchscreen.h>

// Definição dos pinos da touchscreen
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 280
#define SCREEN_HEIGHT 320

//icons
//https://fonts.google.com/icons?selected=Material+Icons:lightbulb_outline:&icon.query=bulb&icon.size=2&icon.color=%23DFBAB1
//convert icons to lvgl lib
//https://lvgl.io/tools/imageconverter
// Tamanho dos ícones
int icon_width = 64;
int icon_height = 64;
int spacing = 12;  // Espaçamento entre os ícones

// Coordenadas X para os ícones
int icon_prev_x = 50;
int icon_play_x = icon_prev_x + icon_width + spacing;
int icon_next_x = icon_play_x + icon_width + spacing;

// Coordenada Y para todos os ícones
int icon_y = 10;

// Variável de controle para alternar entre Play e Pause
bool isPlaying = false;

// Buffer de desenho
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Objetos LVGL
lv_obj_t* img_play_pause = NULL;

// Função de log para debugging
void log_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Função para ler a touchscreen
void touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    int z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;

    // Verifica se o toque está na área do ícone prev
    if (x > icon_prev_x && x < icon_prev_x + icon_width && y > icon_y && y < icon_y + icon_height) {
      Serial.println("prev");
    }
    // Verifica se o toque está na área do ícone play/pause
    else if (x > icon_play_x && x < icon_play_x + icon_width && y > icon_y && y < icon_y + icon_height) {
      togglePlayPause();
      delay(10);     
    }
    // Verifica se o toque está na área do ícone next
    else if (x > icon_next_x && x < icon_next_x + icon_width && y > icon_y && y < icon_y + icon_height) {
      Serial.println("next");
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// Função para alternar entre Play e Pause
void togglePlayPause() {
  if (isPlaying) {
    Serial.println("Pausing...");
    isPlaying = false;
    draw_play();  // Desenha o ícone de Play
  } else {
    Serial.println("Playing...");
    isPlaying = true;
    draw_pause(); // Desenha o ícone de Pause
  }
}

// Função para desenhar o ícone de Play
void draw_play(void) {
  // Remove o ícone anterior (se existir)
  if (img_play_pause != NULL) {
    lv_obj_del(img_play_pause);
  }
  
  LV_IMAGE_DECLARE(btn_play_ico);
  img_play_pause = lv_image_create(lv_scr_act());
  lv_image_set_src(img_play_pause, &btn_play_ico);
  lv_obj_align(img_play_pause, LV_ALIGN_TOP_LEFT, icon_play_x, icon_y);
}

// Função para desenhar o ícone de Pause
void draw_pause(void) {
  // Remove o ícone anterior (se existir)
  if (img_play_pause != NULL) {
    lv_obj_del(img_play_pause);
  }

  LV_IMAGE_DECLARE(btn_pause_ico);
  img_play_pause = lv_image_create(lv_scr_act());
  lv_image_set_src(img_play_pause, &btn_pause_ico);
  lv_obj_align(img_play_pause, LV_ALIGN_TOP_LEFT, icon_play_x, icon_y);
}

// Função para desenhar o ícone anterior
void draw_prev(void) {
  LV_IMAGE_DECLARE(btn_prev_ico);
  lv_obj_t* img_prev = lv_image_create(lv_scr_act());
  lv_image_set_src(img_prev, &btn_prev_ico);
  lv_obj_align(img_prev, LV_ALIGN_TOP_LEFT, icon_prev_x, icon_y);
}

// Função para desenhar o ícone próximo
void draw_next(void) {
  LV_IMAGE_DECLARE(btn_next_ico);
  lv_obj_t* img_next = lv_image_create(lv_scr_act());
  lv_image_set_src(img_next, &btn_next_ico);
  lv_obj_align(img_next, LV_ALIGN_TOP_LEFT, icon_next_x, icon_y);
}

void setup() {
  Serial.begin(115200);
  Serial.println("LVGL Library Version: " + String(lv_version_major()) + "." + lv_version_minor() + "." + lv_version_patch());

  // Inicializa o LVGL
  lv_init();
  lv_log_register_print_cb(log_print);

  // Inicializa a touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(3);

  // Inicializa o display TFT
  lv_display_t* disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  // Inicializa o dispositivo de entrada (touchscreen)
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Desenha os ícones
  draw_prev();
  draw_play();  // Inicialmente, o ícone será o de Play
  draw_next();
}

void loop() {
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);
}
