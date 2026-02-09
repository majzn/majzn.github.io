#ifndef REICH_H
#define REICH_H

#include "stdint.h"
#include <stdarg.h>

#ifdef __linux__
#define REICH_PLATFORM_LINUX
#elif _WIN32
#define REICH_PLATFORM_WIN32
#endif

#if defined(REICH_PLATFORM_WIN32)
#if defined(REICH_EXPORTS)
#define REICH_API __declspec(dllexport)
#else
#if defined(REICH_IMPORTS)
#define REICH_API __declspec(dllimport)
#else
#define REICH_API
#endif
#endif
#else
#define REICH_API
#endif

#ifndef NULL
#define NULL ((void *)0)
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
typedef void *reichHandle;

#define REICH_FILE_READ 1
#define REICH_FILE_WRITE 2
#define REICH_SEEK_SET 0
#define REICH_SEEK_CUR 1
#define REICH_SEEK_END 2

#define REICH_LOG_DEBUG 0
#define REICH_LOG_INFO 1
#define REICH_LOG_WARN 2
#define REICH_LOG_ERROR 3

REICH_API reichHandle reich_file_open(const char *filename, int32 mode);
REICH_API void reich_file_close(reichHandle file);
REICH_API reichSize reich_file_read(reichHandle file, void *buffer,
                                    reichSize bytes);
REICH_API reichSize reich_file_write(reichHandle file, const void *buffer,
                                     reichSize bytes);
REICH_API reichSize reich_file_size(reichHandle file);
REICH_API int32 reich_file_seek(reichHandle file, int32 offset, int32 origin);
REICH_API int32 reich_file_tell(reichHandle file);

REICH_API reichHandle reich_find_first(const char *pattern,
                                       char *filenameBuffer, int32 bufferSize);
REICH_API int32 reich_find_next(reichHandle handle, char *filenameBuffer,
                                int32 bufferSize);
REICH_API void reich_find_close(reichHandle handle);

REICH_API void reich_log(int32 level, const char *format, ...);

#define REICH_KEY_MAX 512
#define REICH_MOUSE_BUTTONS 3
#define REICH_MAX_FONTS 16
#define REICH_TITLE_BAR_HEIGHT 32
#define REICH_TITLE_BUTTON_WIDTH 45

typedef struct reichArena {
  uint8 *base;
  reichSize size;
  reichSize used;
} reichArena;

typedef struct reichCanvas {
  int32 width;
  int32 height;
  uint32 *pixels;
} reichCanvas;

typedef struct reichInput {
  int32 mouseX;
  int32 mouseY;
  int32 scaledMouseX;
  int32 scaledMouseY;
  int32 lastMouseX;
  int32 lastMouseY;
  int32 deltaX;
  int32 deltaY;
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
  int32 activeId;
  int32 hotId;
} reichInput;

typedef struct reichContext reichContext;

typedef void (*PFUSERUPDATE)(reichContext *ctx);
typedef void (*PFUSERRENDER)(reichContext *ctx, real64 alpha);
typedef void (*PFUSERRESIZE)(reichContext *ctx, int32 width, int32 height);
typedef void (*PFUSERINPUT)(reichContext *ctx);

struct reichContext {
  reichArena permMem;
  reichArena frameMem;
  reichCanvas screen;
  reichInput input;
  const char *windowTitle;
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
  uint8 *fonts[REICH_MAX_FONTS];
  int32 fontCount;
  int32 activeFont;
  PFUSERUPDATE userUpdate;
  PFUSERRENDER userRender;
  PFUSERRESIZE userResize;
  PFUSERINPUT userInput;
  void *platform;
};

REICH_API int32 reich_init(reichContext *ctx, const char *title, int32 width,
                           int32 height, real64 targetFps);
REICH_API void reich_set_callbacks(reichContext *ctx, PFUSERUPDATE update,
                                   PFUSERRENDER render, PFUSERRESIZE resize,
                                   PFUSERINPUT input);
REICH_API void reich_run(reichContext *ctx);
REICH_API int32 reich_running(reichContext *ctx);
REICH_API void reich_poll_events(reichContext *ctx);
REICH_API void reich_present(reichContext *ctx);
REICH_API void reich_timer_tick(reichContext *ctx);
REICH_API int32 reich_timer_step(reichContext *ctx);
REICH_API real64 reich_timer_alpha(reichContext *ctx);
REICH_API void reich_clear(reichContext *ctx, uint32 color);
REICH_API int32 reich_key_down(reichContext *ctx, int32 keyCode);
REICH_API int32 reich_key_pressed(reichContext *ctx, int32 keyCode);
REICH_API int32 reich_key_released(reichContext *ctx, int32 keyCode);
REICH_API int32 reich_mouse_down(reichContext *ctx, int32 button);
REICH_API int32 reich_mouse_pressed(reichContext *ctx, int32 button);
REICH_API int32 reich_mouse_released(reichContext *ctx, int32 button);
REICH_API int32 reich_get_mouse_x(reichContext *ctx);
REICH_API int32 reich_get_mouse_y(reichContext *ctx);
REICH_API int32 reich_get_mouse_wheel(reichContext *ctx);
REICH_API uint32 reich_get_char_pressed(reichContext *ctx);
REICH_API void reich_show_cursor(reichContext *ctx, int32 show);
REICH_API void reich_arena_init(reichArena *a, void *mem, reichSize size);
REICH_API void *reich_arena_alloc(reichArena *a, reichSize size);
REICH_API void reich_arena_reset(reichArena *a);
REICH_API void reich_set_scale(reichContext *ctx, int32 scale);
REICH_API void reich_draw_rect(reichContext *ctx, int32 x, int32 y, int32 w,
                               int32 h, uint32 color);
REICH_API void reich_draw_frame(reichContext *ctx, int32 x, int32 y, int32 w,
                                int32 h, uint32 color);
REICH_API void reich_draw_text(reichContext *ctx, int32 x, int32 y,
                               const char *str, uint32 color);
REICH_API int32 reich_ui_btn(reichContext *ctx, int32 id, int32 x, int32 y,
                             int32 w, int32 h, const char *label, uint32 bgCol);
REICH_API void reich_load_fonts(reichContext *ctx, const char *fn, int32 gw,
                                int32 gh, int32 amt);
REICH_API void reich_load_font(reichContext *ctx, const char *filename,
                               int32 numChars, int32 glyphWidth,
                               int32 glyphHeight);
REICH_API void reich_init_default_font(reichContext *ctx);
REICH_API reichCanvas reich_load_bmp(const char *filename);

#define reich_put_pixel(ctx, x, y, color)                                      \
  if ((x) >= 0 && (x) < (ctx)->screen.width && (y) >= 0 &&                     \
      (y) < (ctx)->screen.height) {                                            \
    (ctx)->screen.pixels[(y) * (ctx)->screen.width + (x)] = (color);           \
  }

#ifdef REICH_IMPLEMENTATION

static void *reich_memset(void *dest, int32 c, reichSize count) {
  char *bytes = (char *)dest;
  while (count--) {
    *bytes++ = (char)c;
  }
  return dest;
}

static void *reich_memcpy(void *dest, const void *src, reichSize count) {
  char *d = (char *)dest;
  const char *s = (const char *)src;
  while (count--) {
    *d++ = *s++;
  }
  return dest;
}

static reichSize reich_strlen(const char *str) {
  const char *s = str;
  while (*s) {
    s++;
  }
  return (reichSize)(s - str);
}

static void reich_strncpy(char *dest, const char *src, int32 maxLen) {
  int32 i;
  for (i = 0; i < maxLen - 1 && src[i]; ++i) {
    dest[i] = src[i];
  }
  dest[i] = 0;
}

static int32 reich_vsnprintf(char *buffer, reichSize count, const char *format,
                             va_list args) {
  char *p = buffer;
  char *end = buffer + count - 1;
  const char *f = format;
  if (!buffer || count == 0)
    return 0;
  while (*f && p < end) {
    if (*f != '%') {
      *p++ = *f++;
      continue;
    }
    f++;
    if (*f == 0)
      break;
    int32 width = 0;
    int32 precision = -1;
    int32 padZero = 0;
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
    if (*f == 'l')
      f++;
    if (*f == 's') {
      const char *s = va_arg(args, const char *);
      if (!s)
        s = "(null)";
      while (*s && p < end)
        *p++ = *s++;
    } else if (*f == 'd' || *f == 'i') {
      int32 val = va_arg(args, int32);
      char temp[32];
      int32 pos = 0;
      int32 neg = 0;
      if (val < 0) {
        neg = 1;
        val = -val;
      }
      if (val == 0)
        temp[pos++] = '0';
      while (val > 0) {
        temp[pos++] = (val % 10) + '0';
        val /= 10;
      }
      if (neg)
        temp[pos++] = '-';
      while (width > pos && p < end) {
        *p++ = padZero ? '0' : ' ';
        width--;
      }
      while (pos > 0 && p < end)
        *p++ = temp[--pos];
    } else if (*f == 'x' || *f == 'X') {
      uint32 val = va_arg(args, uint32);
      char temp[32];
      int32 pos = 0;
      if (val == 0)
        temp[pos++] = '0';
      while (val > 0) {
        int32 digit = val % 16;
        temp[pos++] = (digit < 10) ? (digit + '0')
                                   : (digit - 10 + ((*f == 'x') ? 'a' : 'A'));
        val /= 16;
      }
      while (width > pos && p < end) {
        *p++ = padZero ? '0' : ' ';
        width--;
      }
      while (pos > 0 && p < end)
        *p++ = temp[--pos];
    } else if (*f == 'c') {
      char c = (char)va_arg(args, int32);
      *p++ = c;
    } else if (*f == 'f') {
      double val = va_arg(args, double);
      int32 ipart;
      double fpart;
      char temp[64];
      int32 pos = 0;
      if (precision == -1)
        precision = 6;
      if (val < 0) {
        *p++ = '-';
        val = -val;
      }
      ipart = (int32)val;
      fpart = val - (double)ipart;
      if (ipart == 0)
        temp[pos++] = '0';
      while (ipart > 0) {
        temp[pos++] = (ipart % 10) + '0';
        ipart /= 10;
      }
      while (pos > 0 && p < end)
        *p++ = temp[--pos];
      if (precision > 0 && p < end) {
        *p++ = '.';
        while (precision > 0 && p < end) {
          fpart *= 10;
          int32 digit = (int32)fpart;
          *p++ = digit + '0';
          fpart -= digit;
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

static void reich_draw_decorations(reichContext *ctx);

#if defined(REICH_PLATFORM_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#define REICH_CLASS_NAME "LIB_REICH_WINDOW"
#define REICH_RESIZE_BORDER 8

typedef HRESULT(WINAPI *PFN_DwmFlush)(void);

typedef struct reichPlatformContext {
  BITMAPINFO bitmapInfo;
  HDC renderDC;
  HWND windowHandle;
  HMODULE dwmLibrary;
  PFN_DwmFlush dwmFlush;
} reichPlatformContext;

typedef struct reichSearchState {
  HANDLE findHandle;
  WIN32_FIND_DATAA findData;
} reichSearchState;

static void *reich_sys_alloc(reichSize size);
static void reich_sys_free(void *ptr);
static int64 reich_sys_get_ticks(void);
static int64 reich_sys_get_freq(void);
static reichSize reich_sys_get_platform_data_size(void);
static int32 reich_sys_window_init(reichContext *ctx, const char *title,
                                   int32 width, int32 height);
static void reich_sys_poll_events(reichContext *ctx);
static void reich_sys_present(reichContext *ctx);
static void reich_sys_resize_canvas(reichContext *ctx, int32 width,
                                    int32 height);
static void reich_sys_toggle_maximize(reichContext *ctx);
static void reich_sys_minimize(reichContext *ctx);
static void reich_sys_close(reichContext *ctx);

REICH_API reichHandle reich_file_open(const char *filename, int32 mode) {
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
  file = CreateFileA(filename, access, share, NULL, creation,
                     FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    reich_log(REICH_LOG_ERROR, "Failed to open file: %s", filename);
    return NULL;
  }
  return (reichHandle)file;
}

REICH_API void reich_file_close(reichHandle file) {
  if (file) {
    CloseHandle((HANDLE)file);
  }
}

REICH_API reichSize reich_file_read(reichHandle file, void *buffer,
                                    reichSize bytes) {
  DWORD read = 0;
  if (!file || !buffer) {
    return 0;
  }
  if (!ReadFile((HANDLE)file, buffer, (DWORD)bytes, &read, NULL)) {
    reich_log(REICH_LOG_ERROR, "Failed to read from file.");
    return 0;
  }
  return (reichSize)read;
}

REICH_API reichSize reich_file_write(reichHandle file, const void *buffer,
                                     reichSize bytes) {
  DWORD written = 0;
  if (!file || !buffer) {
    return 0;
  }
  if (!WriteFile((HANDLE)file, buffer, (DWORD)bytes, &written, NULL)) {
    reich_log(REICH_LOG_ERROR, "Failed to write to file.");
    return 0;
  }
  return (reichSize)written;
}

REICH_API reichSize reich_file_size(reichHandle file) {
  LARGE_INTEGER size;
  if (!file) {
    return 0;
  }
  if (!GetFileSizeEx((HANDLE)file, &size)) {
    return 0;
  }
  return (reichSize)size.QuadPart;
}

REICH_API int32 reich_file_seek(reichHandle file, int32 offset, int32 origin) {
  DWORD method = FILE_BEGIN;
  if (!file) {
    return 0;
  }
  if (origin == REICH_SEEK_CUR) {
    method = FILE_CURRENT;
  } else if (origin == REICH_SEEK_END) {
    method = FILE_END;
  }
  return (int32)SetFilePointer((HANDLE)file, offset, NULL, method);
}

REICH_API int32 reich_file_tell(reichHandle file) {
  return reich_file_seek(file, 0, REICH_SEEK_CUR);
}

REICH_API reichHandle reich_find_first(const char *pattern,
                                       char *filenameBuffer, int32 bufferSize) {
  reichSearchState *state =
      (reichSearchState *)reich_sys_alloc(sizeof(reichSearchState));
  if (!state) {
    reich_log(REICH_LOG_ERROR, "Failed to allocate memory for search state.");
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

REICH_API int32 reich_find_next(reichHandle handle, char *filenameBuffer,
                                int32 bufferSize) {
  reichSearchState *state = (reichSearchState *)handle;
  if (!state) {
    return 0;
  }
  if (!FindNextFileA(state->findHandle, &state->findData)) {
    return 0;
  }
  if (filenameBuffer && bufferSize > 0) {
    reich_strncpy(filenameBuffer, state->findData.cFileName, bufferSize);
  }
  return 1;
}

REICH_API void reich_find_close(reichHandle handle) {
  reichSearchState *state = (reichSearchState *)handle;
  if (state) {
    FindClose(state->findHandle);
    reich_sys_free(state);
  }
}

REICH_API void reich_log(int32 level, const char *format, ...) {
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
}

LRESULT CALLBACK reich_win32_proc(HWND h, UINT m, WPARAM w, LPARAM l) {
  reichContext *ctx = (reichContext *)NULL;
  reichPlatformContext *pctx = (reichPlatformContext *)NULL;

  if (m == WM_NCCREATE) {
    CREATESTRUCT *createStruct = (CREATESTRUCT *)l;
    ctx = (reichContext *)createStruct->lpCreateParams;
    SetWindowLongPtr(h, GWLP_USERDATA, (LONG_PTR)ctx);
  } else {
    ctx = (reichContext *)GetWindowLongPtr(h, GWLP_USERDATA);
  }

  if (ctx) {
    pctx = (reichPlatformContext *)ctx->platform;
  }

  if (!ctx || !pctx) {
    return DefWindowProcA(h, m, w, l);
  }

  switch (m) {
  case WM_DESTROY:
    reich_log(REICH_LOG_INFO, "Window destroyed.");
    ctx->running = 0;
    PostQuitMessage(0);
    return 0;
  case WM_CLOSE:
    reich_log(REICH_LOG_INFO, "Window closed.");
    ctx->running = 0;
    return 0;
  case WM_ERASEBKGND:
    return 1;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(h, &ps);
    if (ctx->screen.pixels) {
      StretchDIBits(hdc, 0, 0, ctx->windowWidth, ctx->windowHeight, 0, 0,
                    ctx->screen.width, ctx->screen.height, ctx->screen.pixels,
                    &pctx->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    }
    EndPaint(h, &ps);
    return 0;
  }
  case WM_NCCALCSIZE: {
    if (w == (WPARAM)TRUE) {
      if (IsZoomed(h)) {
        NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS *)l;
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

    if (hit != HTCLIENT) {
      return hit;
    }
    if (!IsZoomed(h)) {
      POINT pt;
      RECT rc;
      int border;
      int top;
      int bot;
      int left;
      int right;
      pt.x = GET_X_LPARAM(l);
      pt.y = GET_Y_LPARAM(l);
      GetWindowRect(h, &rc);
      border = REICH_RESIZE_BORDER;
      top = (pt.y < rc.top + border);
      bot = (pt.y >= rc.bottom - border);
      left = (pt.x < rc.left + border);
      right = (pt.x >= rc.right - border);
      if (top && left) {
        return HTTOPLEFT;
      }
      if (top && right) {
        return HTTOPRIGHT;
      }
      if (bot && left) {
        return HTBOTTOMLEFT;
      }
      if (bot && right) {
        return HTBOTTOMRIGHT;
      }
      if (top) {
        return HTTOP;
      }
      if (bot) {
        return HTBOTTOM;
      }
      if (left) {
        return HTLEFT;
      }
      if (right) {
        return HTRIGHT;
      }
    }

    ptLocal.x = GET_X_LPARAM(l);
    ptLocal.y = GET_Y_LPARAM(l);
    ScreenToClient(h, &ptLocal);

    tbHeight = REICH_TITLE_BAR_HEIGHT * ctx->scale;
    if (ptLocal.y >= 0 && ptLocal.y < tbHeight) {
      btnArea = (REICH_TITLE_BUTTON_WIDTH * ctx->scale) * 3;
      if (ptLocal.x >= ctx->windowWidth - btnArea) {
        return HTCLIENT;
      }
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
    if (ctx->userResize) {
      ctx->userResize(ctx, ctx->screen.width, ctx->screen.height);
    }
    if (ctx->userRender) {
      ctx->userRender(ctx, 1.0);
    }
    reich_draw_decorations(ctx);
    reich_sys_present(ctx);
    return 0;
  }
  case WM_GETMINMAXINFO: {
    MINMAXINFO *mmi = (MINMAXINFO *)l;
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
      if (!((l >> 30) & 1)) {
        ctx->input.keysPressed[w] = 1;
      }
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

static void reich_sys_toggle_maximize(reichContext *ctx) {
  reichPlatformContext *pctx = (reichPlatformContext *)ctx->platform;
  if (ctx->isMaximized) {
    ShowWindow(pctx->windowHandle, SW_RESTORE);
  } else {
    ShowWindow(pctx->windowHandle, SW_MAXIMIZE);
  }
}

static void reich_sys_minimize(reichContext *ctx) {
  reichPlatformContext *pctx = (reichPlatformContext *)ctx->platform;
  ShowWindow(pctx->windowHandle, SW_MINIMIZE);
}

static void reich_sys_close(reichContext *ctx) {
  reichPlatformContext *pctx = (reichPlatformContext *)ctx->platform;
  PostMessageA(pctx->windowHandle, WM_CLOSE, 0, 0);
}

static reichSize reich_sys_get_platform_data_size(void) {
  return (reichSize)sizeof(reichPlatformContext);
}

static int32 reich_sys_window_init(reichContext *ctx, const char *title,
                                   int32 width, int32 height) {
  WNDCLASSA wc;
  DWORD style;
  RECT wr;
  reichPlatformContext *plat = (reichPlatformContext *)ctx->platform;
  HMODULE winmm;
  SetProcessDPIAware();
  AttachConsole((DWORD)-1);

  if (!plat) {
    reich_log(REICH_LOG_ERROR, "Platform context is NULL.");
    return 0;
  }

  /* Fix sleep granularity */
  winmm = LoadLibraryA("winmm.dll");
  if (winmm) {
    typedef UINT(WINAPI * PFN_timeBeginPeriod)(UINT);
    PFN_timeBeginPeriod pTimeBeginPeriod =
        (PFN_timeBeginPeriod)GetProcAddress(winmm, "timeBeginPeriod");
    if (pTimeBeginPeriod) {
      pTimeBeginPeriod(1);
    }
  }

  /* Load DWM for V-Sync */
  plat->dwmLibrary = LoadLibraryA("dwmapi.dll");
  if (plat->dwmLibrary) {
    plat->dwmFlush = (PFN_DwmFlush)GetProcAddress(plat->dwmLibrary, "DwmFlush");
  }

  reich_memset(&wc, 0, sizeof(WNDCLASSA));
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; /* Removed CS_DBLCLKS */
  wc.lpfnWndProc = reich_win32_proc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = REICH_CLASS_NAME;

  RegisterClassA(&wc);
  style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  wr.left = 0;
  wr.top = 0;
  wr.right = width;
  wr.bottom = height;
  AdjustWindowRect(&wr, style, FALSE);

  plat->windowHandle = CreateWindowExA(
      WS_EX_APPWINDOW, REICH_CLASS_NAME, title, style, CW_USEDEFAULT,
      CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, (HWND)0, (HMENU)0,
      wc.hInstance, (LPVOID)ctx);

  if (!plat->windowHandle) {
    reich_log(REICH_LOG_ERROR, "Failed to create window.");
    return 0;
  }

  SetWindowPos(plat->windowHandle, (HWND)0, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
  plat->renderDC = GetDC(plat->windowHandle);

  plat->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  plat->bitmapInfo.bmiHeader.biWidth = width;
  plat->bitmapInfo.bmiHeader.biHeight = -height;
  plat->bitmapInfo.bmiHeader.biPlanes = 1;
  plat->bitmapInfo.bmiHeader.biBitCount = 32;
  plat->bitmapInfo.bmiHeader.biCompression = BI_RGB;

  reich_sys_resize_canvas(ctx, width, height);
  reich_clear(ctx, 0xFF000000);
  reich_sys_present(ctx);

  reich_log(REICH_LOG_INFO, "Window initialized successfully: %s (%dx%d)",
            title, width, height);
  return 1;
}

static void reich_sys_resize_canvas(reichContext *ctx, int32 width,
                                    int32 height) {
  reichPlatformContext *pctx = (reichPlatformContext *)ctx->platform;
  int32 cw = width / ctx->scale;
  int32 ch = height / ctx->scale;

  if (cw < 1) {
    cw = 1;
  }
  if (ch < 1) {
    ch = 1;
  }

  if (cw != ctx->screen.width || ch != ctx->screen.height) {
    if (ctx->screen.pixels) {
      reich_sys_free(ctx->screen.pixels);
    }
    ctx->screen.width = cw;
    ctx->screen.height = ch;
    ctx->screen.pixels =
        (uint32 *)reich_sys_alloc((reichSize)cw * ch * sizeof(uint32));
    if (!ctx->screen.pixels) {
      reich_log(REICH_LOG_ERROR, "Failed to allocate canvas memory: %dx%d", cw,
                ch);
    }
    pctx->bitmapInfo.bmiHeader.biWidth = cw;
    pctx->bitmapInfo.bmiHeader.biHeight = -ch;
  }
}

static void reich_sys_poll_events(reichContext *ctx) {
  MSG msg;
  while (PeekMessageA(&msg, (HWND)0, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      ctx->running = 0;
    }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

static void reich_sys_present(reichContext *ctx) {
  reichPlatformContext *plat = (reichPlatformContext *)ctx->platform;
  if (plat->dwmFlush) {
    plat->dwmFlush();
  }
  if (ctx->screen.pixels) {
    StretchDIBits(plat->renderDC, 0, 0, ctx->windowWidth, ctx->windowHeight, 0,
                  0, ctx->screen.width, ctx->screen.height, ctx->screen.pixels,
                  &plat->bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
  }
}

static int64 reich_sys_get_ticks(void) {
  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);
  return (int64)count.QuadPart;
}

static int64 reich_sys_get_freq(void) {
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);
  return (int64)freq.QuadPart;
}

static void *reich_sys_alloc(reichSize size) {
  return VirtualAlloc((LPVOID)0, (SIZE_T)size, MEM_COMMIT | MEM_RESERVE,
                      PAGE_READWRITE);
}

static void reich_sys_free(void *ptr) {
  if (ptr) {
    VirtualFree(ptr, 0, MEM_RELEASE);
  }
}

#else
#error "Platform not supported"
#endif

REICH_API void reich_arena_init(reichArena *a, void *mem, reichSize size) {
  a->base = (uint8 *)mem;
  a->size = size;
  a->used = 0;
}

REICH_API void *reich_arena_alloc(reichArena *a, reichSize size) {
  void *ptr;
  reichSize aligned = (size + 7) & ~7;
  if (a->used + aligned > a->size) {
    reich_log(REICH_LOG_ERROR, "Arena overflow. Capacity: %lu, Requested: %lu",
              a->size, aligned);
    return (void *)0;
  }
  ptr = (void *)(a->base + a->used);
  a->used += aligned;
  return ptr;
}

REICH_API void reich_arena_reset(reichArena *a) { a->used = 0; }

static void reich_engine_process_input(reichContext *ctx) {
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

  reich_memset(ctx->input.buttonsPressed, 0, sizeof(ctx->input.buttonsPressed));
  reich_memset(ctx->input.buttonsReleased, 0,
               sizeof(ctx->input.buttonsReleased));
  reich_memset(ctx->input.keysPressed, 0, sizeof(ctx->input.keysPressed));
  reich_memset(ctx->input.keysReleased, 0, sizeof(ctx->input.keysReleased));

  reich_sys_poll_events(ctx);

  if (reich_key_pressed(ctx, 0x73)) { /* VK_F4 = 0x73 */
    int32 newScale = ctx->scale + 1;
    if (newScale > 4)
      newScale = 1;
    reich_set_scale(ctx, newScale);
  }

  if (ctx->scale > 0) {
    ctx->input.scaledMouseX = ctx->input.mouseX / ctx->scale;
    ctx->input.scaledMouseY = ctx->input.mouseY / ctx->scale;
  }

  ctx->input.deltaX = ctx->input.mouseX - ctx->input.lastMouseX;
  ctx->input.deltaY = ctx->input.mouseY - ctx->input.lastMouseY;
}

static void reich_draw_decorations(reichContext *ctx) {
  int32 w = ctx->screen.width;
  int32 btnW = REICH_TITLE_BUTTON_WIDTH;
  int32 h = REICH_TITLE_BAR_HEIGHT;
  int32 mx = ctx->input.scaledMouseX;
  int32 my = ctx->input.scaledMouseY;
  int32 closeX = w - btnW;
  int32 maxX = closeX - btnW;
  int32 minX = maxX - btnW;
  uint32 cClose = 0xFF333333;
  uint32 cMax = 0xFF333333;
  uint32 cMin = 0xFF333333;
  int32 i;
  reich_draw_rect(ctx, 0, 0, w, h, 0xFF222222);
  reich_draw_rect(ctx, 0, h - 1, w, 1, 0xFF444444);
  if (ctx->windowTitle) {
    reich_draw_text(ctx, 10, 8, ctx->windowTitle, 0xFFAAAAAA);
  }
  if (!ctx->isMaximized) {
    reich_draw_frame(ctx, 0, 0, w, ctx->screen.height, 0xFF444444);
  }
  if (my < h) {
    if (mx >= closeX) {
      cClose = 0xFFE81123;
    } else if (mx >= maxX) {
      cMax = 0xFF555555;
    } else if (mx >= minX) {
      cMin = 0xFF555555;
    }
  }
  reich_draw_rect(ctx, closeX, 0, btnW, h, cClose);
  for (i = 0; i < 10; ++i) {
    reich_put_pixel(ctx, closeX + 17 + i, 11 + i, 0xFFFFFFFF);
    reich_put_pixel(ctx, closeX + 17 + 9 - i, 11 + i, 0xFFFFFFFF);
  }
  reich_draw_rect(ctx, maxX, 0, btnW, h, cMax);
  if (ctx->isMaximized) {
    reich_draw_frame(ctx, maxX + 17, 13, 8, 8, 0xFFFFFFFF);
    reich_draw_frame(ctx, maxX + 19, 11, 8, 8, 0xFFFFFFFF);
  } else {
    reich_draw_frame(ctx, maxX + 17, 11, 10, 10, 0xFFFFFFFF);
  }
  reich_draw_rect(ctx, minX, 0, btnW, h, cMin);
  reich_draw_rect(ctx, minX + 17, 20, 10, 1, 0xFFFFFFFF);
}

static void reich_update_window_controls(reichContext *ctx) {
  int32 mx;
  int32 my;
  int32 btnW;
  int32 h;
  int32 w;
  if (!ctx->input.buttons[0] || ctx->input.lastButtons[0]) {
    return;
  }
  mx = ctx->input.scaledMouseX;
  my = ctx->input.scaledMouseY;
  btnW = REICH_TITLE_BUTTON_WIDTH;
  h = REICH_TITLE_BAR_HEIGHT;
  w = ctx->screen.width;

  if (my < h && mx >= w - (btnW * 3)) {
    if (mx >= w - btnW) {
      reich_sys_close(ctx);
    } else if (mx >= w - (btnW * 2)) {
      reich_sys_toggle_maximize(ctx);
    } else {
      reich_sys_minimize(ctx);
    }
  }
}

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

REICH_API int32 reich_init(reichContext *ctx, const char *title, int32 width,
                           int32 height, real64 targetFps) {
  reichSize memSize;
  reichSize platSize;
  void *memory;
  if (!ctx) {
    reich_log(REICH_LOG_ERROR, "reich_init called with NULL context.");
    return 0;
  }

  reich_memset(ctx, 0, sizeof(reichContext));

  ctx->scale = 1;
  ctx->windowTitle = title;
  ctx->windowWidth = width;
  ctx->windowHeight = height;
  ctx->running = 1;
  ctx->fixedDt = 1.0 / targetFps;
  ctx->accumulator = 0.0;
  ctx->perfFreq = reich_sys_get_freq();
  ctx->lastCounter = reich_sys_get_ticks();

  memSize = (reichSize)256 * 1024 * 1024;
  memory = reich_sys_alloc(memSize);
  if (!memory) {
    reich_log(REICH_LOG_ERROR,
              "Failed to allocate main memory block (%lu bytes).", memSize);
    return 0;
  }

  reich_arena_init(&ctx->permMem, memory, memSize);
  platSize = reich_sys_get_platform_data_size();
  ctx->platform = reich_arena_alloc(&ctx->permMem, platSize);
  if (!ctx->platform) {
    reich_log(REICH_LOG_ERROR, "Failed to allocate platform data.");
    return 0;
  }

  ctx->screen.width = width;
  ctx->screen.height = height;
  ctx->screen.pixels =
      (uint32 *)reich_sys_alloc((reichSize)width * height * sizeof(uint32));
  if (!ctx->screen.pixels) {
    reich_log(REICH_LOG_ERROR, "Failed to allocate screen pixels.");
    return 0;
  }

  reich_init_default_font(ctx);
  if (!reich_sys_window_init(ctx, title, width, height)) {
    reich_log(REICH_LOG_ERROR, "Window initialization failed.");
    return 0;
  }

  reich_log(REICH_LOG_INFO, "Reich initialized. Target FPS: %.2f", targetFps);
  return 1;
}

REICH_API void reich_set_callbacks(reichContext *ctx, PFUSERUPDATE update,
                                   PFUSERRENDER render, PFUSERRESIZE resize,
                                   PFUSERINPUT input) {
  if (!ctx) {
    return;
  }
  ctx->userUpdate = update;
  ctx->userRender = render;
  ctx->userResize = resize;
  ctx->userInput = input;
}

REICH_API void reich_set_scale(reichContext *ctx, int32 scale) {
  if (!ctx || scale < 1) {
    return;
  }
  ctx->scale = scale;
  reich_sys_resize_canvas(ctx, ctx->windowWidth, ctx->windowHeight);
}

REICH_API int32 reich_running(reichContext *ctx) { return ctx->running; }

REICH_API void reich_poll_events(reichContext *ctx) {
  reich_engine_process_input(ctx);
}

REICH_API void reich_timer_tick(reichContext *ctx) {
  int64 currentCounter = reich_sys_get_ticks();
  real64 frameTime =
      (real64)(currentCounter - ctx->lastCounter) / (real64)ctx->perfFreq;
  ctx->lastCounter = currentCounter;
  if (frameTime > 0.25) {
    frameTime = 0.25;
  }
  ctx->accumulator += frameTime;
}

REICH_API int32 reich_timer_step(reichContext *ctx) {
  if (ctx->accumulator >= ctx->fixedDt) {
    ctx->accumulator -= ctx->fixedDt;
    return 1;
  }
  return 0;
}

REICH_API real64 reich_timer_alpha(reichContext *ctx) {
  return ctx->accumulator / ctx->fixedDt;
}

REICH_API void reich_present(reichContext *ctx) { reich_sys_present(ctx); }

REICH_API void reich_run(reichContext *ctx) {
  uint8 bkKP[REICH_KEY_MAX];
  uint8 bkKR[REICH_KEY_MAX];
  uint8 bkBP[REICH_MOUSE_BUTTONS];
  uint8 bkBR[REICH_MOUSE_BUTTONS];
  int32 first;

  reich_log(REICH_LOG_INFO, "Starting main loop.");
  while (reich_running(ctx)) {
    reich_poll_events(ctx);
    reich_update_window_controls(ctx);

    reich_memcpy(bkKP, ctx->input.keysPressed, REICH_KEY_MAX);
    reich_memcpy(bkKR, ctx->input.keysReleased, REICH_KEY_MAX);
    reich_memcpy(bkBP, ctx->input.buttonsPressed, REICH_MOUSE_BUTTONS);
    reich_memcpy(bkBR, ctx->input.buttonsReleased, REICH_MOUSE_BUTTONS);

    reich_timer_tick(ctx);
    first = 1;
    while (reich_timer_step(ctx)) {
      if (ctx->userUpdate) {
        ctx->userUpdate(ctx);
      }
      if (first) {
        reich_memset(ctx->input.keysPressed, 0, REICH_KEY_MAX);
        reich_memset(ctx->input.keysReleased, 0, REICH_KEY_MAX);
        reich_memset(ctx->input.buttonsPressed, 0, REICH_MOUSE_BUTTONS);
        reich_memset(ctx->input.buttonsReleased, 0, REICH_MOUSE_BUTTONS);
        first = 0;
      }
    }

    reich_memcpy(ctx->input.keysPressed, bkKP, REICH_KEY_MAX);
    reich_memcpy(ctx->input.keysReleased, bkKR, REICH_KEY_MAX);
    reich_memcpy(ctx->input.buttonsPressed, bkBP, REICH_MOUSE_BUTTONS);
    reich_memcpy(ctx->input.buttonsReleased, bkBR, REICH_MOUSE_BUTTONS);

    if (ctx->userInput) {
      ctx->userInput(ctx);
    }

    if (ctx->userRender) {
      ctx->userRender(ctx, reich_timer_alpha(ctx));
    }
    reich_draw_decorations(ctx);
    reich_present(ctx);
  }
  reich_log(REICH_LOG_INFO, "Main loop ended.");
}

REICH_API void reich_clear(reichContext *ctx, uint32 color) {
  int32 count = ctx->screen.width * ctx->screen.height;
  uint32 *p = ctx->screen.pixels;
  while (count--) {
    *p++ = color;
  }
}

REICH_API int32 reich_key_down(reichContext *ctx, int32 keyCode) {
  if (keyCode < 0 || keyCode >= REICH_KEY_MAX) {
    return 0;
  }
  return (int32)ctx->input.keys[keyCode];
}

REICH_API int32 reich_key_pressed(reichContext *ctx, int32 keyCode) {
  if (keyCode < 0 || keyCode >= REICH_KEY_MAX) {
    return 0;
  }
  return (int32)ctx->input.keysPressed[keyCode];
}

REICH_API int32 reich_key_released(reichContext *ctx, int32 keyCode) {
  if (keyCode < 0 || keyCode >= REICH_KEY_MAX) {
    return 0;
  }
  return (int32)ctx->input.keysReleased[keyCode];
}

REICH_API int32 reich_mouse_down(reichContext *ctx, int32 button) {
  if (button < 0 || button >= REICH_MOUSE_BUTTONS) {
    return 0;
  }
  return (int32)ctx->input.buttons[button];
}

REICH_API int32 reich_mouse_pressed(reichContext *ctx, int32 button) {
  if (button < 0 || button >= REICH_MOUSE_BUTTONS) {
    return 0;
  }
  return (int32)ctx->input.buttonsPressed[button];
}

REICH_API int32 reich_mouse_released(reichContext *ctx, int32 button) {
  if (button < 0 || button >= REICH_MOUSE_BUTTONS) {
    return 0;
  }
  return (int32)ctx->input.buttonsReleased[button];
}

REICH_API int32 reich_get_mouse_x(reichContext *ctx) {
  return ctx->input.mouseX / ctx->scale;
}

REICH_API int32 reich_get_mouse_y(reichContext *ctx) {
  return ctx->input.mouseY / ctx->scale;
}

REICH_API void reich_draw_rect(reichContext *ctx, int32 x, int32 y, int32 w,
                               int32 h, uint32 color) {
  int32 i;
  int32 j;
  int32 sx;
  int32 sy;
  int32 ex;
  int32 ey;
  uint32 *pixels;
  if (!ctx || !ctx->screen.pixels) {
    return;
  }
  pixels = ctx->screen.pixels;
  sx = x;
  if (sx < 0) {
    sx = 0;
  }
  sy = y;
  if (sy < 0) {
    sy = 0;
  }
  ex = x + w;
  if (ex > ctx->screen.width) {
    ex = ctx->screen.width;
  }
  ey = y + h;
  if (ey > ctx->screen.height) {
    ey = ctx->screen.height;
  }
  for (j = sy; j < ey; ++j) {
    for (i = sx; i < ex; ++i) {
      pixels[j * ctx->screen.width + i] = color;
    }
  }
}

REICH_API void reich_draw_frame(reichContext *ctx, int32 x, int32 y, int32 w,
                                int32 h, uint32 color) {
  reich_draw_rect(ctx, x, y, w, 1, color);
  reich_draw_rect(ctx, x, y + h - 1, w, 1, color);
  reich_draw_rect(ctx, x, y, 1, h, color);
  reich_draw_rect(ctx, x + w - 1, y, 1, h, color);
}

REICH_API void reich_draw_text(reichContext *ctx, int32 x, int32 y,
                               const char *str, uint32 color) {
  int32 i;
  int32 j;
  int32 cx;
  int32 cy;
  int32 fw;
  int32 fh;
  int32 start;
  int32 end;
  uint8 c;
  uint8 *font;
  uint32 kern = 4;
  if (!ctx || !str || ctx->activeFont >= ctx->fontCount) {
    return;
  }
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
            reich_draw_rect(ctx, cx + i, cy + j, 1, 1, color);
          }
        }
      }
    }
    cx += fw + font[2] - kern;
  }
}

REICH_API int32 reich_ui_btn(reichContext *ctx, int32 id, int32 x, int32 y,
                             int32 w, int32 h, const char *label,
                             uint32 bgCol) {
  int32 mx = reich_get_mouse_x(ctx);
  int32 my = reich_get_mouse_y(ctx);
  int32 hover = (mx >= x && mx < x + w && my >= y && my < y + h);
  int32 clicked = 0;
  uint32 col = bgCol;

  if (hover) {
    ctx->input.hotId = id;
    if (ctx->input.activeId == 0 && reich_mouse_pressed(ctx, 0)) {
      ctx->input.activeId = id;
    }
  }

  if (ctx->input.activeId == id) {
    if (!ctx->input.buttons[0]) {
      if (hover) {
        clicked = 1;
      }
      ctx->input.activeId = 0;
    }
  }

  if (hover) {
    col = 0xFF000000 | ((bgCol & 0x00FEFEFE) >> 1);
    if (ctx->input.activeId == id) {
      col = 0xFF000000 | ((col & 0x00FEFEFE) >> 1);
    } else {
      col += 0x00202020;
    }
  }

  reich_draw_rect(ctx, x, y, w, h, col);
  if (label) {
    int32 kern = 4;
    uint8 *f = ctx->fonts[ctx->activeFont];
    reichSize len = reich_strlen(label);
    int32 tw = (int32)len * (f[0] + f[2] - kern);
    reich_draw_text(ctx, x + (w - tw) / 2, y + (h - f[1]) / 2, label, 0xFFFFFF);
  }
  return clicked;
}

static uint8 *reich_font_import(reichArena *a, uint32 *pixels, int32 iw,
                                int32 ih, int32 gw, int32 gh, int32 start,
                                int32 end) {
  int32 x;
  int32 y;
  int32 i;
  int32 count = end - start + 1;
  int32 total = (count * gw * gh + 7) / 8;
  uint8 *data = (uint8 *)reich_arena_alloc(a, (reichSize)(5 + total));
  if (!data) {
    return (void *)0;
  }
  data[0] = (uint8)gw;
  data[1] = (uint8)gh;
  data[2] = 1;
  data[3] = (uint8)start;
  data[4] = (uint8)end;
  for (i = 0; i < total; ++i) {
    data[5 + i] = 0;
  }
  if (gw <= 0)
    return data;
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

REICH_API reichCanvas reich_load_bmp(const char *filename) {
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
  uint8 *data;
  uint32 *palette;
  uint32 i;
  int32 x;
  int32 y;
  uint32 r;
  uint32 g;
  uint32 b;
  uint32 a;
  int32 rowSize;
  uint8 *row;
  uint8 *ptr;
  int32 shiftR;
  int32 shiftG;
  int32 shiftB;
  int32 shiftA;
  int32 scaleR;
  int32 scaleG;
  int32 scaleB;
  int32 scaleA;

  reich_memset(&canvas, 0, sizeof(reichCanvas));
  f = reich_file_open(filename, REICH_FILE_READ);
  if (!f) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Failed to open file %s", filename);
    return canvas;
  }
  if (reich_file_read(f, fileHeader, 14) != 14) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Failed to read file header.");
    reich_file_close(f);
    return canvas;
  }
  if (fileHeader[0] != 'B' || fileHeader[1] != 'M') {
    reich_log(REICH_LOG_ERROR, "BMP Load: Invalid magic number.");
    reich_file_close(f);
    return canvas;
  }
  dataOffset = *(uint32 *)(fileHeader + 10);
  fileSize = *(uint32 *)(fileHeader + 2);
  if (reich_file_read(f, &headerSize, 4) != 4) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Failed to read header size.");
    reich_file_close(f);
    return canvas;
  }
  if (headerSize < 12) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Header size too small.");
    reich_file_close(f);
    return canvas;
  }
  if (headerSize > 124) {
    headerSize = 124;
  }
  *(uint32 *)infoHeader = headerSize;
  if (reich_file_read(f, infoHeader + 4, headerSize - 4) != headerSize - 4) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Failed to read info header.");
    reich_file_close(f);
    return canvas;
  }
  if (headerSize == 12) {
    width = *(int16 *)(infoHeader + 4);
    height = *(int16 *)(infoHeader + 6);
    planes = *(int16 *)(infoHeader + 8);
    bpp = *(int16 *)(infoHeader + 10);
    compression = 0;
    imageSize = 0;
    colorsUsed = 0;
  } else {
    width = *(int32 *)(infoHeader + 4);
    height = *(int32 *)(infoHeader + 8);
    planes = *(int16 *)(infoHeader + 12);
    bpp = *(int16 *)(infoHeader + 14);
    compression = *(uint32 *)(infoHeader + 16);
    imageSize = *(uint32 *)(infoHeader + 20);
    colorsUsed = *(uint32 *)(infoHeader + 32);
  }
  if (width <= 0 || height == 0 || planes != 1) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Invalid dimensions or planes.");
    reich_file_close(f);
    return canvas;
  }
  isTopDown = 0;
  if (height < 0) {
    isTopDown = 1;
    height = -height;
  }
  if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Unsupported BPP: %d", bpp);
    reich_file_close(f);
    return canvas;
  }
  if (compression != 0 && compression != 1 && compression != 2 &&
      compression != 3) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Unsupported compression: %d",
              compression);
    reich_file_close(f);
    return canvas;
  }
  canvas.width = width;
  canvas.height = height;
  canvas.pixels = (uint32 *)reich_sys_alloc((reichSize)width * height * 4);
  if (!canvas.pixels) {
    reich_log(REICH_LOG_ERROR, "BMP Load: Failed to allocate pixels.");
    reich_file_close(f);
    return canvas;
  }
  palette = (uint32 *)NULL;
  if (bpp <= 8) {
    if (colorsUsed == 0) {
      colorsUsed = 1 << bpp;
    }
    palette = (uint32 *)reich_sys_alloc((reichSize)colorsUsed * 4);
    if (headerSize == 12) {
      for (i = 0; i < colorsUsed; ++i) {
        uint8 pal[3];
        reich_file_read(f, pal, 3);
        palette[i] = 0xFF000000 | (pal[2] << 16) | (pal[1] << 8) | pal[0];
      }
    } else {
      for (i = 0; i < colorsUsed; ++i) {
        uint8 pal[4];
        reich_file_read(f, pal, 4);
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
      rMask = *(uint32 *)(infoHeader + 40);
      gMask = *(uint32 *)(infoHeader + 44);
      bMask = *(uint32 *)(infoHeader + 48);
      aMask = *(uint32 *)(infoHeader + 52);
    } else {
      reich_file_read(f, &rMask, 4);
      reich_file_read(f, &gMask, 4);
      reich_file_read(f, &bMask, 4);
    }
  } else if (bpp == 16) {
    rMask = 0x7C00;
    gMask = 0x03E0;
    bMask = 0x001F;
    aMask = 0x0000;
  }
  reich_file_seek(f, dataOffset, REICH_SEEK_SET);
  if (imageSize == 0) {
    imageSize = fileSize - dataOffset;
  }
  data = (uint8 *)reich_sys_alloc((reichSize)imageSize);
  reich_file_read(f, data, imageSize);
  reich_file_close(f);
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
    if (m)
      scaleR = 255 / m;
    else
      scaleR = 0;
    m = gMask;
    while (m && !(m & 1)) {
      shiftG++;
      m >>= 1;
    }
    if (m)
      scaleG = 255 / m;
    else
      scaleG = 0;
    m = bMask;
    while (m && !(m & 1)) {
      shiftB++;
      m >>= 1;
    }
    if (m)
      scaleB = 255 / m;
    else
      scaleB = 0;
    m = aMask;
    while (m && !(m & 1)) {
      shiftA++;
      m >>= 1;
    }
    if (m)
      scaleA = 255 / m;
    else
      scaleA = 0;
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
            if (val & 1) {
              ptr++;
            }
          } else {
            for (i = 0; i < val; ++i) {
              if (x < width) {
                uint8 bByte = *ptr;
                uint8 idx = (i & 1) ? (bByte & 0x0F) : (bByte >> 4);
                int32 dy = isTopDown ? y : (height - 1 - y);
                canvas.pixels[dy * width + x] = palette[idx];
                if (i & 1) {
                  ptr++;
                }
              }
              x++;
            }
            if ((val + 1) / 2 & 1) {
              ptr++;
            }
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
          uint16 wWord = *(uint16 *)(row + x * 2);
          r = ((wWord & rMask) >> shiftR) * scaleR;
          g = ((wWord & gMask) >> shiftG) * scaleG;
          b = ((wWord & bMask) >> shiftB) * scaleB;
          canvas.pixels[dy * width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        } else if (bpp == 24) {
          b = row[x * 3 + 0];
          g = row[x * 3 + 1];
          r = row[x * 3 + 2];
          canvas.pixels[dy * width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        } else if (bpp == 32) {
          uint32 dWord = *(uint32 *)(row + x * 4);
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
  if (palette) {
    reich_sys_free(palette);
  }
  return canvas;
}

REICH_API void reich_init_default_font(reichContext *ctx) {
  if (ctx->fontCount >= REICH_MAX_FONTS) {
    return;
  }
  ctx->fonts[ctx->fontCount] = (uint8 *)reich_arena_alloc(
      &ctx->permMem, (reichSize)sizeof(REICH_FONT_DATA));
  if (ctx->fonts[ctx->fontCount]) {
    uint32 i;
    for (i = 0; i < sizeof(REICH_FONT_DATA); ++i) {
      ctx->fonts[ctx->fontCount][i] = REICH_FONT_DATA[i];
    }
    ctx->fontCount++;
    reich_log(REICH_LOG_INFO, "Default font initialized.");
  }
}

REICH_API void reich_load_font(reichContext *ctx, const char *filename,
                               int32 numChars, int32 glyphWidth,
                               int32 glyphHeight) {
  reichCanvas bmp = reich_load_bmp(filename);
  if (bmp.pixels) {
    int32 gw = glyphWidth;
    int32 gh = glyphHeight;
    int32 rows = 16;
    int32 start = 0;
    int32 end = 255;
    int32 cols = 16;

    if (gw <= 0) {
      gw = bmp.width / 16;
    }

    if (gw > 0)
      cols = bmp.width / gw;
    if (cols < 1)
      cols = 1;

    if (numChars <= 0) {
      if (bmp.height % 16 != 0 && bmp.height % 6 == 0) {
        numChars = 96;
      } else if (bmp.height % 6 == 0 && (bmp.height / 6) >= gw &&
                 (bmp.height / 16) < gw) {
        numChars = 96;
      } else {
        numChars = cols * 16;
        if (numChars > 256)
          numChars = 256;
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
      if (rows < 1)
        rows = 1;
      gh = bmp.height / rows;
    }

    if (gw > 0 && gh > 0) {
      ctx->fonts[ctx->fontCount++] = reich_font_import(
          &ctx->permMem, bmp.pixels, bmp.width, bmp.height, gw, gh, start, end);
      reich_log(REICH_LOG_INFO, "Loaded font: %s (%d chars, %dx%d glyphs)",
                filename, numChars, gw, gh);
    } else {
      reich_log(REICH_LOG_ERROR, "Invalid font dimensions for %s", filename);
    }
    reich_sys_free(bmp.pixels);
  } else {
    reich_log(REICH_LOG_WARN, "Failed to load font bitmap: %s", filename);
  }
}

REICH_API void reich_load_fonts(reichContext *ctx, const char *fn, int32 gw,
                                int32 gh, int32 amt) {
  char filename[260];
  reichHandle h = reich_find_first(fn, filename, 260);
  if (h) {
    do {
      reich_load_font(ctx, filename, amt, gw, gh);
    } while (reich_find_next(h, filename, 260));
    reich_find_close(h);
  } else {
    reich_log(REICH_LOG_INFO, "No font found");
  }
  if (ctx->fontCount == 0) {
    reich_init_default_font(ctx);
  }
}

REICH_API int32 reich_get_mouse_wheel(reichContext *ctx) {
  return ctx->input.mouseWheel;
}

REICH_API uint32 reich_get_char_pressed(reichContext *ctx) {
  return ctx->input.lastChar;
}

REICH_API void reich_show_cursor(reichContext *ctx, int32 show) {
  ShowCursor(show);
}

#endif

#endif
