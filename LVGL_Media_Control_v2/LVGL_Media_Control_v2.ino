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

//int btnPrev_count = 0;
// Callback that is triggered when btn1 is clicked
static void event_handler_btnPrev(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    //btnPrev_count++;
    //LV_LOG_USER("Button Prev clicked %d", (int)btnPrev_count);
    Serial.println("Prev");
  }
}


//int btnNext_count = 0;
// Callback that is triggered when btn1 is clicked
static void event_handler_btnNext(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    //btnNext_count++;
    //LV_LOG_USER("Button  Next clicked %d", (int)btnNext_count);
    Serial.println("Next");
  }
}






// Callback that is triggered when btn2 is clicked/toggled
//static void event_handler_btnMusicStatus(lv_event_t * e) 
//{
//  lv_event_code_t code = lv_event_get_code(e);
//  lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
//  if(code == LV_EVENT_VALUE_CHANGED) 
//  {
//    LV_UNUSED(obj);
//    LV_LOG_USER("Toggled %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "Play" : "Pause");
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
      //lv_label_set_text(btn_label_MusicStatus, "PAUSE"); // Altera para "PAUSE" quando o botão estiver marcado
      //LV_LOG_USER("Toggled Play -> Pause");
      Serial.println("Pause");
    } else {
      //lv_label_set_text(btn_label_MusicStatus, "PLAY");  // Altera para "PLAY" quando o botão estiver desmarcado
      //LV_LOG_USER("Toggled Pause -> Play");
      Serial.println("Play");
    }
  }
}




static lv_obj_t * slider_label;
// Callback that prints the current slider value on the TFT display and Serial Monitor for debugging purposes
static void slider_event_callback(lv_event_t * e) {
  lv_obj_t * slider = (lv_obj_t*) lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  LV_LOG_USER("Slider changed to %d%%", (int)lv_slider_get_value(slider));
}

void lv_create_main_gui(void) {
  // Create a text label aligned center on top ("Hello, world!")
  lv_obj_t * text_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(text_label, "Computer Media Control");
  lv_obj_set_width(text_label, 150);    // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -90);

  // Create a Button Previous
  lv_obj_t * btn_label_Prev;
  lv_obj_t * btn_Prev = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn_Prev, event_handler_btnPrev, LV_EVENT_ALL, NULL);
  lv_obj_align(btn_Prev, LV_ALIGN_CENTER, -100, 10);
  lv_obj_remove_flag(btn_Prev, LV_OBJ_FLAG_PRESS_LOCK);

  btn_label_Prev = lv_label_create(btn_Prev);
  lv_label_set_text(btn_label_Prev, "<< Previus");
  lv_obj_center(btn_label_Prev);

  // Create a Button Next
  lv_obj_t * btn_label_Next;
  lv_obj_t * btn_Next = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn_Next, event_handler_btnNext, LV_EVENT_ALL, NULL);
  lv_obj_align(btn_Next, LV_ALIGN_CENTER, 100, 10);
  lv_obj_remove_flag(btn_Next, LV_OBJ_FLAG_PRESS_LOCK);

  btn_label_Next = lv_label_create(btn_Next);
  lv_label_set_text(btn_label_Next, "Next >>");
  lv_obj_center(btn_label_Next);



  // Create a Button Play/Pause
  lv_obj_t * btn_label_MusicStatus;
  lv_obj_t * btn_MusicStatus = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn_MusicStatus, event_handler_btnMusicStatus, LV_EVENT_ALL, NULL);
  lv_obj_align(btn_MusicStatus, LV_ALIGN_CENTER, 5, 10);
  lv_obj_add_flag(btn_MusicStatus, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn_MusicStatus, LV_SIZE_CONTENT);

  btn_label_MusicStatus = lv_label_create(btn_MusicStatus);
  lv_label_set_text(btn_label_MusicStatus, "PLAY");
  lv_obj_center(btn_label_MusicStatus);
  
  // Create a slider aligned in the center bottom of the TFT display
  lv_obj_t * slider = lv_slider_create(lv_screen_active());
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_anim_duration(slider, 2000, 0);

  // Create a label below the slider to display the current slider value
  slider_label = lv_label_create(lv_screen_active());
  lv_label_set_text(slider_label, "0%");
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
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
  lv_create_main_gui();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}