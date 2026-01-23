#define TB_IMPLEMENTATION
#include "tb.h"
#include <math.h>
#include <stdio.h>

/* Simple helper for PI if not defined */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void) {
    int w, h;
    int x, y;
    int r, g, b;
    int frame_counter = 0;
    char buffer[128];

    /* Audio state */
    float phase = 0.0f;
    float frequency = 440.0f;
    float audio_buffer[1024];
    int samples_to_write;
    int i;

    /* Initialize Termbox with auto-detected size (0, 0) */
    if (!tb_init(0, 0, "Termbox Win32 Feature Test")) {
        fprintf(stderr, "Failed to initialize termbox\n");
        return 1;
    }

    /* Initialize Audio at 44.1kHz */
    tb_audio_init(44100);

    while (tb_update()) {
        /* Input Handling */
        if (tb_key_pressed(TB_KEY_ESCAPE)) {
            break;
        }

        w = tb_width();
        h = tb_height();

        /* 1. Audio Synthesis Logic
           Generate a sine wave based on current frequency */
        samples_to_write = tb_audio_free_space();

        /* Cap chunk size to our local buffer */
        if (samples_to_write > 1024) samples_to_write = 1024;

        if (samples_to_write > 0) {
            float phase_inc = (2.0f * (float)M_PI * frequency) / 44100.0f;

            for (i = 0; i < samples_to_write; ++i) {
                audio_buffer[i] = (float)sin(phase) * 0.1f; /* 10% volume */
                phase += phase_inc;
                if (phase > 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
            }
            tb_audio_push(audio_buffer, samples_to_write);
        }

        /* 2. Rendering Logic
           Clear screen is implicit if we overwrite everything, but good practice */
        tb_clear(TB_RGB(0, 0, 0), TB_RGB(0, 0, 0), ' ');

        /* Draw Full Gamut Gradient
           X axis = Red, Y axis = Green, Time = Blue */
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                /* Calculate colors */
                r = (x * 255) / (w > 0 ? w : 1);
                g = (y * 255) / (h > 0 ? h : 1);
                b = (frame_counter * 2) % 256;

                /* Create a bouncing effect for Blue to avoid hard snapping */
                if (((frame_counter * 2) / 256) % 2 == 1) {
                    b = 255 - b;
                }

                /* We use a block character or similar to make it look solid,
                   or just tint the background of a space */
                tb_put(x, y, ' ', TB_RGB(255, 255, 255), TB_RGB(r, g, b));
            }
        }

        /* 3. Text & Mouse Info Overlay */
        {
            tb_mouse_state_t mouse = tb_mouse_state();

            /* Update frequency based on mouse X position (200Hz to 1000Hz) */
            if (w > 0) {
                frequency = 200.0f + ((float)mouse.x / (float)w) * 800.0f;
            }

            sprintf(buffer, " [ Termbox Win32 Library Test ] ");
            tb_print(2, 1, buffer, TB_RGB(255, 255, 255), TB_RGB(0, 0, 0));

            sprintf(buffer, " Resolution: %dx%d | Frame: %d ", w, h, frame_counter);
            tb_print(2, 2, buffer, TB_RGB(200, 200, 200), TB_RGB(0, 0, 0));

            sprintf(buffer, " Mouse: %d,%d | Btn: %s%s%s ",
                mouse.x, mouse.y,
                mouse.left_down ? "L" : "-",
                mouse.middle_down ? "M" : "-",
                mouse.right_down ? "R" : "-");
            tb_print(2, 3, buffer, TB_RGB(200, 200, 200), TB_RGB(0, 0, 0));

            sprintf(buffer, " Audio Freq: %.2fHz (Move mouse X to change) ", frequency);
            tb_print(2, 4, buffer, TB_RGB(100, 255, 100), TB_RGB(0, 0, 0));

            tb_print(2, h - 2, " PRESS ESC TO EXIT ", TB_RGB(255, 50, 50), TB_RGB(20, 0, 0));
        }

        tb_present();

        frame_counter++;
        tb_sleep(16); /* Cap at ~60 FPS */
    }

    tb_shutdown();
    return 0;
}
