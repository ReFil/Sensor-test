#include <ui.h>
#include <ui/time.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/rtc.h>

#include <time.h>

LOG_MODULE_DECLARE(app, CONFIG_APP_LOG_LEVEL);

static const struct device *rtc = DEVICE_DT_GET(DT_CHOSEN(app_rtc));

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct time_status_state {
    struct rtc_time datetime_get;
};

void set_time_symbol(lv_obj_t *label, struct time_status_state state) {
    /*char time_str[32] = {0};
    sprintf(time_str, "%d %d %d, %02d:%02d", state.datetime_get.tm_mday, state.datetime_get.tm_mon,
            state.datetime_get.tm_year + 1900, state.datetime_get.tm_hour,
            state.datetime_get.tm_min);
*/
    char time_str[5] = {0};
    sprintf(time_str, "%02d:%02d", state.datetime_get.tm_hour, state.datetime_get.tm_min);
    lv_label_set_text(label, time_str);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}

void time_status_update_cb(struct time_status_state state) {
    struct app_widget_time_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_time_symbol(widget->obj, state); }
}

void status_tick_cb(struct k_work *work) {
    struct time_status_state state;
    if (rtc_get_time(rtc, &state.datetime_get)) {
        LOG_ERR("RTC hasn't got the time");
    }
    time_status_update_cb(state);
}

K_WORK_DEFINE(status_tick_work, status_tick_cb);

void status_timer_cb() { k_work_submit_to_queue(app_display_work_q(), &status_tick_work); }

K_TIMER_DEFINE(status_timer, status_timer_cb, NULL);

int app_widget_time_status_init(struct app_widget_time_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_obj_align(widget->obj, LV_ALIGN_RIGHT_MID, 0, 0);

    sys_slist_append(&widgets, &widget->node);
    k_timer_start(&status_timer, K_NO_WAIT, K_SECONDS(15));

    return 0;
}

lv_obj_t *app_widget_time_status_obj(struct app_widget_time_status *widget) { return widget->obj; }