#include <pebble.h>

//the main window
static Window *window;

//where we will display the time
static TextLayer *time_layer;

//where we will display the date
static TextLayer *date_layer;

//where we will display the weekday
static TextLayer *day_layer;

//where our crack in time background image will go
static BitmapLayer *background_layer;

//the actual background image
static GBitmap *background;

//where our battery icon will go
static BitmapLayer *battery_layer;

//icon for < 20 percent
static GBitmap *battery_empty;
//icon for less than 34 percent
static GBitmap *battery_c2;
//icon for less than 48 percent
static GBitmap *battery_c3;
//icon for less than 62 percent
static GBitmap *battery_c4;
//icon for less than 76 percent
static GBitmap *battery_c5;
//icon for less than 90 percent
static GBitmap *battery_c6;
//icon for > 90 percent
static GBitmap *battery_full;
//icon for when plugged in and charging
static GBitmap *battery_charging;



//this method takes in the current charge_state
//and updates the image in the battery layer.
//todo: different images for when the battery is charging, and when the watch is plugged in,
//but fully charged. Right now, I think it will show the battery full image
//if its plugged in, but not charging, which is OK
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

//registered down below to be called every minute
//since we don't display seconds, we don't need to have it called more
//often than every minute
//
//this method also handles checking the date and battery, since you can
//only register one "tick" handler
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {

  static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.
  strftime(time_text, sizeof(time_text), "%R", tick_time); //$R => 00:00 24 hour
  text_layer_set_text(time_layer, time_text);
  
  //since we can't register an additional tick handler that runs when the day changes
  //we'll just check "set" the date here every minute. 
  static char date_text[] = "00/00";
  strftime(date_text,sizeof(date_text),"%m/%d",tick_time);
  text_layer_set_text(date_layer,date_text);
  
  static char day_text[] = "Mon";
  strftime(day_text,sizeof(day_text),"%a",tick_time);
  text_layer_set_text(day_layer,day_text);
  
  //poll and update the battery icon
  //once a minute should give us more than enough
  //feedback on the battery state
  handle_battery(battery_state_service_peek());
}


//called when the window is loaded in the app lifecycle
static void window_load(Window *window) {
  
  //get our window layer
  Layer *window_layer = window_get_root_layer(window);
  //get the bounds of the window, should be 144x168
  GRect bounds = layer_get_bounds(window_layer);

  //create out background layer, add the image, and add the layer to our window
  background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(background_layer,background);
  layer_add_child(window_layer,bitmap_layer_get_layer(background_layer));

  //setup the battery icon layer. no need to add the image, it'll be added when we 
  //have our first "tick"
  //icon is 16x16, and placement is the top left corner. I placed it 2 pixels from the top
  //and 18 pixels from the right (making the right side 2 pixels from the side)
  battery_layer = bitmap_layer_create((GRect) {.origin = {bounds.size.w - 18 ,2}, .size = {16,16}});
  layer_add_child(window_layer,bitmap_layer_get_layer(battery_layer));

  //setup our text layer that contains the time
  //I determined that the proper place for the bottom of the text is 78 pixels down,
  //so, we're playing it 78-fontsize, or 78-34 pixels down, since it's placed by the top left corner
  //we're giving it an x position of 0, so we can use the text alignment to center it
  //the size is the width of the window x the font size
  time_layer = text_layer_create((GRect) { .origin = { 0, 78-34 }, .size = { bounds.size.w, 34 } });
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));

  //setup our text layer that contains the date
  //we want this aligned bottom right. it's positioned by the top left corner,
  //so, we do window height - font size - 2 (for padding)
  //size should be windows length x font size, but, that was too small (causing the date to not display)
  //so I made it 30
  date_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h-24-2 }, .size = { bounds.size.w, 30} });
  text_layer_set_text_alignment(date_layer, GTextAlignmentRight);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  
  day_layer = text_layer_create((GRect) { .origin = { 0, bounds.size.h-24-2 }, .size = { bounds.size.w, 30} });
  text_layer_set_text_alignment(day_layer, GTextAlignmentLeft);
  text_layer_set_text_color(day_layer, GColorWhite);
  text_layer_set_background_color(day_layer, GColorClear);
  text_layer_set_font(day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  //this will actually trigger our first tick with the current time
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);
  
  //this tells the timer even to fire every minute, and call handle_minute_tick
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);

  //finally, we add our date and time layers
  layer_add_child(window_layer,text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(day_layer));

}

static void window_unload(Window *window) {
  //destroy all of our layers
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(day_layer);
  bitmap_layer_destroy(background_layer);
  bitmap_layer_destroy(battery_layer);
}

static void init(void) {
  //initalize all of our images.
  //I don't know if this has to be done here, or, can be done before
  //actually used, but examples all have it done in this method
  background = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  battery_empty = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_EMPTY);
  battery_c2 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C2);
  battery_c3 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C3);
  battery_c4 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C4);
  battery_c5 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C5);
  battery_c6 = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_C6);
  battery_full = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_FULL);
  battery_charging = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);

  //create our window
  window = window_create();
  
  //set the handlers for the load and unload events
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  //part of skeleton code, so, I've left it
  const bool animated = true;
  
  //pushing the window onto the stack "loads" it
  window_stack_push(window, animated);
}

static void deinit(void) {
  //this destroys our window and all of our images
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
