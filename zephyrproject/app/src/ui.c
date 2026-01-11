
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/rtc.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>
#include <zephyr/logging/log.h>
#include <ui.h>
#include <ui/datetime.h>
#include <ui/time.h>

#include <time.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

static const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static const struct device *rtc = DEVICE_DT_GET(DT_CHOSEN(app_rtc));

static struct app_widget_time_status time_status_widget;

static lv_obj_t *screen;

void display_tick_cb(struct k_work *work) { lv_task_handler(); }

#define TICK_MS 10

K_WORK_DEFINE(display_tick_work, display_tick_cb);

#if IS_ENABLED(CONFIG_APP_DISPLAY_WORK_QUEUE_DEDICATED)

K_THREAD_STACK_DEFINE(display_work_stack_area, CONFIG_APP_DISPLAY_DEDICATED_THREAD_STACK_SIZE);

static struct k_work_q display_work_q;

#endif

struct k_work_q *app_display_work_q() {
#if IS_ENABLED(CONFIG_APP_DISPLAY_WORK_QUEUE_DEDICATED)
    return &display_work_q;
#else
    return &k_sys_work_q;
#endif
}

void display_timer_cb() { k_work_submit_to_queue(app_display_work_q(), &display_tick_work); }

K_TIMER_DEFINE(display_timer, display_timer_cb, NULL);

void unblank_display_cb(struct k_work *work) {
    display_blanking_off(display);
    k_timer_start(&display_timer, K_MSEC(TICK_MS), K_MSEC(TICK_MS));
}

void app_display_load_new(lv_obj_t *newscreen) {
    screen = newscreen;
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

lv_obj_t *app_display_status_screen() {
    lv_obj_t *screen2;
    screen2 = lv_obj_create(NULL);

    struct rtc_time datetime_get;
    if (rtc_get_time(rtc, &datetime_get)) {
        LOG_INF("RTC hasn't got the time, go to time setting system");
        lv_obj_del(screen2);
        return year_select_screen();
    }

    app_widget_time_status_init(&time_status_widget, screen2);
    lv_obj_align(app_widget_time_status_obj(&time_status_widget), LV_ALIGN_TOP_LEFT, 0, 0);

    return screen2;
}

void app_display_return_to_status() { app_display_load_new(app_display_status_screen()); }

void initialize_display(struct k_work *work) {
    LOG_INF("Hello from display");

    if (!device_is_ready(display)) {
        LOG_ERR("Failed to find display device");
        return;
    }

    screen = app_display_status_screen();

    if (screen == NULL) {
        LOG_ERR("No status screen provided");
        return;
    }

    lv_scr_load(screen);

    lv_task_handler();
    unblank_display_cb(work);
}

K_WORK_DEFINE(init_work, initialize_display);

int app_display_init() {
#if IS_ENABLED(CONFIG_APP_DISPLAY_WORK_QUEUE_DEDICATED)
    k_work_queue_start(&display_work_q, display_work_stack_area,
                       K_THREAD_STACK_SIZEOF(display_work_stack_area),
                       CONFIG_APP_DISPLAY_DEDICATED_THREAD_PRIORITY, NULL);
#endif

    k_work_submit_to_queue(app_display_work_q(), &init_work);

    LOG_INF("Dispinit");
    return 0;
}
