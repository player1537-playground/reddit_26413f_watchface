/*

   Demonstrate how to display an image with black, white and transparent sections.

   To achieve this effect you need to use a `RotBmpPairContainer` which contains
   two bitmaps: one is black and transparent, the other is white and transparent.

   The two bitmaps are created during the build process from a RGBA format PNG file.

   The image can also be rotated at runtime.

   The demonstration image is a modified version of the following SVG
   exported from Inkscape:

       <http://openclipart.org/detail/48919/panda-with-bamboo-leaves-by-adam_lowe>

 */

#include "pebble.h"

#define NUM_HOURS 12
#define NUM_MINUTES 60
#define MINUTE_STRING_DELTA 5
#define NUM_MINUTE_STRINGS (NUM_MINUTES / MINUTE_STRING_DELTA)
  
static int16_t ENABLE_SECONDS = 0;
static int16_t ENABLE_BORDERS = 1;
  
static Window *window;

static Layer *watch_layer;
static int time_seconds;
static int time_minutes;
static int time_hours;
static GBitmap *dithered_image;
// static TextLayer *text_layer; // Used as a background to help demonstrate transparency.

static void my_watch_layer_draw(Layer *layer, GContext *ctx) {
  const float   OUTER_RADIUS_MULT = 0.8;
  const int16_t OUTER_RADIUS_BORDER_WIDTH = 1;
  const float   INNER_RADIUS_MULT = 0.4;
  const int16_t INNER_RADIUS_BORDER_WIDTH = 1;
  
  const int16_t SECOND_CIRCLE_RADIUS = 2;
  const int16_t SECOND_DIST_FROM_BORDER = 10;
  
  const int16_t MINUTE_CIRCLE_RADIUS = 2;
  const int16_t MINUTE_DIST_FROM_BORDER = 5;
  
  const int16_t HOUR_CIRCLE_RADIUS = 2;
  const int16_t HOUR_DIST_FROM_BORDER = 5;
  
  const float HOUR_TEXT_RADIUS_MULT = 0.9;
  const int16_t TEXT_FRAME_WIDTH = 20;
  const int16_t TEXT_FRAME_HEIGHT = 18 + 2;
  const char *HOUR_STRINGS[NUM_HOURS] = {
    "12",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11"
  };
  
  const float MINUTE_TEXT_RADIUS_MULT = 0.5;
  const char *MINUTE_STRINGS[NUM_MINUTE_STRINGS] = {
    "0",
    "5",
    "10",
    "15",
    "20",
    "25",
    "30",
    "35",
    "40",
    "45",
    "50",
    "55"
  };
  
  GRect bounds = layer_get_bounds(layer);
  
  // Draw a black filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  const int16_t half_h = bounds.size.h / 2;
  const int16_t half_w = bounds.size.w / 2;
  const int16_t radius = half_h < half_w ? half_h : half_w;
  
  const int16_t hour_dithered_prev = (time_hours + NUM_HOURS - 1) % NUM_HOURS;
  const int16_t hour_dithered_next = (time_hours + 1) % NUM_HOURS;
  const int16_t hour_normal = time_hours % NUM_HOURS;
  
  GRect text_frame = GRect(0, 0, TEXT_FRAME_WIDTH, TEXT_FRAME_HEIGHT);
  const int16_t hour_text_length = HOUR_TEXT_RADIUS_MULT * radius;
  int16_t text_angle;
  for (int i=0; i<NUM_HOURS; i++) {
    if (i == hour_normal || i == hour_dithered_prev || i == hour_dithered_next) {
      graphics_context_set_text_color(ctx, GColorWhite);
      text_angle = TRIG_MAX_ANGLE * i / NUM_HOURS;
      text_frame.origin.y = half_h - TEXT_FRAME_HEIGHT / 2 - cos_lookup(text_angle) * hour_text_length / TRIG_MAX_RATIO;
      text_frame.origin.x = half_w -  TEXT_FRAME_WIDTH / 2 + sin_lookup(text_angle) * hour_text_length / TRIG_MAX_RATIO;
      graphics_draw_text(ctx, HOUR_STRINGS[i],
                         fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                         text_frame,
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
      
      if (i == hour_dithered_prev || i == hour_dithered_next) {
        // draw dithered rect over hour text
        graphics_context_set_compositing_mode(ctx, GCompOpAnd);
        graphics_draw_bitmap_in_rect(ctx, dithered_image, text_frame);
        graphics_context_set_compositing_mode(ctx, GCompOpAssign);
      }
    }
  }
  
  const int16_t minute_index = (int)((time_minutes + (float)MINUTE_STRING_DELTA / 2.0) / MINUTE_STRING_DELTA) % NUM_MINUTE_STRINGS;
  const int16_t minute_dithered_prev = (minute_index + NUM_MINUTE_STRINGS - 1) % NUM_MINUTE_STRINGS;
  const int16_t minute_dithered_next = (minute_index + 1) % NUM_MINUTE_STRINGS;
  
  const int16_t minute_text_length = MINUTE_TEXT_RADIUS_MULT * radius;
  for (int i=0; i<NUM_MINUTE_STRINGS; i++) {
    if (i == minute_index || i == minute_dithered_prev || i == minute_dithered_next) {
      graphics_context_set_text_color(ctx, GColorWhite);
      text_angle = TRIG_MAX_ANGLE * i / NUM_MINUTE_STRINGS;
      text_frame.origin.y = half_h - TEXT_FRAME_HEIGHT / 2 - cos_lookup(text_angle) * minute_text_length / TRIG_MAX_RATIO;
      text_frame.origin.x = half_w -  TEXT_FRAME_WIDTH / 2 + sin_lookup(text_angle) * minute_text_length / TRIG_MAX_RATIO;
      graphics_draw_text(ctx, MINUTE_STRINGS[i],
                         fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                         text_frame,
                         GTextOverflowModeFill,
                         GTextAlignmentCenter,
                         NULL);
     
      if (i == minute_dithered_prev || i == minute_dithered_next) {
        // draw dithered rect over minute text
        graphics_context_set_compositing_mode(ctx, GCompOpAnd);
        graphics_draw_bitmap_in_rect(ctx, dithered_image, text_frame);
        graphics_context_set_compositing_mode(ctx, GCompOpAssign);
      }
    }
  }

  if (ENABLE_BORDERS) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, GPoint(half_w, half_h), OUTER_RADIUS_MULT * radius);
    
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, GPoint(half_w, half_h), INNER_RADIUS_MULT * radius);
  }
  
  if (ENABLE_SECONDS) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    const int16_t second_angle = TRIG_MAX_ANGLE * time_seconds / 60;
    const int16_t second_length = radius * INNER_RADIUS_MULT - INNER_RADIUS_BORDER_WIDTH - SECOND_DIST_FROM_BORDER;
    const int16_t second_y = half_h - cos_lookup(second_angle) * second_length / TRIG_MAX_RATIO;
    const int16_t second_x = half_w + sin_lookup(second_angle) * second_length / TRIG_MAX_RATIO;
    graphics_fill_circle(ctx, GPoint(second_x, second_y), SECOND_CIRCLE_RADIUS);
  }
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  const int16_t minute_angle = TRIG_MAX_ANGLE * time_minutes / 60;
  const int16_t minute_length = radius * INNER_RADIUS_MULT - INNER_RADIUS_BORDER_WIDTH - MINUTE_DIST_FROM_BORDER;
  const int16_t minute_y = half_h - cos_lookup(minute_angle) * minute_length / TRIG_MAX_RATIO;
  const int16_t minute_x = half_w + sin_lookup(minute_angle) * minute_length / TRIG_MAX_RATIO;
  graphics_fill_circle(ctx, GPoint(minute_x, minute_y), MINUTE_CIRCLE_RADIUS);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  const int16_t hour_angle = TRIG_MAX_ANGLE * time_hours / 12;
  const int16_t hour_length = radius * OUTER_RADIUS_MULT - OUTER_RADIUS_BORDER_WIDTH - HOUR_DIST_FROM_BORDER;
  const int16_t hour_y = half_h - cos_lookup(hour_angle) * hour_length / TRIG_MAX_RATIO;
  const int16_t hour_x = half_w + sin_lookup(hour_angle) * hour_length / TRIG_MAX_RATIO;
  graphics_fill_circle(ctx, GPoint(hour_x, hour_y), HOUR_CIRCLE_RADIUS);
  

}

static void update_time_variables(struct tm *tick_time) {
  if (ENABLE_SECONDS) {
    time_seconds = tick_time->tm_sec;
  }
  time_minutes = tick_time->tm_min;
  time_hours   = tick_time->tm_hour;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time_variables(tick_time);
  layer_mark_dirty(watch_layer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  watch_layer = layer_create(bounds);
  layer_set_update_proc(watch_layer, my_watch_layer_draw);
  
  layer_add_child(window_layer, watch_layer);
  
  dithered_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DITHER_BLACK);
}

static void window_unload(Window *window) {
  gbitmap_destroy(dithered_image);
  layer_destroy(watch_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_set_fullscreen(window, true);
  window_stack_push(window, true /* Animated */);
  tick_timer_service_subscribe(ENABLE_SECONDS ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
