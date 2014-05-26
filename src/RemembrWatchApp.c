#include "pebble.h"

static Window *window;

static ScrollLayer *scroll_layer;

static TextLayer *text_layer;

static char scroll_text[] = "Please push a note from your iOS Application";

static const int vert_scroll_text_padding = 4;

static GRect windowBounds;

static char retrievedNote[1000];

const unsigned int key = 101;

void in_received_handler(DictionaryIterator *received, void *context) {
    //incoming message received
    Tuple *note_tuple = dict_find(received,key);
    if (note_tuple)
    {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: %s", note_tuple->value->cstring);
        text_layer_set_text(text_layer, note_tuple->value->cstring);
        GSize max_size = text_layer_get_content_size(text_layer);
        text_layer_set_size(text_layer, max_size);
        scroll_layer_set_content_size(scroll_layer, GSize(windowBounds.size.w, max_size.h + vert_scroll_text_padding));
        
        //Vibrates short pulse
        vibes_short_pulse();
    }
}

// Setup the scroll layer on window load
// We do this here in order to be able to get the max used text size
static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);
    windowBounds = bounds;
    GRect max_text_bounds = GRect(5, 0, bounds.size.w, 2000);
    
//    persist_read_string(key,scroll_text,500);
//    APP_LOG(APP_LOG_LEVEL_DEBUG, "found: %s", scroll_text);
    
    app_message_register_inbox_received(in_received_handler);
    const uint32_t inbound_size = 128;
    const uint32_t outbound_size = 64;
    app_message_open(inbound_size, outbound_size);
    
    // create and initialize the scroll layer
    scroll_layer = scroll_layer_create(bounds);
    
    //This sets the up and down button to scroll the layer
    scroll_layer_set_click_config_onto_window(scroll_layer, window);
    
    // create and intialize the text layer
    text_layer = text_layer_create(max_text_bounds);
    if (persist_exists(key)) {
        persist_read_string(key, retrievedNote, 1000);
        text_layer_set_text(text_layer, retrievedNote);
    } else {
        text_layer_set_text(text_layer, scroll_text);
    }
    
    //change font
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    
    // Trim text layer and scroll content to fit text box
    GSize max_size = text_layer_get_content_size(text_layer);
    text_layer_set_size(text_layer, max_size);
    scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, max_size.h + vert_scroll_text_padding));
    
    // Add the layers for display
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(text_layer));
    layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));
}

static void window_unload(Window *window) {
    //persist storage
    persist_write_string(key,text_layer_get_text(text_layer));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Text Being Saved: %s", text_layer_get_text(text_layer));
    text_layer_destroy(text_layer);
    scroll_layer_destroy(scroll_layer);
}

int main(void) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true /* Animated */);
    
    app_event_loop();
    
    window_destroy(window);
}
