#define REICH_IMPLEMENTATION
#include "reich.h"

struct app_state_t {
  int32 box_x;
  int32 box_y;
  int32 box_dir_x;
  int32 box_dir_y;
  uint32 box_color;
  int32 show_grid;
} app_state;

static void append_int(char **buffer, int32 value) {
  char temp[16];
  char *t = temp;
  int32 v = value;
  if (v < 0) {
    *(*buffer)++ = '-';
    v = -v;
  }
  if (v == 0) {
    *t++ = '0';
  } else {
    while (v > 0) {
      *t++ = (char)('0' + (v % 10));
      v /= 10;
    }
  }
  while (t > temp) {
    *(*buffer)++ = *--t;
  }
  **buffer = '\0';
}

static void append_str(char **buffer, const char *str) {
  while (*str) {
    *(*buffer)++ = *str++;
  }
  **buffer = '\0';
}

void update(reichContext *ctx) {
  app_state.box_x += app_state.box_dir_x;
  app_state.box_y += app_state.box_dir_y;
  if (app_state.box_x <= 0 || app_state.box_x + 50 >= ctx->screen.width) {
    app_state.box_dir_x *= -1;
  }
  if (app_state.box_y <= REICH_TITLE_BAR_HEIGHT ||
      app_state.box_y + 50 >= ctx->screen.height) {
    app_state.box_dir_y *= -1;
  }
}

void input(reichContext *ctx) {
  if (reich_key_pressed(ctx, 'G')) {
    app_state.show_grid = !app_state.show_grid;
  }

  if (reich_key_pressed(ctx, 0x1B)) {
    ctx->running = 0;
  }
}

void render(reichContext *ctx, real64 alpha) {
  char buf[64];
  char *ptr = buf;
  int32 i;
  int32 mx, my;
  reich_clear(ctx, 0xFF202020);

  if (app_state.show_grid) {
    for (i = 0; i < ctx->screen.width; i += 40) {
      reich_draw_rect(ctx, i, 0, 1, ctx->screen.height, 0xFF303030);
    }
    for (i = 0; i < ctx->screen.height; i += 40) {
      reich_draw_rect(ctx, 0, i, ctx->screen.width, 1, 0xFF303030);
    }
  }

  reich_draw_rect(ctx, app_state.box_x, app_state.box_y, 50, 50,
                  app_state.box_color);
  reich_draw_frame(ctx, app_state.box_x, app_state.box_y, 50, 50, 0xFFFFFFFF);

  reich_draw_rect(ctx, 10, 50, 200, 150, 0xFF404040);
  reich_draw_frame(ctx, 10, 50, 200, 150, 0xFF808080);

  reich_draw_text(ctx, 20, 60, "DEBUG CONSOLE", 0xFFFFFF00);
  reich_draw_text(ctx, 20, 80, "FPS: 60 (Fixed)", 0xFFAAAAAA);

  mx = reich_get_mouse_x(ctx);
  my = reich_get_mouse_y(ctx);

  ptr = buf;
  append_str(&ptr, "Mouse: ");
  append_int(&ptr, mx);
  append_str(&ptr, ", ");
  append_int(&ptr, my);

  reich_draw_text(ctx, 20, 95, buf, 0xFFFFFFFF);

  if (reich_ui_btn(ctx, 1, 20, 120, 180, 25, "Reset Box", 0xFF606060)) {
    app_state.box_x = 100;
    app_state.box_y = 100;
  }

  if (reich_ui_btn(ctx, 2, 20, 155, 180, 25, "Toggle Color", 0xFF804040)) {
    if (app_state.box_color == 0xFFFF0000)
      app_state.box_color = 0xFF0000FF;
    else
      app_state.box_color = 0xFFFF0000;
  }

  reich_draw_rect(ctx, mx - 10, my, 20, 1, 0xFF00FF00);
  reich_draw_rect(ctx, mx, my - 10, 1, 20, 0xFF00FF00);
  reich_draw_text(ctx, 10, ctx->screen.height - 20,
                  "Press 'G' to toggle grid. ESC to quit.", 0xFF888888);
}

void resize(reichContext *ctx, int32 w, int32 h) {
  (void)ctx;
  (void)w;
  (void)h;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd) {
  reichContext ctx;
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nShowCmd;
  app_state.box_x = 100;
  app_state.box_y = 100;
  app_state.box_dir_x = 4;
  app_state.box_dir_y = 4;
  app_state.box_color = 0xFFFF0000;
  app_state.show_grid = 1;

  if (!reich_init(&ctx, "Reich Debug App", 800, 600, 60.0)) {
    return -1;
  }
  reich_load_fonts(&ctx, "FONT_11X11_TER.bmp", 11, 11, 255);
  ctx.activeFont = 1;
  reich_set_callbacks(&ctx, update, render, resize, input);
  reich_run(&ctx);

  return 0;
}
