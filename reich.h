#define REICH_IMPLEMENTATION
#ifndef REICH_H
#define REICH_H

#include <math.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __linux__
#define REICH_PLATFORM_LINUX
#elif _WIN32
#define REICH_PLATFORM_WIN32
#endif

#if defined(REICH_PLATFORM_WIN32)
#if defined(REICH_EXPORTS)
#define REICH_API __declspec(dllexport)
#elif defined(REICH_IMPORTS)
#define REICH_API __declspec(dllimport)
#else
#define REICH_API
#endif
#else
#define REICH_API
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef signed char int8;
typedef short int16;
typedef int int32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef float real32;
typedef double real64;

#if defined(REICH_PLATFORM_WIN32)
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

typedef unsigned long reichSize;
typedef int32 reichBool;
typedef void* reichHandle;

#define REICH_FILE_READ  1
#define REICH_FILE_WRITE 2
#define REICH_SEEK_SET   0
#define REICH_SEEK_CUR   1
#define REICH_SEEK_END   2

#define REICH_LOG_DEBUG 0
#define REICH_LOG_INFO  1
#define REICH_LOG_WARN  2
#define REICH_LOG_ERROR 3

#define REICH_KEY_MAX       512
#define REICH_MOUSE_BUTTONS 3
#define REICH_MAX_FONTS     32

typedef struct reichRect {
  int32 x1, y1, x2, y2;
} reichRect;

typedef struct reichArena {
  uint8* base;
  reichSize size;
  reichSize used;
} reichArena;

typedef struct reichCanvas {
  int32 width;
  int32 height;
  uint32* pixels;
} reichCanvas;

typedef struct reichInput {
  int32 mouseX, mouseY;
  int32 lastMouseX, lastMouseY;
  int32 deltaX, deltaY;
  int32 mouseWheel;
  uint8 buttons[REICH_MOUSE_BUTTONS];
  uint8 lastButtons[REICH_MOUSE_BUTTONS];
  uint8 buttonsPressed[REICH_MOUSE_BUTTONS];
  uint8 buttonsReleased[REICH_MOUSE_BUTTONS];
  uint8 keys[REICH_KEY_MAX];
  uint8 lastKeys[REICH_KEY_MAX];
  uint8 keysPressed[REICH_KEY_MAX];
  uint8 keysReleased[REICH_KEY_MAX];
  uint32 lastChar;
  int32 activeId, hotId;
} reichInput;

typedef struct reichContext reichContext;

typedef int32 (*PFUSERUPDATE)(reichContext* ctx);
typedef int32 (*PFUSERRENDER)(reichContext* ctx, real64 alpha);
typedef int32 (*PFUSERINPUT)(reichContext* ctx);

typedef int32 (*PFDECOTITLEBAR)(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h);
typedef int32 (*PFDECOBUTTON)(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h, int32 isHovered);
typedef int32 (*PFDECOBUTTONMAX)(
    reichContext* ctx,
    int32 x,
    int32 y,
    int32 w,
    int32 h,
    int32 isHovered,
    int32 isMaximized);

struct reichContext {
  reichArena permMem;
  reichArena frameMem;
  reichCanvas canvas;
  reichRect clip;
  reichRect activeDirty;

  reichInput input;
  const char* windowTitle;
  int32 windowWidth;
  int32 windowHeight;
  int32 running;
  int32 isMaximized;
  int32 scale;

  int64 perfFreq;
  int64 lastCounter;
  real64 accumulator;
  real64 fixedDt;
  real64 alpha;

  /* Window Decoration Theme */
  int32 themeTitleBarHeight;
  int32 themeButtonWidth;
  uint32 themeHoverColor;
  uint32 themeCloseHoverColor;
  uint32 themeIconColor;

  PFDECOTITLEBAR renderTitleBar;
  PFDECOBUTTON renderBtnMin;
  PFDECOBUTTONMAX renderBtnMax;
  PFDECOBUTTON renderBtnClose;

  uint8* fonts[REICH_MAX_FONTS];
  int32 fontCount;
  int32 activeFont;

  PFUSERUPDATE userUpdate;
  PFUSERRENDER userRender;
  PFUSERINPUT userInput;

  void* platform;
};

REICH_API int32 reich_sys_file_close(reichHandle file);
REICH_API int32 reich_sys_file_tell(reichHandle file);
REICH_API int32 reich_sys_find_close(reichHandle handle);
REICH_API reichSize reich_sys_file_size(reichHandle file);
REICH_API int32 reich_sys_log(int32 level, const char* format, ...);
REICH_API reichHandle reich_sys_file_open(const char* filename, int32 mode);
REICH_API int32
reich_sys_file_seek(reichHandle file, int32 offset, int32 origin);
REICH_API reichSize
reich_sys_file_read(reichHandle file, void* buffer, reichSize bytes);
REICH_API reichSize
reich_sys_file_write(reichHandle file, const void* buffer, reichSize bytes);
REICH_API reichHandle reich_sys_find_first(
    const char* pattern, char* filenameBuffer, int32 bufferSize);
REICH_API int32 reich_sys_find_next(
    reichHandle handle, char* filenameBuffer, int32 bufferSize);

REICH_API int32 reich_sys_show_cursor(void);
REICH_API int32 reich_sys_hide_cursor(void);
REICH_API int32 reich_sys_toggle_maximize(reichContext* ctx);
REICH_API reichSize reich_sys_get_platform_data_size(void);
REICH_API int32 reich_sys_poll_events(reichContext* ctx);
REICH_API int32 reich_sys_minimize(reichContext* ctx);
REICH_API int32 reich_sys_present(reichContext* ctx);
REICH_API int32 reich_sys_close(reichContext* ctx);
REICH_API void* reich_sys_alloc(reichSize size);
REICH_API int64 reich_sys_get_ticks(void);
REICH_API int64 reich_sys_get_freq(void);
REICH_API int32 reich_sys_free(void* ptr);

REICH_API int32
reich_sys_resize_canvas(reichContext* ctx, int32 width, int32 height);
REICH_API int32 reich_sys_window_init(
    reichContext* ctx, const char* title, int32 width, int32 height);

REICH_API int32 reich_init(
    reichContext* ctx,
    const char* title,
    int32 width,
    int32 height,
    real64 targetFps);
REICH_API int32 reich_set_callbacks(
    reichContext* ctx,
    PFUSERUPDATE update,
    PFUSERRENDER render,
    PFUSERINPUT input);
REICH_API int32 reich_run(reichContext* ctx);
REICH_API int32 reich_begin_frame(reichContext* ctx);
REICH_API int32 reich_end_frame(reichContext* ctx);

REICH_API int32 reich_timer_tick(reichContext* ctx);
REICH_API int32 reich_timer_step(reichContext* ctx);
REICH_API real64 reich_timer_alpha(reichContext* ctx);

REICH_API int32 reich_input_update(reichContext* ctx);
REICH_API int32 reich_mouse_x(reichContext* ctx);
REICH_API int32 reich_mouse_y(reichContext* ctx);
REICH_API int32 reich_mouse_wheel(reichContext* ctx);
REICH_API uint32 reich_get_char_pressed(reichContext* ctx);
REICH_API int32 reich_key_down(reichContext* ctx, int32 keyCode);
REICH_API int32 reich_mouse_down(reichContext* ctx, int32 button);
REICH_API int32 reich_key_pressed(reichContext* ctx, int32 keyCode);
REICH_API int32 reich_key_released(reichContext* ctx, int32 keyCode);
REICH_API int32 reich_mouse_pressed(reichContext* ctx, int32 button);
REICH_API int32 reich_mouse_released(reichContext* ctx, int32 button);

REICH_API void* reich_arena_alloc(reichArena* a, reichSize size);
REICH_API int32 reich_arena_init(reichArena* a, void* mem, reichSize size);
REICH_API int32 reich_arena_reset(reichArena* a);

REICH_API int32 reich_set_scale(reichContext* ctx, int32 scale);
REICH_API int32 reich_draw_decorations(reichContext* ctx);
REICH_API int32 reich_default_render_titlebar(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h);
REICH_API int32 reich_default_render_btn_close(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h, int32 isHovered);
REICH_API int32 reich_default_render_btn_max(
    reichContext* ctx,
    int32 x,
    int32 y,
    int32 w,
    int32 h,
    int32 isHovered,
    int32 isMaximized);
REICH_API int32 reich_default_render_btn_min(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h, int32 isHovered);

REICH_API reichCanvas reich_load_bmp(const char* filename);
REICH_API int32 reich_init_default_font(reichContext* ctx);
REICH_API uint8* reich_font_import(
    reichArena* a,
    uint32* pixels,
    int32 iw,
    int32 ih,
    int32 gw,
    int32 gh,
    int32 start,
    int32 end);
REICH_API int32 reich_load_fonts(
    reichContext* ctx, const char* fn, int32 gw, int32 gh, int32 amt);
REICH_API int32 reich_load_font(
    reichContext* ctx,
    const char* filename,
    int32 numChars,
    int32 glyphWidth,
    int32 glyphHeight);

#define reich_bounds_check(_x, _y, _sx, _sy, _ex, _ey) \
  ((_x) >= (_sx) && (_x) < (_ex) && (_y) >= (_sy) && (_y) < (_ey))
REICH_API uint8 reich_rgba_channel(uint32 col, int8 channel);
REICH_API float reich_sin(float x);
REICH_API float reich_cos(float x);
REICH_API double reich_sqrt(double n);
REICH_API int32 reich_floor(real32 x);
REICH_API int32 reich_ceil(real32 x);
REICH_API int64 reich_floord(real64 x);
REICH_API int64 reich_ceild(real64 x);
REICH_API reichRect reich_rect(int32 x1, int32 y1, int32 x2, int32 y2);
REICH_API int32 reich_rect_bounds(
    int32 x, int32 y, int32 minX, int32 minY, int32 maxX, int32 maxY);

REICH_API int32
reich_draw_pixel(reichContext* ctx, int32 x, int32 y, uint32 color);
REICH_API int32 reich_draw_clear(reichContext* ctx, uint32 color);
REICH_API int32 reich_draw_line(
    reichContext* ctx,
    real32 x1,
    real32 y1,
    real32 x2,
    real32 y2,
    uint32 colour);
REICH_API int32 reich_draw_line_thick(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float thickness,
    uint32 color);
REICH_API int32 reich_draw_rect_fill(
    reichContext* ctx, float x, float y, float w, float h, uint32 color);
REICH_API int32 reich_draw_rect(
    reichContext* ctx, float x, float y, float w, float h, uint32 color);
REICH_API int32 reich_draw_circle_fill(
    reichContext* ctx, float cx, float cy, float r, uint32 color);
REICH_API int32 reich_draw_circle(
    reichContext* ctx, float cx, float cy, float r, float t, uint32 color);
REICH_API int32 reich_draw_text(
    reichContext* ctx, int32 x, int32 y, const char* str, uint32 color);
REICH_API int32 reich_draw_rect_rounded(
    reichContext* ctx,
    float x,
    float y,
    float w,
    float h,
    float r,
    float thickness,
    uint32 color);
REICH_API int32 reich_draw_rect_gradient(
    reichContext* ctx,
    float x,
    float y,
    float w,
    float h,
    uint32 tl,
    uint32 tr,
    uint32 bl,
    uint32 br);
REICH_API int32 reich_draw_triangle(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    uint32 color);
REICH_API int32 reich_draw_triangle_fill(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    uint32 color);
REICH_API int32 reich_draw_ellipse(
    reichContext* ctx,
    float cx,
    float cy,
    float rx,
    float ry,
    float thickness,
    uint32 color);
REICH_API int32 reich_draw_ellipse_fill(
    reichContext* ctx, float cx, float cy, float rx, float ry, uint32 color);
REICH_API int32 reich_draw_bezier_quad(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    int32 segments,
    float thickness,
    uint32 color);
REICH_API int32 reich_draw_bezier_cubic(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4,
    int32 segments,
    float thickness,
    uint32 color);

REICH_API reichCanvas reich_canvas_create(reichArena* arena, int32 w, int32 h);
REICH_API int32
reich_draw_canvas(reichContext* ctx, reichCanvas* src, int32 x, int32 y);
REICH_API int32 reich_draw_canvas_scaled(
    reichContext* ctx, reichCanvas* src, int32 x, int32 y, int32 w, int32 h);

REICH_API uint32 reich_color_lerp(uint32 c1, uint32 c2, float t);

#ifdef REICH_IMPLEMENTATION

#define REICH_DIV255(x) (((x) + ((x) >> 8) + 1) >> 8)

static const uint8 REICH_FONT_DATA[] = {
    0x05, 0x06, 0x01, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x03, 0x8A, 0x03, 0x00,
    0x00, 0x28, 0x00, 0x00, 0x01, 0xBF, 0xB8, 0x80, 0x23, 0x3C, 0xC4, 0x00,
    0x8E, 0xD9, 0x1C, 0x02, 0x3B, 0xE4, 0x70, 0x00, 0xCD, 0x7B, 0xC0, 0x74,
    0x63, 0x17, 0x00, 0x0E, 0x53, 0x80, 0x0F, 0xC6, 0xB1, 0xF8, 0x04, 0x66,
    0x4A, 0x4C, 0x64, 0xA4, 0xCC, 0x41, 0x8E, 0x56, 0x30, 0x0F, 0x4A, 0x52,
    0xF4, 0xA4, 0xCF, 0x32, 0x40, 0x04, 0x39, 0xEE, 0x40, 0x02, 0x77, 0x9C,
    0x22, 0x38, 0x84, 0x71, 0x14, 0x05, 0x29, 0x40, 0x76, 0x94, 0xA5, 0x01,
    0x92, 0x41, 0x30, 0x00, 0x01, 0x9A, 0x60, 0x08, 0xE7, 0x7C, 0x9F, 0x23,
    0x88, 0x42, 0x00, 0x84, 0x23, 0x88, 0x00, 0x13, 0xC4, 0x00, 0x00, 0x8F,
    0x20, 0x00, 0x00, 0x31, 0xE0, 0x00, 0x0A, 0xFA, 0x80, 0x00, 0x08, 0xCE,
    0xF0, 0x01, 0xEE, 0x62, 0x00, 0x00, 0x00, 0x00, 0x01, 0x08, 0x40, 0x10,
    0x05, 0x28, 0x00, 0x00, 0x15, 0xF5, 0x7D, 0x40, 0x75, 0x18, 0xAE, 0x20,
    0x12, 0x22, 0x24, 0x06, 0x49, 0x12, 0x62, 0x00, 0x00, 0x00, 0x00, 0x22,
    0x10, 0x82, 0x01, 0x04, 0x21, 0x10, 0x00, 0x51, 0x14, 0x00, 0x00, 0x8E,
    0x20, 0x00, 0x00, 0x00, 0x04, 0x40, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x00, 0x22, 0x22, 0x00, 0x64, 0xA5, 0x26, 0x00, 0x8C, 0x21, 0x1C,
    0x0E, 0x09, 0x90, 0xF0, 0x19, 0x22, 0x49, 0x80, 0x32, 0xA5, 0xE1, 0x03,
    0xD0, 0xE0, 0xB8, 0x07, 0x43, 0x92, 0x70, 0x3C, 0x22, 0x21, 0x00, 0x64,
    0x99, 0x26, 0x01, 0x92, 0x70, 0xB8, 0x00, 0x20, 0x08, 0x00, 0x00, 0x80,
    0x21, 0x00, 0x22, 0x20, 0x82, 0x00, 0x1E, 0x07, 0x80, 0x08, 0x20, 0x88,
    0x80, 0x38, 0x26, 0x01, 0x00, 0x57, 0x95, 0xEE, 0x01, 0x92, 0x97, 0xA4,
    0x0E, 0x4B, 0x92, 0xE0, 0x1D, 0x08, 0x41, 0xC0, 0xC5, 0x25, 0x2E, 0x03,
    0xD0, 0xE4, 0x3C, 0x0F, 0x43, 0x90, 0x80, 0x1D, 0x0B, 0x49, 0xC0, 0x94,
    0xBD, 0x29, 0x03, 0x88, 0x42, 0x38, 0x01, 0x0A, 0x52, 0x60, 0x25, 0x4C,
    0x52, 0x40, 0x84, 0x21, 0x0F, 0x02, 0x5E, 0x94, 0xA4, 0x09, 0x6A, 0xD2,
    0x90, 0x19, 0x29, 0x49, 0x80, 0xE4, 0xA5, 0xC8, 0x01, 0x92, 0x95, 0x9C,
    0x0E, 0x4B, 0x92, 0x90, 0x1D, 0x06, 0x0B, 0x80, 0xF2, 0x10, 0x84, 0x02,
    0x52, 0x94, 0x98, 0x09, 0x4A, 0x54, 0x40, 0x25, 0x29, 0x7A, 0x40, 0x94,
    0x99, 0x29, 0x02, 0x52, 0x70, 0xB8, 0x0F, 0x09, 0x90, 0xF0, 0x18, 0x84,
    0x21, 0x80, 0x04, 0x10, 0x41, 0x01, 0x84, 0x21, 0x18, 0x06, 0x48, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1E, 0x82, 0x00, 0x00, 0x00, 0x0E, 0x94, 0x9C,
    0x08, 0x72, 0x52, 0xE0, 0x00, 0xE8, 0x41, 0xC0, 0x13, 0xA5, 0x27, 0x00,
    0x0C, 0x95, 0x1C, 0x06, 0x43, 0x90, 0x80, 0x00, 0xC9, 0x38, 0x5C, 0x84,
    0x39, 0x29, 0x01, 0x00, 0x42, 0x38, 0x02, 0x00, 0x84, 0x26, 0x21, 0x09,
    0x72, 0x40, 0xC2, 0x10, 0x8E, 0x00, 0x12, 0xF4, 0xA4, 0x00, 0x72, 0x52,
    0x90, 0x00, 0xC9, 0x49, 0x80, 0x07, 0x25, 0x2E, 0x40, 0x0E, 0x94, 0x9C,
    0x20, 0x72, 0x50, 0x80, 0x00, 0xEC, 0x1B, 0x80, 0x47, 0x10, 0x84, 0x00,
    0x12, 0x94, 0x9C, 0x00, 0x4A, 0x4A, 0x20, 0x01, 0x29, 0x7A, 0x40, 0x04,
    0x99, 0x29, 0x00, 0x12, 0x93, 0x85, 0xC0, 0x78, 0x88, 0xF0, 0x08, 0x8C,
    0x20, 0x80, 0x42, 0x00, 0x84, 0x02, 0x08, 0x62, 0x20, 0x00, 0x22, 0xA2,
    0x00, 0x00, 0x45, 0x4B, 0xC0, 0xAB, 0xB6, 0xEA, 0x82, 0x40, 0x94, 0x9C,
    0x02, 0x23, 0xD4, 0xF0, 0x38, 0x07, 0x49, 0xC0, 0xA0, 0x1D, 0x27, 0x02,
    0x08, 0x74, 0x9C, 0x06, 0x01, 0xD2, 0x70, 0x00, 0xE8, 0x41, 0xC8, 0xE0,
    0x3D, 0x4F, 0x02, 0x80, 0xF5, 0x3C, 0x08, 0x23, 0xD4, 0xF0, 0x28, 0x04,
    0x23, 0x80, 0xE0, 0x10, 0x8E, 0x02, 0x08, 0x42, 0x38, 0x09, 0x32, 0x5E,
    0x90, 0x18, 0xC9, 0x7A, 0x40, 0x22, 0x3D, 0x4F, 0x03, 0xC4, 0x75, 0x1C,
    0x0F, 0x4B, 0xDE, 0xF0, 0x3C, 0x06, 0x49, 0x80, 0x90, 0x19, 0x26, 0x02,
    0x08, 0x64, 0x98, 0x0F, 0x02, 0x52, 0x70, 0x20, 0xC9, 0x49, 0xC0, 0x90,
    0x24, 0xE1, 0x72, 0x40, 0x64, 0x98, 0x09, 0x02, 0x52, 0x70, 0x08, 0xE8,
    0x38, 0x80, 0x75, 0x18, 0xAE, 0x32, 0x94, 0x47, 0x11, 0xCC, 0x53, 0x16,
    0xA0, 0x0C, 0x47, 0x11, 0x80, 0x22, 0x1D, 0x27, 0x00, 0x88, 0xC2, 0x38,
    0x01, 0x11, 0x92, 0x60, 0x04, 0x49, 0x49, 0x80, 0xF0, 0x39, 0x29, 0x03,
    0xC0, 0xD5, 0xA4, 0x06, 0x49, 0xC0, 0xF0, 0x19, 0xAB, 0x42, 0x00, 0x20,
    0x19, 0x07, 0x00, 0x00, 0xF4, 0x00, 0x00, 0x03, 0xC2, 0x00, 0x25, 0x44,
    0x78, 0x8E, 0x95, 0x11, 0xA7, 0x09, 0x00, 0x42, 0x10, 0x00, 0x2A, 0x94,
    0x50, 0x01, 0x45, 0x2A, 0x80, 0x22, 0x22, 0x22, 0x23, 0xFF, 0xFF, 0xFF,
    0xFD, 0xDD, 0xDD, 0xDD, 0xC8, 0x42, 0x10, 0x84, 0x21, 0x09, 0xC2, 0x10,
    0x84, 0xE1, 0x38, 0x45, 0x29, 0x4A, 0x52, 0x80, 0x00, 0x79, 0x4A, 0x00,
    0x38, 0x4E, 0x11, 0x4A, 0xD0, 0xB4, 0xA5, 0x29, 0x4A, 0x52, 0x80, 0x0F,
    0x0B, 0x4A, 0x52, 0xB4, 0x2F, 0x01, 0x4A, 0x57, 0x80, 0x02, 0x13, 0x84,
    0xE0, 0x00, 0x00, 0x70, 0x84, 0x21, 0x08, 0x70, 0x00, 0x84, 0x27, 0xC0,
    0x00, 0x00, 0x1F, 0x21, 0x08, 0x42, 0x1C, 0x84, 0x00, 0x01, 0xF0, 0x00,
    0x84, 0x27, 0xC8, 0x42, 0x10, 0xE4, 0x39, 0x14, 0xA5, 0x2D, 0x4A, 0x52,
    0x96, 0x87, 0x80, 0x00, 0x7A, 0x16, 0xA5, 0x2B, 0x60, 0xF8, 0x00, 0x0F,
    0x83, 0x6A, 0x52, 0x96, 0x85, 0xA8, 0x00, 0xF8, 0x3E, 0x05, 0x2B, 0x60,
    0xDA, 0x88, 0x4F, 0x83, 0xE0, 0x52, 0x94, 0xF0, 0x00, 0x00, 0xF8, 0x3E,
    0x40, 0x00, 0x0F, 0x52, 0x94, 0xA5, 0x3C, 0x00, 0x21, 0x0E, 0x43, 0x80,
    0x00, 0x39, 0x0E, 0x40, 0x00, 0x0F, 0x52, 0x94, 0xA5, 0x2D, 0x4A, 0x21,
    0x3E, 0x0F, 0x90, 0x84, 0x27, 0x00, 0x00, 0x00, 0x07, 0x21, 0x3F, 0xFF,
    0xFF, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0x18, 0xC6, 0x31, 0x83, 0x9C, 0xE7,
    0x39, 0xFF, 0xFF, 0x80, 0x00, 0x02, 0xA9, 0x45, 0x01, 0x92, 0xA4, 0xA8,
    0x06, 0x4A, 0x10, 0x80, 0x01, 0xE9, 0x7A, 0x40, 0x43, 0xB8, 0x40, 0x00,
    0x1E, 0x62, 0x3C, 0x00, 0x52, 0x9A, 0x84, 0x01, 0x26, 0x20, 0xC0, 0xF3,
    0x24, 0xCF, 0x00, 0x00, 0x17, 0xA4, 0x06, 0x33, 0xCC, 0x90, 0x1D, 0x06,
    0x49, 0x88, 0x02, 0xAA, 0xA0, 0x00, 0x8C, 0x93, 0x10, 0x07, 0x43, 0xD0,
    0x70, 0x19, 0x29, 0x4A, 0x40, 0xF0, 0x3C, 0x0F, 0x01, 0x1C, 0x40, 0x38,
    0x00, 0x10, 0xC8, 0xE0, 0x00, 0x8C, 0x11, 0xC0, 0x01, 0x10, 0x84, 0x21,
    0x08, 0x42, 0x20, 0x0F, 0xCC, 0x21, 0x9F, 0xD1, 0x51, 0x22, 0xA2, 0x01,
    0x14, 0x40, 0x00, 0x04, 0x71, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0C, 0xE1,
    0x91, 0x00, 0x62, 0x94, 0x00, 0x00, 0x86, 0x66, 0x10, 0x00, 0x63, 0x9C,
    0xE0, 0x00, 0x00, 0x00, 0x00};

REICH_API void* reich_memset(void* dest, int32 c, reichSize count) {
  char* bytes = (char*)dest;
  while (count--) { *bytes++ = (char)c; }
  return dest;
}

REICH_API void* reich_memcpy(void* dest, const void* src, reichSize count) {
  char* d = (char*)dest;
  const char* s = (const char*)src;
  while (count--) { *d++ = *s++; }
  return dest;
}

REICH_API reichSize reich_strlen(const char* str) {
  const char* s = str;
  while (*s) { s++; }
  return (reichSize)(s - str);
}

REICH_API int32 reich_strncpy(char* dest, const char* src, int32 maxLen) {
  int32 i;
  for (i = 0; i < maxLen - 1 && src[i]; ++i) { dest[i] = src[i]; }
  dest[i] = 0;
  return 1;
}

REICH_API uint8 reich_rgba_channel(uint32 col, int8 channel) {
  switch (channel) {
  case ('a'): {
    return (col >> 24) & 0xFF;
  }
  case ('r'): {
    return (col >> 16) & 0xFF;
  }
  case ('g'): {
    return (col >> 8) & 0xFF;
  }
  case ('b'): {
    return (col >> 0) & 0xFF;
  }
  }
  return 0;
}

static int32 reich_dirty_reset(reichContext* ctx) {
  ctx->activeDirty.x1 = 999999;
  ctx->activeDirty.y1 = 999999;
  ctx->activeDirty.x2 = -999999;
  ctx->activeDirty.y2 = -999999;
  return 1;
}

static int32 reich_dirty_add(
    reichContext* ctx, int32 x1, int32 y1, int32 x2, int32 y2) {
  if (x1 < ctx->activeDirty.x1) { ctx->activeDirty.x1 = x1; }
  if (y1 < ctx->activeDirty.y1) { ctx->activeDirty.y1 = y1; }
  if (x2 > ctx->activeDirty.x2) { ctx->activeDirty.x2 = x2; }
  if (y2 > ctx->activeDirty.y2) { ctx->activeDirty.y2 = y2; }
  return 1;
}

REICH_API int32 reich_vsnprintf(
    char* buffer, reichSize count, const char* format, va_list args) {
  char* p = buffer;
  char* end = buffer + count - 1;
  const char* f = format;
  if (!buffer || count == 0) { return 0; }

  while (*f && p < end) {
    int32 width = 0;
    int32 precision = -1;
    int32 padZero = 0;

    if (*f != '%') {
      *p++ = *f++;
      continue;
    }

    f++;
    if (*f == 0) { break; }
    if (*f == '0') {
      padZero = 1;
      f++;
    }
    while (*f >= '0' && *f <= '9') {
      width = width * 10 + (*f - '0');
      f++;
    }
    if (*f == '.') {
      f++;
      precision = 0;
      while (*f >= '0' && *f <= '9') {
        precision = precision * 10 + (*f - '0');
        f++;
      }
    }
    if (*f == 'l') { f++; }

    if (*f == 's') {
      const char* s = va_arg(args, const char*);
      if (!s) { s = "(null)"; }
      while (*s && p < end) { *p++ = *s++; }
    } else if (*f == 'd' || *f == 'i') {
      int32 val = va_arg(args, int32);
      char temp[32];
      int32 pos = 0;
      int32 neg = 0;
      if (val < 0) {
        neg = 1;
        val = -val;
      }
      if (val == 0) { temp[pos++] = '0'; }
      while (val > 0) {
        temp[pos++] = (char)((val % 10) + '0');
        val /= 10;
      }
      if (neg) { temp[pos++] = '-'; }
      while (width > pos && p < end) {
        *p++ = padZero ? '0' : ' ';
        width--;
      }
      while (pos > 0 && p < end) { *p++ = temp[--pos]; }
    } else if (*f == 'x' || *f == 'X') {
      uint32 val = va_arg(args, uint32);
      char temp[32];
      int32 pos = 0;
      if (val == 0) { temp[pos++] = '0'; }
      while (val > 0) {
        int32 digit = val % 16;
        temp[pos++] =
            (char)((digit < 10) ? (digit + '0')
                                : (digit - 10 + ((*f == 'x') ? 'a' : 'A')));
        val /= 16;
      }
      while (width > pos && p < end) {
        *p++ = padZero ? '0' : ' ';
        width--;
      }
      while (pos > 0 && p < end) { *p++ = temp[--pos]; }
    } else if (*f == 'c') {
      char c = (char)va_arg(args, int32);
      *p++ = c;
    } else if (*f == 'f') {
      double val = va_arg(args, double);
      int32 ipart;
      double fpart;
      char temp[64];
      int32 pos = 0;
      if (precision == -1) { precision = 6; }
      if (val < 0) {
        *p++ = '-';
        val = -val;
      }
      ipart = (int32)val;
      fpart = val - (double)ipart;
      if (ipart == 0) { temp[pos++] = '0'; }
      while (ipart > 0) {
        temp[pos++] = (char)((ipart % 10) + '0');
        ipart /= 10;
      }
      while (pos > 0 && p < end) { *p++ = temp[--pos]; }
      if (precision > 0 && p < end) {
        *p++ = '.';
        while (precision > 0 && p < end) {
          int32 digit;
          fpart *= 10.0;
          digit = (int32)fpart;
          *p++ = (char)(digit + '0');
          fpart -= (double)digit;
          precision--;
        }
      }
    } else if (*f == '%') {
      *p++ = '%';
    }
    f++;
  }
  *p = 0;
  return (int32)(p - buffer);
}

int32 reich_string_format(char* buffer, int32 size, const char* format, ...) {
  va_list args;
  va_start(args, format);
  reich_vsnprintf(buffer, (reichSize)size, format, args);
  va_end(args);
	return 0;
}

#if defined(REICH_PLATFORM_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#define REICH_CLASS_NAME    "LIB_REICH_WINDOW"
#define REICH_RESIZE_BORDER 8

typedef struct reichPlatformContext {
  BITMAPINFO bitmapInfo;
  HDC renderDC;
  HWND windowHandle;
} reichPlatformContext;

typedef struct reichSearchState {
  HANDLE findHandle;
  WIN32_FIND_DATAA findData;
} reichSearchState;

REICH_API reichHandle reich_sys_file_open(const char* filename, int32 mode) {
  HANDLE file;
  DWORD access = 0;
  DWORD share = 0;
  DWORD creation = 0;
  if (mode == REICH_FILE_READ) {
    access = GENERIC_READ;
    share = FILE_SHARE_READ;
    creation = OPEN_EXISTING;
  } else if (mode == REICH_FILE_WRITE) {
    access = GENERIC_WRITE;
    share = 0;
    creation = CREATE_ALWAYS;
  }
  file = CreateFileA(
      filename, access, share, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    reich_sys_log(REICH_LOG_ERROR, "Failed to open file: %s", filename);
    return NULL;
  }
  return (reichHandle)file;
}

REICH_API int32 reich_sys_file_close(reichHandle file) {
  if (file) { CloseHandle((HANDLE)file); }
  return 1;
}

REICH_API reichSize
reich_sys_file_read(reichHandle file, void* buffer, reichSize bytes) {
  DWORD read = 0;
  if (!file || !buffer) { return 0; }
  if (!ReadFile((HANDLE)file, buffer, (DWORD)bytes, &read, NULL)) {
    reich_sys_log(REICH_LOG_ERROR, "Failed to read from file.");
    return 0;
  }
  return (reichSize)read;
}

REICH_API reichSize
reich_sys_file_write(reichHandle file, const void* buffer, reichSize bytes) {
  DWORD written = 0;
  if (!file || !buffer) { return 0; }
  if (!WriteFile((HANDLE)file, buffer, (DWORD)bytes, &written, NULL)) {
    reich_sys_log(REICH_LOG_ERROR, "Failed to write to file.");
    return 0;
  }
  return (reichSize)written;
}

REICH_API reichSize reich_sys_file_size(reichHandle file) {
  LARGE_INTEGER size;
  if (!file) { return 0; }
  if (!GetFileSizeEx((HANDLE)file, &size)) { return 0; }
  return (reichSize)size.QuadPart;
}

REICH_API int32
reich_sys_file_seek(reichHandle file, int32 offset, int32 origin) {
  DWORD method = FILE_BEGIN;
  if (!file) { return 0; }
  if (origin == REICH_SEEK_CUR) {
    method = FILE_CURRENT;
  } else if (origin == REICH_SEEK_END) {
    method = FILE_END;
  }
  return (int32)SetFilePointer((HANDLE)file, offset, NULL, method);
}

REICH_API int32 reich_sys_file_tell(reichHandle file) {
  return reich_sys_file_seek(file, 0, REICH_SEEK_CUR);
}

REICH_API reichHandle reich_sys_find_first(
    const char* pattern, char* filenameBuffer, int32 bufferSize) {
  reichSearchState* state =
      (reichSearchState*)reich_sys_alloc(sizeof(reichSearchState));
  if (!state) {
    reich_sys_log(
        REICH_LOG_ERROR, "Failed to allocate memory for search state.");
    return NULL;
  }
  state->findHandle = FindFirstFileA(pattern, &state->findData);
  if (state->findHandle == INVALID_HANDLE_VALUE) {
    reich_sys_free(state);
    return NULL;
  }
  if (filenameBuffer && bufferSize > 0) {
    reich_strncpy(filenameBuffer, state->findData.cFileName, bufferSize);
  }
  return (reichHandle)state;
}

REICH_API int32 reich_sys_find_next(
    reichHandle handle, char* filenameBuffer, int32 bufferSize) {
  reichSearchState* state = (reichSearchState*)handle;
  if (!state) { return 0; }
  if (!FindNextFileA(state->findHandle, &state->findData)) { return 0; }
  if (filenameBuffer && bufferSize > 0) {
    reich_strncpy(filenameBuffer, state->findData.cFileName, bufferSize);
  }
  return 1;
}

REICH_API int32 reich_sys_find_close(reichHandle handle) {
  reichSearchState* state = (reichSearchState*)handle;
  if (state) {
    FindClose(state->findHandle);
    reich_sys_free(state);
  }
  return 1;
}

REICH_API int32 reich_sys_log(int32 level, const char* format, ...) {
  char buffer[2048];
  char prefix[16];
  va_list args;
  DWORD written;
  HANDLE stdOut;
  if (level == REICH_LOG_DEBUG) {
    reich_strncpy(prefix, "[DEBUG] ", 16);
  } else if (level == REICH_LOG_INFO) {
    reich_strncpy(prefix, "[INFO]  ", 16);
  } else if (level == REICH_LOG_WARN) {
    reich_strncpy(prefix, "[WARN]  ", 16);
  } else if (level == REICH_LOG_ERROR) {
    reich_strncpy(prefix, "[ERROR] ", 16);
  } else {
    reich_strncpy(prefix, "[LOG]   ", 16);
  }

  va_start(args, format);
  reich_vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  OutputDebugStringA(prefix);
  OutputDebugStringA(buffer);
  OutputDebugStringA("\n");

  stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (stdOut && stdOut != INVALID_HANDLE_VALUE) {
    WriteFile(stdOut, prefix, (DWORD)reich_strlen(prefix), &written, NULL);
    WriteFile(stdOut, buffer, (DWORD)reich_strlen(buffer), &written, NULL);
    WriteFile(stdOut, "\n", 1, &written, NULL);
  }
  return 1;
}

LRESULT CALLBACK
reich_sys_window_callback(HWND h, UINT m, WPARAM w, LPARAM l) {
  reichContext* ctx = (reichContext*)NULL;
  reichPlatformContext* pctx = (reichPlatformContext*)NULL;

  if (m == WM_NCCREATE) {
    CREATESTRUCT* createStruct = (CREATESTRUCT*)l;
    ctx = (reichContext*)createStruct->lpCreateParams;
    SetWindowLongPtr(h, GWLP_USERDATA, (LONG_PTR)ctx);
  } else {
    ctx = (reichContext*)GetWindowLongPtr(h, GWLP_USERDATA);
  }

  if (ctx) { pctx = (reichPlatformContext*)ctx->platform; }
  if (!ctx || !pctx) { return DefWindowProcA(h, m, w, l); }

  switch (m) {
  case WM_DESTROY:
    reich_sys_log(REICH_LOG_INFO, "Window destroyed.");
    ctx->running = 0;
    PostQuitMessage(0);
    return 0;
  case WM_CLOSE:
    reich_sys_log(REICH_LOG_INFO, "Window closed.");
    ctx->running = 0;
    return 0;
  case WM_ERASEBKGND:
    return 1;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(h, &ps);
    if (ctx->canvas.pixels) {
      StretchDIBits(
          hdc,
          0,
          0,
          ctx->windowWidth,
          ctx->windowHeight,
          0,
          0,
          ctx->canvas.width,
          ctx->canvas.height,
          ctx->canvas.pixels,
          &pctx->bitmapInfo,
          DIB_RGB_COLORS,
          SRCCOPY);
    }
    EndPaint(h, &ps);
    return 0;
  }
  case WM_NCCALCSIZE: {
    if (w == (WPARAM)TRUE) {
      if (IsZoomed(h)) {
        NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)l;
        int borderX =
            GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
        int borderY =
            GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
        params->rgrc[0].left += borderX;
        params->rgrc[0].top += borderY;
        params->rgrc[0].right -= borderX;
        params->rgrc[0].bottom -= borderY;
      }
      return 0;
    }
    break;
  }
  case WM_NCHITTEST: {
    LRESULT hit = DefWindowProcA(h, m, w, l);
    POINT ptLocal;
    int btnArea;
    int tbHeight;
    if (hit != HTCLIENT) { return hit; }
    if (!IsZoomed(h)) {
      POINT pt;
      RECT rc;
      int border, top, bot, left, right;
      pt.x = GET_X_LPARAM(l);
      pt.y = GET_Y_LPARAM(l);
      GetWindowRect(h, &rc);
      border = REICH_RESIZE_BORDER;
      top = (pt.y < rc.top + border);
      bot = (pt.y >= rc.bottom - border);
      left = (pt.x < rc.left + border);
      right = (pt.x >= rc.right - border);
      if (top && left) { return HTTOPLEFT; }
      if (top && right) { return HTTOPRIGHT; }
      if (bot && left) { return HTBOTTOMLEFT; }
      if (bot && right) { return HTBOTTOMRIGHT; }
      if (top) { return HTTOP; }
      if (bot) { return HTBOTTOM; }
      if (left) { return HTLEFT; }
      if (right) { return HTRIGHT; }
    }
    ptLocal.x = GET_X_LPARAM(l);
    ptLocal.y = GET_Y_LPARAM(l);
    ScreenToClient(h, &ptLocal);
    tbHeight = ctx->themeTitleBarHeight * ctx->scale;
    ctx->input.mouseX = ptLocal.x;
    ctx->input.mouseY = ptLocal.y;
    if (ptLocal.y >= 0 && ptLocal.y < tbHeight) {
      btnArea = (ctx->themeButtonWidth * ctx->scale) * 3;
      if (ptLocal.x >= ctx->windowWidth - btnArea) { return HTCLIENT; }
      return HTCAPTION;
    }
    return HTCLIENT;
  }
  case WM_SIZE: {
    int32 width = LOWORD(l);
    int32 height = HIWORD(l);
    ctx->windowWidth = width;
    ctx->windowHeight = height;
    ctx->isMaximized = (w == SIZE_MAXIMIZED);
    reich_sys_resize_canvas(ctx, width, height);
    reich_arena_reset(&ctx->frameMem);
    if (ctx->userRender) { ctx->userRender(ctx, 1.0); }
    reich_draw_decorations(ctx);
    reich_sys_present(ctx);
    return 0;
  }
  case WM_GETMINMAXINFO: {
    MINMAXINFO* mmi = (MINMAXINFO*)l;
    mmi->ptMinTrackSize.x = 320;
    mmi->ptMinTrackSize.y = 240;
    return 0;
  }
  case WM_LBUTTONDOWN:
    ctx->input.buttons[0] = 1;
    ctx->input.buttonsPressed[0] = 1;
    SetCapture(h);
    return 0;
  case WM_LBUTTONUP:
    ctx->input.buttons[0] = 0;
    ctx->input.buttonsReleased[0] = 1;
    ReleaseCapture();
    return 0;
  case WM_RBUTTONDOWN:
    ctx->input.buttons[1] = 1;
    ctx->input.buttonsPressed[1] = 1;
    return 0;
  case WM_RBUTTONUP:
    ctx->input.buttons[1] = 0;
    ctx->input.buttonsReleased[1] = 1;
    return 0;
  case WM_MBUTTONDOWN:
    ctx->input.buttons[2] = 1;
    ctx->input.buttonsPressed[2] = 1;
    return 0;
  case WM_MBUTTONUP:
    ctx->input.buttons[2] = 0;
    ctx->input.buttonsReleased[2] = 1;
    return 0;
  case WM_MOUSEWHEEL:
    ctx->input.mouseWheel += GET_WHEEL_DELTA_WPARAM(w);
    return 0;
  case WM_MOUSEMOVE:
    ctx->input.mouseX = GET_X_LPARAM(l);
    ctx->input.mouseY = GET_Y_LPARAM(l);
    return 0;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    if (w < REICH_KEY_MAX) {
      ctx->input.keys[w] = 1;
      if (!((l >> 30) & 1)) { ctx->input.keysPressed[w] = 1; }
    }
    return 0;
  case WM_KEYUP:
  case WM_SYSKEYUP:
    if (w < REICH_KEY_MAX) {
      ctx->input.keys[w] = 0;
      ctx->input.keysReleased[w] = 1;
    }
    return 0;
  case WM_CHAR:
    ctx->input.lastChar = (uint32)w;
    return 0;
  case WM_KILLFOCUS:
    reich_memset(ctx->input.keys, 0, sizeof(ctx->input.keys));
    return 0;
  }
  return DefWindowProcA(h, m, w, l);
}

REICH_API int32 reich_sys_toggle_maximize(reichContext* ctx) {
  reichPlatformContext* pctx = (reichPlatformContext*)ctx->platform;
  if (ctx->isMaximized) {
    ShowWindow(pctx->windowHandle, SW_RESTORE);
  } else {
    ShowWindow(pctx->windowHandle, SW_MAXIMIZE);
  }
  return 1;
}

REICH_API int32 reich_sys_minimize(reichContext* ctx) {
  reichPlatformContext* pctx = (reichPlatformContext*)ctx->platform;
  ShowWindow(pctx->windowHandle, SW_MINIMIZE);
  return 1;
}

REICH_API int32 reich_sys_close(reichContext* ctx) {
  reichPlatformContext* pctx = (reichPlatformContext*)ctx->platform;
  PostMessageA(pctx->windowHandle, WM_CLOSE, 0, 0);
  return 1;
}

REICH_API reichSize reich_sys_get_platform_data_size(void) {
  return (reichSize)sizeof(reichPlatformContext);
}

REICH_API int32 reich_sys_window_init(
    reichContext* ctx, const char* title, int32 width, int32 height) {
  WNDCLASSA wc;
  DWORD style;
  reichPlatformContext* plat = (reichPlatformContext*)ctx->platform;
  SetProcessDPIAware();
  AttachConsole((DWORD)-1);
  if (!plat) {
    reich_sys_log(REICH_LOG_ERROR, "Platform context is NULL.");
    return 0;
  }
  reich_memset(&wc, 0, sizeof(WNDCLASSA));
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = reich_sys_window_callback;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = REICH_CLASS_NAME;
  RegisterClassA(&wc);
  style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

  /* Initialize bitmap info BEFORE window creation so WM_SIZE has valid header fields */
  plat->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  plat->bitmapInfo.bmiHeader.biWidth = width;
  plat->bitmapInfo.bmiHeader.biHeight = -height;
  plat->bitmapInfo.bmiHeader.biPlanes = 1;
  plat->bitmapInfo.bmiHeader.biBitCount = 32;
  plat->bitmapInfo.bmiHeader.biCompression = BI_RGB;

  /* No AdjustWindowRect since WM_NCCALCSIZE strips OS borders, width/height specify client area directly */
  plat->windowHandle = CreateWindowExA(
      WS_EX_APPWINDOW,
      REICH_CLASS_NAME,
      title,
      style,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      width,
      height,
      (HWND)0,
      (HMENU)0,
      wc.hInstance,
      (LPVOID)ctx);

  if (!plat->windowHandle) {
    reich_sys_log(REICH_LOG_ERROR, "Failed to create window.");
    return 0;
  }
  plat->renderDC = GetDC(plat->windowHandle);

  /* Ensure canvas matches the final creation client area */
  {
    RECT cr;
    GetClientRect(plat->windowHandle, &cr);
    ctx->windowWidth = cr.right;
    ctx->windowHeight = cr.bottom;
    reich_sys_resize_canvas(ctx, cr.right, cr.bottom);
  }

  SetWindowPos(
      plat->windowHandle,
      (HWND)0,
      0,
      0,
      0,
      0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
  reich_sys_present(ctx);
  reich_sys_log(
      REICH_LOG_INFO,
      "Window initialized successfully: %s (%dx%d)",
      title,
      width,
      height);
  return 1;
}

static int32 reich_internal_resize_canvas(
    reichCanvas* canvas, int32 w, int32 h) {
  if (canvas->width != w || canvas->height != h) {
    if (canvas->pixels) { reich_sys_free(canvas->pixels); }
    canvas->width = w;
    canvas->height = h;
    canvas->pixels =
        (uint32*)reich_sys_alloc((reichSize)w * h * sizeof(uint32));
    if (canvas->pixels) {
      reich_memset(canvas->pixels, 0, (reichSize)w * h * sizeof(uint32));
    }
  }
  return 1;
}

REICH_API int32
reich_sys_resize_canvas(reichContext* ctx, int32 width, int32 height) {
  reichPlatformContext* pctx = (reichPlatformContext*)ctx->platform;
  int32 cw = ctx->scale > 0 ? width / ctx->scale : width;
  int32 ch = ctx->scale > 0 ? height / ctx->scale : height;
  if (cw < 1) { cw = 1; }
  if (ch < 1) { ch = 1; }

  reich_internal_resize_canvas(&ctx->canvas, cw, ch);
  pctx->bitmapInfo.bmiHeader.biWidth = cw;
  pctx->bitmapInfo.bmiHeader.biHeight = -ch;

  ctx->canvas = ctx->canvas;
  ctx->clip = reich_rect(0, 0, cw, ch);
  return 1;
}

REICH_API int32 reich_sys_poll_events(reichContext* ctx) {
  MSG msg;
  while (PeekMessageA(&msg, (HWND)0, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) { ctx->running = 0; }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
  return 1;
}

REICH_API int32 reich_sys_present(reichContext* ctx) {
  reichPlatformContext* plat = (reichPlatformContext*)ctx->platform;
  if (ctx->canvas.pixels) {
    StretchDIBits(
        plat->renderDC,
        0,
        0,
        ctx->windowWidth,
        ctx->windowHeight,
        0,
        0,
        ctx->canvas.width,
        ctx->canvas.height,
        ctx->canvas.pixels,
        &plat->bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
  }
  return 1;
}

REICH_API int64 reich_sys_get_ticks(void) {
  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);
  return (int64)count.QuadPart;
}

REICH_API int64 reich_sys_get_freq(void) {
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);
  return (int64)freq.QuadPart;
}

REICH_API void* reich_sys_alloc(reichSize size) {
  return VirtualAlloc(
      (LPVOID)0, (SIZE_T)size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

REICH_API int32 reich_sys_free(void* ptr) {
  if (ptr) { VirtualFree(ptr, 0, MEM_RELEASE); }
  return 1;
}

REICH_API int32 reich_sys_show_cursor(void) {
  ShowCursor(TRUE);
  return 1;
}
REICH_API int32 reich_sys_hide_cursor(void) {
  ShowCursor(FALSE);
  return 1;
}

#else
#error "Platform not supported"
#endif

REICH_API int32 reich_rect_hovered(reichRect r, int32 mx, int32 my) {
  return mx >= r.x1 && mx < r.x2 && my >= r.y1 && my < r.y2;
}

/* MEMORY ********************************************************************/

REICH_API int32 reich_arena_init(reichArena* a, void* mem, reichSize size) {
  a->base = (uint8*)mem;
  a->size = size;
  a->used = 0;
  return 1;
}

REICH_API void* reich_arena_alloc(reichArena* a, reichSize size) {
  void* ptr;
  reichSize aligned = (size + 7) & ~7;
  if (a->used + aligned > a->size) {
    reich_sys_log(
        REICH_LOG_ERROR,
        "Arena overflow. Capacity: %lu, Requested: %lu",
        a->size,
        aligned);
    return (void*)0;
  }
  ptr = (void*)(a->base + a->used);
  a->used += aligned;
  return ptr;
}

REICH_API int32 reich_arena_reset(reichArena* a) {
  a->used = 0;
  return 1;
}

/* MATH **********************************************************************/

#define REICH_PI         3.14159265f
#define REICH_HALF_PI    1.57079632f
#define REICH_TWO_PI     6.28318530f
#define REICH_INV_TWO_PI 0.15915494f
#define REICH_MAX(a, b)           ((a) > (b) ? (a) : (b))
#define REICH_MIN(a, b)           ((a) < (b) ? (a) : (b))
#define REICH_CLAMP(v, minV, maxV) REICH_MIN(REICH_MAX(v, minV), maxV)
#define REICH_CLAMP_COORD(val, maxVal) \
  ((val) < 0 ? 0 : ((val) >= (maxVal) ? (maxVal) - 1 : (val)))
#define REICH_RGBA(_r, _g, _b, _a) (((uint8)(_a)) << 24) | (((uint8)(_r)) << 16) | (((uint8)(_g)) << 8) | ((uint8)(_b))

static float reich_wrap(float x) {
  int k = (int)(x * REICH_INV_TWO_PI);
  x -= (float)k * REICH_TWO_PI;
  if (x > REICH_PI) { x -= REICH_TWO_PI; }
  if (x < -REICH_PI) { x += REICH_TWO_PI; }
  return x;
}

REICH_API float reich_sin(float x) {
  float x2, term, result;
  x = reich_wrap(x);
  x2 = x * x;
  result = x;
  term = x * (-x2 * 0.16666667f);
  result += term;
  term *= (-x2 * 0.05f);
  result += term;
  term *= (-x2 * 0.02380952f);
  result += term;
  term *= (-x2 * 0.01388889f);
  result += term;
  return result;
}

REICH_API float reich_cos(float x) {
  return reich_sin(x + REICH_HALF_PI);
}

REICH_API double reich_sqrt(double n) {
  double x, next_x;
  if (n <= 0.0) { return 0.0; }
  x = n * 0.5;
  if (x < 1.0) { x = 1.0; }
  while (1) {
    next_x = 0.5 * (x + n / x);
    if (next_x >= x) { return x; }
    x = next_x;
  }
}

REICH_API int32 reich_floor(real32 x) {
  int32 i = (int32)x;
  return (x < (real32)i) ? (i - 1) : i;
}
REICH_API int32 reich_ceil(real32 x) {
  int32 i = (int32)x;
  return (x > (real32)i) ? (i + 1) : i;
}

REICH_API int64 reich_floord(double x) {
  int64 i = (int64)x;
  return (x < (double)i) ? (i - 1) : i;
}

REICH_API int64 reich_ceild(double x) {
  int64 i = (int64)x;
  return (x > (double)i) ? (i + 1) : i;
}

real32 reich_min(real32 a, real32 b) {
  return a < b ? a : b;
}

real32 reich_max(real32 a, real32 b) {
  return a > b ? a : b;
}

real32 reich_clamp(real32 v, real32 minV, real32 maxV) {
  return reich_min(reich_max(v, minV), maxV);
}

real32 reich_smoothstep(real32 edge0, real32 edge1, real32 x) {
  real32 t;
  if (edge0 == edge1) { return 0.0f; }
  t = reich_clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

REICH_API real32 reich_smin(real32 a, real32 b, real32 k) {
  real32 h;
  if (k <= 0.0001f) { return reich_min(a, b); }
  h = reich_clamp(0.5f + (b - a) / (2.0f * k), 0.0f, 1.0f);
  return b + (a - b) * h - k * h * (1.0f - h);
}

/* MATH::VECTOR **************************************************************/

#define REICH_DECL_VEC2(T, N, S)           \
  typedef struct N {                       \
    T x, y;                                \
  } N;                                     \
  REICH_API N reich_##S(T x, T y);         \
  REICH_API N reich_##S##_add(N a, N b);   \
  REICH_API N reich_##S##_sub(N a, N b);   \
  REICH_API N reich_##S##_mul(N a, N b);   \
  REICH_API N reich_##S##_div(N a, N b);   \
  REICH_API N reich_##S##_scale(N a, T s); \
  REICH_API T reich_##S##_dot(N a, N b);   \
  REICH_API T reich_##S##_len_sq(N a);     \
  REICH_API N reich_##S##_lerp(N a, N b, T t);

#define REICH_DECL_VEC2_FLOAT(T, N, S) \
  REICH_DECL_VEC2(T, N, S)             \
  REICH_API T reich_##S##_len(N a);    \
  REICH_API N reich_##S##_norm(N a);

#define REICH_DECL_VEC3(T, N, S)           \
  typedef struct N {                       \
    T x, y, z;                             \
  } N;                                     \
  REICH_API N reich_##S(T x, T y, T z);    \
  REICH_API N reich_##S##_add(N a, N b);   \
  REICH_API N reich_##S##_sub(N a, N b);   \
  REICH_API N reich_##S##_mul(N a, N b);   \
  REICH_API N reich_##S##_div(N a, N b);   \
  REICH_API N reich_##S##_scale(N a, T s); \
  REICH_API T reich_##S##_dot(N a, N b);   \
  REICH_API N reich_##S##_cross(N a, N b); \
  REICH_API T reich_##S##_len_sq(N a);     \
  REICH_API N reich_##S##_lerp(N a, N b, T t);

#define REICH_DECL_VEC3_FLOAT(T, N, S) \
  REICH_DECL_VEC3(T, N, S)             \
  REICH_API T reich_##S##_len(N a);    \
  REICH_API N reich_##S##_norm(N a);

#define REICH_DECL_VEC4(T, N, S)             \
  typedef struct N {                         \
    T x, y, z, w;                            \
  } N;                                       \
  REICH_API N reich_##S(T x, T y, T z, T w); \
  REICH_API N reich_##S##_add(N a, N b);     \
  REICH_API N reich_##S##_sub(N a, N b);     \
  REICH_API N reich_##S##_mul(N a, N b);     \
  REICH_API N reich_##S##_div(N a, N b);     \
  REICH_API N reich_##S##_scale(N a, T s);   \
  REICH_API T reich_##S##_dot(N a, N b);     \
  REICH_API T reich_##S##_len_sq(N a);       \
  REICH_API N reich_##S##_lerp(N a, N b, T t);

#define REICH_DECL_VEC4_FLOAT(T, N, S) \
  REICH_DECL_VEC4(T, N, S)             \
  REICH_API T reich_##S##_len(N a);    \
  REICH_API N reich_##S##_norm(N a);

REICH_DECL_VEC2_FLOAT(real32, reichVec2, vec2)
REICH_DECL_VEC2(int32, reichVec2i, vec2i)
REICH_DECL_VEC3_FLOAT(real32, reichVec3, vec3)
REICH_DECL_VEC3(int32, reichVec3i, vec3i)
REICH_DECL_VEC4_FLOAT(real32, reichVec4, vec4)
REICH_DECL_VEC4(int32, reichVec4i, vec4i)

typedef struct reichMat4 {
  real32 m[16];
} reichMat4;

REICH_API real32 reich_tan(real32 x);
REICH_API reichMat4 reich_mat4_identity(void);
REICH_API reichMat4 reich_mat4_mul(reichMat4 a, reichMat4 b);
REICH_API reichVec4 reich_mat4_mul_vec4(reichMat4 m, reichVec4 v);
REICH_API reichMat4 reich_mat4_transpose(reichMat4 mat);
REICH_API reichMat4 reich_mat4_inverse(reichMat4 mat);
REICH_API reichMat4 reich_mat4_ortho(
    real32 left,
    real32 right,
    real32 bottom,
    real32 top,
    real32 nearZ,
    real32 farZ);
REICH_API reichMat4
reich_mat4_persp(real32 fovY, real32 aspect, real32 nearZ, real32 farZ);
REICH_API reichMat4
reich_mat4_lookat(reichVec3 eye, reichVec3 center, reichVec3 up);
REICH_API reichMat4 reich_mat4_translate(reichMat4 mat, reichVec3 v);
REICH_API reichMat4 reich_mat4_scale(reichMat4 mat, reichVec3 v);
REICH_API reichMat4
reich_mat4_rotate(reichMat4 mat, real32 angle, reichVec3 axis);
REICH_API reichVec3 reich_vec3_project(
    reichVec3 obj, reichMat4 modelview, reichMat4 proj, reichVec4 viewport);
REICH_API reichVec3 reich_vec3_unproject(
    reichVec3 win, reichMat4 modelview, reichMat4 proj, reichVec4 viewport);

#ifdef REICH_IMPLEMENTATION

#define REICH_IMPL_VEC2(T, N, S)                \
  REICH_API N reich_##S(T x, T y) {             \
    N r;                                        \
    r.x = x;                                    \
    r.y = y;                                    \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_add(N a, N b) {       \
    N r;                                        \
    r.x = a.x + b.x;                            \
    r.y = a.y + b.y;                            \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_sub(N a, N b) {       \
    N r;                                        \
    r.x = a.x - b.x;                            \
    r.y = a.y - b.y;                            \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_mul(N a, N b) {       \
    N r;                                        \
    r.x = a.x * b.x;                            \
    r.y = a.y * b.y;                            \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_div(N a, N b) {       \
    N r;                                        \
    r.x = b.x != (T)0 ? a.x / b.x : (T)0;       \
    r.y = b.y != (T)0 ? a.y / b.y : (T)0;       \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_scale(N a, T s) {     \
    N r;                                        \
    r.x = a.x * s;                              \
    r.y = a.y * s;                              \
    return r;                                   \
  }                                             \
  REICH_API T reich_##S##_dot(N a, N b) {       \
    return a.x * b.x + a.y * b.y;               \
  }                                             \
  REICH_API T reich_##S##_len_sq(N a) {         \
    return a.x * a.x + a.y * a.y;               \
  }                                             \
  REICH_API N reich_##S##_lerp(N a, N b, T t) { \
    N r;                                        \
    r.x = a.x + (b.x - a.x) * t;                \
    r.y = a.y + (b.y - a.y) * t;                \
    return r;                                   \
  }

#define REICH_IMPL_VEC2_FLOAT(T, N, S)                        \
  REICH_IMPL_VEC2(T, N, S)                                    \
  REICH_API T reich_##S##_len(N a) {                          \
    return (T)reich_sqrt((real64)reich_##S##_len_sq(a));      \
  }                                                           \
  REICH_API N reich_##S##_norm(N a) {                         \
    T l = reich_##S##_len(a);                                 \
    return l > (T)0.0 ? reich_##S##_scale(a, (T)1.0 / l) : a; \
  }

#define REICH_IMPL_VEC3(T, N, S)                \
  REICH_API N reich_##S(T x, T y, T z) {        \
    N r;                                        \
    r.x = x;                                    \
    r.y = y;                                    \
    r.z = z;                                    \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_add(N a, N b) {       \
    N r;                                        \
    r.x = a.x + b.x;                            \
    r.y = a.y + b.y;                            \
    r.z = a.z + b.z;                            \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_sub(N a, N b) {       \
    N r;                                        \
    r.x = a.x - b.x;                            \
    r.y = a.y - b.y;                            \
    r.z = a.z - b.z;                            \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_mul(N a, N b) {       \
    N r;                                        \
    r.x = a.x * b.x;                            \
    r.y = a.y * b.y;                            \
    r.z = a.z * b.z;                            \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_div(N a, N b) {       \
    N r;                                        \
    r.x = b.x != (T)0 ? a.x / b.x : (T)0;       \
    r.y = b.y != (T)0 ? a.y / b.y : (T)0;       \
    r.z = b.z != (T)0 ? a.z / b.z : (T)0;       \
    return r;                                   \
  }                                             \
  REICH_API N reich_##S##_scale(N a, T s) {     \
    N r;                                        \
    r.x = a.x * s;                              \
    r.y = a.y * s;                              \
    r.z = a.z * s;                              \
    return r;                                   \
  }                                             \
  REICH_API T reich_##S##_dot(N a, N b) {       \
    return a.x * b.x + a.y * b.y + a.z * b.z;   \
  }                                             \
  REICH_API N reich_##S##_cross(N a, N b) {     \
    N r;                                        \
    r.x = a.y * b.z - a.z * b.y;                \
    r.y = a.z * b.x - a.x * b.z;                \
    r.z = a.x * b.y - a.y * b.x;                \
    return r;                                   \
  }                                             \
  REICH_API T reich_##S##_len_sq(N a) {         \
    return a.x * a.x + a.y * a.y + a.z * a.z;   \
  }                                             \
  REICH_API N reich_##S##_lerp(N a, N b, T t) { \
    N r;                                        \
    r.x = a.x + (b.x - a.x) * t;                \
    r.y = a.y + (b.y - a.y) * t;                \
    r.z = a.z + (b.z - a.z) * t;                \
    return r;                                   \
  }

#define REICH_IMPL_VEC3_FLOAT(T, N, S)                        \
  REICH_IMPL_VEC3(T, N, S)                                    \
  REICH_API T reich_##S##_len(N a) {                          \
    return (T)reich_sqrt((real64)reich_##S##_len_sq(a));      \
  }                                                           \
  REICH_API N reich_##S##_norm(N a) {                         \
    T l = reich_##S##_len(a);                                 \
    return l > (T)0.0 ? reich_##S##_scale(a, (T)1.0 / l) : a; \
  }

#define REICH_IMPL_VEC4(T, N, S)                          \
  REICH_API N reich_##S(T x, T y, T z, T w) {             \
    N r;                                                  \
    r.x = x;                                              \
    r.y = y;                                              \
    r.z = z;                                              \
    r.w = w;                                              \
    return r;                                             \
  }                                                       \
  REICH_API N reich_##S##_add(N a, N b) {                 \
    N r;                                                  \
    r.x = a.x + b.x;                                      \
    r.y = a.y + b.y;                                      \
    r.z = a.z + b.z;                                      \
    r.w = a.w + b.w;                                      \
    return r;                                             \
  }                                                       \
  REICH_API N reich_##S##_sub(N a, N b) {                 \
    N r;                                                  \
    r.x = a.x - b.x;                                      \
    r.y = a.y - b.y;                                      \
    r.z = a.z - b.z;                                      \
    r.w = a.w - b.w;                                      \
    return r;                                             \
  }                                                       \
  REICH_API N reich_##S##_mul(N a, N b) {                 \
    N r;                                                  \
    r.x = a.x * b.x;                                      \
    r.y = a.y * b.y;                                      \
    r.z = a.z * b.z;                                      \
    r.w = a.w * b.w;                                      \
    return r;                                             \
  }                                                       \
  REICH_API N reich_##S##_div(N a, N b) {                 \
    N r;                                                  \
    r.x = b.x != (T)0 ? a.x / b.x : (T)0;                 \
    r.y = b.y != (T)0 ? a.y / b.y : (T)0;                 \
    r.z = b.z != (T)0 ? a.z / b.z : (T)0;                 \
    r.w = b.w != (T)0 ? a.w / b.w : (T)0;                 \
    return r;                                             \
  }                                                       \
  REICH_API N reich_##S##_scale(N a, T s) {               \
    N r;                                                  \
    r.x = a.x * s;                                        \
    r.y = a.y * s;                                        \
    r.z = a.z * s;                                        \
    r.w = a.w * s;                                        \
    return r;                                             \
  }                                                       \
  REICH_API T reich_##S##_dot(N a, N b) {                 \
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; \
  }                                                       \
  REICH_API T reich_##S##_len_sq(N a) {                   \
    return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w; \
  }                                                       \
  REICH_API N reich_##S##_lerp(N a, N b, T t) {           \
    N r;                                                  \
    r.x = a.x + (b.x - a.x) * t;                          \
    r.y = a.y + (b.y - a.y) * t;                          \
    r.z = a.z + (b.z - a.z) * t;                          \
    r.w = a.w + (b.w - a.w) * t;                          \
    return r;                                             \
  }

#define REICH_IMPL_VEC4_FLOAT(T, N, S)                        \
  REICH_IMPL_VEC4(T, N, S)                                    \
  REICH_API T reich_##S##_len(N a) {                          \
    return (T)reich_sqrt((real64)reich_##S##_len_sq(a));      \
  }                                                           \
  REICH_API N reich_##S##_norm(N a) {                         \
    T l = reich_##S##_len(a);                                 \
    return l > (T)0.0 ? reich_##S##_scale(a, (T)1.0 / l) : a; \
  }

REICH_IMPL_VEC2_FLOAT(real32, reichVec2, vec2)
REICH_IMPL_VEC2(int32, reichVec2i, vec2i)
REICH_IMPL_VEC3_FLOAT(real32, reichVec3, vec3)
REICH_IMPL_VEC3(int32, reichVec3i, vec3i)
REICH_IMPL_VEC4_FLOAT(real32, reichVec4, vec4)
REICH_IMPL_VEC4(int32, reichVec4i, vec4i)

REICH_API real32 reich_tan(real32 x) {
  real32 c = reich_cos(x);
  return c != 0.0f ? reich_sin(x) / c : 0.0f;
}

REICH_API reichMat4 reich_mat4_identity(void) {
  reichMat4 r;
  int32 i;
  for (i = 0; i < 16; ++i) { r.m[i] = 0.0f; }
  r.m[0] = 1.0f;
  r.m[5] = 1.0f;
  r.m[10] = 1.0f;
  r.m[15] = 1.0f;
  return r;
}

REICH_API reichMat4 reich_mat4_mul(reichMat4 a, reichMat4 b) {
  reichMat4 r;
  int32 i, j, k;
  for (j = 0; j < 4; ++j) {
    for (i = 0; i < 4; ++i) {
      r.m[j * 4 + i] = 0.0f;
      for (k = 0; k < 4; ++k) {
        r.m[j * 4 + i] += a.m[k * 4 + i] * b.m[j * 4 + k];
      }
    }
  }
  return r;
}

REICH_API reichVec4 reich_mat4_mul_vec4(reichMat4 m, reichVec4 v) {
  reichVec4 r;
  r.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w;
  r.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w;
  r.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w;
  r.w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w;
  return r;
}

REICH_API reichMat4 reich_mat4_transpose(reichMat4 mat) {
  reichMat4 r;
  r.m[0] = mat.m[0];
  r.m[1] = mat.m[4];
  r.m[2] = mat.m[8];
  r.m[3] = mat.m[12];
  r.m[4] = mat.m[1];
  r.m[5] = mat.m[5];
  r.m[6] = mat.m[9];
  r.m[7] = mat.m[13];
  r.m[8] = mat.m[2];
  r.m[9] = mat.m[6];
  r.m[10] = mat.m[10];
  r.m[11] = mat.m[14];
  r.m[12] = mat.m[3];
  r.m[13] = mat.m[7];
  r.m[14] = mat.m[11];
  r.m[15] = mat.m[15];
  return r;
}

REICH_API reichMat4 reich_mat4_inverse(reichMat4 mat) {
  reichMat4 r;
  real32 det;
  int32 i;
  r.m[0] = mat.m[5] * mat.m[10] * mat.m[15] -
      mat.m[5] * mat.m[11] * mat.m[14] - mat.m[9] * mat.m[6] * mat.m[15] +
      mat.m[9] * mat.m[7] * mat.m[14] + mat.m[13] * mat.m[6] * mat.m[11] -
      mat.m[13] * mat.m[7] * mat.m[10];
  r.m[4] = -mat.m[4] * mat.m[10] * mat.m[15] +
      mat.m[4] * mat.m[11] * mat.m[14] + mat.m[8] * mat.m[6] * mat.m[15] -
      mat.m[8] * mat.m[7] * mat.m[14] - mat.m[12] * mat.m[6] * mat.m[11] +
      mat.m[12] * mat.m[7] * mat.m[10];
  r.m[8] = mat.m[4] * mat.m[9] * mat.m[15] - mat.m[4] * mat.m[11] * mat.m[13] -
      mat.m[8] * mat.m[5] * mat.m[15] + mat.m[8] * mat.m[7] * mat.m[13] +
      mat.m[12] * mat.m[5] * mat.m[11] - mat.m[12] * mat.m[7] * mat.m[9];
  r.m[12] = -mat.m[4] * mat.m[9] * mat.m[14] +
      mat.m[4] * mat.m[10] * mat.m[13] + mat.m[8] * mat.m[5] * mat.m[14] -
      mat.m[8] * mat.m[6] * mat.m[13] - mat.m[12] * mat.m[5] * mat.m[10] +
      mat.m[12] * mat.m[6] * mat.m[9];
  r.m[1] = -mat.m[1] * mat.m[10] * mat.m[15] +
      mat.m[1] * mat.m[11] * mat.m[14] + mat.m[9] * mat.m[2] * mat.m[15] -
      mat.m[9] * mat.m[3] * mat.m[14] - mat.m[13] * mat.m[2] * mat.m[11] +
      mat.m[13] * mat.m[3] * mat.m[10];
  r.m[5] = mat.m[0] * mat.m[10] * mat.m[15] -
      mat.m[0] * mat.m[11] * mat.m[14] - mat.m[8] * mat.m[2] * mat.m[15] +
      mat.m[8] * mat.m[3] * mat.m[14] + mat.m[12] * mat.m[2] * mat.m[11] -
      mat.m[12] * mat.m[3] * mat.m[10];
  r.m[9] = -mat.m[0] * mat.m[9] * mat.m[15] +
      mat.m[0] * mat.m[11] * mat.m[13] + mat.m[8] * mat.m[1] * mat.m[15] -
      mat.m[8] * mat.m[3] * mat.m[13] - mat.m[12] * mat.m[1] * mat.m[11] +
      mat.m[12] * mat.m[3] * mat.m[9];
  r.m[13] = mat.m[0] * mat.m[9] * mat.m[14] -
      mat.m[0] * mat.m[10] * mat.m[13] - mat.m[8] * mat.m[1] * mat.m[14] +
      mat.m[8] * mat.m[2] * mat.m[13] + mat.m[12] * mat.m[1] * mat.m[10] -
      mat.m[12] * mat.m[2] * mat.m[9];
  r.m[2] = mat.m[1] * mat.m[6] * mat.m[15] - mat.m[1] * mat.m[7] * mat.m[14] -
      mat.m[5] * mat.m[2] * mat.m[15] + mat.m[5] * mat.m[3] * mat.m[14] +
      mat.m[13] * mat.m[2] * mat.m[7] - mat.m[13] * mat.m[3] * mat.m[6];
  r.m[6] = -mat.m[0] * mat.m[6] * mat.m[15] + mat.m[0] * mat.m[7] * mat.m[14] +
      mat.m[4] * mat.m[2] * mat.m[15] - mat.m[4] * mat.m[3] * mat.m[14] -
      mat.m[12] * mat.m[2] * mat.m[7] + mat.m[12] * mat.m[3] * mat.m[6];
  r.m[10] = mat.m[0] * mat.m[5] * mat.m[15] - mat.m[0] * mat.m[7] * mat.m[13] -
      mat.m[4] * mat.m[1] * mat.m[15] + mat.m[4] * mat.m[3] * mat.m[13] +
      mat.m[12] * mat.m[1] * mat.m[7] - mat.m[12] * mat.m[3] * mat.m[5];
  r.m[14] = -mat.m[0] * mat.m[5] * mat.m[14] +
      mat.m[0] * mat.m[6] * mat.m[13] + mat.m[4] * mat.m[1] * mat.m[14] -
      mat.m[4] * mat.m[2] * mat.m[13] - mat.m[12] * mat.m[1] * mat.m[6] +
      mat.m[12] * mat.m[2] * mat.m[5];
  r.m[3] = -mat.m[1] * mat.m[6] * mat.m[11] + mat.m[1] * mat.m[7] * mat.m[10] +
      mat.m[5] * mat.m[2] * mat.m[11] - mat.m[5] * mat.m[3] * mat.m[10] -
      mat.m[9] * mat.m[2] * mat.m[7] + mat.m[9] * mat.m[3] * mat.m[6];
  r.m[7] = mat.m[0] * mat.m[6] * mat.m[11] - mat.m[0] * mat.m[7] * mat.m[10] -
      mat.m[4] * mat.m[2] * mat.m[11] + mat.m[4] * mat.m[3] * mat.m[10] +
      mat.m[8] * mat.m[2] * mat.m[7] - mat.m[8] * mat.m[3] * mat.m[6];
  r.m[11] = -mat.m[0] * mat.m[5] * mat.m[11] + mat.m[0] * mat.m[7] * mat.m[9] +
      mat.m[4] * mat.m[1] * mat.m[11] - mat.m[4] * mat.m[3] * mat.m[9] -
      mat.m[8] * mat.m[1] * mat.m[7] + mat.m[8] * mat.m[3] * mat.m[5];
  r.m[15] = mat.m[0] * mat.m[5] * mat.m[10] - mat.m[0] * mat.m[6] * mat.m[9] -
      mat.m[4] * mat.m[1] * mat.m[10] + mat.m[4] * mat.m[2] * mat.m[9] +
      mat.m[8] * mat.m[1] * mat.m[6] - mat.m[8] * mat.m[2] * mat.m[5];
  det = mat.m[0] * r.m[0] + mat.m[1] * r.m[4] + mat.m[2] * r.m[8] +
      mat.m[3] * r.m[12];
  if (det == 0.0f) { return reich_mat4_identity(); }
  det = 1.0f / det;
  for (i = 0; i < 16; i++) { r.m[i] *= det; }
  return r;
}

REICH_API reichMat4 reich_mat4_ortho(
    real32 left,
    real32 right,
    real32 bottom,
    real32 top,
    real32 nearZ,
    real32 farZ) {
  reichMat4 r;
  real32 rl, tb, fn;
  r = reich_mat4_identity();
  rl = right - left;
  tb = top - bottom;
  fn = farZ - nearZ;
  if (rl == 0.0f || tb == 0.0f || fn == 0.0f) { return r; }
  r.m[0] = 2.0f / rl;
  r.m[5] = 2.0f / tb;
  r.m[10] = -2.0f / fn;
  r.m[12] = -(right + left) / rl;
  r.m[13] = -(top + bottom) / tb;
  r.m[14] = -(farZ + nearZ) / fn;
  return r;
}

REICH_API reichMat4
reich_mat4_persp(real32 fovY, real32 aspect, real32 nearZ, real32 farZ) {
  reichMat4 r;
  real32 t, f, nf;
  int32 i;
  r = reich_mat4_identity();
  t = reich_tan(fovY * 0.5f);
  nf = nearZ - farZ;
  if (aspect == 0.0f || nf == 0.0f || t == 0.0f) { return r; }
  f = 1.0f / t;
  for (i = 0; i < 16; ++i) { r.m[i] = 0.0f; }
  r.m[0] = f / aspect;
  r.m[5] = f;
  r.m[10] = (farZ + nearZ) / nf;
  r.m[11] = -1.0f;
  r.m[14] = (2.0f * farZ * nearZ) / nf;
  return r;
}

REICH_API reichMat4
reich_mat4_lookat(reichVec3 eye, reichVec3 center, reichVec3 up) {
  reichMat4 r;
  reichVec3 f, s, u;
  f = reich_vec3_sub(center, eye);
  if (reich_vec3_len_sq(f) == 0.0f) { return reich_mat4_identity(); }
  f = reich_vec3_norm(f);
  s = reich_vec3_cross(f, up);
  if (reich_vec3_len_sq(s) == 0.0f) { return reich_mat4_identity(); }
  s = reich_vec3_norm(s);
  u = reich_vec3_cross(s, f);
  r = reich_mat4_identity();
  r.m[0] = s.x;
  r.m[4] = s.y;
  r.m[8] = s.z;
  r.m[1] = u.x;
  r.m[5] = u.y;
  r.m[9] = u.z;
  r.m[2] = -f.x;
  r.m[6] = -f.y;
  r.m[10] = -f.z;
  r.m[12] = -reich_vec3_dot(s, eye);
  r.m[13] = -reich_vec3_dot(u, eye);
  r.m[14] = reich_vec3_dot(f, eye);
  return r;
}

REICH_API reichMat4 reich_mat4_translate(reichMat4 mat, reichVec3 v) {
  reichMat4 r;
  r = mat;
  r.m[12] = mat.m[0] * v.x + mat.m[4] * v.y + mat.m[8] * v.z + mat.m[12];
  r.m[13] = mat.m[1] * v.x + mat.m[5] * v.y + mat.m[9] * v.z + mat.m[13];
  r.m[14] = mat.m[2] * v.x + mat.m[6] * v.y + mat.m[10] * v.z + mat.m[14];
  r.m[15] = mat.m[3] * v.x + mat.m[7] * v.y + mat.m[11] * v.z + mat.m[15];
  return r;
}

REICH_API reichMat4 reich_mat4_scale(reichMat4 mat, reichVec3 v) {
  reichMat4 r;
  int32 i;
  for (i = 0; i < 4; ++i) {
    r.m[i] = mat.m[i] * v.x;
    r.m[i + 4] = mat.m[i + 4] * v.y;
    r.m[i + 8] = mat.m[i + 8] * v.z;
    r.m[i + 12] = mat.m[i + 12];
  }
  return r;
}

REICH_API reichMat4
reich_mat4_rotate(reichMat4 mat, real32 angle, reichVec3 axis) {
  reichMat4 rot;
  real32 c, s, omc, x, y, z;
  axis = reich_vec3_norm(axis);
  c = reich_cos(angle);
  s = reich_sin(angle);
  omc = 1.0f - c;
  x = axis.x;
  y = axis.y;
  z = axis.z;
  rot = reich_mat4_identity();
  rot.m[0] = x * x * omc + c;
  rot.m[1] = y * x * omc + z * s;
  rot.m[2] = x * z * omc - y * s;
  rot.m[4] = x * y * omc - z * s;
  rot.m[5] = y * y * omc + c;
  rot.m[6] = y * z * omc + x * s;
  rot.m[8] = x * z * omc + y * s;
  rot.m[9] = y * z * omc - x * s;
  rot.m[10] = z * z * omc + c;
  return reich_mat4_mul(mat, rot);
}

REICH_API reichVec3 reich_vec3_project(
    reichVec3 obj, reichMat4 modelview, reichMat4 proj, reichVec4 viewport) {
  reichVec4 v;
  reichVec3 r;
  v = reich_vec4(obj.x, obj.y, obj.z, 1.0f);
  v = reich_mat4_mul_vec4(modelview, v);
  v = reich_mat4_mul_vec4(proj, v);
  if (v.w == 0.0f) {
    r.x = 0.0f;
    r.y = 0.0f;
    r.z = 0.0f;
    return r;
  }
  v.x /= v.w;
  v.y /= v.w;
  v.z /= v.w;
  r.x = viewport.x + (1.0f + v.x) * viewport.z * 0.5f;
  r.y = viewport.y + (1.0f + v.y) * viewport.w * 0.5f;
  r.z = (1.0f + v.z) * 0.5f;
  return r;
}

REICH_API reichVec3 reich_vec3_unproject(
    reichVec3 win, reichMat4 modelview, reichMat4 proj, reichVec4 viewport) {
  reichMat4 inv;
  reichVec4 in, out;
  reichVec3 r;
  inv = reich_mat4_inverse(reich_mat4_mul(proj, modelview));
  if (viewport.z == 0.0f || viewport.w == 0.0f) {
    r.x = 0.0f;
    r.y = 0.0f;
    r.z = 0.0f;
    return r;
  }
  in.x = (win.x - viewport.x) / viewport.z * 2.0f - 1.0f;
  in.y = (win.y - viewport.y) / viewport.w * 2.0f - 1.0f;
  in.z = 2.0f * win.z - 1.0f;
  in.w = 1.0f;
  out = reich_mat4_mul_vec4(inv, in);
  if (out.w == 0.0f) {
    r.x = 0.0f;
    r.y = 0.0f;
    r.z = 0.0f;
    return r;
  }
  r.x = out.x / out.w;
  r.y = out.y / out.w;
  r.z = out.z / out.w;
  return r;
}

#endif


/* NOISE *********************************************************************/

static const unsigned char REICH_NOISE_PERM[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,
    20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,
    230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,5,202,
    38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,
    2,44,154,163, 70,221,153,101,155,167, 43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185,
    112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,
    67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,
    20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,
    230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,5,202,
    38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,
    2,44,154,163, 70,221,153,101,155,167, 43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185,
    112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,
    67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

#define REICH_FAST_FLOOR(x) ((int)(x) - ((x) < (real32)(int32)(x)))

/* Hashing */
#define REICH_HASH1(out_hash, x) \
    do { \
        uint32 _x = (x); \
        _x = ((_x >> 16) ^ _x) * 0x45d9f3bU; \
        _x = ((_x >> 16) ^ _x) * 0x45d9f3bU; \
        (out_hash) = (_x >> 16) ^ _x; \
    } while(0)

#define REICH_HASH2(out_hash, x, y) \
    do { \
        uint32 _x = (x), _y = (y); \
        uint32 _h = _x * 374761393U + _y * 668265263U; \
        _h = (_h ^ (_h >> 13)) * 1274126177U; \
        (out_hash) = _h ^ (_h >> 16); \
    } while(0)

#define REICH_HASH3(out_hash, x, y, z) \
    do { \
        uint32 _x = (x), _y = (y), _z = (z); \
        uint32 _h = _x * 374761393U + _y * 668265263U + _z * 3266489917U; \
        _h = (_h ^ (_h >> 13)) * 1274126177U; \
        (out_hash) = _h ^ (_h >> 16); \
    } while(0)

#define REICH_GRAD1_TABLE(out_val, hash, x) \
    do { \
        (out_val) = (((hash) & 1) ? -(x) : (x)); \
    } while(0)

#define REICH_GRAD2_TABLE(out_val, hash, x, y) \
    do { \
        int32 _h = (hash) & 15; \
        real32 _x = (x), _y = (y); \
        real32 _u = _h < 8 ? _x : _y; \
        real32 _v = _h < 4 ? _y : (_h == 12 || _h == 14 ? _x : 0.0f); \
        (out_val) = (((_h & 1) == 0 ? _u : -_u) + ((_h & 2) == 0 ? _v : -_v)); \
    } while(0)

#define REICH_GRAD3_TABLE(out_val, hash, x, y, z) \
    do { \
        int32 _h = (hash) & 15; \
        real32 _x = (x), _y = (y), _z = (z); \
        real32 _u = _h < 8 ? _x : _y; \
        real32 _v = _h < 4 ? _y : (_h == 12 || _h == 14 ? _x : _z); \
        (out_val) = (((_h & 1) == 0 ? _u : -_u) + ((_h & 2) == 0 ? _v : -_v)); \
    } while(0)

real32 reich_noise1(real32 x) {
    int32 X; real32 fx, u, g0, g1;
    X = REICH_FAST_FLOOR(x) & 255; fx = x - REICH_FAST_FLOOR(x);
    u = fx * fx * fx * (fx * (fx * 6.0f - 15.0f) + 10.0f);
    REICH_GRAD1_TABLE(g0, REICH_NOISE_PERM[X], fx);
    REICH_GRAD1_TABLE(g1, REICH_NOISE_PERM[X + 1], fx - 1.0f);
    return g0 + u * (g1 - g0);
}

real32 reich_noise2(real32 x, real32 y) {
    int32 X, Y, A, B;
    real32 fx, fy, u, v;
    real32 g00, g10, g01, g11, nx0, nx1;
    X = REICH_FAST_FLOOR(x) & 255;
    Y = REICH_FAST_FLOOR(y) & 255;
    fx = x - REICH_FAST_FLOOR(x);
    fy = y - REICH_FAST_FLOOR(y);
    u = fx * fx * fx * (fx * (fx * 6.0f - 15.0f) + 10.0f);
    v = fy * fy * fy * (fy * (fy * 6.0f - 15.0f) + 10.0f);
    A = REICH_NOISE_PERM[X] + Y;
    B = REICH_NOISE_PERM[X + 1] + Y;
    REICH_GRAD2_TABLE(g00, REICH_NOISE_PERM[A], fx, fy);
    REICH_GRAD2_TABLE(g10, REICH_NOISE_PERM[B], fx - 1.0f, fy);
    REICH_GRAD2_TABLE(g01, REICH_NOISE_PERM[A + 1], fx, fy - 1.0f);
    REICH_GRAD2_TABLE(g11, REICH_NOISE_PERM[B + 1], fx - 1.0f, fy - 1.0f);
    nx0 = g00 + u * (g10 - g00);
    nx1 = g01 + u * (g11 - g01);
    return nx0 + v * (nx1 - nx0);
}

real32 reich_noise3(real32 x, real32 y, real32 z) {
    int32 X, Y, Z, A, AA, AB, B, BA, BB;
    real32 fx, fy, fz, u, v, w, g000, g100, g010, g110, g001, g101, g011, g111, nx00, nx10, nx01, nx11, nxy0, nxy1;
    X = REICH_FAST_FLOOR(x) & 255; Y = REICH_FAST_FLOOR(y) & 255; Z = REICH_FAST_FLOOR(z) & 255;
    fx = x - REICH_FAST_FLOOR(x); fy = y - REICH_FAST_FLOOR(y); fz = z - REICH_FAST_FLOOR(z);
    u = fx * fx * fx * (fx * (fx * 6.0f - 15.0f) + 10.0f);
    v = fy * fy * fy * (fy * (fy * 6.0f - 15.0f) + 10.0f);
    w = fz * fz * fz * (fz * (fz * 6.0f - 15.0f) + 10.0f);
    A = REICH_NOISE_PERM[X]+Y; AA = REICH_NOISE_PERM[A]+Z; AB = REICH_NOISE_PERM[A+1]+Z;
    B = REICH_NOISE_PERM[X+1]+Y; BA = REICH_NOISE_PERM[B]+Z; BB = REICH_NOISE_PERM[B+1]+Z;
    REICH_GRAD3_TABLE(g000, REICH_NOISE_PERM[AA], fx, fy, fz);
    REICH_GRAD3_TABLE(g100, REICH_NOISE_PERM[BA], fx - 1.0f, fy, fz);
    REICH_GRAD3_TABLE(g010, REICH_NOISE_PERM[AB], fx, fy - 1.0f, fz);
    REICH_GRAD3_TABLE(g110, REICH_NOISE_PERM[BB], fx - 1.0f, fy - 1.0f, fz);
    REICH_GRAD3_TABLE(g001, REICH_NOISE_PERM[AA + 1], fx, fy, fz - 1.0f);
    REICH_GRAD3_TABLE(g101, REICH_NOISE_PERM[BA + 1], fx - 1.0f, fy, fz - 1.0f);
    REICH_GRAD3_TABLE(g011, REICH_NOISE_PERM[AB + 1], fx, fy - 1.0f, fz - 1.0f);
    REICH_GRAD3_TABLE(g111, REICH_NOISE_PERM[BB + 1], fx - 1.0f, fy - 1.0f, fz - 1.0f);
    nx00 = g000 + u * (g100 - g000); nx10 = g010 + u * (g110 - g010);
    nx01 = g001 + u * (g101 - g001); nx11 = g011 + u * (g111 - g011);
    nxy0 = nx00 + v * (nx10 - nx00); nxy1 = nx01 + v * (nx11 - nx01);
    return nxy0 + w * (nxy1 - nxy0);
}

REICH_API real32 reich_noise_fbm1(real32 x, int32 octaves, real32 lacunarity, real32 gain) {
    real32 total = 0.0f, amplitude = 1.0f, frequency = 1.0f, maxVal = 0.0f;
    int32 i;
    if (octaves <= 0) { return 0.0f; }
    for (i = 0; i < octaves; ++i) {
        total += reich_noise1(x * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }
    return maxVal > 0.0f ? (total / maxVal) : 0.0f;
}

REICH_API real32 reich_noise_fbm2(real32 x, real32 y, int32 octaves, real32 lacunarity, real32 gain) {
    real32 total = 0.0f, amplitude = 1.0f, frequency = 1.0f, maxVal = 0.0f;
    int32 i;
    if (octaves <= 0) { return 0.0f; }
    for (i = 0; i < octaves; ++i) {
        total += reich_noise2(x * frequency, y * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }
    return maxVal > 0.0f ? (total / maxVal) : 0.0f;
}

REICH_API real32 reich_noise_fbm3(real32 x, real32 y, real32 z, int32 octaves, real32 lacunarity, real32 gain) {
    real32 total = 0.0f, amplitude = 1.0f, frequency = 1.0f, maxVal = 0.0f;
    int32 i;
    if (octaves <= 0) { return 0.0f; }
    for (i = 0; i < octaves; ++i) {
        total += reich_noise3(x * frequency, y * frequency, z * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }
    return maxVal > 0.0f ? (total / maxVal) : 0.0f;
}

REICH_API real32 reich_noise_warp_2d(real32 x, real32 y, int32 octaves, real32 lacunarity, real32 gain, real32 warpStrength, real32* outX, real32* outY) {
    real32 qx, qy, nx, ny;
    qx = reich_noise_fbm2(x, y, octaves, lacunarity, gain);
    qy = reich_noise_fbm2(x + 5.2f, y + 1.3f, octaves, lacunarity, gain);
    nx = x + warpStrength * qx;
    ny = y + warpStrength * qy;
    if (outX) { *outX = nx; }
    if (outY) { *outY = ny; }
    return reich_noise_fbm2(nx, ny, octaves, lacunarity, gain);
}

REICH_API real32 reich_noise_warp_3d(real32 x, real32 y, real32 z, int32 octaves, real32 lacunarity, real32 gain, real32 warpStrength, real32* outX, real32* outY, real32* outZ) {
    real32 qx, qy, qz, nx, ny, nz;
    qx = reich_noise_fbm3(x, y, z, octaves, lacunarity, gain);
    qy = reich_noise_fbm3(x + 5.2f, y + 1.3f, z - 2.8f, octaves, lacunarity, gain);
    qz = reich_noise_fbm3(x - 2.4f, y + 8.1f, z + 3.5f, octaves, lacunarity, gain);
    nx = x + warpStrength * qx;
    ny = y + warpStrength * qy;
    nz = z + warpStrength * qz;
    if (outX) { *outX = nx; }
    if (outY) { *outY = ny; }
    if (outZ) { *outZ = nz; }
    return reich_noise_fbm3(nx, ny, nz, octaves, lacunarity, gain);
}

/* RANDOM ********************************************************************/

static uint64 REICH_RAND_STATE = 1;
static uint64 REICH_RAND_BIT_CACHE = 0;
static int32 REICH_RAND_BIT_COUNT = 0;

void reich_rand_seed(uint64 seed) {
    REICH_RAND_STATE = seed;
		REICH_RAND_BIT_COUNT = 0;
}

/* LCG */

uint64 reich_rand_lcg(void) {
    REICH_RAND_STATE = REICH_RAND_STATE * 1103515245UL + 12345UL;
    return (uint64)(REICH_RAND_STATE / 65536UL) % 32768UL;
}

int32 reich_rand_lcg_range(int32 min, int32 max) {
    uint64 range;
    if (max <= min) return min;
    range = (uint64)(max - min + 1);
		if(range > 32768UL) { range = 32768UL; }
    return min + (int32)(reich_rand_lcg() % range);
}

real32 reich_rand_lcg_float(void) {
    return (real32)reich_rand_lcg() / 32767.0f;
}

real32 reich_rand_lcg_range_float(real32 min, real32 max) {
    return min + reich_rand_lcg_float() * (max - min);
}

/* XORSHIFT */

uint64 reich_rand_xor(void) {
    uint64 x = REICH_RAND_STATE;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    REICH_RAND_STATE = x;
    return x;
}

int32 reich_rand_xor_range(int32 min, int32 max) {
    uint64 range;
    if (max <= min) return min;
    range = (uint64)(max - min + 1);
    return min + (int32)(reich_rand_xor() % range);
}

real32 reich_rand_xor_float(void) {
    return (real32)reich_rand_xor() / 4294967295.0f;
}

real32 reich_rand_xor_range_float(real32 min, real32 max) {
    return min + reich_rand_xor_float() * (max - min);
}

/* BIT */

int32 reich_rand_bit(void) {
    int32 result;
    if (REICH_RAND_BIT_COUNT == 0) {
        uint64 x = REICH_RAND_STATE;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        REICH_RAND_STATE = x;
        REICH_RAND_BIT_CACHE = x;
        REICH_RAND_BIT_COUNT = 32;
    }
    result = (int32)(REICH_RAND_BIT_CACHE & 1UL);
    REICH_RAND_BIT_CACHE >>= 1;
    REICH_RAND_BIT_COUNT--;
    return result;
}

/* TIMING ********************************************************************/

REICH_API int32 reich_timer_tick(reichContext* ctx) {
  int64 currentCounter = reich_sys_get_ticks();
  real64 frameTime =
      (real64)(currentCounter - ctx->lastCounter) / (real64)ctx->perfFreq;
  ctx->lastCounter = currentCounter;
  if (frameTime > 0.25) { frameTime = 0.25; }
  ctx->accumulator += frameTime;
  return 1;
}

REICH_API int32 reich_timer_step(reichContext* ctx) {
  if (ctx->accumulator >= ctx->fixedDt) {
    ctx->accumulator -= ctx->fixedDt;
    return 1;
  }
  return 0;
}

REICH_API real64 reich_timer_alpha(reichContext* ctx) {
  return ctx->accumulator / ctx->fixedDt;
}

/* WINDOW ********************************************************************/

static int32 reich_update_window_controls(reichContext* ctx) {
  int32 w;
  int32 mx = 0;
  int32 my = 0;
  if (!reich_mouse_released(ctx, 0)) { return 0; }
  mx = reich_mouse_x(ctx);
  my = reich_mouse_y(ctx);
  w = ctx->canvas.width;
  reichRect rClose = reich_rect(w - 22, 6, w - 6, 21);
  reichRect rMax = reich_rect(rClose.x1 - 19, 6, rClose.x1 - 2, 21);
  reichRect rMin = reich_rect(rMax.x1 - 17, 6, rMax.x1, 21);
  if (reich_rect_hovered(rMin, mx, my)) { reich_sys_minimize(ctx); }
  if (reich_rect_hovered(rMax, mx, my)) { reich_sys_toggle_maximize(ctx); }
  if (reich_rect_hovered(rClose, mx, my)) { reich_sys_close(ctx); }
  return 1;
}

REICH_API int32 reich_begin_frame(reichContext* ctx) {
  if (!ctx->running) { return 0; }
  reich_input_update(ctx);
  reich_update_window_controls(ctx);
  ctx->clip = reich_rect(0, 0, ctx->canvas.width, ctx->canvas.height);
  reich_arena_reset(&ctx->frameMem);
  reich_dirty_reset(ctx);
  return 1;
}

REICH_API int32 reich_end_frame(reichContext* ctx) {
  reich_draw_decorations(ctx);
  reich_sys_present(ctx);
  return 1;
}

/* INPUT *********************************************************************/

REICH_API int32 reich_input_update(reichContext* ctx) {
  int i;
  ctx->input.hotId = 0;
  ctx->input.mouseWheel = 0;
  ctx->input.lastChar = 0;
  for (i = 0; i < REICH_KEY_MAX; ++i) {
    ctx->input.lastKeys[i] = ctx->input.keys[i];
  }
  for (i = 0; i < REICH_MOUSE_BUTTONS; ++i) {
    ctx->input.lastButtons[i] = ctx->input.buttons[i];
  }
  ctx->input.lastMouseX = ctx->input.mouseX;
  ctx->input.lastMouseY = ctx->input.mouseY;
  reich_memset(ctx->input.keysPressed, 0, sizeof(ctx->input.keysPressed));
  reich_memset(ctx->input.keysReleased, 0, sizeof(ctx->input.keysReleased));
  reich_memset(
      ctx->input.buttonsPressed, 0, sizeof(ctx->input.buttonsPressed));
  reich_memset(
      ctx->input.buttonsReleased, 0, sizeof(ctx->input.buttonsReleased));

  reich_sys_poll_events(ctx);

  if (reich_key_pressed(ctx, 0x73)) {
    int32 newScale = ctx->scale + 1;
    if (newScale > 4) { newScale = 1; }
    reich_set_scale(ctx, newScale);
  }
  ctx->input.deltaX = ctx->input.mouseX - ctx->input.lastMouseX;
  ctx->input.deltaY = ctx->input.mouseY - ctx->input.lastMouseY;
  if (reich_key_pressed(ctx, 0x74)) {
    ctx->activeFont = (ctx->activeFont + 1) % REICH_MAX_FONTS;
  }
  return 1;
}

REICH_API int32 reich_key_down(reichContext* ctx, int32 keyCode) {
  return (keyCode >= 0 && keyCode < REICH_KEY_MAX) ? ctx->input.keys[keyCode]
                                                   : 0;
}
REICH_API int32 reich_key_pressed(reichContext* ctx, int32 keyCode) {
  return (keyCode >= 0 && keyCode < REICH_KEY_MAX)
      ? ctx->input.keysPressed[keyCode]
      : 0;
}
REICH_API int32 reich_key_released(reichContext* ctx, int32 keyCode) {
  return (keyCode >= 0 && keyCode < REICH_KEY_MAX)
      ? ctx->input.keysReleased[keyCode]
      : 0;
}
REICH_API int32 reich_mouse_down(reichContext* ctx, int32 button) {
  return (button >= 0 && button < REICH_MOUSE_BUTTONS)
      ? ctx->input.buttons[button]
      : 0;
}
REICH_API int32 reich_mouse_pressed(reichContext* ctx, int32 button) {
  return (button >= 0 && button < REICH_MOUSE_BUTTONS)
      ? ctx->input.buttonsPressed[button]
      : 0;
}
REICH_API int32 reich_mouse_released(reichContext* ctx, int32 button) {
  return (button >= 0 && button < REICH_MOUSE_BUTTONS)
      ? ctx->input.buttonsReleased[button]
      : 0;
}
REICH_API int32 reich_mouse_x(reichContext* ctx) {
  return ctx->input.mouseX / ctx->scale;
}
REICH_API int32 reich_mouse_y(reichContext* ctx) {
  return ctx->input.mouseY / ctx->scale;
}
REICH_API int32 reich_mouse_wheel(reichContext* ctx) {
  return ctx->input.mouseWheel;
}
REICH_API uint32 reich_get_char_pressed(reichContext* ctx) {
  return ctx->input.lastChar;
}

/* RECT **********************************************************************/

REICH_API reichRect reich_rect(int32 x1, int32 y1, int32 x2, int32 y2) {
  reichRect ret;
  ret.x1 = x1;
  ret.y1 = y1;
  ret.x2 = x2;
  ret.y2 = y2;
  return ret;
}
REICH_API int32 reich_rect_bounds(
    int32 x, int32 y, int32 minX, int32 minY, int32 maxX, int32 maxY) {
  return (x >= minX) && (y >= minY) && (x < maxX) && (y < maxY);
}

/* RUNTIME *******************************************************************/

REICH_API int32 reich_init_default_font(reichContext* ctx) {
  if (ctx->fontCount < REICH_MAX_FONTS) {
    ctx->fonts[ctx->fontCount++] = (uint8*)REICH_FONT_DATA;
  }
  return 1;
}

REICH_API int32 reich_init(
    reichContext* ctx,
    const char* title,
    int32 width,
    int32 height,
    real64 targetFps) {
  reichSize memSize, platSize;
  void* memory;
  if (!ctx) { return 0; }

  reich_memset(ctx, 0, sizeof(reichContext));
  ctx->scale = 1;
  ctx->windowTitle = title;
  ctx->windowWidth = width;
  ctx->windowHeight = height;
  ctx->running = 1;
  ctx->fixedDt = 1.0 / targetFps;
  ctx->perfFreq = reich_sys_get_freq();
  ctx->lastCounter = reich_sys_get_ticks();

  memSize = (reichSize)1024 * 1024 * 1024;
  memory = reich_sys_alloc(memSize);
  if (!memory) { return 0; }

  reich_arena_init(&ctx->permMem, memory, memSize / 4);
  reich_arena_init(
      &ctx->frameMem, (uint8*)memory + (memSize / 4), memSize - (memSize / 4));

  ctx->themeTitleBarHeight = 22;
  ctx->themeButtonWidth = 20;
  ctx->themeHoverColor = 0xFF555555;
  ctx->themeCloseHoverColor = 0xFFE81123;
  ctx->themeIconColor = 0xFFFFFFFF;

  ctx->renderTitleBar = reich_default_render_titlebar;
  ctx->renderBtnMin = reich_default_render_btn_min;
  ctx->renderBtnMax = reich_default_render_btn_max;
  ctx->renderBtnClose = reich_default_render_btn_close;

  platSize = reich_sys_get_platform_data_size();
  ctx->platform = reich_arena_alloc(&ctx->permMem, platSize);
  if (!ctx->platform) { return 0; }

  ctx->canvas.width = width / ctx->scale;
  ctx->canvas.height = height / ctx->scale;
  ctx->canvas.pixels =
      (uint32*)reich_sys_alloc((reichSize)width * height * sizeof(uint32));

  ctx->clip = reich_rect(0, 0, ctx->canvas.width, ctx->canvas.height);
  reich_load_fonts(ctx, "font*", 0, 0, 255);
  if (ctx->fontCount > 7) { ctx->activeFont = 7; }

  if (!reich_sys_window_init(ctx, title, width, height)) { return 0; }
  return 1;
}

REICH_API int32 reich_set_callbacks(
    reichContext* ctx,
    PFUSERUPDATE update,
    PFUSERRENDER render,
    PFUSERINPUT input) {
  if (!ctx) { return 0; }
  ctx->userUpdate = update;
  ctx->userRender = render;
  ctx->userInput = input;
  return 1;
}

REICH_API int32 reich_run(reichContext* ctx) {
  while (ctx->running) {
    if (!reich_begin_frame(ctx)) { break; }
    reich_timer_tick(ctx);
    while (reich_timer_step(ctx)) {
      if (ctx->userUpdate) { ctx->userUpdate(ctx); }
    }
    if (ctx->userInput) { ctx->userInput(ctx); }
    if (ctx->userRender) { ctx->userRender(ctx, reich_timer_alpha(ctx)); }
    reich_end_frame(ctx);
  }
  return 1;
}

REICH_API int32 reich_set_scale(reichContext* ctx, int32 scale) {
  if (!ctx || scale < 1) { return 0; }
  ctx->scale = scale;
  reich_sys_resize_canvas(ctx, ctx->windowWidth, ctx->windowHeight);
  return 1;
}

/* LOADERS *******************************************************************/

REICH_API reichCanvas reich_load_bmp(const char* filename) {
  reichCanvas canvas;
  reichHandle f;
  uint8 fileHeader[14];
  uint8 infoHeader[124];
  uint32 fileSize;
  uint32 dataOffset;
  uint32 headerSize;
  int32 width;
  int32 height;
  int16 planes;
  int16 bpp;
  uint32 compression;
  uint32 imageSize;
  uint32 colorsUsed;
  uint32 rMask;
  uint32 gMask;
  uint32 bMask;
  uint32 aMask;
  int32 isTopDown;
  uint8* data;
  uint32* palette;
  uint32 i;
  int32 x;
  int32 y;
  uint32 r;
  uint32 g;
  uint32 b;
  uint32 a;
  int32 rowSize;
  uint8* row;
  uint8* ptr;
  int32 shiftR;
  int32 shiftG;
  int32 shiftB;
  int32 shiftA;
  int32 scaleR;
  int32 scaleG;
  int32 scaleB;
  int32 scaleA;
  reich_memset(&canvas, 0, sizeof(reichCanvas));
  f = reich_sys_file_open(filename, REICH_FILE_READ);
  if (!f) {
    reich_sys_log(
        REICH_LOG_ERROR, "BMP Load: Failed to open file %s", filename);
    return canvas;
  }
  if (reich_sys_file_read(f, fileHeader, 14) != 14) {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Failed to read file header.");
    reich_sys_file_close(f);
    return canvas;
  }
  if (fileHeader[0] != 'B' || fileHeader[1] != 'M') {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Invalid magic number.");
    reich_sys_file_close(f);
    return canvas;
  }
  dataOffset = *(uint32*)(fileHeader + 10);
  fileSize = *(uint32*)(fileHeader + 2);
  if (reich_sys_file_read(f, &headerSize, 4) != 4) {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Failed to read header size.");
    reich_sys_file_close(f);
    return canvas;
  }
  if (headerSize < 12) {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Header size too small.");
    reich_sys_file_close(f);
    return canvas;
  }
  if (headerSize > 124) { headerSize = 124; }
  *(uint32*)infoHeader = headerSize;
  if (reich_sys_file_read(f, infoHeader + 4, headerSize - 4) !=
      headerSize - 4) {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Failed to read info header.");
    reich_sys_file_close(f);
    return canvas;
  }
  if (headerSize == 12) {
    width = *(int16*)(infoHeader + 4);
    height = *(int16*)(infoHeader + 6);
    planes = *(int16*)(infoHeader + 8);
    bpp = *(int16*)(infoHeader + 10);
    compression = 0;
    imageSize = 0;
    colorsUsed = 0;
  } else {
    width = *(int32*)(infoHeader + 4);
    height = *(int32*)(infoHeader + 8);
    planes = *(int16*)(infoHeader + 12);
    bpp = *(int16*)(infoHeader + 14);
    compression = *(uint32*)(infoHeader + 16);
    imageSize = *(uint32*)(infoHeader + 20);
    colorsUsed = *(uint32*)(infoHeader + 32);
  }
  if (width <= 0 || height == 0 || planes != 1) {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Invalid dimensions or planes.");
    reich_sys_file_close(f);
    return canvas;
  }
  isTopDown = 0;
  if (height < 0) {
    isTopDown = 1;
    height = -height;
  }
  if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 &&
      bpp != 32) {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Unsupported BPP: %d", bpp);
    reich_sys_file_close(f);
    return canvas;
  }
  if (compression != 0 && compression != 1 && compression != 2 &&
      compression != 3) {
    reich_sys_log(
        REICH_LOG_ERROR, "BMP Load: Unsupported compression: %d", compression);
    reich_sys_file_close(f);
    return canvas;
  }
  canvas.width = width;
  canvas.height = height;
  canvas.pixels = (uint32*)reich_sys_alloc((reichSize)width * height * 4);
  if (!canvas.pixels) {
    reich_sys_log(REICH_LOG_ERROR, "BMP Load: Failed to allocate pixels.");
    reich_sys_file_close(f);
    return canvas;
  }
  palette = (uint32*)NULL;
  if (bpp <= 8) {
    if (colorsUsed == 0) { colorsUsed = 1 << bpp; }
    palette = (uint32*)reich_sys_alloc((reichSize)colorsUsed * 4);
    if (headerSize == 12) {
      for (i = 0; i < colorsUsed; ++i) {
        uint8 pal[3];
        reich_sys_file_read(f, pal, 3);
        palette[i] = 0xFF000000 | (pal[2] << 16) | (pal[1] << 8) | pal[0];
      }
    } else {
      for (i = 0; i < colorsUsed; ++i) {
        uint8 pal[4];
        reich_sys_file_read(f, pal, 4);
        palette[i] = 0xFF000000 | (pal[2] << 16) | (pal[1] << 8) | pal[0];
      }
    }
  }
  rMask = 0x00FF0000;
  gMask = 0x0000FF00;
  bMask = 0x000000FF;
  aMask = 0xFF000000;
  if (compression == 3) {
    if (headerSize >= 56) {
      rMask = *(uint32*)(infoHeader + 40);
      gMask = *(uint32*)(infoHeader + 44);
      bMask = *(uint32*)(infoHeader + 48);
      aMask = *(uint32*)(infoHeader + 52);
    } else {
      reich_sys_file_read(f, &rMask, 4);
      reich_sys_file_read(f, &gMask, 4);
      reich_sys_file_read(f, &bMask, 4);
    }
  } else if (bpp == 16) {
    rMask = 0x7C00;
    gMask = 0x03E0;
    bMask = 0x001F;
    aMask = 0x0000;
  }
  reich_sys_file_seek(f, dataOffset, REICH_SEEK_SET);
  if (imageSize == 0) { imageSize = fileSize - dataOffset; }
  data = (uint8*)reich_sys_alloc((reichSize)imageSize);
  reich_sys_file_read(f, data, imageSize);
  reich_sys_file_close(f);
  shiftR = 0;
  shiftG = 0;
  shiftB = 0;
  shiftA = 0;
  scaleR = 0;
  scaleG = 0;
  scaleB = 0;
  scaleA = 0;
  if (bpp == 16 || bpp == 32) {
    uint32 m;
    m = rMask;
    while (m && !(m & 1)) {
      shiftR++;
      m >>= 1;
    }
    if (m) {
      scaleR = 255 / m;
    } else {
      scaleR = 0;
    }
    m = gMask;
    while (m && !(m & 1)) {
      shiftG++;
      m >>= 1;
    }
    if (m) {
      scaleG = 255 / m;
    } else {
      scaleG = 0;
    }
    m = bMask;
    while (m && !(m & 1)) {
      shiftB++;
      m >>= 1;
    }
    if (m) {
      scaleB = 255 / m;
    } else {
      scaleB = 0;
    }
    m = aMask;
    while (m && !(m & 1)) {
      shiftA++;
      m >>= 1;
    }
    if (m) {
      scaleA = 255 / m;
    } else {
      scaleA = 0;
    }
  }
  if (compression == 1 || compression == 2) {
    x = 0;
    y = 0;
    ptr = data;
    while (ptr < data + imageSize && y < height) {
      uint8 count = *ptr++;
      uint8 val = *ptr++;
      if (count > 0) {
        if (compression == 1) {
          for (i = 0; i < count; ++i) {
            if (x < width) {
              int32 dy = isTopDown ? y : (height - 1 - y);
              canvas.pixels[dy * width + x] = palette[val];
            }
            x++;
          }
        } else {
          for (i = 0; i < count; ++i) {
            if (x < width) {
              uint8 idx = (i & 1) ? (val & 0x0F) : (val >> 4);
              int32 dy = isTopDown ? y : (height - 1 - y);
              canvas.pixels[dy * width + x] = palette[idx];
            }
            x++;
          }
        }
      } else {
        if (val == 0) {
          x = 0;
          y++;
        } else if (val == 1) {
          break;
        } else if (val == 2) {
          x += (uint8)*ptr++;
          y += (uint8)*ptr++;
        } else {
          if (compression == 1) {
            for (i = 0; i < val; ++i) {
              if (x < width) {
                int32 dy = isTopDown ? y : (height - 1 - y);
                canvas.pixels[dy * width + x] = palette[(uint8)*ptr++];
              }
              x++;
            }
            if (val & 1) { ptr++; }
          } else {
            for (i = 0; i < val; ++i) {
              if (x < width) {
                uint8 bByte = *ptr;
                uint8 idx = (i & 1) ? (bByte & 0x0F) : (bByte >> 4);
                int32 dy = isTopDown ? y : (height - 1 - y);
                canvas.pixels[dy * width + x] = palette[idx];
                if (i & 1) { ptr++; }
              }
              x++;
            }
            if ((val + 1) / 2 & 1) { ptr++; }
          }
        }
      }
    }
  } else {
    rowSize = (width * bpp + 31) / 32 * 4;
    for (y = 0; y < height; ++y) {
      int32 dy = isTopDown ? y : (height - 1 - y);
      row = data + y * rowSize;
      for (x = 0; x < width; ++x) {
        if (bpp == 1) {
          uint8 bByte = row[x / 8];
          uint8 idx = (bByte >> (7 - (x % 8))) & 1;
          canvas.pixels[dy * width + x] = palette[idx];
        } else if (bpp == 4) {
          uint8 bByte = row[x / 2];
          uint8 idx = (x & 1) ? (bByte & 0x0F) : (bByte >> 4);
          canvas.pixels[dy * width + x] = palette[idx];
        } else if (bpp == 8) {
          canvas.pixels[dy * width + x] = palette[row[x]];
        } else if (bpp == 16) {
          uint16 wWord = *(uint16*)(row + x * 2);
          r = ((wWord & rMask) >> shiftR) * scaleR;
          g = ((wWord & gMask) >> shiftG) * scaleG;
          b = ((wWord & bMask) >> shiftB) * scaleB;
          canvas.pixels[dy * width + x] =
              0xFF000000 | (r << 16) | (g << 8) | b;
        } else if (bpp == 24) {
          b = row[x * 3 + 0];
          g = row[x * 3 + 1];
          r = row[x * 3 + 2];
          canvas.pixels[dy * width + x] =
              0xFF000000 | (r << 16) | (g << 8) | b;
        } else if (bpp == 32) {
          uint32 dWord = *(uint32*)(row + x * 4);
          r = ((dWord & rMask) >> shiftR) * scaleR;
          g = ((dWord & gMask) >> shiftG) * scaleG;
          b = ((dWord & bMask) >> shiftB) * scaleB;
          a = aMask ? ((dWord & aMask) >> shiftA) * scaleA : 255;
          canvas.pixels[dy * width + x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
      }
    }
  }
  reich_sys_free(data);
  if (palette) { reich_sys_free(palette); }
  return canvas;
}

REICH_API uint8* reich_font_import(
    reichArena* a,
    uint32* pixels,
    int32 iw,
    int32 ih,
    int32 gw,
    int32 gh,
    int32 start,
    int32 end) {
  int32 x;
  int32 y;
  int32 i;
  int32 count = end - start + 1;
  int32 total = (count * gw * gh + 7) / 8;
  uint8* data = (uint8*)reich_arena_alloc(a, (reichSize)(5 + total));
  if (!data) { return NULL; }
  data[0] = (uint8)gw;
  data[1] = (uint8)gh;
  data[2] = 1;
  data[3] = (uint8)start;
  data[4] = (uint8)end;
  for (i = 0; i < total; ++i) { data[5 + i] = 0; }
  if (gw <= 0) { return data; }
  for (i = 0; i < count; ++i) {
    int32 gx = (i % (iw / gw)) * gw;
    int32 gy = (i / (iw / gw)) * gh;
    for (y = 0; y < gh; ++y) {
      for (x = 0; x < gw; ++x) {
        if (gx + x < iw && gy + y < ih) {
          if ((pixels[(gy + y) * iw + (gx + x)] & 0x00FFFFFF) != 0) {
            int32 b = i * gw * gh + y * gw + x;
            data[5 + (b >> 3)] |= (1 << (7 - (b & 7)));
          }
        }
      }
    }
  }
  return data;
}

REICH_API int32 reich_load_font(
    reichContext* ctx,
    const char* filename,
    int32 numChars,
    int32 glyphWidth,
    int32 glyphHeight) {
  reichCanvas bmp = reich_load_bmp(filename);
  if (bmp.pixels) {
    int32 gw = glyphWidth;
    int32 gh = glyphHeight;
    int32 rows = 16;
    int32 start = 0;
    int32 end = 255;
    int32 cols = 16;
    if (gw <= 0) { gw = bmp.width / 16; }
    if (gw > 0) { cols = bmp.width / gw; }
    if (cols < 1) { cols = 1; }
    if (numChars <= 0) {
      if (bmp.height % 16 != 0 && bmp.height % 6 == 0) {
        numChars = 96;
      } else if (
          bmp.height % 6 == 0 && (bmp.height / 6) >= gw &&
          (bmp.height / 16) < gw) {
        numChars = 96;
      } else {
        numChars = cols * 16;
        if (numChars > 256) { numChars = 256; }
      }
    }
    if (numChars == 96) {
      start = 32;
      end = 127;
    } else {
      start = 0;
      end = numChars - 1;
    }
    if (gh <= 0) {
      rows = (numChars + cols - 1) / cols;
      if (rows < 1) { rows = 1; }
      gh = bmp.height / rows;
    }
    if (gw > 0 && gh > 0) {
      ctx->fonts[ctx->fontCount++] = reich_font_import(
          &ctx->permMem,
          bmp.pixels,
          bmp.width,
          bmp.height,
          gw,
          gh,
          start,
          end);
      reich_sys_log(
          REICH_LOG_INFO,
          "Loaded font: %s (%d chars, %dx%d glyphs)",
          filename,
          numChars,
          gw,
          gh);
    } else {
      reich_sys_log(
          REICH_LOG_ERROR, "Invalid font dimensions for %s", filename);
    }
    reich_sys_free(bmp.pixels);
  } else {
    reich_sys_log(REICH_LOG_WARN, "Failed to load font bitmap: %s", filename);
  }
  return 1;
}

REICH_API int32 reich_load_fonts(
    reichContext* ctx, const char* fn, int32 gw, int32 gh, int32 amt) {
  char filename[260];
  reichHandle h = reich_sys_find_first(fn, filename, 260);
  if (h) {
    do {
      reich_load_font(ctx, filename, amt, gw, gh);
      reich_sys_log(REICH_LOG_INFO, "Loaded font %s", filename);
    } while (reich_sys_find_next(h, filename, 260));
    reich_sys_find_close(h);
  } else {
    reich_sys_log(REICH_LOG_INFO, "No font found");
    return 0;
  }
  if (ctx->fontCount == 0) { reich_init_default_font(ctx); }
  return 1;
}

/* DRAWING *****************************************************************+*/

real32 reich_sdf(real32 px, real32 py, real32 bx, real32 by, real32 r) {
  real32 dx, dy, mx, my, len, absPx, absPy;
  absPx = px < 0.0f ? -px : px;
  absPy = py < 0.0f ? -py : py;
  dx = absPx - bx + r;
  dy = absPy - by + r;
  mx = reich_max(dx, 0.0f);
  my = reich_max(dy, 0.0f);
  len = (real32)reich_sqrt((real64)(mx * mx + my * my));
  return reich_min(reich_max(dx, dy), 0.0f) + len - r;
}

static const real32 REICH_GAUSS_NORM[25] = {
    0.0000489f, 0.0001036f, 0.0001330f, 0.0001036f, 0.0000489f,
    0.0001036f, 0.0002193f, 0.0002816f, 0.0002193f, 0.0001036f,
    0.0001330f, 0.0002816f, 0.0003616f, 0.0002816f, 0.0001330f,
    0.0001036f, 0.0002193f, 0.0002816f, 0.0002193f, 0.0001036f,
    0.0000489f, 0.0001036f, 0.0001330f, 0.0001036f, 0.0000489f};

#define REICH_BLEND_ADD      0
#define REICH_BLEND_SCREEN   1
#define REICH_BLEND_OVERLAY  2
#define REICH_BLEND_MULTIPLY 3

#define REICH_GET_A(c) (((c) >> 24) & 0xFF)
#define REICH_GET_R(c) (((c) >> 16) & 0xFF)
#define REICH_GET_G(c) (((c) >> 8) & 0xFF)
#define REICH_GET_B(c) ((c) & 0xFF)

#define REICH_PIXEL_IN_CLIP(ctx, px, py)              \
  ((px) >= (ctx)->clip.x1 && (px) < (ctx)->clip.x2 && \
   (py) >= (ctx)->clip.y1 && (py) < (ctx)->clip.y2)

#define REICH_CLAMP_Y_INCL(ctx, minY, maxY)                    \
  do {                                                         \
    if ((minY) < (ctx)->clip.y1) (minY) = (ctx)->clip.y1;      \
    if ((maxY) >= (ctx)->clip.y2) (maxY) = (ctx)->clip.y2 - 1; \
  } while (0)

#define REICH_CLAMP_X_INCL(ctx, minX, maxX)                    \
  do {                                                         \
    if ((minX) < (ctx)->clip.x1) (minX) = (ctx)->clip.x1;      \
    if ((maxX) >= (ctx)->clip.x2) (maxX) = (ctx)->clip.x2 - 1; \
  } while (0)

#define REICH_CLAMP_BOUNDS_INCL(ctx, minX, minY, maxX, maxY) \
  do {                                                       \
    REICH_CLAMP_X_INCL(ctx, minX, maxX);                     \
    REICH_CLAMP_Y_INCL(ctx, minY, maxY);                     \
  } while (0)

#define REICH_CLAMP_EXCL(ctx, minX, minY, maxX, maxY)     \
  do {                                                    \
    if ((minX) < (ctx)->clip.x1) (minX) = (ctx)->clip.x1; \
    if ((minY) < (ctx)->clip.y1) (minY) = (ctx)->clip.y1; \
    if ((maxX) > (ctx)->clip.x2) (maxX) = (ctx)->clip.x2; \
    if ((maxY) > (ctx)->clip.y2) (maxY) = (ctx)->clip.y2; \
  } while (0)

#define REICH_CALC_BOUNDS_INCL(start_val, size, min_val, max_val)  \
  do {                                                             \
    (min_val) = reich_ceil((double)(start_val) - 0.5);             \
    (max_val) = reich_floor((double)((start_val) + (size)) - 0.5); \
  } while (0)

#define REICH_CALC_BOUNDS_RAD_INCL(center, radius, min_val, max_val)    \
  do {                                                                  \
    (min_val) = reich_ceil((double)(center) - (double)(radius) - 0.5);  \
    (max_val) = reich_floor((double)(center) + (double)(radius) - 0.5); \
  } while (0)

#define REICH_DRAW_PIXEL_FAST(ctx, px, py, color)                          \
  do {                                                                     \
    uint32* _p = &(ctx)->canvas.pixels[(py) * (ctx)->canvas.width + (px)]; \
    REICH_BLEND_FAST(color, *_p);                                          \
  } while (0)

REICH_API uint32 reich_color_lerp(uint32 c1, uint32 c2, float t) {
  uint32 r =
      (uint32)(REICH_GET_R(c1) + (REICH_GET_R(c2) - REICH_GET_R(c1)) * t);
  uint32 g =
      (uint32)(REICH_GET_G(c1) + (REICH_GET_G(c2) - REICH_GET_G(c1)) * t);
  uint32 b =
      (uint32)(REICH_GET_B(c1) + (REICH_GET_B(c2) - REICH_GET_B(c1)) * t);
  uint32 a =
      (uint32)(REICH_GET_A(c1) + (REICH_GET_A(c2) - REICH_GET_A(c1)) * t);
  return (a << 24) | (r << 16) | (g << 8) | b;
}

#define REICH_BLEND_FAST(src, dst)                                  \
  do {                                                              \
    uint32 sa = REICH_GET_A(src);                                   \
    if (sa == 255) {                                                \
      dst = src;                                                    \
    } else if (sa > 0) {                                            \
      uint32 da = REICH_GET_A(dst);                                 \
      uint32 inv_sa = 255 - sa;                                     \
      uint32 out_a = sa + REICH_DIV255(da * inv_sa);                \
      if (out_a > 0) {                                              \
        uint32 r = (REICH_GET_R(src) * sa +                         \
                    REICH_DIV255(REICH_GET_R(dst) * da * inv_sa)) / \
            out_a;                                                  \
        uint32 g = (REICH_GET_G(src) * sa +                         \
                    REICH_DIV255(REICH_GET_G(dst) * da * inv_sa)) / \
            out_a;                                                  \
        uint32 b = (REICH_GET_B(src) * sa +                         \
                    REICH_DIV255(REICH_GET_B(dst) * da * inv_sa)) / \
            out_a;                                                  \
        dst = (out_a << 24) | (r << 16) | (g << 8) | b;             \
      }                                                             \
    }                                                               \
  } while (0)

real32 reich_blend_colour(
    real32 base, real32 blend, real32 alpha, int32 mode) {
  real32 res = base;
  if (mode == REICH_BLEND_ADD) {
    res = base + blend;
  } else if (mode == REICH_BLEND_SCREEN) {
    res = 1.0f - (1.0f - base) * (1.0f - blend);
  } else if (mode == REICH_BLEND_OVERLAY) {
    res = base < 0.5f ? (2.0f * base * blend)
                      : (1.0f - 2.0f * (1.0f - base) * (1.0f - blend));
  } else if (mode == REICH_BLEND_MULTIPLY) {
    res = base * blend;
  }
  res = REICH_CLAMP(res, 0.0f, 1.0f);
  return base * (1.0f - alpha) + res * alpha;
}

static int32 reich_draw_span(
    reichContext* ctx,
    int32 py,
    int32 sx,
    int32 ex,
    uint32 color,
    uint32 alpha) {
  uint32* p;
  int32 n;
  REICH_CLAMP_X_INCL(ctx, sx, ex);
  if (sx > ex) { return 0; }
  p = ctx->canvas.pixels + py * ctx->canvas.width + sx;
  n = ex - sx + 1;
  if (alpha == 255) {
    while (n >= 4) {
      *p++ = color;
      *p++ = color;
      *p++ = color;
      *p++ = color;
      n -= 4;
    }
    while (n--) { *p++ = color; }
  } else {
    while (n--) {
      REICH_BLEND_FAST(color, *p);
      p++;
    }
  }
  reich_dirty_add(ctx, sx, py, ex + 1, py + 1);
  return 1;
}

REICH_API int32
reich_draw_pixel(reichContext* ctx, int32 x, int32 y, uint32 color) {
  if (REICH_PIXEL_IN_CLIP(ctx, x, y)) {
    REICH_DRAW_PIXEL_FAST(ctx, x, y, color);
    reich_dirty_add(ctx, x, y, x + 1, y + 1);
  }
  return 1;
}

REICH_API int32 reich_draw_clear(reichContext* ctx, uint32 color) {
  int32 count = ctx->canvas.width * ctx->canvas.height;
  uint32* p = ctx->canvas.pixels;
  while (count--) { *p++ = color; }
  reich_dirty_add(ctx, 0, 0, ctx->canvas.width, ctx->canvas.height);
  return 1;
}

REICH_API int32 reich_draw_line(
    reichContext* ctx,
    real32 x1,
    real32 y1,
    real32 x2,
    real32 y2,
    uint32 colour) {
  float dx, dy, step, x, y;
  int32 i, steps, ix, iy;
  uint32 alpha = REICH_GET_A(colour);
  if (alpha == 0) { return 0; }

  dx = x2 - x1;
  dy = y2 - y1;
  step = fabs(dx) >= fabs(dy) ? fabs(dx) : fabs(dy);

  if (step <= 0.0f) {
    ix = (int32)(x1 + 0.5f);
    iy = (int32)(y1 + 0.5f);
    if (REICH_PIXEL_IN_CLIP(ctx, ix, iy)) {
      REICH_DRAW_PIXEL_FAST(ctx, ix, iy, colour);
      reich_dirty_add(ctx, ix, iy, ix + 1, iy + 1);
    }
    return 1;
  }

  dx /= step;
  dy /= step;
  x = x1;
  y = y1;
  steps = (int32)step;

  for (i = 0; i <= steps; ++i) {
    ix = (int32)(x + 0.5f);
    iy = (int32)(y + 0.5f);
    if (REICH_PIXEL_IN_CLIP(ctx, ix, iy)) {
      REICH_DRAW_PIXEL_FAST(ctx, ix, iy, colour);
      reich_dirty_add(ctx, ix, iy, ix + 1, iy + 1);
    }
    x += dx;
    y += dy;
  }
  return 1;
}

REICH_API int32 reich_draw_line_thick(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float thickness,
    uint32 color) {
  float minX_f, minY_f, maxX_f, maxY_f, l2, half_thick;
  int32 minX, minY, maxX, maxY, px, py;
  uint32 alpha = REICH_GET_A(color);

  if (thickness <= 1.0f) {
    return reich_draw_line(ctx, x1, y1, x2, y2, color);
  }
  if (alpha == 0) { return 0; }

  half_thick = thickness * 0.5f;
  minX_f = (x1 < x2 ? x1 : x2) - half_thick;
  minY_f = (y1 < y2 ? y1 : y2) - half_thick;
  maxX_f = (x1 > x2 ? x1 : x2) + half_thick;
  maxY_f = (y1 > y2 ? y1 : y2) + half_thick;

  minX = reich_floor((double)minX_f - 0.5);
  minY = reich_floor((double)minY_f - 0.5);
  maxX = reich_ceil((double)maxX_f + 0.5);
  maxY = reich_ceil((double)maxY_f + 0.5);
  REICH_CLAMP_BOUNDS_INCL(ctx, minX, minY, maxX, maxY);

  l2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
  for (py = minY; py <= maxY; ++py) {
    for (px = minX; px <= maxX; ++px) {
      float dx = (float)px + 0.5f - x1;
      float dy = (float)py + 0.5f - y1;
      float t, dist, projX, projY, a;

      if (l2 == 0.0f) {
        t = 0.0f;
      } else {
        t = (dx * (x2 - x1) + dy * (y2 - y1)) / l2;
        if (t < 0.0f) {
          t = 0.0f;
        } else if (t > 1.0f) {
          t = 1.0f;
        }
      }
      projX = x1 + t * (x2 - x1);
      projY = y1 + t * (y2 - y1);
      dist = (float)reich_sqrt(
          (double)(((float)px + 0.5f - projX) * ((float)px + 0.5f - projX) +
                   ((float)py + 0.5f - projY) * ((float)py + 0.5f - projY)));

      if (dist <= half_thick) {
        a = half_thick - dist;
        if (a >= 1.0f) {
          reich_draw_pixel(ctx, px, py, color);
        } else if (a > 0.0f) {
          uint32 fa = (uint32)((float)alpha * a);
          uint32 outColor = (fa << 24) | (color & 0x00FFFFFF);
          reich_draw_pixel(ctx, px, py, outColor);
        }
      }
    }
  }
  reich_dirty_add(ctx, minX, minY, maxX + 1, maxY + 1);
  return 1;
}

REICH_API int32 reich_draw_rect_fill(
    reichContext* ctx, float x, float y, float w, float h, uint32 color) {
  int32 startX, startY, endX, endY, j;
  uint32 alpha;
  if (!ctx || !ctx->canvas.pixels || w <= 0.0f || h <= 0.0f) { return 0; }

  alpha = REICH_GET_A(color);
  if (alpha == 0) { return 0; }

  REICH_CALC_BOUNDS_INCL(x, w, startX, endX);
  REICH_CALC_BOUNDS_INCL(y, h, startY, endY);
  REICH_CLAMP_Y_INCL(ctx, startY, endY);

  for (j = startY; j <= endY; ++j) {
    reich_draw_span(ctx, j, startX, endX, color, alpha);
  }
  return 1;
}

REICH_API int32 reich_draw_rect_gradient(
    reichContext* ctx,
    float x,
    float y,
    float w,
    float h,
    uint32 tl,
    uint32 tr,
    uint32 bl,
    uint32 br) {
  int32 px, py, startX, startY, endX, endY;
  float tX, tY, invX, invY, fa, fr, fg, fb;
  uint32 outColor;
  float a1, r1, g1, b1, a2, r2, g2, b2, a3, r3, g3, b3, a4, r4, g4, b4;

  if (w <= 0.0f || h <= 0.0f) { return 0; }
  REICH_CALC_BOUNDS_INCL(x, w, startX, endX);
  REICH_CALC_BOUNDS_INCL(y, h, startY, endY);
  REICH_CLAMP_BOUNDS_INCL(ctx, startX, startY, endX, endY);

  a1 = (float)REICH_GET_A(tl);
  r1 = (float)REICH_GET_R(tl);
  g1 = (float)REICH_GET_G(tl);
  b1 = (float)REICH_GET_B(tl);
  a2 = (float)REICH_GET_A(tr);
  r2 = (float)REICH_GET_R(tr);
  g2 = (float)REICH_GET_G(tr);
  b2 = (float)REICH_GET_B(tr);
  a3 = (float)REICH_GET_A(bl);
  r3 = (float)REICH_GET_R(bl);
  g3 = (float)REICH_GET_G(bl);
  b3 = (float)REICH_GET_B(bl);
  a4 = (float)REICH_GET_A(br);
  r4 = (float)REICH_GET_R(br);
  g4 = (float)REICH_GET_G(br);
  b4 = (float)REICH_GET_B(br);

  for (py = startY; py <= endY; ++py) {
    tY = ((float)py + 0.5f - y) / h;
    if (tY < 0.0f) {
      tY = 0.0f;
    } else if (tY > 1.0f) {
      tY = 1.0f;
    }
    invY = 1.0f - tY;
    for (px = startX; px <= endX; ++px) {
      tX = ((float)px + 0.5f - x) / w;
      if (tX < 0.0f) {
        tX = 0.0f;
      } else if (tX > 1.0f) {
        tX = 1.0f;
      }
      invX = 1.0f - tX;
      fa = (a1 * invX + a2 * tX) * invY + (a3 * invX + a4 * tX) * tY;
      if (fa > 0.0f) {
        fr = (r1 * invX + r2 * tX) * invY + (r3 * invX + r4 * tX) * tY;
        fg = (g1 * invX + g2 * tX) * invY + (g3 * invX + g4 * tX) * tY;
        fb = (b1 * invX + b2 * tX) * invY + (b3 * invX + b4 * tX) * tY;
        outColor = ((uint32)fa << 24) | ((uint32)fr << 16) |
            ((uint32)fg << 8) | (uint32)fb;
        reich_draw_pixel(ctx, px, py, outColor);
      }
    }
  }
  reich_dirty_add(ctx, startX, startY, endX + 1, endY + 1);
  return 1;
}

REICH_API int32 reich_draw_rect(
    reichContext* ctx, float x, float y, float w, float h, uint32 color) {
  reich_draw_line(ctx, x, y, x + w, y, color);
  reich_draw_line(ctx, x, y + h, x + w, y + h, color);
  reich_draw_line(ctx, x, y, x, y + h, color);
  reich_draw_line(ctx, x + w, y, x + w, y + h, color);
  return 1;
}

REICH_API int32 reich_draw_circle_fill(
    reichContext* ctx, float cx, float cy, float r, uint32 color) {
  int32 min_y, max_y, sx, ex, py;
  double r2, dy, dx;
  uint32 alpha = REICH_GET_A(color);

  if (r <= 0.0f || alpha == 0) { return 0; }
  r2 = (double)r * (double)r;

  REICH_CALC_BOUNDS_RAD_INCL(cy, r, min_y, max_y);
  REICH_CLAMP_Y_INCL(ctx, min_y, max_y);

  for (py = min_y; py <= max_y; ++py) {
    dy = ((double)py + 0.5) - (double)cy;
    dx = reich_sqrt(r2 - dy * dy);
    REICH_CALC_BOUNDS_RAD_INCL(cx, dx, sx, ex);
    reich_draw_span(ctx, py, sx, ex, color, alpha);
  }
  return 1;
}

REICH_API int32 reich_draw_circle(
    reichContext* ctx, float cx, float cy, float r, float t, uint32 color) {
  int32 py, min_y, max_y, s, spans[2][2];
  double r2, rin2, dy, dx_out, dx_in, rin;
  uint32 alpha = REICH_GET_A(color);

  if (r <= 0.0f || t <= 0.0f || alpha == 0) { return 0; }
  if (t >= r) { return reich_draw_circle_fill(ctx, cx, cy, r, color); }

  rin = (double)r - (double)t;
  r2 = (double)r * (double)r;
  rin2 = rin * rin;

  REICH_CALC_BOUNDS_RAD_INCL(cy, r, min_y, max_y);
  REICH_CLAMP_Y_INCL(ctx, min_y, max_y);

  for (py = min_y; py <= max_y; ++py) {
    dy = ((double)py + 0.5) - (double)cy;
    dx_out = reich_sqrt(r2 - dy * dy);
    spans[0][0] = 0;
    spans[0][1] = -1;
    spans[1][0] = 0;
    spans[1][1] = -1;

    if (dy * dy >= rin2) {
      REICH_CALC_BOUNDS_RAD_INCL(cx, dx_out, spans[0][0], spans[0][1]);
    } else {
      dx_in = reich_sqrt(rin2 - dy * dy);
      int32 sx_out, ex_out, sx_in, ex_in;
      REICH_CALC_BOUNDS_RAD_INCL(cx, dx_out, sx_out, ex_out);
      REICH_CALC_BOUNDS_RAD_INCL(cx, dx_in, sx_in, ex_in);

      spans[0][0] = sx_out;
      spans[0][1] = sx_in - 1;
      spans[1][0] = ex_in + 1;
      spans[1][1] = ex_out;
    }

    for (s = 0; s < 2; ++s) {
      reich_draw_span(ctx, py, spans[s][0], spans[s][1], color, alpha);
    }
  }
  return 1;
}

REICH_API int32 reich_draw_rect_rounded(
    reichContext* ctx,
    float x,
    float y,
    float w,
    float h,
    float r,
    float thickness,
    uint32 color) {
  int32 min_y, max_y, py, fill;
  float cx, cy, hx, hy, r_in, hx_in, hy_in;
  uint32 alpha = REICH_GET_A(color);

  if (w <= 0.0f || h <= 0.0f || alpha == 0) { return 0; }

  cx = x + w * 0.5f;
  cy = y + h * 0.5f;
  hx = w * 0.5f - r;
  hy = h * 0.5f - r;

  if (hx < 0.0f) {
    r += hx;
    hx = 0.0f;
  }
  if (hy < 0.0f) {
    r += hy;
    hy = 0.0f;
  }

  fill = (thickness <= 0.0f || thickness * 2.0f >= w || thickness * 2.0f >= h);
  if (!fill) {
    r_in = r - thickness;
    hx_in = hx;
    hy_in = hy;
    if (r_in < 0.0f) {
      hx_in += r_in;
      hy_in += r_in;
      r_in = 0.0f;
    }
  }

  REICH_CALC_BOUNDS_INCL(y, h, min_y, max_y);
  REICH_CLAMP_Y_INCL(ctx, min_y, max_y);

  for (py = min_y; py <= max_y; ++py) {
    double dy = fabs((double)py + 0.5 - (double)cy) - (double)hy;
    double dx_out;
    int32 sx_out, ex_out;

    if (dy > (double)r) { continue; }
    if (dy <= 0.0) {
      dx_out = (double)hx + (double)r;
    } else {
      dx_out = (double)hx + reich_sqrt((double)r * (double)r - dy * dy);
    }

    REICH_CALC_BOUNDS_RAD_INCL(cx, dx_out, sx_out, ex_out);

    if (fill) {
      reich_draw_span(ctx, py, sx_out, ex_out, color, alpha);
    } else {
      double dy_in = fabs((double)py + 0.5 - (double)cy) - (double)hy_in;
      double dx_in;
      int32 sx_in, ex_in, s, spans[2][2];

      spans[0][0] = 0;
      spans[0][1] = -1;
      spans[1][0] = 0;
      spans[1][1] = -1;

      if (dy_in >= (double)r_in) {
        spans[0][0] = sx_out;
        spans[0][1] = ex_out;
      } else {
        if (dy_in <= 0.0) {
          dx_in = (double)hx_in + (double)r_in;
        } else {
          dx_in = (double)hx_in +
              reich_sqrt((double)r_in * (double)r_in - dy_in * dy_in);
        }

        REICH_CALC_BOUNDS_RAD_INCL(cx, dx_in, sx_in, ex_in);

        spans[0][0] = sx_out;
        spans[0][1] = sx_in - 1;
        spans[1][0] = ex_in + 1;
        spans[1][1] = ex_out;
      }

      for (s = 0; s < 2; ++s) {
        reich_draw_span(ctx, py, spans[s][0], spans[s][1], color, alpha);
      }
    }
  }
  return 1;
}

REICH_API int32 reich_draw_triangle(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    uint32 color) {
  reich_draw_line(ctx, x1, y1, x2, y2, color);
  reich_draw_line(ctx, x2, y2, x3, y3, color);
  reich_draw_line(ctx, x3, y3, x1, y1, color);
  return 1;
}

REICH_API int32 reich_draw_triangle_fill(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    uint32 color) {
  float minX_f, minY_f, maxX_f, maxY_f;
  int32 minX, minY, maxX, maxY, px, py;
  uint32 alpha = REICH_GET_A(color);

  if (alpha == 0) { return 0; }
  minX_f = x1;
  minY_f = y1;
  maxX_f = x1;
  maxY_f = y1;
  if (x2 < minX_f) { minX_f = x2; }
  if (x3 < minX_f) { minX_f = x3; }
  if (y2 < minY_f) { minY_f = y2; }
  if (y3 < minY_f) { minY_f = y3; }
  if (x2 > maxX_f) { maxX_f = x2; }
  if (x3 > maxX_f) { maxX_f = x3; }
  if (y2 > maxY_f) { maxY_f = y2; }
  if (y3 > maxY_f) { maxY_f = y3; }

  minX = reich_floor((double)minX_f - 0.5);
  minY = reich_floor((double)minY_f - 0.5);
  maxX = reich_ceil((double)maxX_f + 0.5);
  maxY = reich_ceil((double)maxY_f + 0.5);
  REICH_CLAMP_BOUNDS_INCL(ctx, minX, minY, maxX, maxY);

  for (py = minY; py <= maxY; ++py) {
    float py_f = (float)py + 0.5f;
    for (px = minX; px <= maxX; ++px) {
      float px_f = (float)px + 0.5f;
      float w0 = (x2 - x1) * (py_f - y1) - (y2 - y1) * (px_f - x1);
      float w1 = (x3 - x2) * (py_f - y2) - (y3 - y2) * (px_f - x2);
      float w2 = (x1 - x3) * (py_f - y3) - (y1 - y3) * (px_f - x3);
      if ((w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) ||
          (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f)) {
        REICH_DRAW_PIXEL_FAST(ctx, px, py, color);
      }
    }
  }
  reich_dirty_add(ctx, minX, minY, maxX + 1, maxY + 1);
  return 1;
}


REICH_API void reich_draw_triangle_textured(
    reichContext* ctx,
    real64 v0X, real64 v0Y, real32 u0, real32 v0,
    real64 v1X, real64 v1Y, real32 u1, real32 v1,
    real64 v2X, real64 v2Y, real32 u2, real32 v2,
    uint32* textureData,
    int texWidth,
    int texHeight,
    uint32 tintColor) {

  int minPixelX, maxPixelX, minPixelY, maxPixelY, px, py, sampleX, sampleY;
  real64 determinant, invDeterminant, pixelX, pixelY, barycentric1, barycentric2, barycentric3;
  uint32 finalR, finalG, finalB, texColor;
  uint32 tintR = (tintColor >> 16) & 0xFF, tintG = (tintColor >> 8) & 0xFF, tintB = tintColor & 0xFF;
	uint32* pix = NULL;
  minPixelX = (int)reich_floor((float)REICH_MIN(v0X, REICH_MIN(v1X, v2X)));
  maxPixelX = (int)reich_ceil((float)REICH_MAX(v0X, REICH_MAX(v1X, v2X)));
  minPixelY = (int)reich_floor((float)REICH_MIN(v0Y, REICH_MIN(v1Y, v2Y)));
  maxPixelY = (int)reich_ceil((float)REICH_MAX(v0Y, REICH_MAX(v1Y, v2Y)));

  if (minPixelX < ctx->clip.x1) { minPixelX = ctx->clip.x1; }
  if (maxPixelX > ctx->clip.x2) { maxPixelX = ctx->clip.x2; }
  if (minPixelY < ctx->clip.y1) { minPixelY = ctx->clip.y1; }
  if (maxPixelY > ctx->clip.y2) { maxPixelY = ctx->clip.y2; }

  determinant = (v1Y - v2Y) * (v0X - v2X) + (v2X - v1X) * (v0Y - v2Y);
  if (determinant > -0.0001 && determinant < 0.0001) { return; }

  invDeterminant = 1.0 / determinant;

  for (py = minPixelY; py <= maxPixelY; py++) {
    pix = &ctx->canvas.pixels[minPixelX + py*ctx->canvas.width];
		for (px = minPixelX; px <= maxPixelX; px++) {
      pixelX = (real64)px;
      pixelY = (real64)py;

      barycentric1 = ((v1Y - v2Y) * (pixelX - v2X) + (v2X - v1X) * (pixelY - v2Y)) * invDeterminant;
      barycentric2 = ((v2Y - v0Y) * (pixelX - v2X) + (v0X - v2X) * (pixelY - v2Y)) * invDeterminant;
      barycentric3 = 1.0 - barycentric1 - barycentric2;

      if (barycentric1 >= 0.0 && barycentric2 >= 0.0 && barycentric3 >= 0.0) {
        sampleX = (int)((barycentric1 * u0 + barycentric2 * u1 + barycentric3 * u2) * texWidth) % texWidth;
        sampleY = (int)((barycentric1 * v0 + barycentric2 * v1 + barycentric3 * v2) * texHeight) % texHeight;

        if (sampleX < 0) { sampleX += texWidth; }
        if (sampleY < 0) { sampleY += texHeight; }

        texColor = textureData[sampleY * texWidth + sampleX];

        finalR = (((texColor >> 16) & 0xFF) * tintR) >> 8;
        finalG = (((texColor >> 8) & 0xFF) * tintG) >> 8;
        finalB = ((texColor & 0xFF) * tintB) >> 8;
				*pix =  0xFF000000 | (finalR << 16) | (finalG << 8) | finalB;
      }
			pix++;
    }
  }
}


int32 reich_draw_quad_fill(reichContext* ctx,
    real32 x0, real32 y0, real32 x1, real32 y1, real32 x2, real32 y2,
    real32 x3, real32 y3, uint32 colour) {
  int32 y, x;
  int32 i, j, cnt;
  real32 yi, xi, yj, xj;
  real32 minY, maxY, xMin, xMax;
  real32 vx[4];
  real32 vy[4];
  uint32 *data = ctx->canvas.pixels;
  int32 width = ctx->canvas.width;
  uint32 col = colour;
  real32 inters[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  int32 inBounds = 0;
	uint32* pix = NULL;
	vx[0] = x0;
  vx[1] = x1;
  vx[2] = x2;
  vx[3] = x3;

  vy[0] = y0;
  vy[1] = y1;
  vy[2] = y2;
  vy[3] = y3;

  minY = vy[0];
  maxY = vy[0];
	inBounds =
		x0 >= ctx->clip.x1 && x0 < ctx->clip.x2 &&
		x1 >= ctx->clip.x1 && x1 < ctx->clip.x2 &&
		x2 >= ctx->clip.x1 && x2 < ctx->clip.x2 &&
		x3 >= ctx->clip.x1 && x3 < ctx->clip.x2 &&
		y0 >= ctx->clip.y1 && y0 < ctx->clip.y2 &&
		y1 >= ctx->clip.y1 && y1 < ctx->clip.y2 &&
		y2 >= ctx->clip.y1 && y2 < ctx->clip.y2 &&
		y3 >= ctx->clip.y1 && y3 < ctx->clip.y2;
  for (i = 1; i < 4; i++) {
    if (vy[i] < minY) { minY = vy[i]; }
    if (vy[i] > maxY) { maxY = vy[i]; }
  }
  for (y = (int32)(reich_ceild(minY)); y <= (int32)(REICH_FAST_FLOOR(maxY)); y++) {
    inters[0] = 0.0f;
    inters[1] = 0.0f;
    inters[2] = 0.0f;
    inters[3] = 0.0f;
    cnt = 0;
    for (i = 0, j = 1; i < 4; j = (++i + 1) % 4) {
      yi = vy[i];
      yj = vy[j];
      if ((yi <= y && yj > y) || (yj <= y && yi > y)) {
        xi = vx[i];
        xj = vx[j];
				inters[cnt++] = xi + (y - yi) * (xj - xi) / (yj - yi);
      }
    }
    if (cnt < 2) { continue; }
    xMin = inters[0];
    xMax = inters[0];
    for (i = 1; i < cnt; i++) {
      if (inters[i] < xMin) { xMin = inters[i]; }
      if (inters[i] > xMax) { xMax = inters[i]; }
    }
    if (xMin < 0) { xMin = 0; }
    if (xMax >= width) { xMax = width - 1; }
    if(inBounds) {
			x = (int32)(reich_ceild(xMin));
			pix = &data[x + y * width];
      for (x = (int32)(reich_ceild(xMin)); x <= (int32)(REICH_FAST_FLOOR(xMax)); x++) {
        *pix++ = col;
    	}
		} else {
	  	for (x = (int32)(reich_ceild(xMin)); x <= (int32)(REICH_FAST_FLOOR(xMax)); x++) {
        reich_draw_pixel(ctx, x, y, colour);
    	}		
		}
  }

  return TRUE;
}

int32 reich_draw_quad(reichContext* ctx,
    real32 x0, real32 y0, real32 x1, real32 y1, real32 x2, real32 y2,
    real32 x3, real32 y3, uint32 col) {
  reich_draw_line(ctx, x0, y0, x1, y1, col);
  reich_draw_line(ctx, x1, y1, x2, y2, col);
  reich_draw_line(ctx, x2, y2, x3, y3, col);
  reich_draw_line(ctx, x3, y3, x0, y0, col);
  return TRUE;
}


REICH_API int32 reich_draw_ellipse_fill(
    reichContext* ctx, float cx, float cy, float rx, float ry, uint32 color) {
  int32 minY, maxY, py;
  uint32 alpha = REICH_GET_A(color);
  float ry2;

  if (alpha == 0 || rx <= 0.0f || ry <= 0.0f) { return 0; }

  REICH_CALC_BOUNDS_RAD_INCL(cy, ry, minY, maxY);
  REICH_CLAMP_Y_INCL(ctx, minY, maxY);

  ry2 = ry * ry;
  for (py = minY; py <= maxY; ++py) {
    float dy = ((float)py + 0.5f) - cy;
    float dy2 = dy * dy;
    if (dy2 <= ry2) {
      float dx = rx * (float)reich_sqrt((double)(1.0f - dy2 / ry2));
      int32 sx, ex;
      REICH_CALC_BOUNDS_RAD_INCL(cx, dx, sx, ex);
      reich_draw_span(ctx, py, sx, ex, color, alpha);
    }
  }
  return 1;
}

REICH_API int32 reich_draw_ellipse(
    reichContext* ctx,
    float cx,
    float cy,
    float rx,
    float ry,
    float thickness,
    uint32 color) {
  int32 minY, maxY, py, s;
  uint32 alpha = REICH_GET_A(color);
  float ry2, in_rx, in_ry, in_ry2;

  if (alpha == 0 || rx <= 0.0f || ry <= 0.0f || thickness <= 0.0f) {
    return 0;
  }
  if (thickness >= rx || thickness >= ry) {
    return reich_draw_ellipse_fill(ctx, cx, cy, rx, ry, color);
  }

  REICH_CALC_BOUNDS_RAD_INCL(cy, ry, minY, maxY);
  REICH_CLAMP_Y_INCL(ctx, minY, maxY);

  ry2 = ry * ry;
  in_rx = rx - thickness;
  in_ry = ry - thickness;
  in_ry2 = in_ry * in_ry;

  for (py = minY; py <= maxY; ++py) {
    float dy = ((float)py + 0.5f) - cy;
    float dy2 = dy * dy;
    if (dy2 <= ry2) {
      float dx_out = rx * (float)reich_sqrt((double)(1.0f - dy2 / ry2));
      int32 spans[2][2];
      spans[0][0] = 0;
      spans[0][1] = -1;
      spans[1][0] = 0;
      spans[1][1] = -1;

      if (dy2 >= in_ry2) {
        REICH_CALC_BOUNDS_RAD_INCL(cx, dx_out, spans[0][0], spans[0][1]);
      } else {
        float dx_in = in_rx * (float)reich_sqrt((double)(1.0f - dy2 / in_ry2));
        int32 sx_out, ex_out, sx_in, ex_in;
        REICH_CALC_BOUNDS_RAD_INCL(cx, dx_out, sx_out, ex_out);
        REICH_CALC_BOUNDS_RAD_INCL(cx, dx_in, sx_in, ex_in);

        spans[0][0] = sx_out;
        spans[0][1] = sx_in - 1;
        spans[1][0] = ex_in + 1;
        spans[1][1] = ex_out;
      }
      for (s = 0; s < 2; ++s) {
        reich_draw_span(ctx, py, spans[s][0], spans[s][1], color, alpha);
      }
    }
  }
  return 1;
}

REICH_API int32 reich_draw_bezier_quad(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    int32 segments,
    float thickness,
    uint32 color) {
  int32 i;
  float t, invT, px, py, lastX, lastY;
  if (segments < 1) { segments = 1; }
  lastX = x1;
  lastY = y1;
  for (i = 1; i <= segments; ++i) {
    t = (float)i / (float)segments;
    invT = 1.0f - t;
    px = invT * invT * x1 + 2.0f * invT * t * x2 + t * t * x3;
    py = invT * invT * y1 + 2.0f * invT * t * y2 + t * t * y3;
    reich_draw_line_thick(ctx, lastX, lastY, px, py, thickness, color);
    lastX = px;
    lastY = py;
  }
  return 1;
}

REICH_API int32 reich_draw_bezier_cubic(
    reichContext* ctx,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4,
    int32 segments,
    float thickness,
    uint32 color) {
  int32 i;
  float t, invT, px, py, lastX, lastY;
  if (segments < 1) { segments = 1; }
  lastX = x1;
  lastY = y1;
  for (i = 1; i <= segments; ++i) {
    t = (float)i / (float)segments;
    invT = 1.0f - t;
    px = invT * invT * invT * x1 + 3.0f * invT * invT * t * x2 +
        3.0f * invT * t * t * x3 + t * t * t * x4;
    py = invT * invT * invT * y1 + 3.0f * invT * invT * t * y2 +
        3.0f * invT * t * t * y3 + t * t * t * y4;
    reich_draw_line_thick(ctx, lastX, lastY, px, py, thickness, color);
    lastX = px;
    lastY = py;
  }
  return 1;
}

REICH_API int32 reich_draw_text(
    reichContext* ctx, int32 x, int32 y, const char* str, uint32 color) {
  int32 i, j, cx, cy, fw, fh, start, end;
  uint8 c, *font;
  uint32 kern = 4;
  if (!ctx || !str || ctx->activeFont >= ctx->fontCount) { return 0; }
  font = ctx->fonts[ctx->activeFont];
  fw = font[0];
  fh = font[1];
  start = font[3];
  end = font[4];
  cx = x;
  cy = y;

  while (*str) {
    c = (uint8)*str++;
    if (c == '\n') {
      cx = x;
      cy += fh + 2;
      continue;
    }
    if (c >= start && c <= end) {
      int32 idx = (c - start) * (fw * fh);
      for (j = 0; j < fh; ++j) {
        for (i = 0; i < fw; ++i) {
          int32 bit = idx + (j * fw + i);
          if ((font[5 + (bit >> 3)] >> (7 - (bit & 7))) & 1) {
            if (REICH_PIXEL_IN_CLIP(ctx, cx + i, cy + j)) {
              REICH_DRAW_PIXEL_FAST(ctx, cx + i, cy + j, color);
              reich_dirty_add(ctx, cx + i, cy + j, cx + i + 1, cy + j + 1);
            }
          }
        }
      }
    }
    cx += fw + font[2] - kern;
  }
  return 1;
}

/* DRAW::GUI *****************************************************************/

#define REICH_PALETTE_SIZE 16

#define REICH_PALETTE_HIGHLIGHT            0
#define REICH_PALETTE_NEUTRAL_HIGH         1
#define REICH_PALETTE_NEUTRAL              2
#define REICH_PALETTE_NEUTRAL_LOW          3
#define REICH_PALETTE_SHADOW               4
#define REICH_PALETTE_DARK                 5
#define REICH_PALETTE_TEXT                 6
#define REICH_PALETTE_TITLE_ACTIVE_START   7
#define REICH_PALETTE_TITLE_ACTIVE_END     8
#define REICH_PALETTE_TITLE_INACTIVE_START 9
#define REICH_PALETTE_TITLE_INACTIVE_END   10

static uint32 REICH_GUI_PALETTE_WIN2K[REICH_PALETTE_SIZE] = {
    0xFFFFFFFF,
    0xFFF0F0F0,
    0xFFD4D0C8,
    0xFFC6C0C0,
    0xFF808080,
    0xFF404040,
    0xFF000000,
    0xFF12256C,
    0xFFABCAF2,
    0xFF808080,
    0xFFC0C0C0};

static uint32* REICH_GUI_PALETTE = REICH_GUI_PALETTE_WIN2K;

REICH_API int32 reich_draw_rect_ui_button(
    reichContext* ctx, int32 x1, int32 y1, int32 x2, int32 y2, int32 state) {
  x2--;
  y2--;
  if (state) {
    reich_draw_line(
        ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_DARK]);
    reich_draw_line(
        ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_DARK]);
    reich_draw_line(
        ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    reich_draw_line(
        ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    x1++;
    y1++;
    x2--;
    y2--;
    reich_draw_line(
        ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
    reich_draw_line(
        ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
    reich_draw_line(
        ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
    reich_draw_line(
        ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
    x1++;
    y1++;
  } else {
    reich_draw_line(
        ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    reich_draw_line(
        ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    reich_draw_line(
        ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_DARK]);
    reich_draw_line(
        ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_DARK]);
    x1++;
    y1++;
    x2--;
    y2--;
    reich_draw_line(
        ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
    reich_draw_line(
        ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
    reich_draw_line(
        ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
    reich_draw_line(
        ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
    x1++;
    y1++;
  }

  reich_draw_rect_fill(
      ctx, x1, y1, x2 - x1, y2 - y1, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
  return TRUE;
}

REICH_API int32 reich_draw_rect_ui_panel(
    reichContext* ctx, int32 x1, int32 y1, int32 x2, int32 y2, int32 state) {
  if (state) {
    reich_draw_line(
        ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    reich_draw_line(
        ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    reich_draw_line(
        ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
    reich_draw_line(
        ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
  } else {
    reich_draw_line(
        ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    reich_draw_line(
        ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
    reich_draw_line(
        ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
    reich_draw_line(
        ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
  }
  x1++;
  y1++;
  reich_draw_rect_fill(
      ctx, x1, y1, x2 - x1, y2 - y1, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
  return TRUE;
}

REICH_API int32 reich_draw_rect_ui_window(
    reichContext* ctx, int32 x1, int32 y1, int32 x2, int32 y2) {
  x2--;
  y2--;
  reich_draw_line(
      ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
  reich_draw_line(
      ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
  reich_draw_line(ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_DARK]);
  reich_draw_line(ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_DARK]);
  x1++;
  y1++;
  x2--;
  y2--;
  reich_draw_line(
      ctx, x1, y1, x2, y1, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
  reich_draw_line(
      ctx, x1, y1, x1, y2, REICH_GUI_PALETTE[REICH_PALETTE_HIGHLIGHT]);
  reich_draw_line(
      ctx, x1, y2, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
  reich_draw_line(
      ctx, x2, y1, x2, y2, REICH_GUI_PALETTE[REICH_PALETTE_SHADOW]);
  x1++;
  y1++;
  return TRUE;
}

REICH_API int32 reich_default_render_titlebar(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h) {
  reich_draw_rect_fill(
      ctx, x, y, w, h + 3, REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
  reich_draw_rect_ui_window(ctx, 0, 0, ctx->canvas.width, ctx->canvas.height);
  reich_draw_rect_gradient(
      ctx,
      x + 4,
      y + 4,
      x + w - 8,
      y + h - 2,
      REICH_GUI_PALETTE[REICH_PALETTE_TITLE_ACTIVE_START],
      REICH_GUI_PALETTE[REICH_PALETTE_TITLE_ACTIVE_END],
      REICH_GUI_PALETTE[REICH_PALETTE_TITLE_ACTIVE_START],
      REICH_GUI_PALETTE[REICH_PALETTE_TITLE_ACTIVE_END]);
  return 1;
}

REICH_API int32 reich_default_render_btn_close(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h, int32 isHovered) {
  float cx = (float)x + (float)w * 0.5f;
  float cy = (float)y + (float)h * 0.5f - 1;
  float s = (float)h * 0.2f;
  reich_draw_rect_ui_button(
      ctx, x, y, x + w, y + h, reich_mouse_down(ctx, 0) && isHovered);
  reich_draw_line(
      ctx,
      cx - s,
      cy - s,
      cx + s,
      cy + s,
      REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
  reich_draw_line(
      ctx,
      cx + s,
      cy - s,
      cx - s,
      cy + s,
      REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
  cx--;
  reich_draw_line(
      ctx,
      cx - s,
      cy - s,
      cx + s,
      cy + s,
      REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
  reich_draw_line(
      ctx,
      cx + s,
      cy - s,
      cx - s,
      cy + s,
      REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);

  return 1;
}

REICH_API int32 reich_default_render_btn_max(
    reichContext* ctx,
    int32 x,
    int32 y,
    int32 w,
    int32 h,
    int32 isHovered,
    int32 isMaximized) {
  reich_draw_rect_ui_button(
      ctx, x, y, x + w, y + h, reich_mouse_down(ctx, 0) && isHovered);
  if (isMaximized) {
    reich_draw_rect(
        ctx,
        x + 3,
        y + 2 + 3,
        w - 11,
        h - 9,
        REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
    reich_draw_rect(
        ctx,
        x + 3,
        y + 2 + 4,
        w - 11,
        h - 10,
        REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
    reich_draw_rect(
        ctx,
        x + 3 + 2,
        y + 2,
        w - 11,
        h - 9,
        REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
    reich_draw_rect(
        ctx,
        x + 3 + 2,
        y + 3,
        w - 11,
        h - 10,
        REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
    reich_draw_rect_fill(
        ctx,
        x + 4,
        y + 2 + 5,
        w - 12,
        h - 12,
        REICH_GUI_PALETTE[REICH_PALETTE_NEUTRAL]);
  } else {
    reich_draw_rect(
        ctx,
        x + 3,
        y + 2,
        w - 8,
        h - 6,
        REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
    reich_draw_rect(
        ctx,
        x + 3,
        y + 3,
        w - 8,
        h - 7,
        REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
  }
  return 1;
}

REICH_API int32 reich_default_render_btn_min(
    reichContext* ctx, int32 x, int32 y, int32 w, int32 h, int32 isHovered) {
  reich_draw_rect_ui_button(
      ctx, x, y, x + w, y + h, reich_mouse_down(ctx, 0) && isHovered);
  reich_draw_rect(
      ctx, x + 4, y + h - 6, w - 10, 1, REICH_GUI_PALETTE[REICH_PALETTE_TEXT]);
  return 1;
}

REICH_API int32 reich_draw_decorations(reichContext* ctx) {
  int32 w = ctx->canvas.width;
  int32 h = ctx->themeTitleBarHeight;
  int32 mx = reich_mouse_x(ctx), my = reich_mouse_y(ctx);
  int32 isHoverClose = 0, isHoverMax = 0, isHoverMin = 0;
  reichRect rClose = reich_rect(w - 22, 6, w - 6, 21);
  reichRect rMax = reich_rect(rClose.x1 - 19, 6, rClose.x1 - 2, 21);
  reichRect rMin = reich_rect(rMax.x1 - 17, 6, rMax.x1, 21);
  isHoverClose = reich_rect_hovered(rClose, mx, my);
  isHoverMax = reich_rect_hovered(rMax, mx, my);
  isHoverMin = reich_rect_hovered(rMin, mx, my);
  if (ctx->renderTitleBar) { ctx->renderTitleBar(ctx, 0, 0, w, h); }
  if (ctx->renderBtnMin) {
    ctx->renderBtnMin(
        ctx,
        rMin.x1,
        rMin.y1,
        rMin.x2 - rMin.x1,
        rMin.y2 - rMin.y1,
        isHoverMin);
  }
  if (ctx->renderBtnMax) {
    ctx->renderBtnMax(
        ctx,
        rMax.x1,
        rMax.y1,
        rMax.x2 - rMax.x1,
        rMax.y2 - rMax.y1,
        isHoverMax,
        ctx->isMaximized);
  }
  if (ctx->renderBtnClose) {
    ctx->renderBtnClose(
        ctx,
        rClose.x1,
        rClose.y1,
        rClose.x2 - rClose.x1,
        rClose.y2 - rClose.y1,
        isHoverClose);
  }
  return 1;
}

/* WORLD TRANSFORMATIONS *****************************************************/

int32 reich_point_in_tri(
    real64 pointX,
    real64 pointY,
    real64 v0X,
    real64 v0Y,
    real64 v1X,
    real64 v1Y,
    real64 v2X,
    real64 v2Y) {
  real64 d1 = (pointX - v1X) * (v0Y - v1Y) - (v0X - v1X) * (pointY - v1Y);
  real64 d2 = (pointX - v2X) * (v1Y - v2Y) - (v1X - v2X) * (pointY - v2Y);
  real64 d3 = (pointX - v0X) * (v2Y - v0Y) - (v2X - v0X) * (pointY - v0Y);
  return !(
      ((d1 < 0) || (d2 < 0) || (d3 < 0)) &&
      ((d1 > 0) || (d2 > 0) || (d3 > 0)));
}

typedef struct {
  real64 x;
  real64 y;
  real64 z;
  real64 zoom;
  real64 angle;
  real64 pitch;
} reichCamera;

void reich_world_to_screen(reichContext* ctx, reichCamera* cam, real64 wX, real64 wY, real32 wZ, real64* oX, real64* oY) {
  real64 cY = reich_cos((float)cam->angle);
  real64 sY = reich_sin((float)cam->angle);
  real64 cP = reich_cos((float)cam->pitch);
  real64 sP = reich_sin((float)cam->pitch);
  real64 dX = wX - cam->x;
  real64 dY = wY - cam->y;
  real64 dZ = (real64)wZ - cam->z;
  real64 rX = dX * cY - dY * sY;
  real64 rY = dX * sY + dY * cY;
  *oX = (real64)ctx->canvas.width * 0.5 + rX * cam->zoom;
  *oY = (real64)ctx->canvas.height * 0.5 + (rY * sP - dZ * cP) * cam->zoom;
}

void reich_screen_to_world(reichContext* ctx, reichCamera* cam, real64 sX, real64 sY, real32 wZ, real64* wX, real64* wY) {
  real64 cY = reich_cos((float)cam->angle);
  real64 sinY = reich_sin((float)cam->angle);
  real64 cP = reich_cos((float)cam->pitch);
  real64 sP = reich_sin((float)cam->pitch);
  real64 dZ = (real64)wZ - cam->z;
  real64 cX = (real64)ctx->canvas.width * 0.5;
  real64 cEnY = (real64)ctx->canvas.height * 0.5;
  real64 rX = (sX - cX) / cam->zoom;
  real64 rY = ((sY - cEnY) / cam->zoom + dZ * cP) / sP;
  *wX = cam->x + rX * cY + rY * sinY;
  *wY = cam->y - rX * sinY + rY * cY;
}

REICH_API uint32 reich_apply_lighting(
    uint32 albedo,
    real32 normalDotLight,
    uint32 sunColor,
    uint32 ambientColor) {

  real32 albedoR, albedoG, albedoB;
  real32 sunR, sunG, sunB;
  real32 ambR, ambG, ambB;
  real32 outR, outG, outB;

  albedoR = ((albedo >> 16) & 0xFF) / 255.0f;
  albedoG = ((albedo >> 8) & 0xFF) / 255.0f;
  albedoB = (albedo & 0xFF) / 255.0f;

  sunR = ((sunColor >> 16) & 0xFF) / 255.0f;
  sunG = ((sunColor >> 8) & 0xFF) / 255.0f;
  sunB = (sunColor & 0xFF) / 255.0f;

  ambR = ((ambientColor >> 16) & 0xFF) / 255.0f;
  ambG = ((ambientColor >> 8) & 0xFF) / 255.0f;
  ambB = (ambientColor & 0xFF) / 255.0f;

  outR = albedoR * (normalDotLight * sunR + ambR * 0.5f);
  outG = albedoG * (normalDotLight * sunG + ambG * 0.5f);
  outB = albedoB * (normalDotLight * sunB + ambB * 0.5f);

	outR = REICH_CLAMP(outR, 0.0f, 1.0f);
	outG = REICH_CLAMP(outG, 0.0f, 1.0f);
	outB = REICH_CLAMP(outB, 0.0f, 1.0f);

  return 0xFF000000 | ((uint32)(outR * 255.0f) << 16) | ((uint32)(outG * 255.0f) << 8) | (uint32)(outB * 255.0f);
}

REICH_API uint32 reich_blend_water(
    uint32 landColor, real32 depth, uint32 skyColor) {

  real32 landR, landG, landB;
  real32 skyR, skyG, skyB;
  real32 waterDepthR = 0.01f, waterDepthG = 0.05f, waterDepthB = 0.12f;
  real32 absorbR, absorbG, absorbB;
  real32 waterR, waterG, waterB;
  real32 finalR, finalG, finalB;
  real32 mixFactor = 0.15f;

  landR = ((landColor >> 16) & 0xFF) / 255.0f;
  landG = ((landColor >> 8) & 0xFF) / 255.0f;
  landB = (landColor & 0xFF) / 255.0f;

  skyR = ((skyColor >> 16) & 0xFF) / 255.0f;
  skyG = ((skyColor >> 8) & 0xFF) / 255.0f;
  skyB = (skyColor & 0xFF) / 255.0f;

  absorbR = 1.0f / (1.0f + depth * 0.8f);
  absorbG = 1.0f / (1.0f + depth * 0.3f);
  absorbB = 1.0f / (1.0f + depth * 0.1f);

  waterR = landR * absorbR + waterDepthR * (1.0f - absorbR);
  waterG = landG * absorbG + waterDepthG * (1.0f - absorbG);
  waterB = landB * absorbB + waterDepthB * (1.0f - absorbB);

  finalR = waterR * (1.0f - mixFactor) + skyR * mixFactor;
  finalG = waterG * (1.0f - mixFactor) + skyG * mixFactor;
  finalB = waterB * (1.0f - mixFactor) + skyB * mixFactor;

  if (finalR > 1.0f) finalR = 1.0f;
  if (finalG > 1.0f) finalG = 1.0f;
  if (finalB > 1.0f) finalB = 1.0f;

  return 0xFF000000 | ((uint32)(finalR * 255.0f) << 16) | ((uint32)(finalG * 255.0f) << 8) | (uint32)(finalB * 255.0f);
}

static uint32 reich_lerp_col(uint32 color1, uint32 color2, real32 t) {
  int32 r1, g1, b1, r2, g2, b2;
  if (t <= 0.0f) { return color1; }
  if (t >= 1.0f) { return color2; }

  r1 = (color1 >> 16) & 0xFF;
  g1 = (color1 >> 8) & 0xFF;
  b1 = color1 & 0xFF;

  r2 = (color2 >> 16) & 0xFF;
  g2 = (color2 >> 8) & 0xFF;
  b2 = color2 & 0xFF;

  return 0xFF000000 | ((uint32)(r1 + (int32)((r2 - r1) * t)) << 16) |
      ((uint32)(g1 + (int32)((g2 - g1) * t)) << 8) |
      (uint32)(b1 + (int32)((b2 - b1) * t));
}

REICH_API void reich_compute_atmosphere(
    real64 time,
    uint32* outSunColor,
    uint32* outAmbientColor,
    uint32* outSkyColor,
    real32* outLightDirX,
    real32* outLightDirY,
    real32* outLightDirZ) {

  real32 sunAngle = (real32)(time * 6.28318);
  real32 dirX = (real32)reich_cos(sunAngle);
  real32 dirY = (real32)reich_sin(sunAngle) * 0.5f;
  real32 dirZ = (real32)reich_sin(sunAngle);
  real32 length;

  uint32 daySun = 0xFFF5E6, dayAmbient = 0x446688, daySky = 0x5599FF;
  uint32 setSun = 0xFF6622, setAmbient = 0x332233, setSky = 0xDD5544;
  uint32 twiAmbient = 0x110A1A, twiSky = 0x221133;
  uint32 nitSun = 0x334466, nitAmbient = 0x05050A, nitSky = 0x020208;

  length = (real32)reich_sqrt((real64)(dirX * dirX + dirY * dirY + dirZ * dirZ));
  dirX /= length;
  dirY /= length;
  dirZ /= length;

  if (dirZ >= 0.2f) {
    *outSunColor = daySun;
    *outAmbientColor = dayAmbient;
    *outSkyColor = daySky;
  } else if (dirZ >= 0.05f) {
    real32 t = (dirZ - 0.05f) / 0.15f;
    *outSunColor = reich_lerp_col(setSun, daySun, t);
    *outAmbientColor = reich_lerp_col(setAmbient, dayAmbient, t);
    *outSkyColor = reich_lerp_col(setSky, daySky, t);
  } else if (dirZ >= 0.0f) {
    real32 t = dirZ / 0.05f;
    *outSunColor = reich_lerp_col(0x000000, setSun, t);
    *outAmbientColor = reich_lerp_col(twiAmbient, setAmbient, t);
    *outSkyColor = reich_lerp_col(twiSky, setSky, t);
  } else if (dirZ >= -0.05f) {
    real32 t = (dirZ + 0.05f) / 0.05f;
    *outSunColor = reich_lerp_col(nitSun, 0x000000, t);
    *outAmbientColor = reich_lerp_col(nitAmbient, twiAmbient, t);
    *outSkyColor = reich_lerp_col(nitSky, twiSky, t);
  } else {
    *outSunColor = nitSun;
    *outAmbientColor = nitAmbient;
    *outSkyColor = nitSky;
  }

  if (dirZ >= 0.0f) {
    *outLightDirX = dirX;
    *outLightDirY = dirY;
    *outLightDirZ = dirZ;
  } else {
    *outLightDirX = -dirX;
    *outLightDirY = -dirY;
    *outLightDirZ = -dirZ;
  }

  if (*outLightDirZ < 0.05f) { *outLightDirZ = 0.05f; }

  length = (real32)reich_sqrt((real64)((*outLightDirX) * (*outLightDirX) + (*outLightDirY) * (*outLightDirY) + (*outLightDirZ) * (*outLightDirZ)));
  *outLightDirX /= length;
  *outLightDirY /= length;
  *outLightDirZ /= length;
}

int32 reich_draw_checkerboard(reichContext* ctx, int32 tileSize, uint32 baseColor) {
  int32 pixelX, pixelY;
  uint32 color1 = reich_lerp_col(baseColor, 0xFF000000, 0.2f);
  uint32 color2 = reich_lerp_col(baseColor, 0xFF000000, 0.3f);

  for (pixelY = 0; pixelY < ctx->canvas.height; pixelY++) {
    for (pixelX = 0; pixelX < ctx->canvas.width; pixelX++) {
      reich_draw_pixel(ctx, pixelX, pixelY, ((pixelX / tileSize) + (pixelY / tileSize)) % 2 == 0 ? color1 : color2);
    }
  }
  return 1;
}

/* CANVAS ********************************************************************/

REICH_API reichCanvas
reich_canvas_create(reichArena* arena, int32 w, int32 h) {
  reichCanvas c;
  c.width = w;
  c.height = h;
  c.pixels =
      (uint32*)reich_arena_alloc(arena, (reichSize)w * h * sizeof(uint32));
  if (c.pixels) {
    reich_memset(c.pixels, 0, (reichSize)w * h * sizeof(uint32));
  }
  return c;
}

REICH_API int32
reich_draw_canvas(reichContext* ctx, reichCanvas* src, int32 x, int32 y) {
  int32 sx, sy, dy, startX = 0, startY = 0, endX = src->width,
                    endY = src->height;
  if (x < ctx->clip.x1) { startX = ctx->clip.x1 - x; }
  if (y < ctx->clip.y1) { startY = ctx->clip.y1 - y; }
  if (x + endX > ctx->clip.x2) { endX = ctx->clip.x2 - x; }
  if (y + endY > ctx->clip.y2) { endY = ctx->clip.y2 - y; }
  if (startX >= endX || startY >= endY) { return 0; }

  for (sy = startY; sy < endY; ++sy) {
    dy = y + sy;
    for (sx = startX; sx < endX; ++sx) {
      uint32 s = src->pixels[sy * src->width + sx];
      if (REICH_GET_A(s) > 0) { REICH_DRAW_PIXEL_FAST(ctx, x + sx, dy, s); }
    }
  }
  reich_dirty_add(ctx, x + startX, y + startY, x + endX, y + endY);
  return 1;
}

REICH_API int32 reich_draw_canvas_scaled(
    reichContext* ctx, reichCanvas* src, int32 x, int32 y, int32 w, int32 h) {
  int32 dx, dy, startX, startY, endX, endY;
  float ratioX, ratioY;
  startX = x;
  startY = y;
  endX = x + w;
  endY = y + h;
  REICH_CLAMP_EXCL(ctx, startX, startY, endX, endY);
  if (w <= 0 || h <= 0 || startX >= endX || startY >= endY) { return 0; }
  ratioX = (float)src->width / (float)w;
  ratioY = (float)src->height / (float)h;
  for (dy = startY; dy < endY; ++dy) {
    float sy = (dy - y) * ratioY;
    int32 syi = (int32)sy;
    float syf = sy - syi;
    int32 sy1 = syi < src->height - 1 ? syi + 1 : syi;
    uint32* r0 = src->pixels + (syi * src->width);
    uint32* r1 = src->pixels + (sy1 * src->width);

    for (dx = startX; dx < endX; ++dx) {
      float sx = (dx - x) * ratioX;
      int32 sxi = (int32)sx;
      float sxf = sx - sxi;
      int32 sx1 = sxi < src->width - 1 ? sxi + 1 : sxi;
      uint32 c00 = r0[sxi], c01 = r0[sx1], c10 = r1[sxi], c11 = r1[sx1];
      float fa, fr, fg, fb;
      uint32 s;
      fa = (REICH_GET_A(c00) * (1.0f - sxf) + REICH_GET_A(c01) * sxf) *
              (1.0f - syf) +
          (REICH_GET_A(c10) * (1.0f - sxf) + REICH_GET_A(c11) * sxf) * syf;
      if (fa <= 0.0f) { continue; }
      fr = (REICH_GET_R(c00) * (1.0f - sxf) + REICH_GET_R(c01) * sxf) *
              (1.0f - syf) +
          (REICH_GET_R(c10) * (1.0f - sxf) + REICH_GET_R(c11) * sxf) * syf;
      fg = (REICH_GET_G(c00) * (1.0f - sxf) + REICH_GET_G(c01) * sxf) *
              (1.0f - syf) +
          (REICH_GET_G(c10) * (1.0f - sxf) + REICH_GET_G(c11) * sxf) * syf;
      fb = (REICH_GET_B(c00) * (1.0f - sxf) + REICH_GET_B(c01) * sxf) *
              (1.0f - syf) +
          (REICH_GET_B(c10) * (1.0f - sxf) + REICH_GET_B(c11) * sxf) * syf;
      s = ((uint32)fa << 24) | ((uint32)fr << 16) | ((uint32)fg << 8) |
          (uint32)fb;
      REICH_DRAW_PIXEL_FAST(ctx, dx, dy, s);
    }
  }
  reich_dirty_add(ctx, startX, startY, endX, endY);
  return 1;
}

/* DRAW::GLASS ***************************************************************/

typedef struct reichDrawGlassConfig {
  real32 cornerRadius;
  real32 refractionStrength;
  real32 magnification;
  real32 bevelDepth;
  real32 bevelShape;
  real32 bevelSmoothness;
  real32 blurIntensity;
  uint32 tintColor;
  int32 enableShading;
  real32 lightX;
  real32 lightY;
  uint32 highlightColor;
  int32 highlightBlend;
  uint32 shadowColor;
  int32 shadowBlend;
  real32 csgSmoothness;
  const char* label;
} reichDrawGlassConfig;

reichDrawGlassConfig reich_draw_glass_config(const char* label) {
  reichDrawGlassConfig ret;
  reich_memset(&ret, 0, sizeof(reichDrawGlassConfig));
  ret.label = label;
  ret.cornerRadius = 0;
  ret.refractionStrength = 0.2f;
  ret.magnification = 1.2f;
  ret.bevelDepth = 0.2f;
  ret.bevelShape = 0.0f;
  ret.bevelSmoothness = 0.0f;
  ret.blurIntensity = 0.0f;
  ret.tintColor = 0xFFFFFFFF;
  ret.enableShading = FALSE;
  ret.lightX = -0.5f;
  ret.lightY = -0.5f;
  ret.highlightColor = 0x88FFFFFF;
  ret.shadowColor = 0x88000000;
  ret.highlightBlend = REICH_BLEND_SCREEN;
  ret.shadowBlend = REICH_BLEND_MULTIPLY;
  ret.csgSmoothness = 40.5f;
  return ret;
}

static real32* REICH_SDF_BUFFER;
static reichCanvas REICH_GLASS_CANVAS;
static reichRect REICH_GLASS_BOUNDS;
static reichDrawGlassConfig REICH_GLASS_CONFIG;

REICH_API reichDrawGlassConfig* reich_draw_glass_get_config(void) {
  return &REICH_GLASS_CONFIG;
}

static void reich_draw_glass_sample_blur(
    uint32* bgPix,
    int32 bgW,
    int32 bgH,
    real32 glassColorCoordX,
    real32 glassColorCoordY,
    real32 shiftX,
    real32 shiftY,
    real32 blurRadius,
    real32* outR,
    real32* outG,
    real32* outB) {
  int32 bx, by, i = 0, ry, gy, bY, ryOffset, gyOffset, byOffset, rx, gx, bX;
  real32 r = 0.0f, g = 0.0f, b = 0.0f, oy, ox, wNorm;
  for (by = -2; by <= 2; by++) {
    oy = (real32)by * blurRadius;
    ry = REICH_CLAMP_COORD((int32)(glassColorCoordY - shiftY + oy), bgH);
    gy = REICH_CLAMP_COORD((int32)(glassColorCoordY + oy), bgH);
    bY = REICH_CLAMP_COORD((int32)(glassColorCoordY + shiftY + oy), bgH);
    ryOffset = ry * bgW;
    gyOffset = gy * bgW;
    byOffset = bY * bgW;
    for (bx = -2; bx <= 2; bx++) {
      wNorm = REICH_GAUSS_NORM[i++];
      ox = (real32)bx * blurRadius;
      rx = REICH_CLAMP_COORD((int32)(glassColorCoordX - shiftX + ox), bgW);
      gx = REICH_CLAMP_COORD((int32)(glassColorCoordX + ox), bgW);
      bX = REICH_CLAMP_COORD((int32)(glassColorCoordX + shiftX + ox), bgW);
      r += (real32)((bgPix[ryOffset + rx] >> 16) & 0xFF) * wNorm;
      g += (real32)((bgPix[gyOffset + gx] >> 8) & 0xFF) * wNorm;
      b += (real32)(bgPix[byOffset + bX] & 0xFF) * wNorm;
    }
  }
  *outR = r;
  *outG = g;
  *outB = b;
}

static void reich_draw_glass_apply_lighting(
    real32 nx,
    real32 ny,
    real32 lx,
    real32 ly,
    real32 visualSlope,
    real32 ha,
    real32 hr,
    real32 hg,
    real32 hb,
    int32 hBlend,
    real32 sa,
    real32 sr,
    real32 sg,
    real32 sb,
    int32 sBlend,
    real32* r,
    real32* g,
    real32* b) {
  real32 dot, hi, si, hAlpha, sAlpha;
  dot = nx * lx + ny * ly;
  hi = REICH_MAX(dot, 0.0f) * visualSlope;
  si = REICH_MAX(-dot, 0.0f) * visualSlope;
  if (hi > 0.0f) {
    hAlpha = ha * hi;
    *r = reich_blend_colour(*r, hr, hAlpha, hBlend);
    *g = reich_blend_colour(*g, hg, hAlpha, hBlend);
    *b = reich_blend_colour(*b, hb, hAlpha, hBlend);
  }
  if (si > 0.0f) {
    sAlpha = sa * si;
    *r = reich_blend_colour(*r, sr, sAlpha, sBlend);
    *g = reich_blend_colour(*g, sg, sAlpha, sBlend);
    *b = reich_blend_colour(*b, sb, sAlpha, sBlend);
  }
}

REICH_API int32 reich_draw_glass_rect(
    reichContext* ctx, real32 x, real32 y, real32 w, real32 h, real32 r) {
  int32 minX, minY, maxX, maxY, px, py;
  real32 padding =
      REICH_GLASS_CONFIG.bevelDepth + REICH_GLASS_CONFIG.csgSmoothness + 4.0f;
  real32 gw = w * 0.5f, gh = h * 0.5f;
  real32 cx = x + gw, cy = y + gh;

  minX = (int32)(x - padding);
  minY = (int32)(y - padding);
  maxX = (int32)(x + w + padding);
  maxY = (int32)(y + h + padding);

  if (minX < 0) { minX = 0; }
  if (minY < 0) { minY = 0; }
  if (maxX >= ctx->canvas.width) { maxX = ctx->canvas.width - 1; }
  if (maxY >= ctx->canvas.height) { maxY = ctx->canvas.height - 1; }

  for (py = minY; py <= maxY; ++py) {
    for (px = minX; px <= maxX; ++px) {
      real32 d = reich_sdf((real32)px - cx, (real32)py - cy, gw, gh, r);
      int32 idx = py * ctx->canvas.width + px;
      if (REICH_GLASS_CONFIG.csgSmoothness > 0.0f) {
        REICH_SDF_BUFFER[idx] = reich_smin(
            REICH_SDF_BUFFER[idx], d, REICH_GLASS_CONFIG.csgSmoothness);
      } else if (d < REICH_SDF_BUFFER[idx]) {
        REICH_SDF_BUFFER[idx] = d;
      }
    }
  }

  if (minX < REICH_GLASS_BOUNDS.x1) { REICH_GLASS_BOUNDS.x1 = minX; }
  if (minY < REICH_GLASS_BOUNDS.y1) { REICH_GLASS_BOUNDS.y1 = minY; }
  if (maxX > REICH_GLASS_BOUNDS.x2) { REICH_GLASS_BOUNDS.x2 = maxX; }
  if (maxY > REICH_GLASS_BOUNDS.y2) { REICH_GLASS_BOUNDS.y2 = maxY; }
  return 1;
}

REICH_API int32
reich_draw_glass_circle(reichContext* ctx, real32 cx, real32 cy, real32 r) {
  int32 minX, minY, maxX, maxY, px, py;
  real32 padding =
      REICH_GLASS_CONFIG.bevelDepth + REICH_GLASS_CONFIG.csgSmoothness + 4.0f;

  minX = (int32)(cx - r - padding);
  minY = (int32)(cy - r - padding);
  maxX = (int32)(cx + r + padding);
  maxY = (int32)(cy + r + padding);

  if (minX < 0) { minX = 0; }
  if (minY < 0) { minY = 0; }
  if (maxX >= ctx->canvas.width) { maxX = ctx->canvas.width - 1; }
  if (maxY >= ctx->canvas.height) { maxY = ctx->canvas.height - 1; }

  for (py = minY; py <= maxY; ++py) {
    for (px = minX; px <= maxX; ++px) {
      real32 dx = (real32)px - cx;
      real32 dy = (real32)py - cy;
      real32 d = (real32)reich_sqrt((real64)(dx * dx + dy * dy)) - r;
      int32 idx = py * ctx->canvas.width + px;
      if (REICH_GLASS_CONFIG.csgSmoothness > 0.0f) {
        REICH_SDF_BUFFER[idx] = reich_smin(
            REICH_SDF_BUFFER[idx], d, REICH_GLASS_CONFIG.csgSmoothness);
      } else if (d < REICH_SDF_BUFFER[idx]) {
        REICH_SDF_BUFFER[idx] = d;
      }
    }
  }

  if (minX < REICH_GLASS_BOUNDS.x1) { REICH_GLASS_BOUNDS.x1 = minX; }
  if (minY < REICH_GLASS_BOUNDS.y1) { REICH_GLASS_BOUNDS.y1 = minY; }
  if (maxX > REICH_GLASS_BOUNDS.x2) { REICH_GLASS_BOUNDS.x2 = maxX; }
  if (maxY > REICH_GLASS_BOUNDS.y2) { REICH_GLASS_BOUNDS.y2 = maxY; }
  return 1;
}

REICH_API int32 reich_draw_glass_end(reichContext* ctx) {
  int32 x, y, minX, maxX, minY, maxY, bgW, bgH;
  int32 loopMinX, loopMaxX, loopMinY, loopMaxY;
  uint32* bgPix;
  real32 tr, tg, tb, ha, hr, hg, hb, sa, sr, sg, sb, lx, ly, lightLen;
  real32 mag, refStr, bDepth, bShape, bSmooth, bInt, boxCx, boxCy;
  reichCanvas* srcBg = &REICH_GLASS_CANVAS;
  reichDrawGlassConfig* config = &REICH_GLASS_CONFIG;

  if (!REICH_SDF_BUFFER || !srcBg->pixels) { return 0; }

  bgW = srcBg->width;
  bgH = srcBg->height;
  bgPix = srcBg->pixels;

  minX = REICH_GLASS_BOUNDS.x1;
  minY = REICH_GLASS_BOUNDS.y1;
  maxX = REICH_GLASS_BOUNDS.x2;
  maxY = REICH_GLASS_BOUNDS.y2;

  if (minX > maxX || minY > maxY) { return 1; }
  boxCx = (real32)bgW * 0.5f;
  boxCy = (real32)bgH * 0.5f;
  loopMinX = minX < 0 ? 0 : minX;
  loopMinY = minY < 0 ? 0 : minY;
  loopMaxX = maxX >= bgW ? bgW - 1 : maxX;
  loopMaxY = maxY >= bgH ? bgH - 1 : maxY;

  refStr = config->refractionStrength;
  mag = config->magnification <= 0.001f ? 1.0f : config->magnification;
  bDepth = config->bevelDepth <= 0.001f ? 1.0f : config->bevelDepth;
  bShape = config->bevelShape;
  bSmooth =
      config->bevelSmoothness < 0.0001f ? 0.0001f : config->bevelSmoothness;
  bInt = config->blurIntensity;

  tr = ((config->tintColor >> 16) & 0xFF) / 255.0f;
  tg = ((config->tintColor >> 8) & 0xFF) / 255.0f;
  tb = (config->tintColor & 0xFF) / 255.0f;

  if (config->enableShading) {
    ha = ((config->highlightColor >> 24) & 0xFF) / 255.0f;
    hr = ((config->highlightColor >> 16) & 0xFF) / 255.0f;
    hg = ((config->highlightColor >> 8) & 0xFF) / 255.0f;
    hb = (config->highlightColor & 0xFF) / 255.0f;
    sa = ((config->shadowColor >> 24) & 0xFF) / 255.0f;
    sr = ((config->shadowColor >> 16) & 0xFF) / 255.0f;
    sg = ((config->shadowColor >> 8) & 0xFF) / 255.0f;
    sb = (config->shadowColor & 0xFF) / 255.0f;
    lightLen = (real32)reich_sqrt((real64)(config->lightX * config->lightX +
                                           config->lightY * config->lightY));
    if (lightLen > 0.0f) {
      lx = config->lightX / lightLen;
      ly = config->lightY / lightLen;
    } else {
      lx = 0.0f;
      ly = -1.0f;
    }
  } else {
    ha = hr = hg = hb = sa = sr = sg = sb = lx = ly = 0.0f;
  }

  for (y = loopMinY; y <= loopMaxY; y++) {
    for (x = loopMinX; x <= loopMaxX; x++) {
      int32 idx = y * bgW + x;
      real32 s = REICH_SDF_BUFFER[idx];
      if (s <= 1.5f) {
        real32 nx = 0.0f, ny = 0.0f, nlen;
        real32 distFromCenter, distSpherical, flatSlope, visualSlope,
            distortion;
        real32 offsetX, offsetY, glassColorCoordX, glassColorCoordY,
            blurRadius, edge, shiftX, shiftY;
        real32 rR = 0.0f, gG = 0.0f, bB = 0.0f, shapeAlpha;
        uint32 cr, cg, cb, finalColor, origBg;
        if (x > 0 && x < bgW - 1 && y > 0 && y < bgH - 1) {
          nx = (REICH_SDF_BUFFER[idx + 1] - REICH_SDF_BUFFER[idx - 1]) * 0.5f;
          ny = (REICH_SDF_BUFFER[idx + bgW] - REICH_SDF_BUFFER[idx - bgW]) *
              0.5f;
          nlen = (real32)reich_sqrt((real64)(nx * nx + ny * ny));
          if (nlen > 0.0001f) {
            nx /= nlen;
            ny /= nlen;
          }
        }

        distFromCenter = 1.0f - REICH_CLAMP(-s / bDepth, 0.0f, 1.0f);
        distSpherical = 1.0f -
            (real32)reich_sqrt(
                            1.0 - (real64)(distFromCenter * distFromCenter));
        flatSlope = reich_smoothstep(0.0f, bSmooth, distFromCenter);
        visualSlope = flatSlope * (1.0f - bShape) + distSpherical * bShape;
        distortion = visualSlope;

        offsetX = distortion * nx * refStr;
        offsetY = distortion * ny * refStr;

        glassColorCoordX = boxCx + ((real32)x - boxCx - offsetX) / mag;
        glassColorCoordY = boxCy + ((real32)y - boxCy - offsetY) / mag;

        blurRadius = bInt * (1.0f - distFromCenter * 0.5f);
        edge = reich_smoothstep(0.0f, 2.0f, -s);
        shiftX = nx * edge * 3.0f * (refStr / 48.0f);
        shiftY = ny * edge * 3.0f * (refStr / 48.0f);

        reich_draw_glass_sample_blur(
            bgPix,
            bgW,
            bgH,
            glassColorCoordX,
            glassColorCoordY,
            shiftX,
            shiftY,
            blurRadius,
            &rR,
            &gG,
            &bB);

        rR *= tr;
        gG *= tg;
        bB *= tb;

        if (config->enableShading) {
          reich_draw_glass_apply_lighting(
              nx,
              ny,
              lx,
              ly,
              visualSlope,
              ha,
              hr,
              hg,
              hb,
              config->highlightBlend,
              sa,
              sr,
              sg,
              sb,
              config->shadowBlend,
              &rR,
              &gG,
              &bB);
        }
        shapeAlpha = reich_smoothstep(1.5f, 0.0f, s);
        origBg = bgPix[idx];

        if (shapeAlpha < 1.0f) {
          real32 bgR = (real32)REICH_GET_R(origBg) / 255.0f;
          real32 bgG = (real32)REICH_GET_G(origBg) / 255.0f;
          real32 bgB = (real32)REICH_GET_B(origBg) / 255.0f;
          rR = rR * shapeAlpha + bgR * (1.0f - shapeAlpha);
          gG = gG * shapeAlpha + bgG * (1.0f - shapeAlpha);
          bB = bB * shapeAlpha + bgB * (1.0f - shapeAlpha);
        }

        cr = (uint32)(REICH_CLAMP(rR, 0.0f, 1.0f) * 255.0f);
        cg = (uint32)(REICH_CLAMP(gG, 0.0f, 1.0f) * 255.0f);
        cb = (uint32)(REICH_CLAMP(bB, 0.0f, 1.0f) * 255.0f);
        finalColor = 0xFF000000 | (cr << 16) | (cg << 8) | cb;
        reich_draw_pixel(ctx, x, y, finalColor);
      }
    }
  }
  return 1;
}

REICH_API int32 reich_draw_glass_begin(reichContext* ctx) {
  int32 bgW = ctx->canvas.width;
  int32 bgH = ctx->canvas.height;
  int32 i, total = bgW * bgH;
  uint32* bgPixels = (uint32*)reich_arena_alloc(
      &ctx->frameMem, (reichSize)(total * sizeof(uint32)));
  real32* sdfPixels = (real32*)reich_arena_alloc(
      &ctx->frameMem, (reichSize)(total * sizeof(real32)));

  reich_memset(&REICH_GLASS_CANVAS, 0, sizeof(reichCanvas));
  REICH_GLASS_CONFIG = reich_draw_glass_config("default");

  if (bgPixels && sdfPixels) {
    reich_memcpy(
        bgPixels, ctx->canvas.pixels, (reichSize)(total * sizeof(uint32)));
    REICH_GLASS_CANVAS.width = bgW;
    REICH_GLASS_CANVAS.height = bgH;
    REICH_GLASS_CANVAS.pixels = bgPixels;
    REICH_SDF_BUFFER = sdfPixels;

    for (i = 0; i < total; ++i) { REICH_SDF_BUFFER[i] = 999999.0f; }

    REICH_GLASS_BOUNDS.x1 = bgW;
    REICH_GLASS_BOUNDS.y1 = bgH;
    REICH_GLASS_BOUNDS.x2 = 0;
    REICH_GLASS_BOUNDS.y2 = 0;
    return 1;
  }
  return 0;
}

#endif
#endif
