#include <signal.h>
#include <stdio.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#define SLEEP(s) Sleep((DWORD)s)
#else
#include <unistd.h>
#define SLEEP(s) usleep(s * 1000)
#endif
#include "lvgl.h"
#include "sdl/sdl.h"

static int demo_status;

static void demo_release(int signal) {
  printf("got signal[%d]\n", signal);
  demo_status = 0;
}

static void btn_event_cb(lv_event_t *e) {
  // from https://docs.lvgl.io/master/examples.html
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    static uint8_t cnt = 0;
    cnt++;

    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "clicked: %d", cnt);
  }
}

static void create_ui(void) {
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0x000000),
                              LV_PART_MAIN);

  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_t *lbl = lv_label_create(btn);
  lv_obj_set_style_bg_color(btn, lv_color_black(), LV_PART_MAIN);

  // style
  static lv_style_t style;
  lv_style_init(&style);
  lv_style_set_radius(&style, 0);
  lv_style_set_bg_opa(&style, LV_OPA_COVER);
  lv_style_set_bg_color(&style, lv_color_white());
  lv_style_set_outline_width(&style, 2);
  lv_style_set_outline_color(&style, lv_color_black());
  lv_style_set_outline_pad(&style, 8);
  lv_style_set_shadow_width(&style, 6);
  lv_style_set_shadow_color(&style, lv_color_black());
  lv_style_set_shadow_ofs_x(&style, 5);
  lv_style_set_shadow_ofs_y(&style, 5);
  lv_obj_add_style(btn, &style, LV_PART_MAIN);
  // style end

  lv_label_set_text(lbl, "Hello world!");

  lv_obj_center(lbl);
  lv_obj_center(btn);

  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL,
                      NULL); /*Assign a callback to the button*/
}

static void driver_init(void) {
#define BUFFER_SIZE (SDL_HOR_RES * SDL_VER_RES)
  /*A static or global variable to store the buffers*/
  static lv_disp_draw_buf_t disp_buf;

  /*Static or global buffer(s). The second buffer is optional*/
  static lv_color_t *buf_1[BUFFER_SIZE] = {0};

  /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL
   * instead buf_2 */
  lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, BUFFER_SIZE);

  static lv_disp_drv_t
      disp_drv; /*A variable to hold the drivers. Must be static or global.*/
  lv_disp_drv_init(&disp_drv);   /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf; /*Set an initialized buffer*/
  disp_drv.flush_cb =
      sdl_display_flush; /*Set a flush callback to draw to the display*/
  disp_drv.hor_res = SDL_HOR_RES; /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = SDL_VER_RES; /*Set the vertical resolution in pixels*/

  lv_disp_t *disp = lv_disp_drv_register(
      &disp_drv); /*Register the driver and save the created display objects*/
  lv_theme_default_init(disp, lv_color_make(0x77, 0x44, 0xBB),
                        lv_color_make(0x14, 0x14, 0x3C), 1, lv_font_default());

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv); /*Basic initialization*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = sdl_mouse_read;
  lv_indev_drv_register(&indev_drv);
}

int main(void) {

  demo_status = 1;
  signal(SIGINT, demo_release);
  signal(SIGTERM, demo_release);

  lv_init();
  sdl_init();

  driver_init();
  create_ui();

  while (demo_status) {
    // Run LVGL engine
    lv_tick_inc(1);
    lv_timer_handler();

    SLEEP(1);
  }

  return 0;
}
