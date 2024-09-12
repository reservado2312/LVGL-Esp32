#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

unsigned long previousMillis = 0;   // Armazena o último tempo em que o código foi executado
const long interval = 1000;         // Intervalo de 1 segundo


int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}


// Variáveis para labels de data e hora
static lv_obj_t * time_label;
static lv_obj_t * date_label;


void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static void event_handler_btnPrev(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    Serial.println("Prev");
  }
}

static void event_handler_btnNext(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    Serial.println("Next");
  }
}

//static void event_handler_btnMusicStatus(lv_event_t * e) {
//  lv_event_code_t code = lv_event_get_code(e);
//  lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
//  if(code == LV_EVENT_VALUE_CHANGED) {
//    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
//      Serial.println("Pause");
//    } else {
//      Serial.println("Play");
//    }
//  }
//}

// Callback que é acionado quando btnMusicStatus é clicado/toggled
static void event_handler_btnMusicStatus(lv_event_t * e) 
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
  lv_obj_t * btn_label_MusicStatus = lv_obj_get_child(obj, NULL); // Obtém o label do botão

  if(code == LV_EVENT_VALUE_CHANGED) 
  {
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
      lv_label_set_text(btn_label_MusicStatus, "PAUSE"); // Altera para "PAUSE" quando o botão estiver marcado
      //LV_LOG_USER("Toggled Play -> Pause");
      Serial.println("Pause");
    } else {
      lv_label_set_text(btn_label_MusicStatus, "PLAY");  // Altera para "PLAY" quando o botão estiver desmarcado
      //LV_LOG_USER("Toggled Pause -> Play");
      Serial.println("Play");
    }
  }
}

static lv_obj_t * slider_label;
static void slider_event_callback(lv_event_t * e) {
  lv_obj_t * slider = (lv_obj_t*) lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  LV_LOG_USER("Slider changed to %d%%", (int)lv_slider_get_value(slider));
}

// Atualiza a hora e a data no display
void update_time_and_date(const char* time_str, const char* date_str) {
  lv_label_set_text(time_label, time_str);
  lv_label_set_text(date_label, date_str);
}

void lv_create_main_gui(void) {
  //lv_obj_t * text_label = lv_label_create(lv_screen_active());
  //lv_label_set_text(text_label, "Computer Media Control");
  //lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -90);

  // Criar o label da hora no topo
  time_label = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_40, 0);
  lv_label_set_text(time_label, "00:00");
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -70);

  // Criar o label da data logo abaixo
  date_label = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(date_label, &lv_font_montserrat_26, 0);
  lv_label_set_text(date_label, "01/01/2024");
  lv_obj_align(date_label, LV_ALIGN_CENTER, 0, -40);

  lv_obj_t * btn_label_Prev;
  lv_obj_t * btn_Prev = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn_Prev, event_handler_btnPrev, LV_EVENT_ALL, NULL);
  lv_obj_align(btn_Prev, LV_ALIGN_CENTER, -100, 10);

  btn_label_Prev = lv_label_create(btn_Prev);
  lv_label_set_text(btn_label_Prev, "<< Previous");
  lv_obj_center(btn_label_Prev);

  lv_obj_t * btn_label_Next;
  lv_obj_t * btn_Next = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn_Next, event_handler_btnNext, LV_EVENT_ALL, NULL);
  lv_obj_align(btn_Next, LV_ALIGN_CENTER, 100, 10);

  btn_label_Next = lv_label_create(btn_Next);
  lv_label_set_text(btn_label_Next, "Next >>");
  lv_obj_center(btn_label_Next);

  lv_obj_t * btn_label_MusicStatus;
  lv_obj_t * btn_MusicStatus = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn_MusicStatus, event_handler_btnMusicStatus, LV_EVENT_ALL, NULL);
  lv_obj_align(btn_MusicStatus, LV_ALIGN_CENTER, 5, 10);
  lv_obj_add_flag(btn_MusicStatus, LV_OBJ_FLAG_CHECKABLE);
  
  btn_label_MusicStatus = lv_label_create(btn_MusicStatus);
  lv_label_set_text(btn_label_MusicStatus, "PLAY");
  lv_obj_center(btn_label_MusicStatus);

  lv_obj_t * slider = lv_slider_create(lv_screen_active());
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
  lv_slider_set_range(slider, 0, 100);

  slider_label = lv_label_create(lv_screen_active());
  lv_label_set_text(slider_label, "0%");
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}


void setup() {
  Serial.begin(115200);
  
  lv_init();
  lv_log_register_print_cb(log_print);

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);

  lv_display_t * disp;
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  lv_create_main_gui();



}

void loop() {
  lv_task_handler();
  lv_tick_inc(5);
  // Verificar se dados foram recebidos na serial
  unsigned long currentMillis = millis();  // Obtém o tempo atual em milissegundos

  // Verifica se já passou 1 segundo
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Atualiza o tempo anterior para o tempo atual

    if(Serial.available()) 
    {
      String serialData = Serial.readStringUntil('\n');
      // Separar a hora e a data
      int spaceIndex = serialData.indexOf(' ');
      if (spaceIndex > 0) {
        String time_str = serialData.substring(0, spaceIndex);
        String date_str = serialData.substring(spaceIndex + 1);
        
        update_time_and_date(time_str.c_str(), date_str.c_str());
      }
    }
  }
}
