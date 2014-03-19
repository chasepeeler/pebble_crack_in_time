#include <pebble.h>

static Window *window;
static TextLayer *time_layer;

static BitmapLayer *background_layer;

static GBitmap *background;

static BitmapLayer *battery_layer;
static GBitmap *battery_empty;
static GBitmap *battery_c2;
static GBitmap *battery_c3;
static GBitmap *battery_c4;
static GBitmap *battery_c5;
static GBitmap *battery_c6;
static GBitmap *battery_full;
static GBitmap *battery_charging;

static TextLayer *date_layer;


static void handle_battery(BatteryChargeState charge_state) {

  if (charge_state.is_charging) {
    bitmap_layer_set_bitmap(battery_layer,battery_charging);
  } else if(charge_state.charge_percent < 20) {
    bitmap_layer_set_bitmap(battery_layer,battery_empty);
  } else if(charge_state.charge_percent < 34){
    bitmap_layer_set_bitmap(battery_layer,battery_c2);
  } else if(charge_state.charge_percent < 48){
    bitmap_layer_set_bitmap(battery_layer,battery_c3);
  } else if(charge_state.charge_percent < 62){
    bitmap_layer_set_bitmap(battery_layer,battery_c4);
  } else if(charge_state.charge_percent < 76){
    bitmap_layer_set_bitmap(battery_layer,battery_c5);
  } else if(charge_state.charge_percent < 90){
    bitmap_layer_set_bitmap(battery_layer,battery_c6);
  } else {
    bitmap_layer_set_bitmap(battery_layer,battery_full);
  }
}


static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {

  static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.
  static char date_text[] = "00/00";
  strftime(time_text, sizeof(time_text), "%R", tick_time);
  strftime(date_text,sizeof(date_text),"%m/%d",tick_time);
  text_layer_set_text(time_layer, time_text);
  text_layer_set_text(date_layer,date_text);
  handle_battery(battery_state_service_peek());
}



static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(background_layer,background);
  layer_add_child(window_layer,bitmap_layer_get_layer(background_layer));

  battery_layer = bitmap_layer_create((GRect) {.origin = {bounds.size.w - 18 ,2}, .size = {16,16}});
  layer_add_child(window_layer,bitmap_layer_get_layer(battery_layer));

  time_layer = text_layer_create((GRect) { .origin = { 0, 78-34 }, .size = { bounds.size.w, 34 } });
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));

  date_layer = text_layer_create((GRect) { .origin = { 0, 168-24-2 }, .size = { bounds.size.w, 30} });
  text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);

  layer_add_child(window_layer,text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  bitmap_layer_destroy(background_layer);
  bitmap_layer_destroy(battery_layer);
}

static void init(void) {
  background = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  battery_empty = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_EMPTY);
  battery_c2 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C2);
  battery_c3 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C3);
  battery_c4 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C4);
  battery_c5 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C5);
  battery_c6 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C6);

  battery_full = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_FULL);
  battery_charging = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);

  window = window_create();
  //window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  gbitmap_destroy(background);
  gbitmap_destroy(battery_empty);
  gbitmap_destroy(battery_c2);
  gbitmap_destroy(battery_c3);
  gbitmap_destroy(battery_c4);
  gbitmap_destroy(battery_c5);
  gbitmap_destroy(battery_c6);
  gbitmap_destroy(battery_full);
  gbitmap_destroy(battery_charging);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
