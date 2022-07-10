/*
 * Copyright (c) 2020, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>
#include <modm/debug.hpp>
#include <modm/processing.hpp>

#include <lvgl/lvgl.h>


// Set the log level
#undef	MODM_LOG_LEVEL
#define	MODM_LOG_LEVEL modm::log::DEBUG

using namespace Board;
using namespace modm::glcd;
using namespace Board::ft6;
using namespace modm::literals;


static uint16_t* displayBuffer;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t* buf;
static constexpr size_t buf_size = LV_HOR_RES_MAX * LV_VER_RES_MAX;

Touch::Data touchData;
Touch touch{touchData, TouchAddress};

void my_touchpad_read(lv_indev_drv_t*, lv_indev_data_t* data)
{
	RF_CALL_BLOCKING(touch.readTouches());
	Touch::touch_t tp;
	touch.getData().getTouch(&tp, 0);
	// mirror and rotate correctly
	uint16_t x{tp.y}, y{uint16_t(480 - tp.x)};
	data->state = (tp.event == Touch::Event::Contact) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
	if(data->state == LV_INDEV_STATE_PR) {
		data->point.x = x;
		data->point.y = y;
	}
}

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	for(lv_coord_t y = area->y1; y <= area->y2; y++) {
		std::memcpy(
			&displayBuffer[(800*y) + area->x1],
			&color_p[(y - area->y1) * (area->x2 - area->x1 + 1)],
			(area->x2 - area->x1 + 1) * sizeof(lv_color_t)
			);
	}
	lv_disp_flush_ready(disp_drv);
}

static const auto color_vampireblack = lv_color_hex(0x0D0208);
static const auto color_darkgreen = lv_color_hex(0x003B00);
static const auto color_islamicgreen = lv_color_hex(0x008F11);
static const auto color_malachite = lv_color_hex(0x00FF41);

int
main()
{
	Board::initialize();
	Board::initializeDisplay();
	Board::initializeTouchscreen();

	MODM_LOG_INFO << "modm LVGL example on STM32F469-Discovery board!\n\n";

	RF_CALL_BLOCKING(touch.configure(Touch::InterruptMode::Trigger, 30, 30));

	lv_init();

	buf = new(modm::MemoryExternal) lv_color_t[buf_size];
	displayBuffer = new(modm::MemoryExternal) uint16_t[buf_size];

	setDisplayBuffer(displayBuffer);

	// Initialize the display buffer
	lv_disp_draw_buf_init(&disp_buf, buf, NULL, buf_size);

	// Initialize the display:
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.draw_buf = &disp_buf;
	disp_drv.flush_cb = my_flush_cb;
	disp_drv.hor_res = LV_HOR_RES_MAX;
	disp_drv.ver_res = LV_VER_RES_MAX;
	lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

	// Initialize touchscreen driver:
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_touchpad_read;
	lv_indev_drv_register(&indev_drv);

	lv_obj_t* scr = lv_disp_get_scr_act(disp); // Get the current screen
	lv_obj_set_style_bg_color(scr, color_vampireblack, 0);

	// lv_obj_t* labelA =  lv_label_create(scr);
	// lv_label_set_text(labelA, "Hello world!");
	// lv_obj_set_pos(labelA, 10, 10);
	// lv_obj_set_size(labelA, 120, 50);

	static lv_style_t style_border;
	lv_style_init(&style_border);
	// lv_style_set_bg_color(&style_btn, lv_color_hex(0x115588));
	// lv_style_set_bg_opa(&style_btn, LV_OPA_50);
	lv_style_set_border_width(&style_border, 2);
	lv_style_set_border_color(&style_border, color_malachite);

	static lv_style_t style_active;
	lv_style_init(&style_active);
	lv_style_set_bg_color(&style_active, color_islamicgreen);

	static lv_style_t style_inactive;
	lv_style_init(&style_inactive);
	lv_style_set_bg_color(&style_inactive, color_darkgreen);
	// lv_style_set_bg_opa(&style_btn_red, LV_OPA_COVER);

	lv_obj_t* btn2 = lv_btn_create(lv_scr_act());
	lv_obj_add_style(btn2, &style_inactive, LV_PART_MAIN);
	lv_obj_add_style(btn2, &style_border, LV_PART_MAIN);
	lv_obj_set_pos(btn2, 10, 10);
	lv_obj_set_size(btn2, 150, 90);
	lv_obj_t* label2 = lv_label_create(btn2);
	lv_label_set_text(label2, "Button 2:");
	lv_obj_add_event_cb(btn2, [](lv_event_t *event) {
		static uint16_t btn2Counter = 0;
		lv_label_set_text_fmt(lv_obj_get_child(event->target, 0), "Button 2: %d", ++btn2Counter);
	}, LV_EVENT_PRESSED, NULL);

	lv_obj_t* btn3 = lv_btn_create(lv_scr_act());
	lv_obj_add_style(btn3, &style_inactive, LV_PART_MAIN);
	lv_obj_add_style(btn3, &style_border, LV_PART_MAIN);
	lv_obj_set_pos(btn3, 10, 110);
	lv_obj_set_size(btn3, 150, 90);
	lv_obj_t* label3 = lv_label_create(btn3);
	lv_label_set_text(label3, "Button 2:");
	lv_obj_add_event_cb(btn3, [](lv_event_t *event) {
		static uint16_t btn3Counter = 0;
		lv_label_set_text_fmt(lv_obj_get_child(event->target, 0), "Button 3: %d", ++btn3Counter);
	}, LV_EVENT_PRESSED, NULL);

	lv_obj_t* btn4 = lv_btn_create(lv_scr_act());
	lv_obj_add_style(btn4, &style_inactive, LV_PART_MAIN);
	lv_obj_add_style(btn4, &style_border, LV_PART_MAIN);
	lv_obj_add_style(btn4, &style_active, LV_STATE_CHECKED);
	lv_obj_set_pos(btn4, 10, 210);
	lv_obj_set_size(btn4, 150, 90);
	lv_obj_add_flag(btn4, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_t* label4 = lv_label_create(btn4);
	lv_label_set_text(label4, "Button 2:");
	lv_obj_add_event_cb(btn4, [](lv_event_t *event) {
		static uint16_t btn4Counter = 0;
		lv_label_set_text_fmt(lv_obj_get_child(event->target, 0), "Button 4: %d", ++btn4Counter);
	}, LV_EVENT_PRESSED, NULL);

    static lv_obj_t * cw = lv_colorwheel_create(lv_scr_act(), true);
    lv_obj_set_pos(cw, 470, 30);
    lv_obj_set_size(cw, 300, 300);
	lv_obj_set_style_arc_width(cw,30,LV_PART_MAIN);
	// static lv_style_t style_opa;
	// lv_style_init(&style_opa);
	// // lv_style_set_bg_color(&style_btn, lv_color_hex(0x115588));
	// lv_style_set_bg_opa(&style_opa, LV_OPA_50);
	// lv_obj_add_style(cw, &style_opa, LV_PART_MAIN);
	
	static lv_style_t style_round;
	lv_style_init(&style_round);
	lv_style_set_radius(&style_round, 50);
	lv_obj_t* btn_slidermode = lv_btn_create(lv_scr_act());
	lv_obj_add_style(btn_slidermode, &style_active, LV_PART_MAIN);
	lv_obj_add_style(btn_slidermode, &style_border, LV_PART_MAIN);
	lv_obj_set_pos(btn_slidermode, 575, 125);
	lv_obj_set_size(btn_slidermode, 100, 100);
	lv_obj_add_style(btn_slidermode, &style_round, LV_PART_MAIN);
	lv_colorwheel_set_hsv(cw, lv_color_hsv_t(180, 100, 50));
	// lv_obj_add_flag(btn4, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_t* label_btn_slidermode = lv_label_create(btn_slidermode);
	lv_label_set_text(label_btn_slidermode, "mode");
	lv_obj_add_event_cb(btn_slidermode, [cw](lv_event_t *event) {
		static lv_colorwheel_mode_t slidermode = LV_COLORWHEEL_MODE_HUE;
		if (slidermode == LV_COLORWHEEL_MODE_VALUE){ slidermode = LV_COLORWHEEL_MODE_HUE;}
		else { slidermode++;}
		lv_colorwheel_set_mode(cw, slidermode);
	}, LV_EVENT_PRESSED, NULL);


	lv_obj_t* btn_sliderreset = lv_btn_create(lv_scr_act());
	lv_obj_add_style(btn_sliderreset, &style_inactive, LV_PART_MAIN);
	lv_obj_add_style(btn_sliderreset, &style_border, LV_PART_MAIN);
	lv_obj_set_pos(btn_sliderreset, 520, 380);
	lv_obj_set_size(btn_sliderreset, 200, 90);
	lv_color_hsv_t default_hsv = lv_colorwheel_get_hsv(cw);
	// lv_obj_add_flag(btn4, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_t* label_btn_sliderreset = lv_label_create(btn_sliderreset);
	lv_label_set_text(label_btn_sliderreset, "reset");
	lv_obj_add_event_cb(btn_sliderreset, [cw](lv_event_t *event) {
		lv_colorwheel_set_hsv(cw, lv_color_hsv_t(180, 100, 50));
	}, LV_EVENT_PRESSED, NULL);

	// static lv_obj_t * slider_label;
	// lv_obj_t * slider = lv_slider_create(lv_scr_act());
	// lv_obj_add_style(slider, &style_border, LV_PART_INDICATOR);
	// lv_obj_add_style(slider, &style_active, LV_PART_INDICATOR);
	// lv_obj_add_style(slider, &style_border, LV_PART_MAIN);
	// lv_obj_add_style(slider, &style_inactive, LV_PART_MAIN);
	// lv_obj_add_style(slider, &style_border, LV_PART_KNOB);
	// lv_obj_add_style(slider, &style_active, LV_PART_KNOB);
	// lv_obj_set_pos(slider, 50, 400);
	// lv_obj_set_size(slider, 700, 50);
	// // lv_obj_center(slider);
	// auto slider_event_cb = [](lv_event_t *event) {
	// 	lv_obj_t * slider = lv_event_get_target(event);
	// 	char buf[8];
	// 	lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
	// 	lv_label_set_text(slider_label, buf);
	// 	lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	// };
	// lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	// slider_label = lv_label_create(lv_scr_act());
	// lv_label_set_text(slider_label, "0%");
	// lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	uint16_t counter = 0;

	modm::ShortPeriodicTimer tmr{1000ms};
	while (true)
	{
		lv_task_handler();

		if (tmr.execute())
		{
			// lv_label_set_text_fmt(labelA, "counter=%d", ++counter);
		}
	}

	return 0;
}
