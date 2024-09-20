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

// Tamanho dos ícones
int icon_width = 64;
int icon_height = 64;
int spacing = 12;  // Espaçamento entre os ícones

// Coordenadas X para os ícones
int icon_prev_x = 12;
int icon_play_x = icon_prev_x + icon_width + spacing;
int icon_pause_x = icon_play_x + icon_width + spacing;
int icon_next_x = icon_pause_x + icon_width + spacing;

// Coordenada Y para todos os ícones
int icon_y = 10;

// Buffer de desenho
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

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
    // Verifica se o toque está na área do ícone play
    else if (x > icon_play_x && x < icon_play_x + icon_width && y > icon_y && y < icon_y + icon_height) {
      Serial.println("play");
    }
    // Verifica se o toque está na área do ícone pause
    else if (x > icon_pause_x && x < icon_pause_x + icon_width && y > icon_y && y < icon_y + icon_height) {
      Serial.println("pause");
    }
    // Verifica se o toque está na área do ícone next
    else if (x > icon_next_x && x < icon_next_x + icon_width && y > icon_y && y < icon_y + icon_height) {
      Serial.println("next");
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}



// Funções para desenhar os ícones
void draw_prev(void) {
  LV_IMAGE_DECLARE(btn_prev_ico);
  lv_obj_t* img1 = lv_image_create(lv_scr_act());
  lv_image_set_src(img1, &btn_prev_ico);  
  //lv_obj_set_pos(img1, icon_prev_x, icon_y);
  lv_obj_align(img1, LV_ALIGN_TOP_LEFT, icon_prev_x, icon_y);
}

void draw_play(void) {
  LV_IMAGE_DECLARE(btn_play_ico);
  lv_obj_t* img2 = lv_image_create(lv_scr_act());
  lv_image_set_src(img2, &btn_play_ico);
  //lv_obj_set_pos(img2, icon_play_x, icon_y);
  lv_obj_align(img2, LV_ALIGN_TOP_LEFT, icon_play_x, icon_y);
}

void draw_pause(void) {
  LV_IMAGE_DECLARE(btn_pause_ico);
  lv_obj_t* img3 = lv_image_create(lv_scr_act());
  lv_image_set_src(img3, &btn_pause_ico);
  //lv_obj_set_pos(img3, icon_pause_x, icon_y);
  lv_obj_align(img3, LV_ALIGN_TOP_LEFT, icon_pause_x, icon_y);
}

void draw_next(void) {
  LV_IMAGE_DECLARE(btn_next_ico);
  lv_obj_t* img4 = lv_image_create(lv_scr_act());
  lv_image_set_src(img4, &btn_next_ico);
  //lv_obj_set_pos(img4, icon_next_x, icon_y);
  lv_obj_align(img4, LV_ALIGN_TOP_LEFT, icon_next_x, icon_y);
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
  draw_play();
  draw_pause();
  draw_next();
}

void loop() {
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);
}
