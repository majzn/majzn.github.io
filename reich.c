#define REICH_IMPLEMENTATION
#include "reich.h"
#include <emmintrin.h>
#include <windows.h>

#define TRUE                  1
#define FALSE                 0
#define REICH_MAX_GRID_WIDTH  1024
#define REICH_MAX_GRID_HEIGHT 1024
#define REICH_MAX_GRID_SIZE   (REICH_MAX_GRID_WIDTH * REICH_MAX_GRID_HEIGHT)

#define CAMERA_MIN_ZOOM       0.1
#define CAMERA_MAX_ZOOM       200.0
#define CAMERA_MIN_PITCH      0.1
#define CAMERA_MAX_PITCH      1.57079632679 
#define TERRAIN_MIN_Z         -50.0
#define TERRAIN_MAX_Z         350.0
#define TIME_SPEED            0.001
#define SHADOW_STEPS          18
#define SHADOW_START_T        0.5
#define WATER_LVL             40.0f
#define TILE_TEX_WIDTH        64
#define TILE_TEX_HEIGHT       64
#define MAX_FADE_OCTAVES      8

#define NUM_THREADS           8
#define REICH_ABS(x)          ((x) < 0 ? -(x) : (x))

uint32 REICH_GRID_DATA[REICH_MAX_GRID_SIZE];
real32 REICH_HEIGHT_DATA[REICH_MAX_GRID_SIZE];      /* Pre-clamped */
real32 REICH_RAW_HEIGHT_DATA[REICH_MAX_GRID_SIZE];  /* Original terrain */

uint32 TILE_TEXTURE[TILE_TEX_WIDTH * TILE_TEX_HEIGHT];

static reichCamera mainCamera = {REICH_MAX_GRID_WIDTH/2.0f, REICH_MAX_GRID_HEIGHT/2.0f, 10.0, 2.0, REICH_PI/4, 0.5};
static reichContext mainContext;

static int32 selectionMinX = -1;
static int32 selectionMinY = -1;
static int32 selectionMaxX = -1;
static int32 selectionMaxY = -1;

static int32 hoverGridX = -1;
static int32 hoverGridY = -1;
static int32 nextHoverGridX = -1;
static int32 nextHoverGridY = -1;

static int32 isSelectingBox = 0;
static int32 isDraggingCamera = 0;

static real64 selectionStartWorldX = 0;
static real64 selectionStartWorldY = 0;
static real64 selectionEndWorldX = 0;
static real64 selectionEndWorldY = 0;

static real64 globalTime = 0.25;

typedef struct {
  float x00, y00, x10, y10, x11, y11, x01, y01;
  float cullTri1, cullTri2;
  uint32 quadColor;
  int32 visible;
  int32 isTextured;
} QuadDrawCmd;

typedef struct {
  int32 startIdx;
  int32 endIdx;
  int32 numX;
  int32 gStart_Y;
  int32 gStep_Y;
  int32 gStart_X;
  int32 gStep_X;
  int32 levelOfDetail;
  int32 selBoxMinX;
  int32 selBoxMaxX;
  int32 selBoxMinY;
  int32 selBoxMaxY;
  float hw, hh, cx, cy, cz, zm, cY, sY, cP, sP;
  real32 lightDirX, lightDirY, lightDirZ;
  uint32 sunColor, ambientColor, skyColor;
  uint32 *grid;
  reichRect clipRect;
  real64 mouseX, mouseY;
  int32 threadHoverX, threadHoverY;
  QuadDrawCmd* cmds;
} ThreadData;

static real32 get_terrain_height(real64 worldX, real64 worldY) {
  int32 gridX = (int32)reich_floor((float)worldX);
  int32 gridY = (int32)reich_floor((float)worldY);
  if (gridX >= 0 && gridX < REICH_MAX_GRID_WIDTH - 1 &&
      gridY >= 0 && gridY < REICH_MAX_GRID_HEIGHT - 1) {
    real32 height00 = REICH_HEIGHT_DATA[gridY * REICH_MAX_GRID_WIDTH + gridX];
    real32 height10 = REICH_HEIGHT_DATA[gridY * REICH_MAX_GRID_WIDTH + (gridX + 1)];
    real32 height11 = REICH_HEIGHT_DATA[(gridY + 1) * REICH_MAX_GRID_WIDTH + (gridX + 1)];
    real32 height01 = REICH_HEIGHT_DATA[(gridY + 1) * REICH_MAX_GRID_WIDTH + gridX];

    real64 fractX = worldX - (real64)gridX;
    real64 fractY = worldY - (real64)gridY;

    if (fractX >= fractY) {
      return (real32)(height00 + (height10 - height00) * (real32)fractX + (height11 - height10) * (real32)fractY);
    } else {
      return (real32)(height00 + (height11 - height01) * (real32)fractX + (height01 - height00) * (real32)fractY);
    }
  }
  if (gridX >= 0 && gridX < REICH_MAX_GRID_WIDTH &&
      gridY >= 0 && gridY < REICH_MAX_GRID_HEIGHT) {
    return REICH_HEIGHT_DATA[gridY * REICH_MAX_GRID_WIDTH + gridX];
  }
  return WATER_LVL;
}

/* Highly optimized version for shadow raycasting bypassing bounds checking entirely */
static real32 get_terrain_height_fast(real32 worldX, real32 worldY) {
  int32 gridX = (int32)worldX;
  int32 gridY = (int32)worldY;
  real32 fractX = worldX - (real32)gridX;
  real32 fractY = worldY - (real32)gridY;

  int32 idx = gridY * REICH_MAX_GRID_WIDTH + gridX;
  real32 h00 = REICH_HEIGHT_DATA[idx];
  real32 h10 = REICH_HEIGHT_DATA[idx + 1];
  real32 h01 = REICH_HEIGHT_DATA[idx + REICH_MAX_GRID_WIDTH];
  real32 h11 = REICH_HEIGHT_DATA[idx + REICH_MAX_GRID_WIDTH + 1];

  if (fractX >= fractY) {
    return h00 + (h10 - h00) * fractX + (h11 - h10) * fractY;
  } else {
    return h00 + (h11 - h01) * fractX + (h01 - h00) * fractY;
  }
}

void apply_pan_delta(reichCamera* cam, real64 dSx, real64 dSy) {
  real64 cY = reich_cos((float)cam->angle);
  real64 sY = reich_sin((float)cam->angle);
  real64 sP = reich_sin((float)cam->pitch);
  real64 rX = dSx / cam->zoom;
  real64 rY = dSy / (cam->zoom * sP);
  cam->x -= rX * cY + rY * sY;
  cam->y -= -rX * sY + rY * cY;
}

static void anchor_camera_to_terrain(reichCamera* cam) {
  real64 sinYaw = reich_sin((float)cam->angle);
  real64 cosYaw = reich_cos((float)cam->angle);
  real64 cosPitch = reich_cos((float)cam->pitch);
  real64 sinPitch = reich_sin((float)cam->pitch);
  real64 slopeZ = sinPitch / (cosPitch < 0.001 ? 0.001 : cosPitch);
  real64 tLow = -1000.0, tHigh = 1000.0, tMid;
  int32 i;
  for (i = 0; i < 40; i++) {
    tMid = (tLow + tHigh) * 0.5;
    if (cam->z + tMid * slopeZ <
        (real64)get_terrain_height(cam->x + tMid * sinYaw, cam->y + tMid * cosYaw)) {
      tLow = tMid;
    } else {
      tHigh = tMid;
    }
  }

  tMid = (tLow + tHigh) * 0.5;
  cam->x += tMid * sinYaw;
  cam->y += tMid * cosYaw;
  cam->z += tMid * slopeZ;
}

/* Rewritten for single-precision fast-path */
static real32 cast_shadow_fast(
    real32 worldX, real32 worldY, real32 worldZ,
    real32 lightDirX, real32 lightDirY, real32 lightDirZ,
    int32 levelOfDetail) {
  
  real32 currentT, deltaT, sampleX, sampleY, sampleZ;
  real32 shadowFactor = 1.0f, terrainHeight, currentFactor;
  int32 i;
  currentT = (real32)levelOfDetail * SHADOW_START_T;
  deltaT = currentT;

  for (i = 0; i < SHADOW_STEPS; i++) {
    sampleX = worldX + lightDirX * currentT;
    sampleY = worldY + lightDirY * currentT;
    sampleZ = worldZ + lightDirZ * currentT;

    if (sampleZ > TERRAIN_MAX_Z || sampleX < 0.0f || sampleX >= (real32)(REICH_MAX_GRID_WIDTH - 1) || sampleY < 0.0f || sampleY >= (real32)(REICH_MAX_GRID_HEIGHT - 1)) {
      break;
    }

    terrainHeight = get_terrain_height_fast(sampleX, sampleY);
    if (sampleZ < terrainHeight) {
        return 0.1f;
    }

    currentFactor = 8.0f * (sampleZ - terrainHeight) / currentT;
    if (currentFactor < shadowFactor) {
      shadowFactor = currentFactor;
    }

    currentT += deltaT;
    deltaT *= 1.2f;
  }
  return shadowFactor < 0.1f ? 0.1f : shadowFactor;
}

void init_textures() {
  int32 texX, texY;
  real64 noiseVal;
  uint32 colorVal;
  for (texY = 0; texY < TILE_TEX_HEIGHT; texY++) {
    for (texX = 0; texX < TILE_TEX_WIDTH; texX++) {
      noiseVal = reich_noise_fbm2((real64)texX * 0.45, (real64)texY * 0.35, 4, 0.5f, 1.0f);
      colorVal = (uint32)(140.0 + noiseVal * 115.0);
      if (colorVal > 255) { colorVal = 255; }
      TILE_TEXTURE[texY * TILE_TEX_WIDTH + texX] = 0xFF000000 | (colorVal << 16) | (colorVal << 8) | colorVal;
    }
  }
}

int32 grid_init(uint32* grid, uint32 width, uint32 height) {
  uint32 gridX, gridY;
  real64 noiseValue;
  real32 rawHeight;
  init_textures();
  for (gridY = 0; gridY < height; gridY++) {
    for (gridX = 0; gridX < width; gridX++) {
      noiseValue = reich_noise_fbm2((real64)gridX * 0.006, (real64)gridY * 0.006, MAX_FADE_OCTAVES, 1.9f, 0.5f)*0.5f + 0.5f;
      noiseValue *= (real64)reich_sqrt((double)noiseValue);
      
      /* Store un-clamped height for fast water depth calculations during drawing */
      rawHeight = (real32)(noiseValue * 255.0) - 40.0f;
      REICH_RAW_HEIGHT_DATA[gridX + gridY * width] = rawHeight;
      
      /* Store pre-clamped heights so draw_world and cast_shadow do zero branching */
      REICH_HEIGHT_DATA[gridX + gridY * width] = rawHeight < WATER_LVL ? WATER_LVL : rawHeight;

      if (rawHeight < 12.0f) {
        grid[gridX + gridY * width] = 0xFFEEDD99;
      } else if (rawHeight < 25.0f) {
        grid[gridX + gridY * width] = 0xFF66AA44;
      } else if (rawHeight < 45.0f) {
        grid[gridX + gridY * width] = 0xFF448833;
      } else if (rawHeight < 70.0f) {
        grid[gridX + gridY * width] = 0xFF666666;
      } else {
        grid[gridX + gridY * width] = 0xFFF0F0F0;
      }
    }
  }
  return TRUE;
}

DWORD WINAPI process_quads_thread(LPVOID lpParam) {
  ThreadData* td = (ThreadData*)lpParam;
  int32 idx;

  td->threadHoverX = -1;
  td->threadHoverY = -1;

  for (idx = td->startIdx; idx < td->endIdx; idx++) {
    int32 iy = idx / td->numX;
    int32 ix = idx % td->numX;
    int32 gridY = td->gStart_Y + iy * td->gStep_Y;
    int32 gridX = td->gStart_X + ix * td->gStep_X;
    int32 nextGridX = gridX + td->levelOfDetail;
    int32 nextGridY = gridY + td->levelOfDetail;
    
    real64 rotCenterX, rotCenterY, projectCenterX, screenRadius;
    real64 projectCenterY, minScreenY, maxScreenY;
    real32 height00, height10, height11, height01;
    __m128 v_wX, v_wY, v_wZ, v_dX, v_dY, v_dZ;
    __m128 v_cY, v_sY, v_cP, v_sP, v_rX, v_rY, v_zoom, v_hw, v_hh, v_oX, v_oY;
    float oX[4], oY[4];
    float screenX00, screenY00, screenX10, screenY10, screenX11, screenY11, screenX01, screenY01;
    float mX1, mX2, boundMinX, pX1, pX2, boundMaxX, mY1, mY2, boundMinY, pY1, pY2, boundMaxY;
    float cullTri1, cullTri2;
    uint32 tileColor;
    int32 isSelectedQuad = 0;
    float normalXUnnormalized, normalYUnnormalized, normalZUnnormalized, normalLength, normalX, normalY, normalZ;
    float averageHeight;
    int32 isWater;
    float shadowFactor, normalDotLight;
    uint32 finalColor, quadColor;
    real32 rawHeight00, rawHeight10, rawHeight11, rawHeight01, rawAverageHeight, waterDepth;
    QuadDrawCmd* cmd = &td->cmds[idx];

    if (nextGridX >= REICH_MAX_GRID_WIDTH) { nextGridX = REICH_MAX_GRID_WIDTH - 1; }
    if (nextGridY >= REICH_MAX_GRID_HEIGHT) { nextGridY = REICH_MAX_GRID_HEIGHT - 1; }

    /* Outer Box Frustum Cull Check */
    rotCenterX = ((real64)gridX + (real64)td->levelOfDetail * 0.5 - (real64)td->cx) * (real64)td->cY - ((real64)gridY + (real64)td->levelOfDetail * 0.5 - (real64)td->cy) * (real64)td->sY;
    rotCenterY = ((real64)gridX + (real64)td->levelOfDetail * 0.5 - (real64)td->cx) * (real64)td->sY + ((real64)gridY + (real64)td->levelOfDetail * 0.5 - (real64)td->cy) * (real64)td->cY;
    projectCenterX = (real64)td->hw + rotCenterX * (real64)td->zm;
    screenRadius = (real64)td->levelOfDetail * 1.5 * (real64)td->zm;

    if (projectCenterX + screenRadius < td->clipRect.x1 || projectCenterX - screenRadius > td->clipRect.x2) {
      cmd->visible = 0;
      continue;
    }

    projectCenterY = (real64)td->hh + (rotCenterY * (real64)td->sP - (0.0 - (real64)td->cz) * (real64)td->cP) * (real64)td->zm;
    minScreenY = projectCenterY - screenRadius - (TERRAIN_MAX_Z - (real64)td->cz) * (real64)td->zm * (real64)td->cP;
    maxScreenY = projectCenterY + screenRadius - (TERRAIN_MIN_Z - (real64)td->cz) * (real64)td->zm * (real64)td->cP;

    if (maxScreenY < td->clipRect.y1 || minScreenY > td->clipRect.y2) {
      cmd->visible = 0;
      continue;
    }

    /* Height array is pre-clamped, fetch instantly */
    height00 = REICH_HEIGHT_DATA[gridY * REICH_MAX_GRID_WIDTH + gridX];
    height10 = REICH_HEIGHT_DATA[gridY * REICH_MAX_GRID_WIDTH + nextGridX];
    height11 = REICH_HEIGHT_DATA[nextGridY * REICH_MAX_GRID_WIDTH + nextGridX];
    height01 = REICH_HEIGHT_DATA[nextGridY * REICH_MAX_GRID_WIDTH + gridX];

    /* SSE2 SIMD transform projection replacing 4 function calls */
    v_wX = _mm_set_ps((float)gridX, (float)nextGridX, (float)nextGridX, (float)gridX);
    v_wY = _mm_set_ps((float)nextGridY, (float)nextGridY, (float)gridY, (float)gridY);
    v_wZ = _mm_set_ps(height01, height11, height10, height00);

    v_dX = _mm_sub_ps(v_wX, _mm_set1_ps(td->cx));
    v_dY = _mm_sub_ps(v_wY, _mm_set1_ps(td->cy));
    v_dZ = _mm_sub_ps(v_wZ, _mm_set1_ps(td->cz));

    v_cY = _mm_set1_ps(td->cY); v_sY = _mm_set1_ps(td->sY);
    v_cP = _mm_set1_ps(td->cP); v_sP = _mm_set1_ps(td->sP);

    v_rX = _mm_sub_ps(_mm_mul_ps(v_dX, v_cY), _mm_mul_ps(v_dY, v_sY));
    v_rY = _mm_add_ps(_mm_mul_ps(v_dX, v_sY), _mm_mul_ps(v_dY, v_cY));

    v_zoom = _mm_set1_ps(td->zm); v_hw = _mm_set1_ps(td->hw); v_hh = _mm_set1_ps(td->hh);

    v_oX = _mm_add_ps(v_hw, _mm_mul_ps(v_rX, v_zoom));
    v_oY = _mm_add_ps(v_hh, _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(v_rY, v_sP), _mm_mul_ps(v_dZ, v_cP)), v_zoom));

    _mm_storeu_ps(oX, v_oX);
    _mm_storeu_ps(oY, v_oY);

    screenX00 = oX[0]; screenY00 = oY[0];
    screenX10 = oX[1]; screenY10 = oY[1];
    screenX11 = oX[2]; screenY11 = oY[2];
    screenX01 = oX[3]; screenY01 = oY[3];

    /* Safe Min/Max checks (Branchless style mapping to minss/maxss natively) */
    mX1 = screenX00 < screenX10 ? screenX00 : screenX10;
    mX2 = screenX11 < screenX01 ? screenX11 : screenX01;
    boundMinX = mX1 < mX2 ? mX1 : mX2;

    pX1 = screenX00 > screenX10 ? screenX00 : screenX10;
    pX2 = screenX11 > screenX01 ? screenX11 : screenX01;
    boundMaxX = pX1 > pX2 ? pX1 : pX2;

    mY1 = screenY00 < screenY10 ? screenY00 : screenY10;
    mY2 = screenY11 < screenY01 ? screenY11 : screenY01;
    boundMinY = mY1 < mY2 ? mY1 : mY2;

    pY1 = screenY00 > screenY10 ? screenY00 : screenY10;
    pY2 = screenY11 > screenY01 ? screenY11 : screenY01;
    boundMaxY = pY1 > pY2 ? pY1 : pY2;

    if (boundMaxX < td->clipRect.x1 || boundMinX > td->clipRect.x2 || boundMaxY < td->clipRect.y1 || boundMinY > td->clipRect.y2) {
      cmd->visible = 0;
      continue;
    }

    cullTri1 = (screenX10 - screenX00) * (screenY11 - screenY00) - (screenY10 - screenY00) * (screenX11 - screenX00);
    cullTri2 = (screenX11 - screenX00) * (screenY01 - screenY00) - (screenY11 - screenY00) * (screenX01 - screenX00);

    if (cullTri1 > 0.0f && reich_point_in_tri(td->mouseX, td->mouseY, screenX00, screenY00, screenX10, screenY10, screenX11, screenY11)) {
      td->threadHoverX = gridX;
      td->threadHoverY = gridY;
    }
    if (cullTri2 > 0.0f && reich_point_in_tri(td->mouseX, td->mouseY, screenX00, screenY00, screenX11, screenY11, screenX01, screenY01)) {
      td->threadHoverX = gridX;
      td->threadHoverY = gridY;
    }

    tileColor = td->grid[gridY * REICH_MAX_GRID_WIDTH + gridX];

    if (td->selBoxMinX != -1 && td->selBoxMaxX != -1) { /* isSelectingBox */
      if (!(nextGridX < td->selBoxMinX || gridX > td->selBoxMaxX || nextGridY < td->selBoxMinY || gridY > td->selBoxMaxY)) {
        isSelectedQuad = 1;
      }
    } else if (selectionMinX != -1) {
      if (!(nextGridX < selectionMinX || gridX > selectionMaxX || nextGridY < selectionMinY || gridY > selectionMaxY)) {
        isSelectedQuad = 2;
      }
    }

    normalXUnnormalized = (height00 + height01) - (height10 + height11);
    normalYUnnormalized = (height00 + height10) - (height11 + height01);
    normalZUnnormalized = 2.0f * (float)td->levelOfDetail;
    normalLength = (float)reich_sqrt((double)(normalXUnnormalized * normalXUnnormalized + normalYUnnormalized * normalYUnnormalized + normalZUnnormalized * normalZUnnormalized));
    normalX = normalXUnnormalized / normalLength;
    normalY = normalYUnnormalized / normalLength;
    normalZ = normalZUnnormalized / normalLength;

    averageHeight = (height00 + height10 + height11 + height01) * 0.25f;
    isWater = (averageHeight <= WATER_LVL + 0.1f);

    shadowFactor = cast_shadow_fast(
        (float)gridX + td->levelOfDetail * 0.5f,
        (float)gridY + td->levelOfDetail * 0.5f,
        averageHeight + 0.1f,
        td->lightDirX, td->lightDirY, td->lightDirZ,
        td->levelOfDetail);

    normalDotLight = normalX * td->lightDirX + normalY * td->lightDirY + normalZ * td->lightDirZ;
    if (normalDotLight < 0.0f) normalDotLight = 0.0f;

    finalColor = reich_apply_lighting(tileColor, normalDotLight * shadowFactor, td->sunColor, td->ambientColor);

    if (isWater) {
      /* Read precomputed unclamped terrain bounds */
      rawHeight00 = REICH_RAW_HEIGHT_DATA[gridY * REICH_MAX_GRID_WIDTH + gridX];
      rawHeight10 = REICH_RAW_HEIGHT_DATA[gridY * REICH_MAX_GRID_WIDTH + nextGridX];
      rawHeight11 = REICH_RAW_HEIGHT_DATA[nextGridY * REICH_MAX_GRID_WIDTH + nextGridX];
      rawHeight01 = REICH_RAW_HEIGHT_DATA[nextGridY * REICH_MAX_GRID_WIDTH + gridX];
      rawAverageHeight = (rawHeight00 + rawHeight10 + rawHeight11 + rawHeight01) * 0.25f;
      waterDepth = WATER_LVL - rawAverageHeight;
      if (waterDepth < 0.0f) waterDepth = 0.0f;
      finalColor = reich_blend_water(finalColor, waterDepth, td->skyColor);
    }

    quadColor = (isSelectedQuad == 2)
        ? 0xFFFFFFFF
        : (isSelectedQuad == 1 ? reich_lerp_col(finalColor, 0xFFFFFFFF, 0.4f)
                     : ((td->threadHoverX != -1 && td->threadHoverX >= gridX && td->threadHoverX < nextGridX && td->threadHoverY >= gridY && td->threadHoverY < nextGridY)
                            ? reich_lerp_col(finalColor, 0xFFFFFF00, 0.5f)
                            : finalColor));

    cmd->visible = 1;
    cmd->x00 = screenX00; cmd->y00 = screenY00;
    cmd->x10 = screenX10; cmd->y10 = screenY10;
    cmd->x11 = screenX11; cmd->y11 = screenY11;
    cmd->x01 = screenX01; cmd->y01 = screenY01;
    cmd->cullTri1 = cullTri1;
    cmd->cullTri2 = cullTri2;
    cmd->quadColor = quadColor;
    cmd->isTextured = (td->levelOfDetail == 1 && td->zm >= 8.0f);
  }
  return 0;
}

int32 draw_world(
    reichContext* ctx,
    reichRect clipRect,
    void* data,
    uint32 sunColor,
    uint32 ambientColor,
    uint32 skyColor,
    real32 lightDirX,
    real32 lightDirY,
    real32 lightDirZ) {

  int32 k, levelOfDetail = 1, drawBounds, startGridX, endGridX, startGridY, endGridY;
  int32 selBoxMinX = -1, selBoxMaxX = -1, selBoxMinY = -1, selBoxMaxY = -1;
  uint32 *grid = (uint32*)data;
  reichRect originalClip;

  real64 mouseX, mouseY, minWorldX = 1e9, maxWorldX = -1e9, minWorldY = 1e9, maxWorldY = -1e9;
  real64 worldX, worldY, cosYaw, sinYaw, cosPitch, sinPitch;
  
  int32 gStart_Y, gStep_Y, numY, gStart_X, gStep_X, numX, totalQuads, idx, t;
  QuadDrawCmd* quadCmds;
  ThreadData tData[NUM_THREADS];
  HANDLE hThreads[NUM_THREADS];
  int32 itemsPerThread, remainder, startIdx = 0;

  mouseX = (real64)reich_mouse_x(ctx);
  mouseY = (real64)reich_mouse_y(ctx);
  
  if (mainCamera.zoom < 4.0) { levelOfDetail = 2; }
  if (mainCamera.zoom < 2.0) { levelOfDetail = 4; }
  if (mainCamera.zoom < 1.0) { levelOfDetail = 8; }
  if (mainCamera.zoom < 0.5) { levelOfDetail = 16; }
  if (mainCamera.zoom < 0.25) { levelOfDetail = 32; }

  drawBounds = mainCamera.zoom >= 15.0;
  originalClip = ctx->clip;
  ctx->clip = clipRect;

  for (k = 0; k < 8; k++) {
    reich_screen_to_world(ctx, &mainCamera,
        (k & 1) ? (real64)clipRect.x1 : (real64)clipRect.x2,
        (k & 2) ? (real64)clipRect.y1 : (real64)clipRect.y2,
        (k & 4) ? (real32)TERRAIN_MIN_Z : (real32)TERRAIN_MAX_Z,
        &worldX, &worldY);
    if (worldX < minWorldX) { minWorldX = worldX; }
    if (worldX > maxWorldX) { maxWorldX = worldX; }
    if (worldY < minWorldY) { minWorldY = worldY; }
    if (worldY > maxWorldY) { maxWorldY = worldY; }
  }

  startGridX = (int32)reich_floor((float)minWorldX) - levelOfDetail * 2;
  startGridY = (int32)reich_floor((float)minWorldY) - levelOfDetail * 2;
  endGridX = (int32)reich_ceil((float)maxWorldX) + levelOfDetail * 2;
  endGridY = (int32)reich_ceil((float)maxWorldY) + levelOfDetail * 2;

  if (startGridX < 0) { startGridX = 0; }
  if (startGridY < 0) { startGridY = 0; }
  if (endGridX > REICH_MAX_GRID_WIDTH) { endGridX = REICH_MAX_GRID_WIDTH; }
  if (endGridY > REICH_MAX_GRID_HEIGHT) { endGridY = REICH_MAX_GRID_HEIGHT; }

  startGridX = (startGridX / levelOfDetail) * levelOfDetail;
  startGridY = (startGridY / levelOfDetail) * levelOfDetail;
  endGridX = ((endGridX + levelOfDetail - 1) / levelOfDetail) * levelOfDetail;
  endGridY = ((endGridY + levelOfDetail - 1) / levelOfDetail) * levelOfDetail;

  if (isSelectingBox) {
    selBoxMinX = (int32)reich_floor((float)REICH_MIN(selectionStartWorldX, selectionEndWorldX));
    selBoxMaxX = (int32)reich_floor((float)REICH_MAX(selectionStartWorldX, selectionEndWorldX));
    selBoxMinY = (int32)reich_floor((float)REICH_MIN(selectionStartWorldY, selectionEndWorldY));
    selBoxMaxY = (int32)reich_floor((float)REICH_MAX(selectionStartWorldY, selectionEndWorldY));
  }

  cosYaw = reich_cos((float)mainCamera.angle);
  sinYaw = reich_sin((float)mainCamera.angle);
  cosPitch = reich_cos((float)mainCamera.pitch);
  sinPitch = reich_sin((float)mainCamera.pitch);

  gStart_Y = (cosYaw > 0 ? startGridY : endGridY - levelOfDetail);
  gStep_Y  = (cosYaw > 0 ? levelOfDetail : -levelOfDetail);
  numY     = REICH_ABS(endGridY - startGridY) / levelOfDetail;

  gStart_X = (sinYaw > 0 ? startGridX : endGridX - levelOfDetail);
  gStep_X  = (sinYaw > 0 ? levelOfDetail : -levelOfDetail);
  numX     = REICH_ABS(endGridX - startGridX) / levelOfDetail;

  totalQuads = numX * numY;
  if (totalQuads <= 0) {
      ctx->clip = originalClip;
      reich_draw_rect(ctx, (float)clipRect.x1, (float)clipRect.y1, (float)(clipRect.x2 - clipRect.x1), (float)(clipRect.y2 - clipRect.y1), 0xFF00FF00);
      return TRUE;
  }

  /* Safe frame allocation utilizing engine bounds memory constraints */
  quadCmds = (QuadDrawCmd*)reich_arena_alloc(&ctx->frameMem, totalQuads * sizeof(QuadDrawCmd));
  if (!quadCmds) {
      ctx->clip = originalClip;
      return TRUE;
  }

  itemsPerThread = totalQuads / NUM_THREADS;
  remainder = totalQuads % NUM_THREADS;

  /* -- PHASE 1: Process vertices, math & lighting via native Win32 Threads & SSE2 -- */
  for (t = 0; t < NUM_THREADS; t++) {
    tData[t].startIdx = startIdx;
    tData[t].endIdx = startIdx + itemsPerThread + (t < remainder ? 1 : 0);
    startIdx = tData[t].endIdx;

    tData[t].numX = numX;
    tData[t].gStart_Y = gStart_Y;
    tData[t].gStep_Y = gStep_Y;
    tData[t].gStart_X = gStart_X;
    tData[t].gStep_X = gStep_X;
    tData[t].levelOfDetail = levelOfDetail;
    tData[t].selBoxMinX = selBoxMinX;
    tData[t].selBoxMaxX = selBoxMaxX;
    tData[t].selBoxMinY = selBoxMinY;
    tData[t].selBoxMaxY = selBoxMaxY;
    tData[t].hw = (float)ctx->canvas.width * 0.5f;
    tData[t].hh = (float)ctx->canvas.height * 0.5f;
    tData[t].cx = (float)mainCamera.x;
    tData[t].cy = (float)mainCamera.y;
    tData[t].cz = (float)mainCamera.z;
    tData[t].zm = (float)mainCamera.zoom;
    tData[t].cY = (float)cosYaw;
    tData[t].sY = (float)sinYaw;
    tData[t].cP = (float)cosPitch;
    tData[t].sP = (float)sinPitch;
    tData[t].lightDirX = lightDirX;
    tData[t].lightDirY = lightDirY;
    tData[t].lightDirZ = lightDirZ;
    tData[t].sunColor = sunColor;
    tData[t].ambientColor = ambientColor;
    tData[t].skyColor = skyColor;
    tData[t].grid = grid;
    tData[t].clipRect = clipRect;
    tData[t].mouseX = mouseX;
    tData[t].mouseY = mouseY;
    tData[t].cmds = quadCmds;

    hThreads[t] = CreateThread(NULL, 0, process_quads_thread, &tData[t], 0, NULL);
  }

  WaitForMultipleObjects(NUM_THREADS, hThreads, TRUE, INFINITE);

  for (t = 0; t < NUM_THREADS; t++) {
    CloseHandle(hThreads[t]);
    if (tData[t].threadHoverX != -1) {
      nextHoverGridX = tData[t].threadHoverX;
      nextHoverGridY = tData[t].threadHoverY;
    }
  }

  /* -- PHASE 2: Standard Sequential drawing guarantees pure painter's algorithm behavior -- */
  for (idx = 0; idx < totalQuads; idx++) {
      QuadDrawCmd* c = &quadCmds[idx];
      if (!c->visible) continue;

      if (c->isTextured) {
          if (c->cullTri1 > 0.0f) {
              reich_draw_triangle_textured(ctx, c->x00, c->y00, 0.0f, 0.0f, c->x10, c->y10, 1.0f, 0.0f, c->x11, c->y11, 1.0f, 1.0f, TILE_TEXTURE, TILE_TEX_WIDTH, TILE_TEX_HEIGHT, c->quadColor);
          }
          if (c->cullTri2 > 0.0f) {
              reich_draw_triangle_textured(ctx, c->x00, c->y00, 0.0f, 0.0f, c->x11, c->y11, 1.0f, 1.0f, c->x01, c->y01, 0.0f, 1.0f, TILE_TEXTURE, TILE_TEX_WIDTH, TILE_TEX_HEIGHT, c->quadColor);
          }
      } else {
          reich_draw_quad_fill(ctx, c->x00, c->y00, c->x10, c->y10, c->x11, c->y11, c->x01, c->y01, c->quadColor);
      }

      if (drawBounds) {
          if (c->cullTri1 > 0.0f) {
              reich_draw_line(ctx, c->x00, c->y00, c->x10, c->y10, 0x44000000);
              reich_draw_line(ctx, c->x10, c->y10, c->x11, c->y11, 0x44000000);
          }
          if (c->cullTri2 > 0.0f) {
              reich_draw_line(ctx, c->x11, c->y11, c->x01, c->y01, 0x44000000);
              reich_draw_line(ctx, c->x01, c->y01, c->x00, c->y00, 0x44000000);
          }
      }
  }

  ctx->clip = originalClip;
  reich_draw_rect(
      ctx,
      (float)clipRect.x1,
      (float)clipRect.y1,
      (float)(clipRect.x2 - clipRect.x1),
      (float)(clipRect.y2 - clipRect.y1),
      0xFF00FF00);

  return TRUE;
}

int32 my_update(reichContext* ctx) {
  globalTime += TIME_SPEED;
  if (globalTime > 1.0) { globalTime -= 1.0; }
  return ctx->running;
}

int32 my_input(reichContext* ctx) {
  int32 mouseX = reich_mouse_x(ctx);
  int32 mouseY = reich_mouse_y(ctx);
  int32 isShiftDown = reich_key_down(ctx, 0x10);
  int32 isRightMouseDown = reich_mouse_down(ctx, 1);
  int32 isLeftMouseDown = reich_mouse_down(ctx, 0);
  int32 isLeftMousePressed = reich_mouse_pressed(ctx, 0);
  int32 isLeftMouseReleased = reich_mouse_released(ctx, 0);
  int32 scrollWheel = reich_mouse_wheel(ctx);

  real64 mouseWorldX = 0, mouseWorldY = 0, preZoomScreenX, preZoomScreenY, postZoomScreenX, postZoomScreenY;

  hoverGridX = nextHoverGridX;
  hoverGridY = nextHoverGridY;
  nextHoverGridX = -1;
  nextHoverGridY = -1;

  if (hoverGridX != -1 && hoverGridY != -1) {
    mouseWorldX = (real64)hoverGridX + 0.5;
    mouseWorldY = (real64)hoverGridY + 0.5;
  } else {
    reich_screen_to_world(ctx, &mainCamera, (real64)mouseX, (real64)mouseY, 0.0f, &mouseWorldX, &mouseWorldY);
  }

  if (isLeftMousePressed) {
    isDraggingCamera = 0;
    if (isShiftDown) {
      isSelectingBox = 1;
      selectionStartWorldX = mouseWorldX;
      selectionStartWorldY = mouseWorldY;
    }

  	anchor_camera_to_terrain(&mainCamera);
  }

  if (isLeftMouseDown && !isShiftDown && !isSelectingBox &&
      (ctx->input.deltaX != 0 || ctx->input.deltaY != 0)) {
    isDraggingCamera = 1;
    apply_pan_delta(&mainCamera, (real64)ctx->input.deltaX, (real64)ctx->input.deltaY);
  }

  if (reich_key_down(ctx, 0x25)) { apply_pan_delta(&mainCamera, 15.0, 0); }
  if (reich_key_down(ctx, 0x27)) { apply_pan_delta(&mainCamera, -15.0, 0); }
  if (reich_key_down(ctx, 0x26)) { apply_pan_delta(&mainCamera, 0, 15.0); }
  if (reich_key_down(ctx, 0x28)) { apply_pan_delta(&mainCamera, 0, -15.0); }

  if (scrollWheel != 0) {
    reich_world_to_screen(ctx, &mainCamera, mouseWorldX, mouseWorldY, get_terrain_height(mouseWorldX, mouseWorldY), &preZoomScreenX, &preZoomScreenY);
    mainCamera.zoom *= (scrollWheel > 0 ? 1.1 : 0.9);
    if (mainCamera.zoom < CAMERA_MIN_ZOOM) { mainCamera.zoom = CAMERA_MIN_ZOOM; }
    if (mainCamera.zoom > CAMERA_MAX_ZOOM) { mainCamera.zoom = CAMERA_MAX_ZOOM; }
    reich_world_to_screen(ctx, &mainCamera, mouseWorldX, mouseWorldY, get_terrain_height(mouseWorldX, mouseWorldY), &postZoomScreenX, &postZoomScreenY);
    apply_pan_delta(&mainCamera, preZoomScreenX - postZoomScreenX, preZoomScreenY - postZoomScreenY);
  }

  if (isRightMouseDown) {
    mainCamera.angle += (real64)ctx->input.deltaX * 0.01;
    mainCamera.pitch -= (real64)ctx->input.deltaY * 0.005;
    if (mainCamera.pitch < CAMERA_MIN_PITCH) { mainCamera.pitch = CAMERA_MIN_PITCH; }
    if (mainCamera.pitch > CAMERA_MAX_PITCH) { mainCamera.pitch = CAMERA_MAX_PITCH; }
  }

  if (mainCamera.x < 0) { mainCamera.x = 0; }
  if (mainCamera.y < 0) { mainCamera.y = 0; }
  if (mainCamera.x > REICH_MAX_GRID_WIDTH) { mainCamera.x = REICH_MAX_GRID_WIDTH; }
  if (mainCamera.y > REICH_MAX_GRID_HEIGHT) { mainCamera.y = REICH_MAX_GRID_HEIGHT; }

  if (isLeftMouseDown && isSelectingBox) {
    selectionEndWorldX = mouseWorldX;
    selectionEndWorldY = mouseWorldY;
  }

  if (isLeftMouseReleased) {
    if (isSelectingBox) {
      selectionMinX = (int32)reich_floor((float)REICH_MIN(selectionStartWorldX, selectionEndWorldX));
      selectionMaxX = (int32)reich_floor((float)REICH_MAX(selectionStartWorldX, selectionEndWorldX));
      selectionMinY = (int32)reich_floor((float)REICH_MIN(selectionStartWorldY, selectionEndWorldY));
      selectionMaxY = (int32)reich_floor((float)REICH_MAX(selectionStartWorldY, selectionEndWorldY));
      isSelectingBox = 0;
    } else if (!isDraggingCamera && hoverGridX != -1) {
      selectionMinX = hoverGridX;
      selectionMaxX = hoverGridX;
      selectionMinY = hoverGridY;
      selectionMaxY = hoverGridY;
    }
  }
  return ctx->running;
}

int32 my_render(reichContext* ctx, real64 alpha) {
  uint32 sunColor, ambientColor, skyColor;
  real32 lightDirX, lightDirY, lightDirZ;
  reich_compute_atmosphere(globalTime, &sunColor, &ambientColor, &skyColor, &lightDirX, &lightDirY, &lightDirZ);
  reich_draw_clear(ctx, skyColor);
  reich_draw_checkerboard(ctx, 32, skyColor);
	(void)alpha;
  draw_world(
      ctx,
      reich_rect(50, 40, ctx->canvas.width - 50, ctx->canvas.height - 40),
      REICH_GRID_DATA,
      sunColor,
      ambientColor,
      skyColor,
      lightDirX,
      lightDirY,
      lightDirZ);
  return 1;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow) {
  (void)hInst;
	(void)hPrev;
	(void)cmdLine;
	(void)cmdShow;
	grid_init(REICH_GRID_DATA, REICH_MAX_GRID_WIDTH, REICH_MAX_GRID_HEIGHT);
  if (!reich_init(&mainContext, "REICH Production Shading", 800, 600, 60.0)) {
    return -1;
  }
  reich_set_callbacks(&mainContext, my_update, my_render, my_input);
  reich_run(&mainContext);
  return 0;
}

void mainCRTStartup(void) {
  ExitProcess(WinMain(
      GetModuleHandleA(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT));
}
