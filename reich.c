#define REICH_IMPLEMENTATION
#include "reich.h"

void update(reichContext* ctx) {
}

void input(reichContext* ctx) {
  if (reich_key_pressed(ctx, 0x1B)) { ctx->running = 0; }
}

int32 t_time = 0;

void render(reichContext* ctx, real64 alpha) {
  reich_draw_clear(ctx, 0xFF202020);
	reich_draw_grid(ctx, 0, 0, ctx->screen.width, ctx->screen.height, t_time, t_time, 32, 0x020202);
	t_time++;
	Sleep(100);
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd) {
  reichContext ctx;
  if (!reich_init(&ctx, "Reich Debug App", 800, 600, 60.0)) { return -1; }
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
