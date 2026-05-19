// #include "core/timer.h"
#include "notification/notification.h"
#include <furi.h>
#include <furi_hal.h>

#include "core/timer.h"
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include "orgasmotron2_icons.h"

/*
 * PHASE SYSTEM:
 * Each mode now uses phases. Each phase lasts a certain amount of time, before either advancing to the next one or looping back to phase zero.
 * For now, the most phases a mode will have will be two.
 * I plan to implement a mode editor, wherein you can create a sequence of phases and bind it to a slot.
 * Why? Because i hate myself and i am a glutton for punishment.
 *
 * PHASE ANATOMY:
 * Duration - how long to stay in the phase (in ms)
 * Vibro - Boolean, determines if the vibro is on or off during the phase
 *
 * MODE ANATOMY:
 * Phase count - How many phases there are, duh.
 * Phases - An array of phases, maximum of 32.
 * This way, modes can be stored and easily modified. Any unused phase slots will have junk data ( duration 0, vibro false ).
 * These phases will never be invoked if the phase count is accurate, as it will just loop around when it reaches it.
*/
// typedef enum {
//     EventTypeTick,
//     EventTypeKey,
// } EventType;
//
// typedef struct {
//     EventType type;
//     InputEvent input;
// } PluginEvent;

typedef struct {
    int duration;
    bool vibro;
} Phase;

typedef struct {
    int count;
    Phase phases[32];
} Mode;


typedef struct {
    int mode; // index of current mode
    Mode modes[5]; // list of modes
    int animation; // animation frame index
    int phase; // current phase index
    int phase_timer; // time left in current phase
    bool update_screen;
    NotificationApp* notification;
    FuriMutex* mutex;
} PluginState;


static const Mode off = {
  .count = 1,
  .phases[0] = (Phase){1, false}
};

static const Mode strong = {
  .count = 1,
  .phases[0] = (Phase){1, true}
};
static const Mode medium = {
    .count = 2,
    .phases[0] = (Phase){ 10, true },
    .phases[1] = (Phase){ 5, false }
};
static const Mode soft = {
    .count = 2,
    .phases[0] = (Phase){ 10, true },
    .phases[1] = (Phase){ 20, false }
};
static const Mode pulsed = {
    .count = 2,
    .phases[0] = (Phase){ 5, true },
    .phases[1] = (Phase){ 1, false }
};

Mode get_current_mode(PluginState* plugin_state) {
    return plugin_state->modes[plugin_state->mode];
}

Phase get_current_phase(PluginState* plugin_state) {
    return plugin_state->modes[plugin_state->mode].phases[plugin_state->phase];
}


// static void render_awake(Canvas* canvas, void* ctx) {
//     const PluginState* plugin_state = ctx;
//     if (plugin_state->animation == 0) {
//         canvas_draw_icon(canvas, 30, 5, &I_eye_1_18x14);
//         canvas_draw_icon(canvas, 32, 40, &I_fin_1_20x21);
//         canvas_draw_icon(canvas, 1, 6, &I_nose_1_28x18);
//     } else if(plugin_state->animation == 1) {
//         canvas_draw_icon(canvas, 30, 5, &I_eye_2_18x14);
//         canvas_draw_icon(canvas, 32, 40, &I_fin_2_20x21);
//         canvas_draw_icon(canvas, 1, 6, &I_nose_2_28x18);
//     } else if(plugin_state->animation == 2) {
//         canvas_draw_icon(canvas, 30, 5, &I_eye_3_18x14);
//         canvas_draw_icon(canvas, 32, 40, &I_fin_3_20x21);
//         canvas_draw_icon(canvas, 1, 6, &I_nose_3_28x18);
//     }
// }

static void timer_callback(void* ctx) {
    PluginState* plugin_state = (PluginState*)ctx;
    if (!plugin_state) return;
    if (plugin_state->mode > 2) { // If, off or on strong ( not always off or always on ), ignore timer
        return;
    }
    if (plugin_state->mode != 4) {
        plugin_state->animation = 0;
        // if (plugin_state->animation > 2) {
        //     plugin_state->animation = 0;
        // }
    }
    plugin_state->phase_timer--;

    if (plugin_state->phase_timer <= 0) {

        plugin_state->phase++;
        if (plugin_state->phase == get_current_mode(plugin_state).count) {
            plugin_state->phase = 0;
        }
        plugin_state->phase_timer = get_current_phase(plugin_state).duration;

        if (get_current_phase(plugin_state).vibro) {
            notification_message(plugin_state->notification, &sequence_set_vibro_on);
        } else {
            notification_message(plugin_state->notification, &sequence_reset_vibro);
        }

        // FURI_LOG_D("orgasmotron", "Vibro: %b", get_current_phase(plugin_state).vibro);
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    // furi_assert(ctx);
    PluginState* plugin_state = (PluginState*)ctx;
    if (!plugin_state->update_screen) {
        return;
    }
    // furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);
    // if (!plugin_state->update_screen) {
        // canvas_commit(canvas);
        // return;
    // }
    canvas_clear(canvas);
/*
    plugin_state->update_screen = false;
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, 0, 0, &I_ui_128x64);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_icon(canvas, 56, 37, &I_left_4x7);
    canvas_draw_str(canvas, 61, 44, "Strong");
    canvas_draw_str(canvas, 97, 44, "Soft");
    canvas_draw_icon(canvas, 117, 37, &I_right_4x7);
    canvas_draw_str(canvas, 76, 33, "Pulsed");
    canvas_draw_icon(canvas, 85, 21, &I_up_7x4);
    canvas_draw_str(canvas, 76, 56, "Medium");
    canvas_draw_icon(canvas, 85, 57, &I_down_7x4);
    canvas_draw_str(canvas, 38, 39, "Off");
    canvas_draw_icon(canvas, 38, 24, &I_center_7x7);
    furi_mutex_release(plugin_state->mutex);

    canvas_set_color(canvas, ColorXOR);*/
    switch (plugin_state->mode) {
        case 0:
            canvas_draw_icon(canvas, 0, 0, &I_pulsed_128x64);
            // canvas_draw_box(canvas, 74, 20, 30, 16);
            // canvas_invert_color(canvas);
            // canvas_draw_str(canvas, 76, 33, "Pulsed");
            // canvas_draw_icon(canvas, 85, 21, &I_up_7x4);
            // canvas_invert_color(canvas);
            // render_awake(canvas, ctx);
            break;
        case 1:
            canvas_draw_icon(canvas, 0, 0, &I_medium_128x64);
            // canvas_draw_box(canvas, 74, 47, 32, 16);
            // canvas_invert_color(canvas);
            // canvas_draw_str(canvas, 76, 56, "Medium");
            // canvas_draw_icon(canvas, 85, 57, &I_down_7x4);
            // canvas_invert_color(canvas);
            // render_awake(canvas, ctx);
            break;
        case 2:
            canvas_draw_icon(canvas, 0, 0, &I_soft_128x64);
            // canvas_draw_box(canvas, 95, 36, 28, 10);
            // canvas_invert_color(canvas);
            // canvas_draw_str(canvas, 97, 44, "Soft");
            // canvas_draw_icon(canvas, 117, 37, &I_right_4x7);
            // canvas_invert_color(canvas);
            // render_awake(canvas, ctx);
            break;
        case 3:
            canvas_draw_icon(canvas, 0, 0, &I_strong_128x64);
            // canvas_draw_box(canvas, 55, 36, 36, 10);
            // canvas_invert_color(canvas);
            // canvas_draw_str(canvas, 61, 44, "Strong");
            // canvas_draw_icon(canvas, 56, 37, &I_left_4x7);
            // canvas_invert_color(canvas);
            // render_awake(canvas, ctx);
            break;
        case 4:
            canvas_draw_icon(canvas, 0, 0, &I_off_128x64);
            // canvas_draw_icon(canvas, 30, 5, &I_eye_closed_18x14);
            // canvas_draw_box(canvas, 36, 23, 15, 18);
            // canvas_invert_color(canvas);
            // canvas_draw_str(canvas, 38, 39, "Off");
            // canvas_draw_icon(canvas, 38, 24, &I_center_7x7);
            // canvas_invert_color(canvas);
            break;
        default:
            break;
    }
    // canvas_commit(canvas);

}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
    // FuriMessageQueue* event_queue = ctx;
    // PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    // furi_message_queue_put(event_queue, &event, FuriWaitForever);
}
/*
 *         Pulsed
 * Strong          Soft
 *         Medium
 * OK = Off
 *
 * INPUT ENUM DETAILS:
 * 0 - Up
 * 1 - Down
 * 2 - Right
 * 3 - Left
 * 4 - Ok
 * 5 - Back
 *
*/
bool try_mode_switch(PluginState* plugin_state, InputKey key) {
    bool switched = false;
    if (plugin_state->mode != (int)key) { // If the key pressed isn't the same as the current mode
        if (key == InputKeyOk) {
            plugin_state->mode = 4;
            switched = true;
        }

        // No switching from one mode directly to another, must turn off first.
        // This is to prevent accidentally switching modes during "action,"
        // because the OK button needs a lot more force to click.
        if (plugin_state->mode == 4) {
            plugin_state->mode = (int)key;
            switched = true;
        }
    }
    return switched;
}


bool handle_input(PluginState* plugin_state, FuriTimer* timer, FuriStatus event_status, InputEvent event) {
    bool processing = true;
    furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);
    if (event_status != FuriStatusOk) {
        // FURI_LOG_D("orgasmotron", "FuriMessageQueue: event timeout - in handle_input");
        return processing;
    } /*else {
        FURI_LOG_D("orgasmotron", "FuriMessageQueue: Handling input");
    }*/

    if (event.type == InputTypePress) {
        // FURI_LOG_D("orgasmotron", "Input type press");
        switch(event.key) {
            case InputKeyUp:
            case InputKeyDown:
            case InputKeyLeft:
            case InputKeyRight:
            case InputKeyOk:
                if (event.key != InputKeyOk) {
                    furi_timer_start(timer, 10);
                } else {
                    furi_timer_stop(timer);
                }
                bool switched = try_mode_switch(plugin_state, event.key);
                if (switched) {
                    // FURI_LOG_D("orgasmotron", "Switched is true");
                    plugin_state->phase = 0; // Reset phase
                    plugin_state->phase_timer = get_current_phase(plugin_state).duration; // Reset timer
                    // FURI_LOG_D("orgasmotron", "Phase 0, duration: %d", get_current_phase(plugin_state).duration);
                    if (get_current_phase(plugin_state).vibro) {
                        notification_message(plugin_state->notification, &sequence_set_vibro_on);
                    } else {
                        notification_message(plugin_state->notification, &sequence_reset_vibro);
                    }
                }
                break;
            default:
                break;
        }
    } else if (event.key == InputKeyBack && event.type == InputTypeShort) {
        notification_message(plugin_state->notification, &sequence_reset_vibro);
        notification_message(plugin_state->notification, &sequence_reset_green);
        furi_timer_stop(timer);
        processing = false;
    } else if (event.key == InputKeyBack && event.type == InputTypeLong) {
        plugin_state->update_screen = !plugin_state->update_screen;
    }/*else {
        FURI_LOG_D("orgasmotron", "Other Input Type: %s", input_get_type_name(event.type));
        FURI_LOG_D("orgasmotron", "Input key: %s", input_get_key_name(event.key));
    }*/

    return processing;

}

int32_t orgasmotron_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));

    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, (void*) plugin_state);
    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    plugin_state->notification = furi_record_open(RECORD_NOTIFICATION);
    if(!plugin_state->mutex) {
        FURI_LOG_E("Orgasmatron", "cannot create mutex\r\n");
        free(plugin_state);
        return 255;
    }

    plugin_state->animation = 0;
    plugin_state->mode = 4;
    plugin_state->update_screen = true;
    plugin_state->modes[0] = pulsed;
    plugin_state->modes[1] = medium;
    plugin_state->modes[2] = soft;
    plugin_state->modes[3] = strong;
    plugin_state->modes[4] = off;
    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, plugin_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // plugin_state->notification = furi_record_open(RECORD_NOTIFICATION);

    InputEvent event;
    notification_message(plugin_state->notification, &sequence_display_backlight_on);

    for( bool processing = true; processing; ) {
        // if (plugin_state->animation > 2) {
        //     plugin_state->animation = 0;
        // }
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        processing = handle_input(plugin_state, timer, event_status, event);
        // view_port_update(view_port);
        furi_mutex_release(plugin_state->mutex);
    }
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_mutex_free(plugin_state->mutex);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    free(plugin_state);
    return 0;
}
