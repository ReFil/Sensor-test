
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>
#include <zephyr/logging/log.h>
#include <ui.h>

#include <zephyr/drivers/rtc.h>
#include <time.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

static int setyear;
static int setmonth;
static int setday;
static int sethour;
static int setmin;

static const struct device *lvgl_encoder =
    DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_lvgl_encoder_input));

static const struct device *rtc = DEVICE_DT_GET(DT_CHOSEN(app_rtc));

void set_time_cb(struct k_work *work) {
    struct rtc_time datetime_set;
    datetime_set.tm_year = setyear - 1900;
    datetime_set.tm_mon = setmonth - 1;
    datetime_set.tm_mday = setday;
    datetime_set.tm_hour = sethour;
    datetime_set.tm_min = setmin;
    datetime_set.tm_sec = 0;
    int ret = rtc_set_time(rtc, &datetime_set);
    if (ret)
        LOG_ERR("Failed to set RTC time %d", ret);
    app_display_return_to_status();
}

K_WORK_DEFINE(set_time_work, set_time_cb);

static void min_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    setmin = lv_roller_get_selected(obj);
    LOG_INF("Selected min: %d \n", setmin);

    LOG_INF("Chosen date/time: %d %d %d, %d %d", setday, setmonth, setyear, sethour, setmin);
    k_work_submit_to_queue(app_display_work_q(), &set_time_work);
}

lv_obj_t *min_select_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *title;
    title = lv_label_create(screen);
    lv_label_set_text(title, "Minute");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    lv_group_t *roller_group;
    lv_obj_t *roller1 = lv_roller_create(screen);
    lv_roller_set_options(roller1,
                          "0\n"
                          "1\n"
                          "2\n"
                          "3\n"
                          "4\n"
                          "5\n"
                          "6\n"
                          "7\n"
                          "8\n"
                          "9\n"
                          "10\n"
                          "11\n"
                          "12\n"
                          "13\n"
                          "14\n"
                          "15\n"
                          "16\n"
                          "17\n"
                          "18\n"
                          "19\n"
                          "20\n"
                          "21\n"
                          "22\n"
                          "23\n"
                          "24\n"
                          "25\n"
                          "26\n"
                          "27\n"
                          "28\n"
                          "29\n"
                          "30\n"
                          "31\n"
                          "32\n"
                          "33\n"
                          "34\n"
                          "35\n"
                          "36\n"
                          "37\n"
                          "38\n"
                          "39\n"
                          "40\n"
                          "41\n"
                          "42\n"
                          "43\n"
                          "44\n"
                          "45\n"
                          "46\n"
                          "47\n"
                          "48\n"
                          "49\n"
                          "50\n"
                          "51\n"
                          "52\n"
                          "53\n"
                          "54\n"
                          "55\n"
                          "56\n"
                          "57\n"
                          "58\n"
                          "59",
                          LV_ROLLER_MODE_INFINITE);

    lv_obj_set_size(roller1, 20, LV_VER_RES - 16);
    lv_obj_align(roller1, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_roller_set_visible_row_count(roller1, 3);
    lv_obj_add_event_cb(roller1, min_event_handler, LV_EVENT_PRESSED, NULL);

    roller_group = lv_group_create();
    lv_group_add_obj(roller_group, roller1);
    lv_indev_set_group(lvgl_input_get_indev(lvgl_encoder), roller_group);
    // lv_group_set_editing(roller_group, true);
    return screen;
};

static void hour_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    sethour = lv_roller_get_selected(obj);
    LOG_INF("Selected hour: %d \n", sethour);
    app_display_load_new(min_select_screen());
}

lv_obj_t *hour_select_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *title;
    title = lv_label_create(screen);
    lv_label_set_text(title, "Hour");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    lv_group_t *roller_group;
    lv_obj_t *roller1 = lv_roller_create(screen);
    lv_roller_set_options(roller1,
                          "0\n"
                          "1\n"
                          "2\n"
                          "3\n"
                          "4\n"
                          "5\n"
                          "6\n"
                          "7\n"
                          "8\n"
                          "9\n"
                          "10\n"
                          "11\n"
                          "12\n"
                          "13\n"
                          "14\n"
                          "15\n"
                          "16\n"
                          "17\n"
                          "18\n"
                          "19\n"
                          "20\n"
                          "21\n"
                          "22\n"
                          "23\n"
                          "24",
                          LV_ROLLER_MODE_INFINITE);

    lv_obj_set_size(roller1, 20, LV_VER_RES - 16);
    lv_obj_align(roller1, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_roller_set_visible_row_count(roller1, 3);
    lv_obj_add_event_cb(roller1, hour_event_handler, LV_EVENT_PRESSED, NULL);

    roller_group = lv_group_create();
    lv_group_add_obj(roller_group, roller1);
    lv_indev_set_group(lvgl_input_get_indev(lvgl_encoder), roller_group);
    // lv_group_set_editing(roller_group, true);
    return screen;
};

static void day_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    setday = lv_roller_get_selected(obj) + 1;
    LOG_INF("Selected day: %d \n", setmonth);
    app_display_load_new(hour_select_screen());
}

lv_obj_t *day_select_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *title;
    title = lv_label_create(screen);
    lv_label_set_text(title, "Day");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    lv_group_t *roller_group;
    lv_obj_t *roller1 = lv_roller_create(screen);
    lv_roller_set_options(roller1,
                          "1\n"
                          "2\n"
                          "3\n"
                          "4\n"
                          "5\n"
                          "6\n"
                          "7\n"
                          "8\n"
                          "9\n"
                          "10\n"
                          "11\n"
                          "12\n"
                          "13\n"
                          "14\n"
                          "15\n"
                          "16\n"
                          "17\n"
                          "18\n"
                          "19\n"
                          "20\n"
                          "21\n"
                          "22\n"
                          "23\n"
                          "24\n"
                          "25\n"
                          "26\n"
                          "27\n"
                          "28\n"
                          "29\n"
                          "30\n"
                          "31",
                          LV_ROLLER_MODE_INFINITE);

    lv_obj_set_size(roller1, 20, LV_VER_RES - 20);
    lv_obj_align(roller1, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_roller_set_visible_row_count(roller1, 3);
    lv_obj_add_event_cb(roller1, day_event_handler, LV_EVENT_PRESSED, NULL);

    roller_group = lv_group_create();
    lv_group_add_obj(roller_group, roller1);
    lv_indev_set_group(lvgl_input_get_indev(lvgl_encoder), roller_group);
    // lv_group_set_editing(roller_group, true);
    return screen;
};

static void month_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);

    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));

    setmonth = lv_roller_get_selected(obj) + 1;
    LOG_INF("Selected month: %s, %d \n", buf, setmonth);
    app_display_load_new(day_select_screen());
}

lv_obj_t *month_select_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *title;
    title = lv_label_create(screen);
    lv_label_set_text(title, "Month");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    lv_group_t *roller_group;
    lv_obj_t *roller1 = lv_roller_create(screen);
    lv_roller_set_options(roller1,
                          "January\n"
                          "February\n"
                          "March\n"
                          "April\n"
                          "May\n"
                          "June\n"
                          "July\n"
                          "August\n"
                          "September\n"
                          "October\n"
                          "November\n"
                          "December",
                          LV_ROLLER_MODE_INFINITE);

    lv_obj_set_size(roller1, 84, LV_VER_RES - 20);
    lv_obj_align(roller1, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_roller_set_visible_row_count(roller1, 3);
    lv_obj_add_event_cb(roller1, month_event_handler, LV_EVENT_PRESSED, NULL);

    roller_group = lv_group_create();
    lv_group_add_obj(roller_group, roller1);
    lv_indev_set_group(lvgl_input_get_indev(lvgl_encoder), roller_group);
    // lv_group_set_editing(roller_group, true);
    return screen;
};

static void year_event_handler(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);

    char buf[5];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));

    setyear = atoi(buf);
    LOG_INF("Selected year: %s, %d\n", buf, setyear);
    app_display_load_new(month_select_screen());
}

lv_obj_t *year_select_screen() {
    lv_obj_t *screen;
    screen = lv_obj_create(NULL);

    lv_obj_t *title;
    title = lv_label_create(screen);
    lv_label_set_text(title, "Year");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    lv_group_t *roller_group;
    lv_obj_t *roller1 = lv_roller_create(screen);
    lv_roller_set_options(roller1,
                          "2024\n"
                          "2025\n"
                          "2026\n"
                          "2027\n"
                          "2028",
                          LV_ROLLER_MODE_INFINITE);

    lv_obj_set_size(roller1, 36, LV_VER_RES - 20);
    lv_obj_align(roller1, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_roller_set_visible_row_count(roller1, 3);
    lv_obj_add_event_cb(roller1, year_event_handler, LV_EVENT_PRESSED, NULL);

    roller_group = lv_group_create();
    lv_group_add_obj(roller_group, roller1);
    lv_indev_set_group(lvgl_input_get_indev(lvgl_encoder), roller_group);
    lv_group_set_editing(roller_group, true);
    return screen;
};
