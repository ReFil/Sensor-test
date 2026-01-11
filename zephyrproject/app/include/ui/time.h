#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct app_widget_time_status {
    sys_snode_t node;
    lv_obj_t *obj;
};

int app_widget_time_status_init(struct app_widget_time_status *widget, lv_obj_t *parent);
lv_obj_t *app_widget_time_status_obj(struct app_widget_time_status *widget);