#define REICH_IMPLEMENTATION
#include "reich.h"

void reich_stress_test_render(reichContext* ctx, real64 alpha) {
    static uint32 s = 0x9E3779B9;
    int32 i;
    int32 sw = ctx->screen.width;
    int32 sh = ctx->screen.height;
    int32 limit = 10000;

    for (i = 0; i < limit; ++i) {
        int32 x1, y1, x2, y2, w, h, type;
        uint32 c;

        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        x1 = (int32)(s % (uint32)(sw * 3)) - sw;
        
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        y1 = (int32)(s % (uint32)(sh * 3)) - sh;
        
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        x2 = (int32)(s % (uint32)(sw * 3)) - sw;
        
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        y2 = (int32)(s % (uint32)(sh * 3)) - sh;

        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        c = s; 

        if ((c >> 24) == 0) c |= 0xFF000000;

        type = i % 4;

        if (type == 0) {
#if 0
            reich_draw_line(ctx, (real32)x1, (real32)y1, (real32)x2, (real32)y2, c);
#endif
        } else if (type == 1) {
            w = (x2 - x1) / 4;
            h = (y2 - y1) / 4;
#if 0
            reich_draw_rect(ctx, x1, y1, w, h, c);
#endif
				} else if (type == 2) {
            w = (x2 - x1) / 4;
            h = (y2 - y1) / 4;
#if 0
            reich_draw_circle_fill(ctx, x1, y1, w, c);
            reich_draw_rect_fill(ctx, x1, y1, w, h, c);
#endif
#if 1
            reich_draw_circle(ctx, x1, y1, w/10, h, c);
#endif
				} else {
#if 0
            reich_draw_pixel_safe(ctx, x1, y1, c);
#endif
        }
    }
}

float my_cool_shape(float x, float y, void* userdata) {
    /* Let's animate it slightly using userdata (optional) */
    float time = *(float*)userdata;
    float box_w = 60.0f + reich_sin(time) * 10.0f;
    
    /* Center the shapes */
    float cx = 400.0f;
    float cy = 300.0f;

    /* Define two primitives */
    float d_box = reich_sd_rounded_box(x - cx, y - cy, box_w, 40.0f, 10.0f);
    float d_circle = reich_sd_circle(x - (cx + 50.0f), y - (cy - 50.0f), 40.0f);

    /* Merge them smoothly! k=20.0f is the blend radius */
    return reich_op_smooth_union(d_box, d_circle, 20.0f);
}

void update(reichContext* ctx) {
}

void input(reichContext* ctx) {
  if (reich_key_pressed(ctx, 0x1B)) { ctx->running = 0; }
}

int32 t_time = 0;

   static float time = 0.0f;
void render(reichContext* ctx, real64 alpha) {
#if 0
	reich_stress_test_render(ctx, alpha);
#endif
    time += 0.05f;

    /* Clear screen */
    reich_draw_clear(ctx, 0xFF202020);

    /* Define the area to update (optimization) */
    reichRect bounds = reich_rect(0, 0, ctx->screen.width, ctx->screen.height);

    /* Draw! 
       color = Red (0xFFE81123)
       bevel = 5.0 (Nice 3D edge effect)
    */
    reich_draw_sdf(ctx, bounds, my_cool_shape, &time, 0xFFE81123, 5.0f);

	t_time++;
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd) {
  reichContext ctx;
  if (!reich_init(&ctx, " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", 800, 600, 60.0)) { return -1; }
  reich_load_fonts(&ctx, "FONT_*", 0, 0, 255);
  ctx.activeFont = 5;
	reich_set_callbacks(&ctx, update, render, input);
  reich_run(&ctx);
  return 0;
}

void mainCRTStartup(void) {
  int result;
  STARTUPINFOA si;
  reich_memset(&si, 0, sizeof(STARTUPINFOA));
  si.cb = sizeof(STARTUPINFOA);
  GetStartupInfoA(&si);
  result = WinMain(
      GetModuleHandleA(NULL),
      NULL,
      GetCommandLineA(),
      (si.dwFlags & STARTF_USESHOWWINDOW) ? si.wShowWindow : SW_SHOWDEFAULT);
  ExitProcess(result);
}
