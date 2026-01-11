#pragma once

#include <lvgl.h>


int app_display_init();
void app_display_load_new(lv_obj_t *newscreen);
struct k_work_q *app_display_work_q(void);

void app_display_return_to_status();