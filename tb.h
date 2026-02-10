
#ifndef TERMBOX_H
#define TERMBOX_H

#ifdef __cplusplus
extern "C" {
#endif

#define TB_KEY_ESCAPE    0x1B
#define TB_KEY_ENTER     0x0D
#define TB_KEY_SPACE     0x20
#define TB_KEY_BACKSPACE 0x08
#define TB_KEY_TAB       0x09
#define TB_KEY_UP        0xA0
#define TB_KEY_DOWN      0xA1
#define TB_KEY_LEFT      0xA2
#define TB_KEY_RIGHT     0xA3
#define TB_KEY_INSERT    0xA4
#define TB_KEY_DELETE    0xA5
#define TB_KEY_HOME      0xA6
#define TB_KEY_END       0xA7
#define TB_KEY_PGUP      0xA8
#define TB_KEY_PGDN      0xA9
#define TB_KEY_F1        0xB0
#define TB_KEY_F2        0xB1
#define TB_KEY_F3        0xB2
#define TB_KEY_F4        0xB3
#define TB_KEY_F5        0xB4
#define TB_KEY_F6        0xB5
#define TB_KEY_F7        0xB6
#define TB_KEY_F8        0xB7
#define TB_KEY_F9        0xB8
#define TB_KEY_F10       0xB9
#define TB_KEY_F11       0xBA
#define TB_KEY_F12       0xBB

#define TB_MOUSE_LEFT   1
#define TB_MOUSE_RIGHT  2
#define TB_MOUSE_MIDDLE 3

#define TB_RGB(r, g, b) \
  ((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF))

typedef struct {
  int x;
  int y;
  int left_down;
  int right_down;
  int middle_down;
  int wheel;
} tb_mouse_state_t;

int tb_init(int width, int height, const char* title);
void tb_shutdown(void);
int tb_update(void);
void tb_present(void);

void tb_clear(unsigned int fg, unsigned int bg, char c);
void tb_put(int x, int y, char c, unsigned int fg, unsigned int bg);
void tb_print(int x, int y, const char* str, unsigned int fg, unsigned int bg);
int tb_width(void);
int tb_height(void);

int tb_key_down(int key);
int tb_key_pressed(int key);
tb_mouse_state_t tb_mouse_state(void);

void tb_audio_init(int sample_rate);
void tb_audio_push(float* samples, int count);
int tb_audio_free_space(void);

typedef void* tb_thread_t;
typedef void* tb_mutex_t;

/* Platform-specific thread function signature */
#ifdef _WIN32
typedef unsigned(__stdcall* tb_thread_func_t)(void* user_data);
#else
typedef void* (*tb_thread_func_t)(void* user_data);
#endif

tb_thread_t tb_thread_create(tb_thread_func_t func, void* user_data);
void tb_thread_join(tb_thread_t thread);
tb_mutex_t tb_mutex_init(void);
void tb_mutex_lock(tb_mutex_t mutex);
void tb_mutex_unlock(tb_mutex_t mutex);
void tb_mutex_destroy(tb_mutex_t mutex);
void tb_sleep(int ms);

#ifdef __cplusplus
}
#endif

#endif

#ifdef TB_IMPLEMENTATION

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYS          512
#define MAX_AUDIO_BUFFER  32768
#define AUDIO_BUFFER_MASK (MAX_AUDIO_BUFFER - 1)

typedef struct {
  char ch;
  unsigned int fg;
  unsigned int bg;
} eng_cell_t;

typedef struct {
  int width;
  int height;
  eng_cell_t* back_buffer;
  int keys[MAX_KEYS];
  int prev_keys[MAX_KEYS];
  tb_mouse_state_t mouse;
  volatile long running;

  float audio_buffer[MAX_AUDIO_BUFFER];
  volatile unsigned int audio_write_idx;
  volatile unsigned int audio_read_idx;

  float last_sample;
  float gain_ramp;
  int is_starving;

  tb_thread_t audio_thread;
  int audio_sample_rate;
  char* render_buffer;
  int render_buffer_cap;
} eng_state_t;

static eng_state_t g_engine;

static void eng_init(int w, int h);
static void eng_resize_buffer(int w, int h);
static void eng_handle_input(int key, int down);
static void eng_handle_mouse(int x, int y, int btn, int wheel, int down);
static void eng_handle_resize(int w, int h);
static void eng_audio_mix(float* output, int frames, int channels);

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <process.h>
#include <windows.h>

#ifndef __cplusplus
#define COBJMACROS
#endif

#ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
#endif
#ifndef AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY
#define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

typedef struct {
  HANDLE h_out;
  HANDLE h_in;
  DWORD original_in_mode;
  DWORD original_out_mode;
  UINT original_cp;
  int console_initialized;

  IMMDeviceEnumerator* p_enumerator;
  IMMDevice* p_device;
  IAudioClient* p_audio_client;
  IAudioRenderClient* p_render_client;
  HANDLE h_audio_event;

  int hw_channels;
  int hw_sample_rate;
  int hw_bits_per_sample;
  int hw_is_float;
  float* conversion_buffer;
  int conversion_buffer_cap;
} pf_state_t;

static pf_state_t g_platform;

static int pf_init_console(int w, int h, const char* title);
static void pf_close_console(void);
static void pf_poll_events(void);
static void pf_present_buffer(const eng_cell_t* buffer, int w, int h);
static int pf_init_audio_backend(int sample_rate);
static void pf_close_audio_backend(void);
static void pf_process_audio_chunk(void);
static void pf_convert_mix(
    BYTE* dst,
    const float* src,
    int frames,
    int channels,
    int bits,
    int is_float);
static unsigned __stdcall pf_audio_thread_proc(void* data);
static BOOL WINAPI pf_ctrl_handler(DWORD fdwCtrlType);

tb_thread_t tb_thread_create(tb_thread_func_t func, void* user_data) {
  HANDLE h = (HANDLE)_beginthreadex(
      NULL, 0, (unsigned(__stdcall*)(void*))func, user_data, 0, NULL);
  return (tb_thread_t)h;
}

void tb_thread_join(tb_thread_t thread) {
  if (thread) {
    WaitForSingleObject((HANDLE)thread, INFINITE);
    CloseHandle((HANDLE)thread);
  }
}

tb_mutex_t tb_mutex_init(void) {
  CRITICAL_SECTION* cs = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
  if (cs) { InitializeCriticalSection(cs); }
  return (tb_mutex_t)cs;
}

void tb_mutex_lock(tb_mutex_t mutex) {
  if (mutex) { EnterCriticalSection((CRITICAL_SECTION*)mutex); }
}

void tb_mutex_unlock(tb_mutex_t mutex) {
  if (mutex) { LeaveCriticalSection((CRITICAL_SECTION*)mutex); }
}

void tb_mutex_destroy(tb_mutex_t mutex) {
  if (mutex) {
    DeleteCriticalSection((CRITICAL_SECTION*)mutex);
    free(mutex);
  }
}

void tb_sleep(int ms) {
  Sleep(ms);
}

static BOOL WINAPI pf_ctrl_handler(DWORD fdwCtrlType) {
  switch (fdwCtrlType) {
  case CTRL_C_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    InterlockedExchange(&g_engine.running, 0);
    if (g_platform.h_audio_event) { SetEvent(g_platform.h_audio_event); }
    return TRUE;
  default:
    return FALSE;
  }
}

static int pf_init_console(int w, int h, const char* title) {
  COORD coord;
  DWORD mode;
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  g_platform.h_out = GetStdHandle(STD_OUTPUT_HANDLE);
  g_platform.h_in = GetStdHandle(STD_INPUT_HANDLE);

  if (g_platform.h_out == INVALID_HANDLE_VALUE ||
      g_platform.h_in == INVALID_HANDLE_VALUE) {
    return 0;
  }

  g_platform.original_cp = GetConsoleOutputCP();
  SetConsoleOutputCP(CP_UTF8);

  GetConsoleMode(g_platform.h_in, &g_platform.original_in_mode);
  GetConsoleMode(g_platform.h_out, &g_platform.original_out_mode);

  mode = g_platform.original_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(g_platform.h_out, mode)) {
    mode = g_platform.original_out_mode;
    SetConsoleMode(g_platform.h_out, mode);
  }

  if (!SetConsoleCtrlHandler(pf_ctrl_handler, TRUE)) { return 0; }

  if (w == 0 || h == 0) {
    if (GetConsoleScreenBufferInfo(g_platform.h_out, &csbi)) {
      w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
      h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
      w = 80;
      h = 25;
    }
  } else {
    SMALL_RECT rect;
    coord.X = (SHORT)w;
    coord.Y = (SHORT)h;

    rect.Left = 0;
    rect.Top = 0;
    rect.Right = (SHORT)(w - 1);
    rect.Bottom = (SHORT)(h - 1);

    SetConsoleWindowInfo(g_platform.h_out, TRUE, &rect);
    SetConsoleScreenBufferSize(g_platform.h_out, coord);
    SetConsoleWindowInfo(g_platform.h_out, TRUE, &rect);
  }

  SetConsoleTitle(title);

  GetConsoleMode(g_platform.h_in, &mode);
  mode &= ~ENABLE_QUICK_EDIT_MODE;
  mode |= ENABLE_EXTENDED_FLAGS;
  mode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
  SetConsoleMode(g_platform.h_in, mode);

  {
    DWORD written;
    const char* enter_alt = "\x1b[?1049h\x1b[?25l";
    WriteConsoleA(
        g_platform.h_out, enter_alt, (DWORD)strlen(enter_alt), &written, NULL);
  }

  g_platform.console_initialized = 1;
  eng_init(w, h);

  return 1;
}

static void pf_close_console(void) {
  if (!g_platform.console_initialized) { return; }

  {
    DWORD written;
    const char* exit_alt = "\x1b[?25h\x1b[?1049l";
    WriteConsoleA(
        g_platform.h_out, exit_alt, (DWORD)strlen(exit_alt), &written, NULL);
  }

  SetConsoleMode(g_platform.h_in, g_platform.original_in_mode);
  SetConsoleMode(g_platform.h_out, g_platform.original_out_mode);
  SetConsoleOutputCP(g_platform.original_cp);
  SetConsoleCtrlHandler(pf_ctrl_handler, FALSE);

  g_platform.console_initialized = 0;
}

static void pf_poll_events(void) {
  DWORD count;
  DWORD i;
  INPUT_RECORD ir[128];
  int key_code;

  g_engine.mouse.wheel = 0;

  GetNumberOfConsoleInputEvents(g_platform.h_in, &count);
  if (count > 0) {
    if (count > 128) { count = 128; }
    ReadConsoleInput(g_platform.h_in, ir, count, &count);

    for (i = 0; i < count; ++i) {
      if (ir[i].EventType == KEY_EVENT) {
        key_code = ir[i].Event.KeyEvent.wVirtualKeyCode;
        if (key_code >= 96 && key_code <= 105) { key_code -= 48; }

        if (key_code == VK_ESCAPE) {
          eng_handle_input(TB_KEY_ESCAPE, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_RETURN) {
          eng_handle_input(TB_KEY_ENTER, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_SPACE) {
          eng_handle_input(TB_KEY_SPACE, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_BACK) {
          eng_handle_input(TB_KEY_BACKSPACE, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_TAB) {
          eng_handle_input(TB_KEY_TAB, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_UP) {
          eng_handle_input(TB_KEY_UP, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_DOWN) {
          eng_handle_input(TB_KEY_DOWN, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_LEFT) {
          eng_handle_input(TB_KEY_LEFT, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_RIGHT) {
          eng_handle_input(TB_KEY_RIGHT, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_INSERT) {
          eng_handle_input(TB_KEY_INSERT, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_DELETE) {
          eng_handle_input(TB_KEY_DELETE, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_HOME) {
          eng_handle_input(TB_KEY_HOME, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_END) {
          eng_handle_input(TB_KEY_END, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_PRIOR) {
          eng_handle_input(TB_KEY_PGUP, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code == VK_NEXT) {
          eng_handle_input(TB_KEY_PGDN, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code >= 0x30 && key_code <= 0x5A) {
          eng_handle_input(key_code, ir[i].Event.KeyEvent.bKeyDown);
        } else if (key_code >= VK_F1 && key_code <= VK_F12) {
          eng_handle_input(
              TB_KEY_F1 + (key_code - VK_F1), ir[i].Event.KeyEvent.bKeyDown);
        }
      } else if (ir[i].EventType == MOUSE_EVENT) {
        int x = ir[i].Event.MouseEvent.dwMousePosition.X;
        int y = ir[i].Event.MouseEvent.dwMousePosition.Y;
        int btn = ir[i].Event.MouseEvent.dwButtonState;
        int wheel = 0;

        if (ir[i].Event.MouseEvent.dwEventFlags & MOUSE_WHEELED) {
          short delta = (short)(btn >> 16);
          if (delta > 0) {
            wheel = 1;
          } else if (delta < 0) {
            wheel = -1;
          }
        }

        eng_handle_mouse(
            x,
            y,
            TB_MOUSE_LEFT,
            wheel,
            (btn & FROM_LEFT_1ST_BUTTON_PRESSED) != 0);
        eng_handle_mouse(
            x,
            y,
            TB_MOUSE_RIGHT,
            wheel,
            (btn & RIGHTMOST_BUTTON_PRESSED) != 0);
        eng_handle_mouse(
            x,
            y,
            TB_MOUSE_MIDDLE,
            wheel,
            (btn & FROM_LEFT_2ND_BUTTON_PRESSED) != 0);
      } else if (ir[i].EventType == WINDOW_BUFFER_SIZE_EVENT) {
        int new_w = ir[i].Event.WindowBufferSizeEvent.dwSize.X;
        int new_h = ir[i].Event.WindowBufferSizeEvent.dwSize.Y;

        eng_handle_resize(new_w, new_h);
      }
    }
  }
}

static void pf_present_buffer(const eng_cell_t* buffer, int w, int h) {
  if (!buffer) { return; }

  int est_size = w * h * 45;

  if (g_engine.render_buffer_cap < est_size) {
    char* temp = (char*)realloc(g_engine.render_buffer, est_size);
    if (!temp) { return; }
    g_engine.render_buffer = temp;
    g_engine.render_buffer_cap = est_size;
  }

  if (!g_engine.render_buffer) { return; }

  char* ptr = g_engine.render_buffer;
  char* end = g_engine.render_buffer + g_engine.render_buffer_cap;
  int last_fg = -1;
  int last_bg = -1;
  int i;
  int count = w * h;
  int written_bytes;

  written_bytes = _snprintf(ptr, (size_t)(end - ptr), "\x1b[H");
  if (written_bytes > 0) { ptr += written_bytes; }

  for (i = 0; i < count; ++i) {
    eng_cell_t c = buffer[i];

    if (c.fg != last_fg || c.bg != last_bg) {
      written_bytes = _snprintf(
          ptr,
          (size_t)(end - ptr),
          "\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm",
          (c.fg >> 16) & 0xFF,
          (c.fg >> 8) & 0xFF,
          c.fg & 0xFF,
          (c.bg >> 16) & 0xFF,
          (c.bg >> 8) & 0xFF,
          c.bg & 0xFF);
      if (written_bytes > 0) { ptr += written_bytes; }
      last_fg = c.fg;
      last_bg = c.bg;
    }

    if (ptr < end - 1) { *ptr++ = c.ch; }
  }

  DWORD written;
  WriteConsoleA(
      g_platform.h_out,
      g_engine.render_buffer,
      (DWORD)(ptr - g_engine.render_buffer),
      &written,
      NULL);
}

static const CLSID CLSID_MMDeviceEnumerator_Local = {
    0xBCDE0395,
    0xE52F,
    0x467C,
    {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
static const IID IID_IMMDeviceEnumerator_Local = {
    0xA95664D2,
    0x9614,
    0x4F35,
    {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
static const IID IID_IAudioClient_Local = {
    0x1CB9AD4C,
    0xDBFA,
    0x4c32,
    {0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2}};
static const IID IID_IAudioRenderClient_Local = {
    0xF294ACFC,
    0x3146,
    0x4483,
    {0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2}};

static int pf_init_audio_backend(int sample_rate) {
  HRESULT hr;
  WAVEFORMATEX* pwfx_mix = NULL;
  WAVEFORMATEXTENSIBLE wfx_target;
  WAVEFORMATEX* pwfx_final = NULL;
  UINT32 buffer_frame_count;

  hr = CoInitialize(NULL);
  if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) { return 0; }

  g_platform.h_audio_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!g_platform.h_audio_event) { return 0; }

  hr = CoCreateInstance(
      &CLSID_MMDeviceEnumerator_Local,
      NULL,
      CLSCTX_ALL,
      &IID_IMMDeviceEnumerator_Local,
      (void**)&g_platform.p_enumerator);
  if (FAILED(hr)) { return 0; }

  hr = g_platform.p_enumerator->lpVtbl->GetDefaultAudioEndpoint(
      g_platform.p_enumerator, eRender, eConsole, &g_platform.p_device);
  if (FAILED(hr)) { return 0; }

  hr = g_platform.p_device->lpVtbl->Activate(
      g_platform.p_device,
      &IID_IAudioClient_Local,
      CLSCTX_ALL,
      NULL,
      (void**)&g_platform.p_audio_client);
  if (FAILED(hr)) { return 0; }

  hr = g_platform.p_audio_client->lpVtbl->GetMixFormat(
      g_platform.p_audio_client, &pwfx_mix);
  if (FAILED(hr)) { return 0; }

  ZeroMemory(&wfx_target, sizeof(wfx_target));
  wfx_target.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
  wfx_target.Format.nChannels = pwfx_mix->nChannels;
  wfx_target.Format.nSamplesPerSec = sample_rate;
  wfx_target.Format.wBitsPerSample = 32;
  wfx_target.Format.nBlockAlign =
      wfx_target.Format.nChannels * (wfx_target.Format.wBitsPerSample / 8);
  wfx_target.Format.nAvgBytesPerSec =
      wfx_target.Format.nSamplesPerSec * wfx_target.Format.nBlockAlign;
  wfx_target.Format.cbSize = 22;
  wfx_target.Samples.wValidBitsPerSample = 32;

  if (pwfx_mix->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
    wfx_target.dwChannelMask =
        ((WAVEFORMATEXTENSIBLE*)pwfx_mix)->dwChannelMask;
  } else {
    wfx_target.dwChannelMask = (pwfx_mix->nChannels == 2) ? 0x3 : 0x4;
  }

  wfx_target.SubFormat.Data1 = 0x00000003;
  wfx_target.SubFormat.Data2 = 0x0000;
  wfx_target.SubFormat.Data3 = 0x0010;
  wfx_target.SubFormat.Data4[0] = 0x80;
  wfx_target.SubFormat.Data4[1] = 0x00;
  wfx_target.SubFormat.Data4[2] = 0x00;
  wfx_target.SubFormat.Data4[3] = 0xaa;
  wfx_target.SubFormat.Data4[4] = 0x00;
  wfx_target.SubFormat.Data4[5] = 0x38;
  wfx_target.SubFormat.Data4[6] = 0x9b;
  wfx_target.SubFormat.Data4[7] = 0x71;

  hr = g_platform.p_audio_client->lpVtbl->Initialize(
      g_platform.p_audio_client,
      AUDCLNT_SHAREMODE_SHARED,
      AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
          AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
      600000,
      0,
      (WAVEFORMATEX*)&wfx_target,
      NULL);

  if (SUCCEEDED(hr)) {
    pwfx_final = (WAVEFORMATEX*)&wfx_target;
    g_platform.hw_is_float = 1;
  } else {
    hr = g_platform.p_audio_client->lpVtbl->Initialize(
        g_platform.p_audio_client,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
            AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        600000,
        0,
        pwfx_mix,
        NULL);

    if (FAILED(hr)) {
      CoTaskMemFree(pwfx_mix);
      return 0;
    }
    pwfx_final = pwfx_mix;
    g_platform.hw_is_float =
        (pwfx_mix->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
        (pwfx_mix->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
         ((WAVEFORMATEXTENSIBLE*)pwfx_mix)->SubFormat.Data1 == 3);
  }

  g_platform.hw_channels = pwfx_final->nChannels;
  g_platform.hw_sample_rate = pwfx_final->nSamplesPerSec;
  g_platform.hw_bits_per_sample = pwfx_final->wBitsPerSample;

  hr = g_platform.p_audio_client->lpVtbl->GetBufferSize(
      g_platform.p_audio_client, &buffer_frame_count);
  if (SUCCEEDED(hr)) {
    int cap_bytes =
        buffer_frame_count * g_platform.hw_channels * sizeof(float);
    g_platform.conversion_buffer = (float*)malloc(cap_bytes);
    g_platform.conversion_buffer_cap = buffer_frame_count;
  }

  CoTaskMemFree(pwfx_mix);

  hr = g_platform.p_audio_client->lpVtbl->SetEventHandle(
      g_platform.p_audio_client, g_platform.h_audio_event);
  if (FAILED(hr)) { return 0; }

  hr = g_platform.p_audio_client->lpVtbl->GetService(
      g_platform.p_audio_client,
      &IID_IAudioRenderClient_Local,
      (void**)&g_platform.p_render_client);
  if (FAILED(hr)) { return 0; }

  hr = g_platform.p_audio_client->lpVtbl->Start(g_platform.p_audio_client);
  if (FAILED(hr)) { return 0; }

  return 1;
}

static void pf_close_audio_backend(void) {
  if (g_platform.p_audio_client) {
    g_platform.p_audio_client->lpVtbl->Stop(g_platform.p_audio_client);
  }
  if (g_platform.p_render_client) {
    g_platform.p_render_client->lpVtbl->Release(g_platform.p_render_client);
  }
  if (g_platform.p_audio_client) {
    g_platform.p_audio_client->lpVtbl->Release(g_platform.p_audio_client);
  }
  if (g_platform.p_device) {
    g_platform.p_device->lpVtbl->Release(g_platform.p_device);
  }
  if (g_platform.p_enumerator) {
    g_platform.p_enumerator->lpVtbl->Release(g_platform.p_enumerator);
  }
  if (g_platform.h_audio_event) { CloseHandle(g_platform.h_audio_event); }
  if (g_platform.conversion_buffer) { free(g_platform.conversion_buffer); }
  CoUninitialize();
}

static void pf_convert_mix(
    BYTE* dst,
    const float* src,
    int frames,
    int channels,
    int bits,
    int is_float) {
  int i;
  short* s16 = (short*)dst;
  int total = frames * channels;

  if (is_float && bits == 32) {
    memcpy(dst, src, total * sizeof(float));
    return;
  }

  if (!is_float && bits == 16) {
    for (i = 0; i < total; ++i) {
      float s = src[i];
      if (s > 1.0f) { s = 1.0f; }
      if (s < -1.0f) { s = -1.0f; }
      s16[i] = (short)(s * 32767.0f);
    }
    return;
  }

  if (!is_float && bits == 32) {
    int* s32 = (int*)dst;
    for (i = 0; i < total; ++i) {
      float s = src[i];
      if (s > 1.0f) { s = 1.0f; }
      if (s < -1.0f) { s = -1.0f; }
      s32[i] = (int)(s * 2147483647.0f);
    }
    return;
  }
}

static void pf_process_audio_chunk(void) {
  UINT32 padding;
  UINT32 available_frames;
  BYTE* data;
  HRESULT hr;

  hr = g_platform.p_audio_client->lpVtbl->GetCurrentPadding(
      g_platform.p_audio_client, &padding);
  if (FAILED(hr)) { return; }

  {
    UINT32 buffer_size;
    g_platform.p_audio_client->lpVtbl->GetBufferSize(
        g_platform.p_audio_client, &buffer_size);
    available_frames = buffer_size - padding;
  }

  if (available_frames > 0) {
    if (available_frames > (UINT32)g_platform.conversion_buffer_cap) {
      available_frames = (UINT32)g_platform.conversion_buffer_cap;
    }

    hr = g_platform.p_render_client->lpVtbl->GetBuffer(
        g_platform.p_render_client, available_frames, &data);
    if (SUCCEEDED(hr)) {
      eng_audio_mix(
          g_platform.conversion_buffer,
          available_frames,
          g_platform.hw_channels);

      pf_convert_mix(
          data,
          g_platform.conversion_buffer,
          available_frames,
          g_platform.hw_channels,
          g_platform.hw_bits_per_sample,
          g_platform.hw_is_float);

      g_platform.p_render_client->lpVtbl->ReleaseBuffer(
          g_platform.p_render_client, available_frames, 0);
    }
  }
}

static unsigned __stdcall pf_audio_thread_proc(void* data) {
  if (!pf_init_audio_backend(g_engine.audio_sample_rate)) { return 1; }

  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

  while (InterlockedCompareExchange(&g_engine.running, 1, 1)) {
    DWORD waitResult = WaitForSingleObject(g_platform.h_audio_event, 2000);
    if (waitResult == WAIT_OBJECT_0) { pf_process_audio_chunk(); }
  }

  pf_close_audio_backend();
  return 0;
}

static void tb_shutdown_platform(void) {
  InterlockedExchange(&g_engine.running, 0);
  if (g_platform.h_audio_event) { SetEvent(g_platform.h_audio_event); }

  if (g_engine.audio_thread) {
    tb_thread_join(g_engine.audio_thread);
    g_engine.audio_thread = NULL;
  }
  pf_close_console();
}

static int tb_update_platform(void) {
  memcpy(g_engine.prev_keys, g_engine.keys, sizeof(g_engine.keys));
  pf_poll_events();
  return (int)InterlockedCompareExchange(&g_engine.running, 1, 1);
}

#else

#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifndef TB_NO_AUDIO
#include <alsa/asoundlib.h>
#endif

typedef struct {
  struct termios orig_termios;
  int console_initialized;
#ifndef TB_NO_AUDIO
  snd_pcm_t* audio_handle;
#endif
  float* conversion_buffer;
  int conversion_buffer_cap;
  int hw_channels;
  int hw_is_float;
  int hw_bits_per_sample;
} pf_state_t;

static pf_state_t g_platform;

static int pf_init_console(int w, int h, const char* title);
static void pf_close_console(void);
static void pf_poll_events(void);
static void pf_present_buffer(const eng_cell_t* buffer, int w, int h);
static int pf_init_audio_backend(int sample_rate);
static void pf_close_audio_backend(void);
static void pf_process_audio_chunk(void);
static void* pf_audio_thread_proc(void* data);
static void pf_signal_handler(int sig);
static void pf_winch_handler(int sig);

tb_thread_t tb_thread_create(tb_thread_func_t func, void* user_data) {
  pthread_t thread;
  if (pthread_create(&thread, NULL, (void* (*)(void*))func, user_data) != 0) {
    return NULL;
  }
  return (tb_thread_t)thread;
}

void tb_thread_join(tb_thread_t thread) {
  if (thread) { pthread_join((pthread_t)thread, NULL); }
}

tb_mutex_t tb_mutex_init(void) {
  pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  if (mutex) { pthread_mutex_init(mutex, NULL); }
  return (tb_mutex_t)mutex;
}

void tb_mutex_lock(tb_mutex_t mutex) {
  if (mutex) { pthread_mutex_lock((pthread_mutex_t*)mutex); }
}

void tb_mutex_unlock(tb_mutex_t mutex) {
  if (mutex) { pthread_mutex_unlock((pthread_mutex_t*)mutex); }
}

void tb_mutex_destroy(tb_mutex_t mutex) {
  if (mutex) {
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    free(mutex);
  }
}

void tb_sleep(int ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

static void pf_signal_handler(int sig) {
  __sync_lock_test_and_set(&g_engine.running, 0);
}

static void pf_winch_handler(int sig) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
    eng_handle_resize(ws.ws_col, ws.ws_row);
  }
}

static int pf_init_console(int w, int h, const char* title) {
  struct termios raw;
  struct winsize ws;

  if (tcgetattr(STDIN_FILENO, &g_platform.orig_termios) == -1) { return 0; }

  raw = g_platform.orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) { return 0; }

  printf("\x1b[?1049h\x1b[?25l");
  printf("\x1b[?1000h\x1b[?1002h\x1b[?1015h\x1b[?1006h");
  fflush(stdout);

  signal(SIGINT, pf_signal_handler);
  signal(SIGTERM, pf_signal_handler);
  signal(SIGWINCH, pf_winch_handler);

  if (w == 0 || h == 0) {
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
      w = ws.ws_col;
      h = ws.ws_row;
    } else {
      w = 80;
      h = 25;
    }
  }

  g_platform.console_initialized = 1;
  eng_init(w, h);
  return 1;
}

static void pf_close_console(void) {
  if (!g_platform.console_initialized) { return; }

  printf("\x1b[?1006l\x1b[?1015l\x1b[?1002l\x1b[?1000l");
  printf("\x1b[?25h\x1b[?1049l");
  fflush(stdout);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_platform.orig_termios);
  g_platform.console_initialized = 0;
}

static void pf_parse_escape_seq(const char* buf, int len, int* consumed) {
  int i = 0;
  *consumed = 0;

  if (len < 1) { return; }
  if (buf[0] != 0x1B) {
    if (buf[0] >= 32 && buf[0] < 127) {
      eng_handle_input(buf[0], 1);
    } else if (buf[0] == 127) {
      eng_handle_input(TB_KEY_BACKSPACE, 1);
    } else if (buf[0] == 13) {
      eng_handle_input(TB_KEY_ENTER, 1);
    } else if (buf[0] == 9) {
      eng_handle_input(TB_KEY_TAB, 1);
    }
    *consumed = 1;
    return;
  }

  if (len == 1) {
    eng_handle_input(TB_KEY_ESCAPE, 1);
    *consumed = 1;
    return;
  }

  if (buf[1] == '[' || buf[1] == 'O') {
    if (len < 3) { return; }

    if (buf[2] == 'A') {
      eng_handle_input(TB_KEY_UP, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'B') {
      eng_handle_input(TB_KEY_DOWN, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'C') {
      eng_handle_input(TB_KEY_RIGHT, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'D') {
      eng_handle_input(TB_KEY_LEFT, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'H') {
      eng_handle_input(TB_KEY_HOME, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'F') {
      eng_handle_input(TB_KEY_END, 1);
      *consumed = 3;
      return;
    }

    if (buf[2] == 'P') {
      eng_handle_input(TB_KEY_F1, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'Q') {
      eng_handle_input(TB_KEY_F2, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'R') {
      eng_handle_input(TB_KEY_F3, 1);
      *consumed = 3;
      return;
    }
    if (buf[2] == 'S') {
      eng_handle_input(TB_KEY_F4, 1);
      *consumed = 3;
      return;
    }

    if (buf[2] == 'M') {
      *consumed = 3;
      return;
    }

    if (buf[2] == '<') {
      int b, x, y;
      char type;
      int n, end = 0;
      for (n = 3; n < len; n++) {
        if (buf[n] == 'm' || buf[n] == 'M') {
          end = n;
          break;
        }
      }
      if (!end) { return; }

      if (sscanf(buf + 3, "%d;%d;%d%c", &b, &x, &y, &type) == 4) {
        int btn = 0;
        int wheel = 0;
        int is_down = (type == 'M');

        if (b == 0) {
          btn = TB_MOUSE_LEFT;
        } else if (b == 1) {
          btn = TB_MOUSE_MIDDLE;
        } else if (b == 2) {
          btn = TB_MOUSE_RIGHT;
        } else if (b == 64) {
          wheel = 1;
          btn = TB_MOUSE_LEFT;
        } else if (b == 65) {
          wheel = -1;
          btn = TB_MOUSE_LEFT;
        }

        eng_handle_mouse(x - 1, y - 1, btn, wheel, is_down);
      }
      *consumed = end + 1;
      return;
    }

    if (len >= 4 && buf[3] == '~') {
      if (buf[2] == '1') {
        eng_handle_input(TB_KEY_HOME, 1);
      } else if (buf[2] == '2') {
        eng_handle_input(TB_KEY_INSERT, 1);
      } else if (buf[2] == '3') {
        eng_handle_input(TB_KEY_DELETE, 1);
      } else if (buf[2] == '4') {
        eng_handle_input(TB_KEY_END, 1);
      } else if (buf[2] == '5') {
        eng_handle_input(TB_KEY_PGUP, 1);
      } else if (buf[2] == '6') {
        eng_handle_input(TB_KEY_PGDN, 1);
      }
      *consumed = 4;
      return;
    }

    if (len >= 5 && buf[4] == '~') {
      int f = 0;
      if (buf[2] == '1' && buf[3] == '5') {
        f = TB_KEY_F5;
      } else if (buf[2] == '1' && buf[3] == '7') {
        f = TB_KEY_F6;
      } else if (buf[2] == '1' && buf[3] == '8') {
        f = TB_KEY_F7;
      } else if (buf[2] == '1' && buf[3] == '9') {
        f = TB_KEY_F8;
      } else if (buf[2] == '2' && buf[3] == '0') {
        f = TB_KEY_F9;
      } else if (buf[2] == '2' && buf[3] == '1') {
        f = TB_KEY_F10;
      } else if (buf[2] == '2' && buf[3] == '3') {
        f = TB_KEY_F11;
      } else if (buf[2] == '2' && buf[3] == '4') {
        f = TB_KEY_F12;
      }

      if (f) { eng_handle_input(f, 1); }
      *consumed = 5;
      return;
    }
  }

  *consumed = 1;
}

static void pf_poll_events(void) {
  struct pollfd pfd;
  char buf[128];
  int n, processed;

  g_engine.mouse.wheel = 0;

  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;

  while (poll(&pfd, 1, 0) > 0) {
    if (pfd.revents & POLLIN) {
      n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
      if (n > 0) {
        buf[n] = 0;
        int offset = 0;
        while (offset < n) {
          pf_parse_escape_seq(buf + offset, n - offset, &processed);
          if (processed == 0) { break; }
          offset += processed;
        }
      }
    }
  }
}

static void pf_present_buffer(const eng_cell_t* buffer, int w, int h) {
  if (!buffer) { return; }

  int est_size = w * h * 45;

  if (g_engine.render_buffer_cap < est_size) {
    char* temp = (char*)realloc(g_engine.render_buffer, est_size);
    if (!temp) { return; }
    g_engine.render_buffer = temp;
    g_engine.render_buffer_cap = est_size;
  }

  if (!g_engine.render_buffer) { return; }

  char* ptr = g_engine.render_buffer;
  char* end = g_engine.render_buffer + g_engine.render_buffer_cap;
  int last_fg = -1;
  int last_bg = -1;
  int i;
  int count = w * h;
  int written_bytes;

  written_bytes = snprintf(ptr, (size_t)(end - ptr), "\x1b[H");
  if (written_bytes > 0) { ptr += written_bytes; }

  for (i = 0; i < count; ++i) {
    eng_cell_t c = buffer[i];

    if (c.fg != last_fg || c.bg != last_bg) {
      written_bytes = snprintf(
          ptr,
          (size_t)(end - ptr),
          "\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm",
          (c.fg >> 16) & 0xFF,
          (c.fg >> 8) & 0xFF,
          c.fg & 0xFF,
          (c.bg >> 16) & 0xFF,
          (c.bg >> 8) & 0xFF,
          c.bg & 0xFF);
      if (written_bytes > 0) { ptr += written_bytes; }
      last_fg = c.fg;
      last_bg = c.bg;
    }

    if (ptr < end - 1) { *ptr++ = c.ch; }
  }

  if (write(
          STDOUT_FILENO,
          g_engine.render_buffer,
          ptr - g_engine.render_buffer) == -1) {}
}

#ifndef TB_NO_AUDIO
static int pf_init_audio_backend(int sample_rate) {
  int err;
  unsigned int rate = sample_rate;
  snd_pcm_hw_params_t* hw_params;

  if ((err = snd_pcm_open(
           &g_platform.audio_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) <
      0) {
    return 0;
  }

  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) { return 0; }
  if ((err = snd_pcm_hw_params_any(g_platform.audio_handle, hw_params)) < 0) {
    return 0;
  }
  if ((err = snd_pcm_hw_params_set_access(
           g_platform.audio_handle,
           hw_params,
           SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    return 0;
  }

  g_platform.hw_is_float = 1;
  if ((err = snd_pcm_hw_params_set_format(
           g_platform.audio_handle, hw_params, SND_PCM_FORMAT_FLOAT_LE)) < 0) {
    g_platform.hw_is_float = 0;
    if ((err = snd_pcm_hw_params_set_format(
             g_platform.audio_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
      return 0;
    }
  }

  if ((err = snd_pcm_hw_params_set_rate_near(
           g_platform.audio_handle, hw_params, &rate, 0)) < 0) {
    return 0;
  }
  if ((err = snd_pcm_hw_params_set_channels(
           g_platform.audio_handle, hw_params, 2)) < 0) {
    return 0;
  }

  if ((err = snd_pcm_hw_params(g_platform.audio_handle, hw_params)) < 0) {
    return 0;
  }
  snd_pcm_hw_params_free(hw_params);

  if ((err = snd_pcm_prepare(g_platform.audio_handle)) < 0) { return 0; }

  g_platform.hw_channels = 2;
  g_platform.hw_bits_per_sample = g_platform.hw_is_float ? 32 : 16;
  g_platform.conversion_buffer_cap = 4096;
  g_platform.conversion_buffer =
      (float*)malloc(g_platform.conversion_buffer_cap * 2 * sizeof(float));

  return 1;
}

static void pf_close_audio_backend(void) {
  if (g_platform.audio_handle) {
    snd_pcm_close(g_platform.audio_handle);
    g_platform.audio_handle = NULL;
  }
  if (g_platform.conversion_buffer) { free(g_platform.conversion_buffer); }
}

static void pf_process_audio_chunk(void) {
  snd_pcm_sframes_t frames_to_deliver;
  int avail;

  if (!g_platform.audio_handle) { return; }

  avail = snd_pcm_avail_update(g_platform.audio_handle);
  if (avail < 0) {
    snd_pcm_recover(g_platform.audio_handle, avail, 1);
    return;
  }

  if (avail > g_platform.conversion_buffer_cap) {
    avail = g_platform.conversion_buffer_cap;
  }
  if (avail == 0) { return; }

  eng_audio_mix(g_platform.conversion_buffer, avail, 2);

  if (!g_platform.hw_is_float) {
    short* s16 = (short*)g_platform.conversion_buffer;
    int i;
    int total = avail * 2;
    for (i = 0; i < total; ++i) {
      float s = g_platform.conversion_buffer[i];
      if (s > 1.0f) { s = 1.0f; }
      if (s < -1.0f) { s = -1.0f; }
      s16[i] = (short)(s * 32767.0f);
    }
  }

  frames_to_deliver = snd_pcm_writei(
      g_platform.audio_handle, g_platform.conversion_buffer, avail);
  if (frames_to_deliver < 0) {
    snd_pcm_recover(g_platform.audio_handle, frames_to_deliver, 1);
  }
}
#else
static int pf_init_audio_backend(int sample_rate) {
  return 0;
}
static void pf_close_audio_backend(void) {}
static void pf_process_audio_chunk(void) {}
#endif

static void* pf_audio_thread_proc(void* data) {
  if (!pf_init_audio_backend(g_engine.audio_sample_rate)) { return NULL; }

  while (__sync_add_and_fetch(&g_engine.running, 0)) {
    pf_process_audio_chunk();
    struct timespec ts = {0, 5000000};
    nanosleep(&ts, NULL);
  }

  pf_close_audio_backend();
  return NULL;
}

static void tb_shutdown_platform(void) {
  __sync_lock_test_and_set(&g_engine.running, 0);
  if (g_engine.audio_thread) {
    tb_thread_join(g_engine.audio_thread);
    g_engine.audio_thread = NULL;
  }
  pf_close_console();
}

static int tb_update_platform(void) {
  memcpy(g_engine.prev_keys, g_engine.keys, sizeof(g_engine.keys));
  pf_poll_events();
  return (int)__sync_add_and_fetch(&g_engine.running, 0);
}

#endif

static void eng_init(int w, int h) {
  g_engine.width = w;
  g_engine.height = h;
  g_engine.back_buffer = (eng_cell_t*)malloc(sizeof(eng_cell_t) * w * h);
  g_engine.render_buffer = NULL;
  g_engine.render_buffer_cap = 0;
  memset(g_engine.keys, 0, sizeof(g_engine.keys));
  memset(g_engine.prev_keys, 0, sizeof(g_engine.prev_keys));
  memset(&g_engine.mouse, 0, sizeof(tb_mouse_state_t));
#ifdef _WIN32
  InterlockedExchange(&g_engine.running, 1);
#else
  __sync_lock_test_and_set(&g_engine.running, 1);
#endif

  g_engine.audio_write_idx = 0;
  g_engine.audio_read_idx = 0;

  g_engine.last_sample = 0.0f;
  g_engine.gain_ramp = 0.0f;
  g_engine.is_starving = 1;

  eng_resize_buffer(w, h);
}

static void eng_resize_buffer(int w, int h) {
  int i;
  int safe_size = w * h;
  if (!g_engine.back_buffer) { return; }

  for (i = 0; i < safe_size; ++i) {
    g_engine.back_buffer[i].ch = ' ';
    g_engine.back_buffer[i].fg = TB_RGB(200, 200, 200);
    g_engine.back_buffer[i].bg = TB_RGB(0, 0, 0);
  }
}

static void eng_handle_resize(int w, int h) {
  eng_cell_t* new_buffer;
  if (w <= 0 || h <= 0) { return; }

  new_buffer =
      (eng_cell_t*)realloc(g_engine.back_buffer, sizeof(eng_cell_t) * w * h);
  if (new_buffer) {
    g_engine.back_buffer = new_buffer;
    g_engine.width = w;
    g_engine.height = h;
    eng_resize_buffer(w, h);
  }
}

static void eng_handle_input(int key, int down) {
  if (key >= 0 && key < MAX_KEYS) { g_engine.keys[key] = down; }
}

static void eng_handle_mouse(int x, int y, int btn, int wheel, int down) {
  g_engine.mouse.x = x;
  g_engine.mouse.y = y;
  g_engine.mouse.wheel = wheel;
  if (btn == TB_MOUSE_LEFT) { g_engine.mouse.left_down = down; }
  if (btn == TB_MOUSE_RIGHT) { g_engine.mouse.right_down = down; }
  if (btn == TB_MOUSE_MIDDLE) { g_engine.mouse.middle_down = down; }
}

static void eng_audio_mix(float* output, int frames, int channels) {
  int f, c;
  float sample;

  unsigned int read_ptr = g_engine.audio_read_idx;
  unsigned int write_ptr = g_engine.audio_write_idx;

  for (f = 0; f < frames; ++f) {
    int available = (int)(write_ptr - read_ptr);

    if (available > 0) {
      sample = g_engine.audio_buffer[read_ptr & AUDIO_BUFFER_MASK];
      read_ptr++;

      if (g_engine.is_starving) {
        sample *= g_engine.gain_ramp;
        g_engine.gain_ramp += 0.01f;
        if (g_engine.gain_ramp >= 1.0f) {
          g_engine.gain_ramp = 1.0f;
          g_engine.is_starving = 0;
        }
      }
      g_engine.last_sample = sample;
    } else {
      g_engine.is_starving = 1;
      g_engine.gain_ramp = 0.0f;
      sample = g_engine.last_sample * 0.95f;
      if (fabsf(sample) < 0.0001f) { sample = 0.0f; }
      g_engine.last_sample = sample;
    }

    if (sample > 1.0f) { sample = 1.0f; }
    if (sample < -1.0f) { sample = -1.0f; }

    for (c = 0; c < channels; ++c) { *output++ = sample; }
  }

#ifdef _WIN32
  MemoryBarrier();
#else
  __sync_synchronize();
#endif
  g_engine.audio_read_idx = read_ptr;
}

void tb_clear(unsigned int fg, unsigned int bg, char c) {
  int i;
  int size = g_engine.width * g_engine.height;
  for (i = 0; i < size; ++i) {
    g_engine.back_buffer[i].ch = c;
    g_engine.back_buffer[i].fg = fg;
    g_engine.back_buffer[i].bg = bg;
  }
}

void tb_put(int x, int y, char c, unsigned int fg, unsigned int bg) {
  if (x >= 0 && x < g_engine.width && y >= 0 && y < g_engine.height) {
    int idx = y * g_engine.width + x;
    g_engine.back_buffer[idx].ch = c;
    g_engine.back_buffer[idx].fg = fg;
    g_engine.back_buffer[idx].bg = bg;
  }
}

void tb_print(
    int x, int y, const char* str, unsigned int fg, unsigned int bg) {
  int cx = x;
  while (*str) { tb_put(cx++, y, *str++, fg, bg); }
}

int tb_width(void) {
  return g_engine.width;
}
int tb_height(void) {
  return g_engine.height;
}

int tb_key_down(int key) {
  if (key >= 0 && key < MAX_KEYS) { return g_engine.keys[key]; }
  return 0;
}

int tb_key_pressed(int key) {
  if (key >= 0 && key < MAX_KEYS) {
    return g_engine.keys[key] && !g_engine.prev_keys[key];
  }
  return 0;
}

tb_mouse_state_t tb_mouse_state(void) {
  return g_engine.mouse;
}

int tb_init(int width, int height, const char* title) {
  if (!pf_init_console(width, height, title)) { return 0; }
  return 1;
}

void tb_shutdown(void) {
#ifdef _WIN32
  tb_shutdown_platform();
#else
  tb_shutdown_platform();
#endif
  if (g_engine.back_buffer) {
    free(g_engine.back_buffer);
    g_engine.back_buffer = NULL;
  }
  if (g_engine.render_buffer) {
    free(g_engine.render_buffer);
    g_engine.render_buffer = NULL;
  }
}

int tb_update(void) {
#ifdef _WIN32
  return tb_update_platform();
#else
  return tb_update_platform();
#endif
}

void tb_present(void) {
  pf_present_buffer(g_engine.back_buffer, g_engine.width, g_engine.height);
}

void tb_audio_init(int sample_rate) {
  g_engine.audio_sample_rate = sample_rate;
  g_engine.audio_thread = tb_thread_create(pf_audio_thread_proc, NULL);
}

void tb_audio_push(float* samples, int count) {
  int i;
  unsigned int read_ptr = g_engine.audio_read_idx;
  unsigned int write_ptr = g_engine.audio_write_idx;

  for (i = 0; i < count; ++i) {
    if ((int)(write_ptr - read_ptr) < MAX_AUDIO_BUFFER) {
      g_engine.audio_buffer[write_ptr & AUDIO_BUFFER_MASK] = samples[i];
      write_ptr++;
    }
  }

#ifdef _WIN32
  MemoryBarrier();
#else
  __sync_synchronize();
#endif
  g_engine.audio_write_idx = write_ptr;
}

int tb_audio_free_space(void) {
  unsigned int read_ptr = g_engine.audio_read_idx;
  unsigned int write_ptr = g_engine.audio_write_idx;
  int used = (int)(write_ptr - read_ptr);
  return MAX_AUDIO_BUFFER - used;
}

#endif
