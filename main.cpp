/*
================================================================================
  COMPUTER GRAPHICS LAB PROJECT
  Department of CSE | Course: Computer Graphics Lab
  Project Title   : Moon Land — OpenGL Scene with 3D Rocket Launch & Moon Landing
  Total Marks     : 40
================================================================================
  Group Members:
       1. Rafi    — Scene Setup, 3D Rocket System, Launch Mechanics, Moon Landing,
                    Midpoint Circle Algorithm
       2. Raka    — Bus Animation, C-Building, Annex Tower, Annex 1 & 2,
                    D-Building, Field & Road, Bresenham's Line Algorithm
       3. Samsul  — Sky/Sun/Clouds, Trees, Pool, Moon Scene (display5)
================================================================================
  EP Mapping:
       EP1 — Algorithms used: Midpoint Circle (Sun), Bresenham's Line (D-Building floors)
       EP3 — Scene decomposed into modular drawable objects; transformations &
              animation timing managed via timer callbacks
       EP4 — Standard OpenGL primitives, GL_QUADS, GL_TRIANGLE_FAN, glTranslatef,
              glRotatef, glPushMatrix/glPopMatrix used throughout
================================================================================
  Controls:
       1     → Switch to Day Scene (display2) — Campus + Rocket Launch Pad
       2     → Switch to Moon Landing Scene (display5)
       v     → Start cloud drift (left) + bus movement (right)
       b     → Stop cloud drift and bus
       n     → Ignite rocket / start 3D Y-spin
       m     → Pause rocket thrust (spin continues)
       r     → Full reset (rocket, moon lander, spin, clouds, bus)
       UP    → Resume moon lander descent (in display5)
       DOWN  → Pause moon lander descent (in display5)
================================================================================
*/

// ─────────────────────────────────────────────────────────────────────────────
//  INCLUDES & LIBRARIES
// ─────────────────────────────────────────────────────────────────────────────
#include <iostream>
using namespace std;
#include <windows.h>    // Windows API — used for PlaySound()
#include <mmsystem.h>   // Multimedia sound playback
#include <GL/glut.h>    // GLUT — includes glu.h and gl.h automatically
#include <stdio.h>
#include <math.h>       // cos(), sin(), cosf(), sinf(), fabsf()
#include <GL/gl.h>
#include <GL/glu.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")   // Link Windows multimedia library

// ─────────────────────────────────────────────────────────────────────────────
//  GLOBAL CONSTANTS & SHARED VARIABLES
// ─────────────────────────────────────────────────────────────────────────────
#define PI 3.14159265358979323846   // Pi constant for circle/arc calculations

int       triangleAmount = 100;           // Segments used in GL_TRIANGLE_FAN circles
GLfloat   twicePi       = 2.0f * PI;     // 2π — full revolution in radians
int       i;                              // General-purpose loop index
int       lineAmount     = 100;           // Segments used in GL_LINE_LOOP circles

// ─────────────────────────────────────────────────────────────────────────────
//  OPENGL INITIALISATION
// ─────────────────────────────────────────────────────────────────────────────
void init(void)
{
    glMatrixMode(GL_PROJECTION);    // Switch to projection matrix
    glLoadIdentity();               // Reset projection to identity
    glOrtho(0.0, 1.0, 0.0, 1.0, -10.0, 10.0); // Orthographic: (0,0) bottom-left → (1,1) top-right
    glEnable(GL_DEPTH_TEST);        // Enable depth buffering for 3D overlap resolution
    glDepthFunc(GL_LEQUAL);         // Draw fragment if depth ≤ stored value
}

// ─────────────────────────────────────────────────────────────────────────────
//  IDLE CALLBACK — triggers continuous redraws
// ─────────────────────────────────────────────────────────────────────────────
void Idle()
{
    glutPostRedisplay();    // Mark window as needing redraw each idle cycle
}



// ─────────────────────────────────────────────────────────────────────────────
//  HELPER — Filled Ellipse (solid polygon)
//  Draws a filled ellipse centred at (centerX, centerY) with radii (radiusX, radiusY).
// ─────────────────────────────────────────────────────────────────────────────
void drawEllipse(float centerX, float centerY, float radiusX, float radiusY, int numSegments)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < numSegments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(numSegments);
        float x = radiusX * cosf(theta);
        float y = radiusY * sinf(theta);
        glVertex2f(centerX + x, centerY + y);
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  HELPER — Outlined Ellipse (thick border ring)
//  Draws an outline-only ellipse using GL_LINE_LOOP with 5px width.
// ─────────────────────────────────────────────────────────────────────────────
void drawEllipse1(float centerX, float centerY, float radiusX, float radiusY, int numSegments)
{
    glLineWidth(5);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < numSegments/2; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(numSegments);
        float x = radiusX * cosf(theta);
        float y = radiusY * sinf(theta);
        glVertex2f(centerX + x, centerY + y);
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  EP1 ALGORITHM 1 — Midpoint Circle Algorithm (filled)
//  Used by: Rafi — draws the filled Sun in  display2
//  Implements the Midpoint Circle rasterisation algorithm adapted for floating-
//  point OpenGL coordinates (step size 0.001f instead of 1 pixel).
// ─────────────────────────────────────────────────────────────────────────────
void drawSunMidpoint(float xc, float yc, float r)
{
    float x = 0, y = r;
    float p = 0.001f - r;   // Initial decision parameter (p0 = 1 - r)

    while (x <= y) {
        // Draw horizontal scan-lines across all 8 octant reflections
        glBegin(GL_LINES);
        glVertex2f(xc - x, yc + y); glVertex2f(xc + x, yc + y);   // top band
        glVertex2f(xc - x, yc - y); glVertex2f(xc + x, yc - y);   // bottom band
        glVertex2f(xc - y, yc + x); glVertex2f(xc + y, yc + x);   // right upper
        glVertex2f(xc - y, yc - x); glVertex2f(xc + y, yc - x);   // right lower
        glEnd();

        // Update decision parameter
        if (p < 0) p += 2* x + 0.003f;                      // midpoint is inside circle
        else { p += 2 * (x - y) + 0.005f; y -= 0.001f; }    // midpoint is outside — step y inward
        x += 0.001f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  EP1 ALGORITHM 2 — Bresenham's Line Algorithm
//  Used by: Raka — draws D-Building floor separation lines.
//  Plots a straight line between two floating-point coordinates using the
//  classic Bresenham error-accumulation approach (step size 0.001f).
// ─────────────────────────────────────────────────────────────────────────────
void drawLineBresenham(float x1, float y1, float x2, float y2)
{
    float dx  = abs(x2 - x1), dy  = abs(y2 - y1);
    float x   = x1,           y   = y1;
    float sx  = (x1 < x2) ? 0.001f : -0.001f;  // step direction on X
    float sy  = (y1 < y2) ? 0.001f : -0.001f;  // step direction on Y
    float err = dx - dy, e2;                    // initial error term

    glBegin(GL_POINTS);
    while (true) {
        glVertex2f(x, y);
        if (abs(x - x2) < 0.001f && abs(y - y2) < 0.001f) break;  // reached endpoint
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }  // horizontal step
        if (e2 <  dx) { err += dx; y += sy; }  // vertical step
    }
    glEnd();
}


// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         RAFI  —  START                                      ║
// ║  Responsibilities:                                                           ║
// ║    • Global state variables for rocket, launch & moon landing                ║
// ║    • 3D helper primitives (cylinder, disk, cone, wing face)      ║
// ║    • Shared draw3DRocket() function (display2 + display5)                    ║
// ║    • update2()  — smooth rocket launch timer                                 ║
// ║    • updateRocketSpin() — 3D Y-axis spin timer                               ║
// ║    • updateMoonLanding() — moon descent timer                                ║
// ║    • Keyboard handlers for rocket controls (n/m/r/1/2)                       ║
// ║    • Arrow key handler for moon lander (UP/DOWN)                             ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — GLOBAL STATE: Clouds & Bus positions (shared with Raka/Samsul timers)
// ─────────────────────────────────────────────────────────────────────────────
// Animation positions
GLfloat Position1 = 0.0f;  // Cloud group horizontal offset (update1 timer — Samsul)
GLfloat Position2 = 0.0f;  // Rocket vertical offset — rocket moves, stand stays fixed
GLfloat Position3 = 0.0f;  // Bus horizontal offset (update3 timer — Raka)
GLfloat Speed1    = 0.0f;  // Cloud drift speed  (set by 'v'/'b' key)
GLfloat Speed2    = 0.0f;  // (reserved — not directly used; launchSpeed handles rocket)
GLfloat Speed3    = 0.0f;  // Bus travel speed   (set by 'v'/'b' key)

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — GLOBAL STATE: 3D Rocket system
// ─────────────────────────────────────────────────────────────────────────────
// Rocket 3D state
GLfloat rocketAngleY     = 0.0f;   // Current Y-rotation angle (0–360°) for 3D spin
GLfloat rocketSpinSpeed  = 0.0f;   // Degrees per frame — starts at 0, eased when 'n' pressed
bool    rocketLaunching  = false;  // true = rocket is ascending in display2
bool    rocketSpinActive = false;  // false = no spin until 'n' first pressed
GLfloat launchSpeed      = 0.0f;   // Current ascent speed (smoothly accelerated)

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — GLOBAL STATE: Moon landing (display5)
//  positions starts at 1.0 (above visible screen top), descends toward -0.60.
// ─────────────────────────────────────────────────────────────────────────────
// Moon landing state
GLfloat positions  = 1.0f;   // Vertical translate: rocket descends from 1.0 → -0.60
GLfloat speeds     = 0.0f;   // Descent speed per frame (0 = paused, >0 = descending)
bool    moonLanded = false;  // true = rocket resting on moon surface; fire extinguished

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — ROCKET GEOMETRY CONSTANTS
//  These constants define the 3D oval cross-section of the rocket body.
// ─────────────────────────────────────────────────────────────────────────────
static const float RCX = 0.405f;  // Rocket centre X (midpoint of body)
static const float RHW = 0.035f;  // Half-width in X  (body spans X: 0.370–0.440)
static const float RHD = 0.030f;  // Half-depth in Z  (gives an oval, not circular, cross-section)

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — 3D HELPER: Elliptical Cylinder Band
//  Draws one vertical section of the rocket body as a lit QUAD_STRIP.
//  Lighting is simulated with cosine of angle: front bright, back dark.
//  Parameters: cx/cz = centre, y0/y1 = bottom/top Y, rx/rz = X and Z radii,
//              segs = polygon segments, rT/gT/bT = top colour, rB/gB/bB = bottom colour.
// ─────────────────────────────────────────────────────────────────────────────
void drawCylinderStrip(float cx, float y0, float y1, float cz,
                       float rx, float rz, int segs,
                       float rT, float gT, float bT,
                       float rB, float gB, float bB)
{
    glBegin(GL_QUAD_STRIP);
    for (int k = 0; k <= segs; k++) {
        float t   = twicePi * k / segs;
        float dx  = rx * cosf(t);
        float dz  = rz * sinf(t);
        float lit = 0.50f + 0.50f * cosf(t);       // cosine diffuse: front=1.0, back=0.0
        glColor3f(rT * lit, gT * lit, bT * lit);   glVertex3f(cx + dx, y1, cz + dz); // top ring
        glColor3f(rB * lit, gB * lit, bB * lit);   glVertex3f(cx + dx, y0, cz + dz); // bottom ring
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — 3D HELPER: Filled Disk Cap
//  Draws a solid oval disk (top or bottom cap of a cylinder) using TRIANGLE_FAN.
// ─────────────────────────────────────────────────────────────────────────────
void drawDisk(float cx, float cy, float cz,
              float rx, float rz, int segs,
              float r, float g, float b)
{
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(r, g, b);
    glVertex3f(cx, cy, cz);    // fan centre
    for (int k = 0; k <= segs; k++) {
        float t   = twicePi * k / segs;
        float lit = 0.55f + 0.45f * cosf(t);
        glColor3f(r * lit, g * lit, b * lit);
        glVertex3f(cx + rx * cosf(t), cy, cz + rz * sinf(t));
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — 3D HELPER: Semi-transparent Oval Rim
//  Draws a translucent collar ring — used for nozzle edges and shoulder rings.
//  Requires GL_BLEND; blending is enabled/disabled internally.
// ─────────────────────────────────────────────────────────────────────────────
void drawOvalRim(float cx, float cy, float cz,
                 float rx, float rz, int segs,
                 float r, float g, float b, float a)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(r, g, b, a);
    glVertex3f(cx, cy, cz);
    for (int k = 0; k <= segs; k++) {
        float t   = twicePi * k / segs;
        float lit = 0.55f + 0.45f * cosf(t);
        glColor4f(r * lit, g * lit, b * lit, a);
        glVertex3f(cx + rx * cosf(t), cy, cz + rz * sinf(t));
    }
    glEnd();
    glDisable(GL_BLEND);
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — 3D HELPER: Cone
//  Draws a tapered cone from a circular base at (baseY) to a tip point at (tipY).
//  Used for nose cone and mini-booster tips.
// ─────────────────────────────────────────────────────────────────────────────
void drawCone(float cx, float tipY, float baseY, float cz,
              float rx, float rz, int segs,
              float rTip, float gTip, float bTip,
              float rBase, float gBase, float bBase)
{
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(rTip, gTip, bTip);
    glVertex3f(cx, tipY, cz);   // apex of cone
    for (int k = 0; k <= segs; k++) {
        float t   = twicePi * k / segs;
        float lit = 0.50f + 0.50f * cosf(t);
        glColor3f(rBase * lit, gBase * lit, bBase * lit);
        glVertex3f(cx + rx * cosf(t), baseY, cz + rz * sinf(t));
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — 3D HELPER: Single Wing Face (one flat quad panel at fixed Z depth)
//  Called twice per wing (front face + back face) to give thickness.
// ─────────────────────────────────────────────────────────────────────────────
void drawWingFace(float x0, float y0, float x1, float y1,
                  float x2, float y2, float x3, float y3,
                  float zDepth, float r, float g, float b, float brightness)
{
    glColor3f(r * brightness, g * brightness, b * brightness);
    glBegin(GL_QUADS);
    glVertex3f(x0, y0, zDepth); glVertex3f(x1, y1, zDepth);
    glVertex3f(x2, y2, zDepth); glVertex3f(x3, y3, zDepth);
    glEnd();
}


//  RAFI — SHARED 3D ROCKET DRAW FUNCTION
//
//  Called by both display2 (launch pad scene) and display5 (moon landing).
//  The rocket body rotates on the Y-axis according to rocketAngleY.
//
//  Parameters:
//    posY      — vertical offset applied to entire rocket (Position2 or positions)
//    showStand — true: also draw the metal launch stand + transparent tube (display2 only)
//    fireOn    — true: draw exhaust fire plumes beneath the nozzle
//
//  Drawing order inside the rocket:
//    1. Launch stand + transparent tube  (if showStand)
//    2. Nozzle collar
//    3. Lower booster body (orange)
//    4. Main body (blue)
//    5. Shoulder ring
//    6. Tapered upper section (orange taper)
//    7. Nose cone (grey)
//    8. Porthole window (3D convex lens)
//    9. Side boosters (×2)
//   10. Wings (right + left, front/back faces + edge slabs)
//   11. Base trapezoid slab
//   12. Fire plumes (if fireOn)
// ─────────────────────────────────────────────────────────────────────────────
void draw3DRocket(float posY, bool showStand, bool fireOn)
{
    int segs = 36;  // polygon segment count for circular cross-sections

    // ─────────────────────────────────────────────────────────────────────────
    //  1. LAUNCH PAD STAND + TRANSPARENT TUBE (fixed — never moves with rocket)
    // ─────────────────────────────────────────────────────────────────────────
    if (showStand)
    {
        // ── Metal structural stand (grey/blue) ──
        glBegin(GL_QUADS);

        // Main vertical pillar (outer shell)
        glColor3f(0.549f, 0.639f, 0.635f);
        glVertex2f(0.52f, 0.56f); glVertex2f(0.52f, 0.20f);
        glVertex2f(0.58f, 0.20f); glVertex2f(0.58f, 0.56f);

        // Horizontal arm (left bracket)
        glColor3f(0.549f, 0.639f, 0.635f);
        glVertex2f(0.436f, 0.51f); glVertex2f(0.436f, 0.46f);
        glVertex2f(0.52f,  0.46f); glVertex2f(0.52f,  0.51f);

        // Inner pillar accent (darker blue)
        glColor3f(0.035f, 0.506f, 0.733f);
        glVertex2f(0.53f, 0.54f); glVertex2f(0.53f, 0.20f);
        glVertex2f(0.57f, 0.20f); glVertex2f(0.57f, 0.54f);
        glVertex2f(0.436f, 0.50f); glVertex2f(0.436f, 0.47f);
        glVertex2f(0.53f,  0.47f); glVertex2f(0.53f,  0.50f);

        // Base plate highlight
        glColor3f(0.635f, 0.733f, 0.82f);
        glVertex2f(0.54f, 0.25f); glVertex2f(0.54f, 0.20f);
        glVertex2f(0.56f, 0.20f); glVertex2f(0.56f, 0.25f);

        glEnd();

        // ── Stand edge outlines (black hairlines for depth) ──
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glColor3f(0.0f, 0.0f, 0.0f);

        // Main pillar outline
        glVertex2f(0.52f, 0.56f); glVertex2f(0.52f, 0.20f);
        glVertex2f(0.58f, 0.20f); glVertex2f(0.58f, 0.56f);
        glVertex2f(0.52f, 0.56f); glVertex2f(0.58f, 0.56f);
        glVertex2f(0.52f, 0.20f); glVertex2f(0.58f, 0.20f);

        // Left arm outline
        glVertex2f(0.436f, 0.51f); glVertex2f(0.436f, 0.46f);
        glVertex2f(0.52f,  0.46f); glVertex2f(0.52f,  0.51f);
        glVertex2f(0.436f, 0.51f); glVertex2f(0.52f,  0.51f);
        glVertex2f(0.436f, 0.46f); glVertex2f(0.52f,  0.46f);

        // Inner pillar outline
        glVertex2f(0.53f, 0.54f); glVertex2f(0.53f, 0.20f);
        glVertex2f(0.57f, 0.20f); glVertex2f(0.57f, 0.54f);
        glVertex2f(0.53f, 0.54f); glVertex2f(0.57f, 0.54f);
        glVertex2f(0.53f, 0.20f); glVertex2f(0.57f, 0.20f);

        // Inner arm outline
        glVertex2f(0.436f, 0.50f); glVertex2f(0.436f, 0.47f);
        glVertex2f(0.53f,  0.47f); glVertex2f(0.53f,  0.50f);
        glVertex2f(0.436f, 0.50f); glVertex2f(0.53f,  0.50f);
        glVertex2f(0.436f, 0.47f); glVertex2f(0.53f,  0.47f);

        // Base plate outline
        glVertex2f(0.54f, 0.25f); glVertex2f(0.54f, 0.20f);
        glVertex2f(0.56f, 0.20f); glVertex2f(0.56f, 0.25f);
        glVertex2f(0.54f, 0.25f); glVertex2f(0.56f, 0.25f);
        glVertex2f(0.54f, 0.20f); glVertex2f(0.56f, 0.20f);

        // Diagonal cross-brace pattern (structural zig-zag detail)
        glVertex2f(0.44f, 0.50f); glVertex2f(0.45f, 0.47f);
        glVertex2f(0.46f, 0.47f); glVertex2f(0.47f, 0.50f);
        glVertex2f(0.48f, 0.50f); glVertex2f(0.49f, 0.47f);
        glVertex2f(0.50f, 0.47f); glVertex2f(0.51f, 0.50f);

        // Horizontal grating lines (engineering detail)
        glVertex2f(0.435f, 0.39f);  glVertex2f(0.52f, 0.39f);
        glVertex2f(0.435f, 0.36f);  glVertex2f(0.52f, 0.36f);
        glVertex2f(0.435f, 0.375f); glVertex2f(0.52f, 0.375f);
        glVertex2f(0.435f, 0.39f);  glVertex2f(0.435f, 0.36f);

        // Diagonal X-brace fills on grating
        glVertex2f(0.438f, 0.39f); glVertex2f(0.458f, 0.36f);
        glVertex2f(0.458f, 0.36f); glVertex2f(0.478f, 0.39f);
        glVertex2f(0.478f, 0.39f); glVertex2f(0.498f, 0.36f);
        glVertex2f(0.498f, 0.36f); glVertex2f(0.518f, 0.39f);
        glVertex2f(0.438f, 0.36f); glVertex2f(0.458f, 0.39f);
        glVertex2f(0.458f, 0.39f); glVertex2f(0.478f, 0.36f);
        glVertex2f(0.478f, 0.36f); glVertex2f(0.498f, 0.39f);
        glVertex2f(0.498f, 0.39f); glVertex2f(0.518f, 0.36f);

        // Vertical centre guide rail (blue)
        glColor3f(0.157f, 0.6f, 0.835f);
        glVertex2f(0.55f, 0.538f); glVertex2f(0.55f, 0.25f);

        glEnd();

        // ── Transparent launch tube (3D box around rocket base) ──
        // Semi-transparent so player can see rocket inside
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);  // don't write to depth buffer (glass effect)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);
        // Front face (most visible, highest alpha)
        glColor4f(0.455f, 0.796f, 0.949f, 0.40f);
        glVertex3f(0.32f, 0.20f,  RHD); glVertex3f(0.32f, 0.30f,  RHD);
        glVertex3f(0.49f, 0.30f,  RHD); glVertex3f(0.49f, 0.20f,  RHD);
        // Back face (lowest alpha — depth cue)
        glColor4f(0.455f, 0.796f, 0.949f, 0.22f);
        glVertex3f(0.32f, 0.20f, -RHD); glVertex3f(0.32f, 0.30f, -RHD);
        glVertex3f(0.49f, 0.30f, -RHD); glVertex3f(0.49f, 0.20f, -RHD);
        // Left side face
        glColor4f(0.455f, 0.796f, 0.949f, 0.30f);
        glVertex3f(0.32f, 0.20f,  RHD); glVertex3f(0.32f, 0.30f,  RHD);
        glVertex3f(0.32f, 0.30f, -RHD); glVertex3f(0.32f, 0.20f, -RHD);
        // Right side face
        glVertex3f(0.49f, 0.20f,  RHD); glVertex3f(0.49f, 0.30f,  RHD);
        glVertex3f(0.49f, 0.30f, -RHD); glVertex3f(0.49f, 0.20f, -RHD);
        glEnd();

        // Edge highlight lines on tube (white, semi-transparent)
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 1.0f, 1.0f, 0.55f);
        glVertex3f(0.32f, 0.20f,  RHD); glVertex3f(0.49f, 0.20f,  RHD);
        glVertex3f(0.32f, 0.30f,  RHD); glVertex3f(0.49f, 0.30f,  RHD);
        glVertex3f(0.32f, 0.20f, -RHD); glVertex3f(0.49f, 0.20f, -RHD);
        glVertex3f(0.32f, 0.30f, -RHD); glVertex3f(0.49f, 0.30f, -RHD);
        glVertex3f(0.32f, 0.20f,  RHD); glVertex3f(0.32f, 0.30f,  RHD);
        glVertex3f(0.49f, 0.20f,  RHD); glVertex3f(0.49f, 0.30f,  RHD);
        glVertex3f(0.32f, 0.20f, -RHD); glVertex3f(0.32f, 0.30f, -RHD);
        glVertex3f(0.49f, 0.20f, -RHD); glVertex3f(0.49f, 0.30f, -RHD);
        glEnd();

        glDepthMask(GL_TRUE);   // restore depth writing
        glDisable(GL_BLEND);
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  ROCKET BODY (moves with posY — glTranslatef applies the launch height,
    //               then Y-axis rotation is applied around rocket centre)
    // ─────────────────────────────────────────────────────────────────────────
    glPushMatrix();
    glTranslatef(0.0f, posY, 0.0f);                  // vertical offset (launch / descent)
    glTranslatef(RCX,  0.375f, 0.0f);               // move pivot to rocket centre
    glRotatef(rocketAngleY, 0.0f, 1.0f, 0.0f);      // apply Y-axis spin
    glTranslatef(-RCX, -0.375f, 0.0f);              // restore coordinate origin

    // ── Part 1: Nozzle collar (bottom rim of engine) ──
    drawCylinderStrip(RCX, 0.23f, 0.25f, 0.0f, RHW, RHD, segs,
                      0.455f, 0.525f, 0.553f,    // top colour (steel blue)
                      0.345f, 0.416f, 0.424f);   // bottom colour (darker steel)
    drawDisk(RCX, 0.25f, 0.0f, RHW, RHD, segs, 0.42f, 0.49f, 0.51f);      // top cap
    drawDisk(RCX, 0.23f, 0.0f, RHW * 0.92f, RHD * 0.92f, segs, 0.22f, 0.28f, 0.30f); // nozzle opening

    // ── Part 2: Lower booster body (orange) ──
    drawCylinderStrip(RCX, 0.25f, 0.34f, 0.0f, RHW, RHD, segs,
                      0.976f, 0.475f, 0.133f,   // bright orange top
                      0.835f, 0.388f, 0.090f);  // darker orange bottom
    // Separation ring between booster and main body
    drawCylinderStrip(RCX, 0.34f, 0.35f, 0.0f, RHW + 0.003f, RHD + 0.003f, segs,
                      0.588f, 0.647f, 0.663f,   // light grey-blue
                      0.450f, 0.500f, 0.510f);
    drawOvalRim(RCX, 0.35f, 0.0f, RHW + 0.003f, RHD + 0.003f, segs, 0.55f, 0.62f, 0.64f, 1.0f);

    // ── Part 3: Main body (blue) ──
    drawCylinderStrip(RCX, 0.35f, 0.50f, 0.0f, RHW, RHD, segs,
                      0.224f, 0.490f, 0.886f,   // bright blue top
                      0.125f, 0.420f, 0.776f);  // deeper blue bottom
    // Thin dark stripe detail band
    drawCylinderStrip(RCX, 0.449f, 0.452f, 0.0f, RHW + 0.002f, RHD + 0.002f, segs,
                      0.05f, 0.05f, 0.05f,
                      0.05f, 0.05f, 0.05f);

    // ── Part 4: Shoulder ring (transition from main body to upper taper) ──
    drawCylinderStrip(RCX, 0.50f, 0.51f, 0.0f, RHW + 0.004f, RHD + 0.004f, segs,
                      0.478f, 0.537f, 0.565f,
                      0.349f, 0.420f, 0.435f);
    drawOvalRim(RCX, 0.51f, 0.0f, RHW + 0.004f, RHD + 0.004f, segs, 0.40f, 0.46f, 0.48f, 1.0f);

    // ── Part 5: Tapered upper section (orange frustum) ──
    {
        float rxTop = RHW * 0.86f, rzTop = RHD * 0.86f;      // narrower top radius
        float rxBot = RHW + 0.004f, rzBot = RHD + 0.004f;    // wider bottom (matches shoulder ring)
        glBegin(GL_QUAD_STRIP);
        for (int k = 0; k <= segs; k++) {
            float t   = twicePi * k / segs;
            float lit = 0.50f + 0.50f * cosf(t);
            glColor3f(0.925f * lit, 0.431f * lit, 0.075f * lit);
            glVertex3f(RCX + rxTop * cosf(t), 0.56f, rxTop * sinf(t) * RHD / RHW); // top edge
            glColor3f(0.976f * lit, 0.475f * lit, 0.133f * lit);
            glVertex3f(RCX + rxBot * cosf(t), 0.51f, rzBot * sinf(t));              // bottom edge
        }
        glEnd();
    }

    // ── Part 6: Nose cone (grey pointed tip) ──
    drawCone(RCX, 0.59f, 0.56f, 0.0f, RHW * 0.86f, RHD * 0.86f, segs,
             0.52f, 0.58f, 0.60f,       // tip colour (light grey)
             0.471f, 0.529f, 0.557f);   // base colour (mid grey)
    drawDisk(RCX, 0.59f, 0.0f, 0.004f, 0.004f, 12, 0.28f, 0.32f, 0.33f); // tiny tip cap
    drawOvalRim(RCX, 0.56f, 0.0f, RHW * 0.86f, RHD * 0.86f, segs, 0.40f, 0.46f, 0.48f, 1.0f);

    // ── Part 7: Porthole window (3D convex lens with highlight) ──
    {
        float wX = RCX, wY = 0.45f, wR = 0.015f;     // window centre and radius
        float fZ = RHD + 0.002f;                       // front Z (protrudes slightly)
        float bZ = -RHD - 0.002f;                      // back Z (interior side)

        // Orange window frame ring
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.976f, 0.482f, 0.125f); glVertex3f(wX, wY, fZ);
        for (int k = 0; k <= segs; k++) {
            float t = twicePi * k / segs, lit = 0.70f + 0.30f * cosf(t);
            glColor3f(0.976f * lit, 0.482f * lit, 0.125f * lit);
            glVertex3f(wX + wR * cosf(t), wY + wR * sinf(t), fZ);
        }
        glEnd();

        // Blue glass fill (inner 65% of window radius)
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.55f, 0.92f, 1.0f); glVertex3f(wX, wY, fZ + 0.002f);
        for (int k = 0; k <= segs; k++) {
            float t = twicePi * k / segs, lit = 0.60f + 0.40f * cosf(t - 0.5f);
            glColor3f(0.314f * lit, 0.863f * lit, 0.98f * lit);
            glVertex3f(wX + wR * 0.65f * cosf(t), wY + wR * 0.65f * sinf(t), fZ + 0.002f);
        }
        glEnd();

        // White specular highlight (upper-left glint on glass)
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(wX - 0.003f, wY + 0.004f, fZ + 0.004f);
        for (int k = 0; k <= 12; k++) {
            float t = twicePi * k / 12;
            glColor3f(0.88f, 0.94f, 1.0f);
            glVertex3f(wX - 0.003f + 0.003f * cosf(t), wY + 0.004f + 0.003f * sinf(t), fZ + 0.004f);
        }
        glEnd();

        // Dark interior (back face gives depth illusion)
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.07f, 0.09f, 0.11f); glVertex3f(wX, wY, bZ);
        for (int k = 0; k <= segs; k++) {
            float t = twicePi * k / segs;
            glColor3f(0.04f, 0.06f, 0.08f);
            glVertex3f(wX + wR * cosf(t), wY + wR * sinf(t), bZ);
        }
        glEnd();
    }

    // ── Part 8: Side boosters (×2 — left and right) ──
    {
        float bRX = 0.005f, bRZ = 0.004f;  // mini-booster cross-section radii
        for (int side = 0; side < 2; side++) {
            float bX = (side == 0) ? 0.440f : 0.370f;  // right booster : left booster
            drawCylinderStrip(bX, 0.23f, 0.39f, 0.0f, bRX, bRZ, segs,
                              0.196f, 0.510f, 0.984f,   // blue top
                              0.150f, 0.400f, 0.800f);  // darker blue bottom
            drawCone(bX, 0.42f, 0.39f, 0.0f, bRX, bRZ, segs,
                     0.99f, 0.62f, 0.12f,   // orange tip
                     0.99f, 0.55f, 0.10f);  // slightly darker base
            drawOvalRim(bX, 0.39f, 0.0f, bRX, bRZ, segs, 0.18f, 0.48f, 0.90f, 1.0f);
        }
    }

    // ── Part 9: Wings (right + left, each with front/back face + edge slabs) ──
    {
        float sinA = sinf(rocketAngleY * 3.14159f / 180.0f); // spin-angle sine for brightness variation
        float wT   = 0.008f;  // half-thickness of wing slab in Z

        // Right wing corner vertices [0]=root-top, [1]=tip-low, [2]=tip-low-front, [3]=root-low
        float rw[4][2] = {{0.434f, 0.36f}, {0.48f, 0.24f}, {0.49f, 0.23f}, {0.43f, 0.25f}};
        // Left wing corner vertices (mirrored about RCX)
        float lw[4][2] = {{0.376f, 0.36f}, {0.33f, 0.24f}, {0.32f, 0.23f}, {0.38f, 0.25f}};

        // Right wing — front face (brighter when facing viewer)
        drawWingFace(rw[0][0], rw[0][1], rw[1][0], rw[1][1], rw[2][0], rw[2][1], rw[3][0], rw[3][1],
                      wT, 0.224f, 0.494f, 0.882f, 0.62f + 0.38f * sinA);
        // Right wing — back face (darker)
        drawWingFace(rw[3][0], rw[3][1], rw[2][0], rw[2][1], rw[1][0], rw[1][1], rw[0][0], rw[0][1],
                    -wT, 0.125f, 0.420f, 0.776f, 0.38f + 0.22f * sinA);
        // Right wing — top edge slab
        float rLit = (0.55f + 0.45f * sinA) * 0.90f;
        glColor3f(0.18f * rLit, 0.45f * rLit, 0.88f * rLit);
        glBegin(GL_QUADS);
        glVertex3f(rw[0][0], rw[0][1],  wT); glVertex3f(rw[3][0], rw[3][1],  wT);
        glVertex3f(rw[3][0], rw[3][1], -wT); glVertex3f(rw[0][0], rw[0][1], -wT);
        glEnd();
        // Right wing — bottom edge slab
        glColor3f(0.10f, 0.33f, 0.70f);
        glBegin(GL_QUADS);
        glVertex3f(rw[1][0], rw[1][1],  wT); glVertex3f(rw[2][0], rw[2][1],  wT);
        glVertex3f(rw[2][0], rw[2][1], -wT); glVertex3f(rw[1][0], rw[1][1], -wT);
        glEnd();

        // Left wing — front face
        drawWingFace(lw[0][0], lw[0][1], lw[1][0], lw[1][1], lw[2][0], lw[2][1], lw[3][0], lw[3][1],
                      wT, 0.224f, 0.494f, 0.882f, 0.62f - 0.38f * sinA);
        // Left wing — back face
        drawWingFace(lw[3][0], lw[3][1], lw[2][0], lw[2][1], lw[1][0], lw[1][1], lw[0][0], lw[0][1],
                    -wT, 0.125f, 0.420f, 0.776f, 0.38f - 0.22f * sinA);
        // Left wing — top edge slab
        float lLit = (0.55f - 0.35f * sinA) * 0.90f;
        glColor3f(0.18f * lLit, 0.45f * lLit, 0.88f * lLit);
        glBegin(GL_QUADS);
        glVertex3f(lw[0][0], lw[0][1],  wT); glVertex3f(lw[3][0], lw[3][1],  wT);
        glVertex3f(lw[3][0], lw[3][1], -wT); glVertex3f(lw[0][0], lw[0][1], -wT);
        glEnd();
        // Left wing — bottom edge slab
        glColor3f(0.10f, 0.33f, 0.70f);
        glBegin(GL_QUADS);
        glVertex3f(lw[1][0], lw[1][1],  wT); glVertex3f(lw[2][0], lw[2][1],  wT);
        glVertex3f(lw[2][0], lw[2][1], -wT); glVertex3f(lw[1][0], lw[1][1], -wT);
        glEnd();
    }

    // ── Part 10: Base trapezoid slab (engine base platform) ──
    {
        float d = RHD;
        glBegin(GL_QUADS);
        // Front face (lighter)
        glColor3f(0.52f, 0.50f, 0.48f);
        glVertex3f(0.37f, 0.23f,  d); glVertex3f(0.32f, 0.20f,  d);
        glVertex3f(0.49f, 0.20f,  d); glVertex3f(0.44f, 0.23f,  d);
        // Back face (darker)
        glColor3f(0.26f, 0.25f, 0.24f);
        glVertex3f(0.44f, 0.23f, -d); glVertex3f(0.49f, 0.20f, -d);
        glVertex3f(0.32f, 0.20f, -d); glVertex3f(0.37f, 0.23f, -d);
        // Top edge
        glColor3f(0.62f, 0.60f, 0.58f);
        glVertex3f(0.37f, 0.23f,  d); glVertex3f(0.44f, 0.23f,  d);
        glVertex3f(0.44f, 0.23f, -d); glVertex3f(0.37f, 0.23f, -d);
        // Bottom edge
        glColor3f(0.34f, 0.33f, 0.32f);
        glVertex3f(0.32f, 0.20f,  d); glVertex3f(0.49f, 0.20f,  d);
        glVertex3f(0.49f, 0.20f, -d); glVertex3f(0.32f, 0.20f, -d);
        glEnd();
    }

    // ── Part 11: Exhaust fire plumes (only when fireOn == true) ──

    glPopMatrix();  // restore matrix before rocket transformations
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         RAFI  —  END                                        ║
// ╚══════════════════════════════════════════════════════════════════════════════╝


// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         RAKA  —  START                                      ║
// ║  Responsibilities:                                                           ║
// ║    • display2(): C-Building (circle architecture)                            ║
// ║    • display2(): Annex Tower (pylons, signal arms, satellite dishes)         ║
// ║    • display2(): Annex 1 & Annex 2 (side buildings with windows)             ║
// ║    • display2(): D-Building (multi-storey main academic block)               ║
// ║    • display2(): Field & Road (ground, road strip, dashed centre line)       ║
// ║    • display2(): Bus Animation (2 buses moving right across road)            ║
// ║    • Bresenham's Line used for D-Building floor separator lines              ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

// ─────────────────────────────────────────────────────────────────────────────
//  RAKA — display2(): Main day scene (campus + rocket launch)
//  Draws sky, sun, clouds, all campus buildings, field, road, buses, and rocket.
// ─────────────────────────────────────────────────────────────────────────────
void display2()
{
    // Clear screen and depth buffer; set white sky background as base
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // ╔══════════════════════════════════════════════════════════════════════╗
    // ║                      SAMSUL — START (in display2)                   ║
    // ║  Draws: Sky gradient, Sun (Midpoint Circle), and Cloud groups 1–4   ║
    // ╚══════════════════════════════════════════════════════════════════════╝

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — SKY GRADIENT
    //  A gradient quad from medium-blue (top-left) → pale blue (bottom)
    //  → deep blue (top-right) simulates daytime sky depth.
    // ─────────────────────────────────────────────────────────────────────
    glBegin(GL_QUADS);
    glColor3f(0.184f, 0.702f, 0.945f); glVertex2f(0.0f, 1.00f);  // top-left  (medium blue)
    glColor3f(0.514f, 0.792f, 0.976f); glVertex2f(0.0f, 0.00f);  // bottom-left (pale blue)
                                        glVertex2f(1.0f, 0.00f);  // bottom-right
    glColor3f(0.000f, 0.502f, 0.757f); glVertex2f(1.0f, 1.00f);  // top-right  (deep blue)
    glEnd();

    // ─────────────────────────────────────────────────────────────────────
    //  Rafi — SUN (Midpoint Circle Algorithm — EP1 Algorithm 1)
    //  Orange filled disc at top-right, drawn with Midpoint rasterisation.
    // ─────────────────────────────────────────────────────────────────────
    glColor3ub(243, 149, 0);                        // orange sun colour
    drawSunMidpoint(0.79f, 0.89f, 0.045f);          // centre (0.79, 0.89), radius 0.045

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — CLOUD GROUP 1 (left side, animated — moves with Position1)
    //  Six overlapping circles form one fluffy cloud.
    // ─────────────────────────────────────────────────────────────────────
    glPushMatrix();
    glTranslatef(Position1, 0.0f, 0.0f);    // horizontal drift driven by update1()
    glColor3f(1.0f, 1.0f, 1.0f);           // all cloud puffs are white
    glBegin(GL_TRIANGLE_FAN);

    // Puff 11 — leftmost puff, bottom row
    GLfloat xc11 = 0.120f, yc11 = 0.92f, rc11 = 0.015f;
    glVertex2f(xc11, yc11);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc11 + rc11 * cos(i * twicePi / triangleAmount),
                   yc11 + rc11 * sin(i * twicePi / triangleAmount));

    // Puff 12 — centre puff, top row
    GLfloat xc12 = 0.140f, yc12 = 0.94f, rc12 = 0.015f;
    glVertex2f(xc12, yc12);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc12 + rc12 * cos(i * twicePi / triangleAmount),
                   yc12 + rc12 * sin(i * twicePi / triangleAmount));

    // Puff 13 — second from left, bottom row
    GLfloat xc13 = 0.135f, yc13 = 0.92f, rc13 = 0.015f;
    glVertex2f(xc13, yc13);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc13 + rc13 * cos(i * twicePi / triangleAmount),
                   yc13 + rc13 * sin(i * twicePi / triangleAmount));

    // Puff 16 — right-centre, top row
    GLfloat xc16 = 0.160f, yc16 = 0.94f, rc16 = 0.015f;
    glVertex2f(xc16, yc16);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc16 + rc16 * cos(i * twicePi / triangleAmount),
                   yc16 + rc16 * sin(i * twicePi / triangleAmount));

    // Puff 17 — second from right, bottom row
    GLfloat xc17 = 0.155f, yc17 = 0.92f, rc17 = 0.015f;
    glVertex2f(xc17, yc17);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc17 + rc17 * cos(i * twicePi / triangleAmount),
                   yc17 + rc17 * sin(i * twicePi / triangleAmount));

    // Puff 18 — rightmost puff, bottom row
    GLfloat xc18 = 0.175f, yc18 = 0.92f, rc18 = 0.015f;
    glVertex2f(xc18, yc18);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc18 + rc18 * cos(i * twicePi / triangleAmount),
                   yc18 + rc18 * sin(i * twicePi / triangleAmount));

    glEnd();

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — CLOUD GROUP 2 (adjacent to group 1, same drift)
    // ─────────────────────────────────────────────────────────────────────
    GLfloat xc21 = 0.210f, yc21 = 0.90f, rc21 = 0.015f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xc21, yc21);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc21 + rc21 * cos(i * twicePi / triangleAmount),
                   yc21 + rc21 * sin(i * twicePi / triangleAmount));

    GLfloat xc22 = 0.230f, yc22 = 0.92f, rc22 = 0.015f;
    glVertex2f(xc22, yc22);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc22 + rc22 * cos(i * twicePi / triangleAmount),
                   yc22 + rc22 * sin(i * twicePi / triangleAmount));

    GLfloat xc23 = 0.225f, yc23 = 0.90f, rc23 = 0.015f;
    glVertex2f(xc23, yc23);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc23 + rc23 * cos(i * twicePi / triangleAmount),
                   yc23 + rc23 * sin(i * twicePi / triangleAmount));

    GLfloat xc26 = 0.250f, yc26 = 0.92f, rc26 = 0.015f;
    glVertex2f(xc26, yc26);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc26 + rc26 * cos(i * twicePi / triangleAmount),
                   yc26 + rc26 * sin(i * twicePi / triangleAmount));

    GLfloat xc27 = 0.245f, yc27 = 0.90f, rc27 = 0.015f;
    glVertex2f(xc27, yc27);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc27 + rc27 * cos(i * twicePi / triangleAmount),
                   yc27 + rc27 * sin(i * twicePi / triangleAmount));

    GLfloat xc28 = 0.265f, yc28 = 0.90f, rc28 = 0.015f;
    glVertex2f(xc28, yc28);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc28 + rc28 * cos(i * twicePi / triangleAmount),
                   yc28 + rc28 * sin(i * twicePi / triangleAmount));
    glEnd();

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — CLOUD GROUP 3 (right side of sky, same drift transform)
    // ─────────────────────────────────────────────────────────────────────
    GLfloat xc31 = 0.720f, yc31 = 0.92f, rc31 = 0.015f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xc31, yc31);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc31 + rc31 * cos(i * twicePi / triangleAmount),
                   yc31 + rc31 * sin(i * twicePi / triangleAmount));

    GLfloat xc32 = 0.740f, yc32 = 0.94f, rc32 = 0.015f;
    glVertex2f(xc32, yc32);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc32 + rc32 * cos(i * twicePi / triangleAmount),
                   yc32 + rc32 * sin(i * twicePi / triangleAmount));

    GLfloat xc33 = 0.735f, yc33 = 0.92f, rc33 = 0.015f;
    glVertex2f(xc33, yc33);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc33 + rc33 * cos(i * twicePi / triangleAmount),
                   yc33 + rc33 * sin(i * twicePi / triangleAmount));

    GLfloat xc36 = 0.760f, yc36 = 0.94f, rc36 = 0.015f;
    glVertex2f(xc36, yc36);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc36 + rc36 * cos(i * twicePi / triangleAmount),
                   yc36 + rc36 * sin(i * twicePi / triangleAmount));

    GLfloat xc37 = 0.755f, yc37 = 0.92f, rc37 = 0.015f;
    glVertex2f(xc37, yc37);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc37 + rc37 * cos(i * twicePi / triangleAmount),
                   yc37 + rc37 * sin(i * twicePi / triangleAmount));

    GLfloat xc38 = 0.775f, yc38 = 0.92f, rc38 = 0.015f;
    glVertex2f(xc38, yc38);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc38 + rc38 * cos(i * twicePi / triangleAmount),
                   yc38 + rc38 * sin(i * twicePi / triangleAmount));
    glEnd();

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — CLOUD GROUP 4 (far right, adjacent to group 3)
    // ─────────────────────────────────────────────────────────────────────
    GLfloat xc41 = 0.810f, yc41 = 0.90f, rc41 = 0.015f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xc41, yc41);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc41 + rc41 * cos(i * twicePi / triangleAmount),
                   yc41 + rc41 * sin(i * twicePi / triangleAmount));

    GLfloat xc42 = 0.830f, yc42 = 0.92f, rc42 = 0.015f;
    glVertex2f(xc42, yc42);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc42 + rc42 * cos(i * twicePi / triangleAmount),
                   yc42 + rc42 * sin(i * twicePi / triangleAmount));

    GLfloat xc43 = 0.825f, yc43 = 0.90f, rc43 = 0.015f;
    glVertex2f(xc43, yc43);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc43 + rc43 * cos(i * twicePi / triangleAmount),
                   yc43 + rc43 * sin(i * twicePi / triangleAmount));

    GLfloat xc46 = 0.850f, yc46 = 0.92f, rc46 = 0.015f;
    glVertex2f(xc46, yc46);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc46 + rc46 * cos(i * twicePi / triangleAmount),
                   yc46 + rc46 * sin(i * twicePi / triangleAmount));

    GLfloat xc47 = 0.845f, yc47 = 0.90f, rc47 = 0.015f;
    glVertex2f(xc47, yc47);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc47 + rc47 * cos(i * twicePi / triangleAmount),
                   yc47 + rc47 * sin(i * twicePi / triangleAmount));

    GLfloat xc48 = 0.865f, yc48 = 0.90f, rc48 = 0.015f;
    glVertex2f(xc48, yc48);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xc48 + rc48 * cos(i * twicePi / triangleAmount),
                   yc48 + rc48 * sin(i * twicePi / triangleAmount));
    glEnd();

    glPopMatrix();  // end cloud drift transform

    // ╔══════════════════════════════════════════════════════════════════════╗
    // ║                    SAMSUL — END (in display2)                        ║
    // ╚══════════════════════════════════════════════════════════════════════╝


    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — C-BUILDING (circular iconic building — left side of campus)
    //  The building is modelled as a circle with window panels cut into it.
    // ─────────────────────────────────────────────────────────────────────────

    // ── C-Building: Circular body (silver/grey) ──
    glLineWidth(1);
    GLfloat xl = -0.05f, yl = 0.41f, rl = 0.15f;   // centre (-0.05, 0.41), radius 0.15
    glColor3f(0.675f, 0.663f, 0.643f);              // silver-grey fill
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xl, yl);
    for (i = 0; i <= triangleAmount; i++)
        glVertex2f(xl + rl * cos(i * twicePi / triangleAmount),
                   yl + rl * sin(i * twicePi / triangleAmount));
    glEnd();

    // ── C-Building: Circle border (dark outline) ──
    GLfloat x2 = -0.05f, y2 = 0.41f, r2 = 0.15f;
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINE_LOOP);
    for (i = 0; i <= lineAmount; i++)
        glVertex2f(x2 + r2 * cos(i * twicePi / lineAmount),
                   y2 + r2 * sin(i * twicePi / lineAmount));
    glEnd();

    // ── C-Building: Window panels (1–3, blue glass) ──
    glBegin(GL_QUADS);
    glColor3f(0.294f, 0.525f, 0.71f);
    // Window 1 — upper slot
    glVertex2f(0.000f, 0.50f); glVertex2f(0.000f, 0.48f);
    glVertex2f(0.084f, 0.48f); glVertex2f(0.070f, 0.50f);
    // Window 2 — middle slot
    glVertex2f(0.000f, 0.44f); glVertex2f(0.000f, 0.42f);
    glVertex2f(0.100f, 0.42f); glVertex2f(0.098f, 0.44f);
    // Window 3 — lower slot
    glVertex2f(0.000f, 0.38f); glVertex2f(0.000f, 0.36f);
    glVertex2f(0.092f, 0.36f); glVertex2f(0.098f, 0.38f);
    glEnd();

    // ── C-Building: Window outlines ──
    glBegin(GL_LINES);
    glColor3f(0.2f, 0.2f, 0.2f);
    // Window 1 edges
    glVertex2f(0.000f, 0.50f); glVertex2f(0.070f, 0.50f);
    glVertex2f(0.000f, 0.48f); glVertex2f(0.083f, 0.48f);
    glVertex2f(0.084f, 0.48f); glVertex2f(0.070f, 0.50f);
    // Window 2 edges
    glVertex2f(0.000f, 0.42f); glVertex2f(0.100f, 0.42f);
    glVertex2f(0.098f, 0.44f); glVertex2f(0.100f, 0.42f);
    glVertex2f(0.098f, 0.44f); glVertex2f(0.000f, 0.44f);
    // Window 3 edges
    glVertex2f(0.000f, 0.38f); glVertex2f(0.098f, 0.38f);
    glVertex2f(0.000f, 0.36f); glVertex2f(0.092f, 0.36f);
    glVertex2f(0.098f, 0.38f); glVertex2f(0.092f, 0.36f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — ANNEX TOWER (lattice communication tower + signal arms)
    // ─────────────────────────────────────────────────────────────────────────

    // ── Tower: Main vertical pillar (two thick parallel lines) ──
    glLineWidth(4);
    glBegin(GL_LINES);
    glColor3f(0.412f, 0.412f, 0.412f);
    glVertex2f(0.210f, 0.60f); glVertex2f(0.210f, 0.20f);   // left rail
    glVertex2f(0.238f, 0.60f); glVertex2f(0.238f, 0.20f);   // right rail
    glVertex2f(0.208f, 0.60f); glVertex2f(0.240f, 0.60f);   // top crossbar
    // Upper X-brace (structural diagonal)
    glVertex2f(0.212f, 0.60f); glVertex2f(0.236f, 0.40f);
    glVertex2f(0.212f, 0.40f); glVertex2f(0.236f, 0.60f);
    // Lower X-brace
    glVertex2f(0.212f, 0.40f); glVertex2f(0.236f, 0.20f);
    glVertex2f(0.212f, 0.20f); glVertex2f(0.236f, 0.40f);
    glEnd();

    // ── Tower: Signal arms (thin lateral projections) ──
    glLineWidth(2);
    glBegin(GL_LINES);
    glColor3f(0.412f, 0.412f, 0.412f);
    // Left signal arm L1
    glVertex2f(0.188f, 0.50f); glVertex2f(0.210f, 0.50f);
    // Right signal arm R1
    glVertex2f(0.260f, 0.56f); glVertex2f(0.238f, 0.56f);
    // Right signal arm R2
    glVertex2f(0.260f, 0.44f); glVertex2f(0.238f, 0.44f);
    glEnd();

    // ── Tower: Satellite dish L1 (circular dish on left arm) ──
    glColor3f(0.086f, 0.149f, 0.149f);
    GLfloat xl1 = 0.190f, yl1 = 0.505f, rl1 = 0.013f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xl1, yl1);
    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(xl1 + rl1 * cos(i * twicePi / triangleAmount),
                   yl1 + rl1 * sin(i * twicePi / triangleAmount));
        glColor3f(0.294f, 0.388f, 0.388f);     // gradient from dark to teal
    }
    glEnd();

    // ── Tower: Satellite dish R1 (upper right arm) ──
    glColor3f(0.086f, 0.149f, 0.149f);
    GLfloat xr11 = 0.260f, yr11 = 0.565f, rr11 = 0.013f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xr11, yr11);
    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(xr11 + rr11 * cos(i * twicePi / triangleAmount),
                   yr11 + rr11 * sin(i * twicePi / triangleAmount));
        glColor3f(0.294f, 0.388f, 0.388f);
    }
    glEnd();

    // ── Tower: Satellite dish R2 (lower right arm) ──
    glColor3f(0.086f, 0.149f, 0.149f);
    GLfloat xr21 = 0.260f, yr21 = 0.445f, rr21 = 0.013f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xr21, yr21);
    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(xr21 + rr21 * cos(i * twicePi / triangleAmount),
                   yr21 + rr21 * sin(i * twicePi / triangleAmount));
        glColor3f(0.294f, 0.388f, 0.388f);
    }
    glEnd();

    // ── Tower: Signal panel R1 (upper right box, blue gradient) ──
    glBegin(GL_QUADS);
    glColor3f(0.275f, 0.690f, 0.910f); glVertex2f(0.247f, 0.585f);
    glColor3f(0.286f, 0.690f, 0.910f); glVertex2f(0.269f, 0.545f);
    glColor3f(0.282f, 0.686f, 0.906f); glVertex2f(0.280f, 0.545f);
    glColor3f(0.271f, 0.682f, 0.906f); glVertex2f(0.280f, 0.585f);
    glEnd();

    // ── Tower: Signal panel R2 (lower right box) ──
    glBegin(GL_QUADS);
    glColor3f(0.314f, 0.702f, 0.914f); glVertex2f(0.269f, 0.425f);
    glColor3f(0.325f, 0.698f, 0.914f); glVertex2f(0.247f, 0.465f);
    glColor3f(0.322f, 0.698f, 0.910f); glVertex2f(0.280f, 0.465f);
    glColor3f(0.310f, 0.694f, 0.910f); glVertex2f(0.280f, 0.425f);
    glEnd();

    // ── Tower: Signal panel L1 (left box) ──
    glBegin(GL_QUADS);
    glColor3f(0.310f, 0.710f, 0.929f); glVertex2f(0.170f, 0.482f);
    glColor3f(0.322f, 0.714f, 0.929f); glVertex2f(0.170f, 0.522f);
    glColor3f(0.322f, 0.714f, 0.925f); glVertex2f(0.203f, 0.522f);
    glColor3f(0.306f, 0.706f, 0.922f); glVertex2f(0.181f, 0.482f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — ANNEX 2 (small service building to the left of Annex Tower)
    // ─────────────────────────────────────────────────────────────────────────

    // ── Annex 2: Main body (dark grey) ──
    glBegin(GL_QUADS);
    glColor3f(0.318f, 0.341f, 0.341f);
    glVertex2f(0.135f, 0.36f); glVertex2f(0.135f, 0.20f);
    glVertex2f(0.220f, 0.20f); glVertex2f(0.220f, 0.36f);

    // ── Annex 2: Sloped roof (cream/yellow tone) ──
    glColor3f(0.910f, 0.902f, 0.745f);
    glVertex2f(0.135f, 0.38f); glVertex2f(0.125f, 0.34f);
    glVertex2f(0.230f, 0.34f); glVertex2f(0.220f, 0.38f);
    glEnd();

    // ── Annex 2: Windows (1–2, cream colour) ──
    glBegin(GL_QUADS);
    glColor3f(0.910f, 0.902f, 0.745f);
    // Window 1 (right side)
    glVertex2f(0.188f, 0.32f); glVertex2f(0.188f, 0.30f);
    glVertex2f(0.203f, 0.30f); glVertex2f(0.203f, 0.32f);
    // Window 2 (left side)
    glVertex2f(0.150f, 0.32f); glVertex2f(0.150f, 0.30f);
    glVertex2f(0.165f, 0.30f); glVertex2f(0.165f, 0.32f);
    glEnd();

    // ── Annex 2: Roof border outline ──
    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.135f, 0.38f); glVertex2f(0.125f, 0.34f);
    glVertex2f(0.230f, 0.34f); glVertex2f(0.220f, 0.38f);
    glVertex2f(0.125f, 0.34f); glVertex2f(0.230f, 0.34f);
    glVertex2f(0.135f, 0.38f); glVertex2f(0.220f, 0.38f);
    // Window 1 border
    glVertex2f(0.188f, 0.32f); glVertex2f(0.188f, 0.30f);
    glVertex2f(0.203f, 0.30f); glVertex2f(0.203f, 0.32f);
    glVertex2f(0.188f, 0.32f); glVertex2f(0.203f, 0.32f);
    glVertex2f(0.188f, 0.30f); glVertex2f(0.203f, 0.30f);
    glVertex2f(0.196f, 0.32f); glVertex2f(0.196f, 0.30f);  // centre divider
    // Window 2 border
    glVertex2f(0.150f, 0.32f); glVertex2f(0.150f, 0.30f);
    glVertex2f(0.165f, 0.30f); glVertex2f(0.165f, 0.32f);
    glVertex2f(0.150f, 0.32f); glVertex2f(0.165f, 0.32f);
    glVertex2f(0.150f, 0.30f); glVertex2f(0.165f, 0.30f);
    glVertex2f(0.158f, 0.32f); glVertex2f(0.158f, 0.30f);  // centre divider
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — ANNEX 1 (long low building — ground-floor corridor)
    // ─────────────────────────────────────────────────────────────────────────

    // ── Annex 1: Main body (terracotta / brown) ──
    glBegin(GL_QUADS);
    glColor3f(0.647f, 0.486f, 0.412f);
    glVertex2f(0.00f, 0.26f); glVertex2f(0.00f, 0.20f);
    glVertex2f(0.20f, 0.20f); glVertex2f(0.20f, 0.26f);

    // ── Annex 1: Green roof band ──
    glColor3f(0.039f, 0.459f, 0.020f);
    glVertex2f(0.00f, 0.29f); glVertex2f(0.00f, 0.26f);
    glVertex2f(0.21f, 0.26f); glVertex2f(0.20f, 0.29f);

    // ── Annex 1: Cream cornice under roof ──
    glColor3f(0.910f, 0.902f, 0.745f);
    glVertex2f(0.00f, 0.26f); glVertex2f(0.00f, 0.256f);
    glVertex2f(0.20f, 0.256f); glVertex2f(0.20f, 0.260f);

    // ── Annex 1: Windows 1–5 (cream, equally spaced) ──
    glColor3f(0.910f, 0.902f, 0.745f);
    glVertex2f(0.000f, 0.24f); glVertex2f(0.000f, 0.22f); glVertex2f(0.015f, 0.22f); glVertex2f(0.015f, 0.24f);
    glVertex2f(0.045f, 0.24f); glVertex2f(0.045f, 0.22f); glVertex2f(0.060f, 0.22f); glVertex2f(0.060f, 0.24f);
    glVertex2f(0.090f, 0.24f); glVertex2f(0.090f, 0.22f); glVertex2f(0.105f, 0.22f); glVertex2f(0.105f, 0.24f);
    glVertex2f(0.135f, 0.24f); glVertex2f(0.135f, 0.22f); glVertex2f(0.150f, 0.22f); glVertex2f(0.150f, 0.24f);
    glVertex2f(0.180f, 0.24f); glVertex2f(0.180f, 0.22f); glVertex2f(0.195f, 0.22f); glVertex2f(0.195f, 0.24f);
    glEnd();

    // ── Annex 1: All outlines (roof, body, windows + dividers) ──
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    // Roof outline
    glVertex2f(0.00f,0.29f); glVertex2f(0.00f,0.26f);
    glVertex2f(0.21f,0.26f); glVertex2f(0.20f,0.29f);
    glVertex2f(0.00f,0.29f); glVertex2f(0.20f,0.29f);
    glVertex2f(0.00f,0.26f); glVertex2f(0.21f,0.26f);
    // Body outline
    glVertex2f(0.00f,0.26f); glVertex2f(0.00f,0.20f);
    glVertex2f(0.20f,0.20f); glVertex2f(0.20f,0.26f);
    glVertex2f(0.00f,0.20f); glVertex2f(0.20f,0.20f);
    glVertex2f(0.00f,0.26f); glVertex2f(0.20f,0.26f);
    // Window 1 outline + divider
    glVertex2f(0.000f,0.24f); glVertex2f(0.000f,0.22f);
    glVertex2f(0.015f,0.22f); glVertex2f(0.015f,0.24f);
    glVertex2f(0.015f,0.22f); glVertex2f(0.000f,0.22f);
    glVertex2f(0.015f,0.24f); glVertex2f(0.000f,0.24f);
    glVertex2f(0.007f,0.22f); glVertex2f(0.007f,0.24f);
    // Window 2 outline + divider
    glVertex2f(0.045f,0.24f); glVertex2f(0.045f,0.22f);
    glVertex2f(0.060f,0.22f); glVertex2f(0.060f,0.24f);
    glVertex2f(0.045f,0.24f); glVertex2f(0.060f,0.24f);
    glVertex2f(0.045f,0.22f); glVertex2f(0.060f,0.22f);
    glVertex2f(0.053f,0.22f); glVertex2f(0.053f,0.24f);
    // Window 3 outline + divider
    glVertex2f(0.090f,0.24f); glVertex2f(0.090f,0.22f);
    glVertex2f(0.105f,0.22f); glVertex2f(0.105f,0.24f);
    glVertex2f(0.090f,0.24f); glVertex2f(0.105f,0.24f);
    glVertex2f(0.090f,0.22f); glVertex2f(0.105f,0.22f);
    glVertex2f(0.098f,0.22f); glVertex2f(0.098f,0.24f);
    // Window 4 outline + divider
    glVertex2f(0.135f,0.24f); glVertex2f(0.135f,0.22f);
    glVertex2f(0.150f,0.22f); glVertex2f(0.150f,0.24f);
    glVertex2f(0.135f,0.24f); glVertex2f(0.150f,0.24f);
    glVertex2f(0.135f,0.22f); glVertex2f(0.150f,0.22f);
    glVertex2f(0.143f,0.22f); glVertex2f(0.143f,0.24f);
    // Window 5 outline + divider
    glVertex2f(0.180f,0.24f); glVertex2f(0.180f,0.22f);
    glVertex2f(0.195f,0.22f); glVertex2f(0.195f,0.24f);
    glVertex2f(0.180f,0.24f); glVertex2f(0.195f,0.24f);
    glVertex2f(0.180f,0.22f); glVertex2f(0.195f,0.22f);
    glVertex2f(0.188f,0.22f); glVertex2f(0.188f,0.24f);
    // Body partition lines (vertical bay dividers)
    glVertex2f(0.120f,0.26f); glVertex2f(0.120f,0.20f);
    glVertex2f(0.030f,0.26f); glVertex2f(0.030f,0.20f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAFI — ROCKET STAND + 3D ROCKET (fixed stand; rocket body moves up)
    //  draw3DRocket() is called here — showStand=true (display2 only),
    //  fireOn=rocketLaunching (fire shown only when 'n' is pressed).
    // ─────────────────────────────────────────────────────────────────────────
    draw3DRocket(Position2, true, rocketLaunching);

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — D-BUILDING (large multi-storey main academic block, right side)
    // ─────────────────────────────────────────────────────────────────────────

    // ── D-Building: Floor panel geometry (blue glass + grey panels) ──
    glBegin(GL_QUADS);

    // Panel 1: right-side vertical strip (deep blue)
    glColor3f(0.196f, 0.376f, 0.510f);
    glVertex2f(0.95f, 0.20f); glVertex2f(0.95f, 0.61f);
    glVertex2f(1.00f, 0.61f); glVertex2f(1.00f, 0.20f);

    // Panel 2: main body (silver-grey)
    glColor3f(0.765f, 0.761f, 0.769f);
    glVertex2f(0.74f, 0.20f); glVertex2f(0.74f, 0.58f);
    glVertex2f(0.98f, 0.58f); glVertex2f(0.98f, 0.20f);

    // Panels 3–13: horizontal blue glass bands and sub-sections
    glColor3f(0.294f, 0.525f, 0.710f);
    glVertex2f(0.74f, 0.545f); glVertex2f(0.74f, 0.565f); glVertex2f(0.98f, 0.565f); glVertex2f(0.98f, 0.545f);
    glVertex2f(0.74f, 0.530f); glVertex2f(0.74f, 0.510f); glVertex2f(0.98f, 0.510f); glVertex2f(0.98f, 0.530f);
    glVertex2f(0.75f, 0.495f); glVertex2f(0.75f, 0.445f); glVertex2f(0.98f, 0.445f); glVertex2f(0.98f, 0.495f);
    glVertex2f(0.74f, 0.430f); glVertex2f(0.74f, 0.495f); glVertex2f(0.75f, 0.495f); glVertex2f(0.75f, 0.445f);
    glVertex2f(0.75f, 0.410f); glVertex2f(0.98f, 0.410f); glVertex2f(0.98f, 0.430f); glVertex2f(0.75f, 0.430f);
    glVertex2f(0.76f, 0.390f); glVertex2f(0.76f, 0.340f); glVertex2f(0.98f, 0.340f); glVertex2f(0.98f, 0.390f);
    glVertex2f(0.75f, 0.390f); glVertex2f(0.75f, 0.325f); glVertex2f(0.76f, 0.340f); glVertex2f(0.76f, 0.390f);
    glVertex2f(0.76f, 0.325f); glVertex2f(0.76f, 0.305f); glVertex2f(0.98f, 0.305f); glVertex2f(0.98f, 0.325f);
    glVertex2f(0.76f, 0.290f); glVertex2f(0.76f, 0.270f); glVertex2f(0.98f, 0.270f); glVertex2f(0.98f, 0.290f);
    glVertex2f(0.76f, 0.255f); glVertex2f(0.76f, 0.230f); glVertex2f(0.98f, 0.230f); glVertex2f(0.98f, 0.255f);
    glVertex2f(0.76f, 0.212f); glVertex2f(0.76f, 0.200f); glVertex2f(0.98f, 0.200f); glVertex2f(0.98f, 0.212f);

    // Panel 14: left wing of D-building
    glColor3f(0.765f, 0.761f, 0.769f);
    glVertex2f(0.68f, 0.555f); glVertex2f(0.68f, 0.200f);
    glVertex2f(0.74f, 0.200f); glVertex2f(0.74f, 0.580f);

    // Panels 15–17: narrow accent strips (blue glass)
    glColor3f(0.294f, 0.525f, 0.710f);
    glVertex2f(0.690f, 0.544f); glVertex2f(0.690f, 0.200f); glVertex2f(0.732f, 0.200f); glVertex2f(0.732f, 0.560f);
    glVertex2f(0.730f, 0.400f); glVertex2f(0.730f, 0.200f); glVertex2f(0.743f, 0.200f); glVertex2f(0.743f, 0.420f);
    glVertex2f(0.730f, 0.285f); glVertex2f(0.730f, 0.200f); glVertex2f(0.753f, 0.200f); glVertex2f(0.753f, 0.315f);

    glEnd();

    // ── D-Building: Side windows (small vertical slots in right edge strip) ──
    glBegin(GL_QUADS);
    glColor3f(0.298f, 0.510f, 0.671f);
    glVertex2f(0.987f, 0.56f); glVertex2f(0.987f, 0.54f); glVertex2f(0.993f, 0.54f); glVertex2f(0.993f, 0.56f);
    glVertex2f(0.987f, 0.52f); glVertex2f(0.987f, 0.50f); glVertex2f(0.993f, 0.50f); glVertex2f(0.993f, 0.52f);
    glVertex2f(0.987f, 0.48f); glVertex2f(0.987f, 0.46f); glVertex2f(0.993f, 0.46f); glVertex2f(0.993f, 0.48f);
    glVertex2f(0.987f, 0.44f); glVertex2f(0.987f, 0.42f); glVertex2f(0.993f, 0.42f); glVertex2f(0.993f, 0.44f);
    glVertex2f(0.987f, 0.40f); glVertex2f(0.987f, 0.38f); glVertex2f(0.993f, 0.38f); glVertex2f(0.993f, 0.40f);
    glVertex2f(0.987f, 0.36f); glVertex2f(0.987f, 0.34f); glVertex2f(0.993f, 0.34f); glVertex2f(0.993f, 0.36f);
    glVertex2f(0.987f, 0.32f); glVertex2f(0.987f, 0.30f); glVertex2f(0.993f, 0.30f); glVertex2f(0.993f, 0.32f);
    glVertex2f(0.987f, 0.28f); glVertex2f(0.987f, 0.26f); glVertex2f(0.993f, 0.26f); glVertex2f(0.993f, 0.28f);
    glVertex2f(0.987f, 0.24f); glVertex2f(0.987f, 0.22f); glVertex2f(0.993f, 0.22f); glVertex2f(0.993f, 0.24f);
    glEnd();

    // ── D-Building: Structural outlines ──
    glLineWidth(1.4f);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glEnd();

    // ── D-Building: Floor separator lines (EP1 — Bresenham's Line Algorithm) ──
    // 7th floor horizontal separator drawn using Bresenham's algorithm
    drawLineBresenham(0.98f, 0.453f, 0.75f, 0.453f);

    // Remaining structural edge lines (standard GL_LINES)
    glBegin(GL_LINES);
    // 7th floor sub-verticals
    glVertex2f(0.75f, 0.453f); glVertex2f(0.74f, 0.439f);
    glVertex2f(0.75f, 0.453f); glVertex2f(0.75f, 0.445f);
    glVertex2f(0.81f, 0.453f); glVertex2f(0.81f, 0.445f);
    glVertex2f(0.87f, 0.453f); glVertex2f(0.87f, 0.445f);
    glVertex2f(0.93f, 0.453f); glVertex2f(0.93f, 0.445f);
    // 5th floor separator
    glVertex2f(0.76f, 0.348f); glVertex2f(0.98f, 0.348f);
    glVertex2f(0.76f, 0.348f); glVertex2f(0.75f, 0.335f);
    glVertex2f(0.76f, 0.348f); glVertex2f(0.76f, 0.340f);
    glVertex2f(0.81f, 0.348f); glVertex2f(0.81f, 0.340f);
    glVertex2f(0.87f, 0.348f); glVertex2f(0.87f, 0.340f);
    glVertex2f(0.93f, 0.348f); glVertex2f(0.93f, 0.340f);
    // Panel 1 edges
    glVertex2f(0.95f, 0.58f); glVertex2f(0.95f, 0.61f);
    glVertex2f(0.95f, 0.61f); glVertex2f(1.00f, 0.61f);
    glVertex2f(0.98f, 0.20f); glVertex2f(0.98f, 0.58f);
    // Panel 2 top edge
    glVertex2f(0.74f, 0.58f); glVertex2f(0.98f, 0.58f);
    // Bands 3–4
    glVertex2f(0.74f, 0.545f); glVertex2f(0.98f, 0.545f);
    glVertex2f(0.74f, 0.565f); glVertex2f(0.98f, 0.565f);
    glVertex2f(0.74f, 0.530f); glVertex2f(0.98f, 0.530f);
    glVertex2f(0.74f, 0.510f); glVertex2f(0.98f, 0.510f);
    // 7th floor zone
    glVertex2f(0.75f, 0.495f); glVertex2f(0.98f, 0.495f);
    glVertex2f(0.75f, 0.445f); glVertex2f(0.98f, 0.445f);
    glVertex2f(0.74f, 0.430f); glVertex2f(0.75f, 0.445f);
    glVertex2f(0.74f, 0.495f); glVertex2f(0.75f, 0.495f);
    glVertex2f(0.75f, 0.410f); glVertex2f(0.98f, 0.410f);
    glVertex2f(0.98f, 0.430f); glVertex2f(0.75f, 0.430f);
    glVertex2f(0.75f, 0.410f); glVertex2f(0.75f, 0.430f);
    // 5th floor zone
    glVertex2f(0.76f, 0.390f); glVertex2f(0.98f, 0.390f);
    glVertex2f(0.76f, 0.340f); glVertex2f(0.98f, 0.340f);
    glVertex2f(0.75f, 0.390f); glVertex2f(0.76f, 0.390f);
    glVertex2f(0.75f, 0.325f); glVertex2f(0.76f, 0.340f);
    glVertex2f(0.75f, 0.390f); glVertex2f(0.75f, 0.325f);
    // Lower floors
    glVertex2f(0.76f, 0.325f); glVertex2f(0.98f, 0.325f);
    glVertex2f(0.76f, 0.305f); glVertex2f(0.98f, 0.305f);
    glVertex2f(0.76f, 0.305f); glVertex2f(0.76f, 0.325f);
    glVertex2f(0.76f, 0.290f); glVertex2f(0.98f, 0.290f);
    glVertex2f(0.76f, 0.270f); glVertex2f(0.98f, 0.270f);
    glVertex2f(0.76f, 0.290f); glVertex2f(0.76f, 0.270f);
    glVertex2f(0.76f, 0.255f); glVertex2f(0.98f, 0.255f);
    glVertex2f(0.76f, 0.230f); glVertex2f(0.98f, 0.230f);
    glVertex2f(0.76f, 0.255f); glVertex2f(0.76f, 0.230f);
    glVertex2f(0.76f, 0.212f); glVertex2f(0.98f, 0.212f);
    glVertex2f(0.76f, 0.200f); glVertex2f(0.98f, 0.200f);
    glVertex2f(0.76f, 0.212f); glVertex2f(0.76f, 0.200f);
    // Left wing panel outlines
    glVertex2f(0.68f, 0.555f); glVertex2f(0.68f, 0.200f);
    glVertex2f(0.74f, 0.430f); glVertex2f(0.74f, 0.580f);
    glVertex2f(0.68f, 0.555f); glVertex2f(0.74f, 0.580f);
    // Strip 15
    glVertex2f(0.690f, 0.544f); glVertex2f(0.690f, 0.200f);
    glVertex2f(0.732f, 0.405f); glVertex2f(0.732f, 0.560f);
    glVertex2f(0.690f, 0.544f); glVertex2f(0.732f, 0.560f);
    glVertex2f(0.690f, 0.200f); glVertex2f(0.732f, 0.200f);
    glVertex2f(0.732f, 0.403f); glVertex2f(0.743f, 0.420f);
    // Strip 16
    glVertex2f(0.743f, 0.300f); glVertex2f(0.743f, 0.420f);
    // Strip 17
    glVertex2f(0.743f, 0.300f); glVertex2f(0.753f, 0.315f);
    glVertex2f(0.753f, 0.200f); glVertex2f(0.753f, 0.315f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — GRASS FIELD (green gradient ground in front of buildings)
    // ─────────────────────────────────────────────────────────────────────────
    glBegin(GL_QUADS);
    glColor3f(0.690f, 0.851f, 0.310f); glVertex2f(0.0f, 0.16f);   // top-left  (light green)
    glColor3f(0.396f, 0.671f, 0.141f); glVertex2f(0.0f, 0.00f);   // bottom-left (dark green)
                                        glVertex2f(1.0f, 0.00f);   // bottom-right
    glColor3f(0.690f, 0.851f, 0.310f); glVertex2f(1.0f, 0.16f);   // top-right
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — ROAD (dark asphalt strip between buildings and field)
    // ─────────────────────────────────────────────────────────────────────────
    glBegin(GL_QUADS);
    glColor3f(0.004f, 0.000f, 0.004f);  // near-black asphalt
    glVertex2f(0.0f, 0.20f); glVertex2f(0.0f, 0.16f);
    glVertex2f(1.0f, 0.16f); glVertex2f(1.0f, 0.20f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — ROAD CENTRE DASHES (white dashed line on road)
    // ─────────────────────────────────────────────────────────────────────────
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(0.10f, 0.18f); glVertex2f(0.20f, 0.18f);   // dash 1
    glVertex2f(0.30f, 0.18f); glVertex2f(0.40f, 0.18f);   // dash 2
    glVertex2f(0.50f, 0.18f); glVertex2f(0.60f, 0.18f);   // dash 3
    glVertex2f(0.70f, 0.18f); glVertex2f(0.80f, 0.18f);   // dash 4
    glVertex2f(0.90f, 0.18f); glVertex2f(1.00f, 0.18f);   // dash 5
    glEnd();


    // ╔══════════════════════════════════════════════════════════════════════╗
    // ║                    SAMSUL — START (Trees in display2)               ║
    // ║  Draws 4 trees along the field — each with trunk + 3 leaf layers    ║
    // ║  + triangular crown.                                                ║
    // ╚══════════════════════════════════════════════════════════════════════╝

    // ── Tree 1 (far left) ──
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.235f, 0.012f);                   // brown trunk
    glVertex2f(0.049f,0.00f); glVertex2f(0.049f,0.02f);
    glVertex2f(0.052f,0.02f); glVertex2f(0.052f,0.00f);
    glColor3f(0.063f, 0.510f, 0.063f);                  // dark green leaves
    glVertex2f(0.075f,0.02f); glVertex2f(0.075f,0.04f); glVertex2f(0.025f,0.04f); glVertex2f(0.025f,0.02f); // layer 1
    glVertex2f(0.030f,0.04f); glVertex2f(0.030f,0.06f); glVertex2f(0.070f,0.06f); glVertex2f(0.070f,0.04f); // layer 2
    glVertex2f(0.035f,0.06f); glVertex2f(0.035f,0.08f); glVertex2f(0.067f,0.08f); glVertex2f(0.067f,0.06f); // layer 3
    glEnd();
    glBegin(GL_TRIANGLES);
    glColor3f(0.063f, 0.510f, 0.063f);
    glVertex2f(0.040f,0.08f); glVertex2f(0.051f,0.12f); glVertex2f(0.062f,0.08f); // crown
    glEnd();

    // ── Tree 2 (second from left) ──
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.235f, 0.012f);
    glVertex2f(0.109f,0.00f); glVertex2f(0.109f,0.02f);
    glVertex2f(0.112f,0.02f); glVertex2f(0.112f,0.00f);
    glBegin(GL_QUADS);
    glColor3f(0.063f, 0.510f, 0.063f);
    glVertex2f(0.085f,0.02f); glVertex2f(0.085f,0.04f); glVertex2f(0.135f,0.04f); glVertex2f(0.135f,0.02f);
    glVertex2f(0.090f,0.04f); glVertex2f(0.090f,0.06f); glVertex2f(0.130f,0.06f); glVertex2f(0.130f,0.04f);
    glVertex2f(0.095f,0.06f); glVertex2f(0.095f,0.08f); glVertex2f(0.125f,0.08f); glVertex2f(0.125f,0.06f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glColor3f(0.063f, 0.510f, 0.063f);
    glVertex2f(0.100f,0.08f); glVertex2f(0.110f,0.12f); glVertex2f(0.120f,0.08f);
    glEnd();

    // ── Tree 3 (second from right) ──
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.235f, 0.012f);
    glVertex2f(0.950f,0.00f); glVertex2f(0.950f,0.02f);
    glVertex2f(0.953f,0.02f); glVertex2f(0.953f,0.00f);
    glBegin(GL_QUADS);
    glColor3f(0.063f, 0.510f, 0.063f);
    glVertex2f(0.925f,0.02f); glVertex2f(0.925f,0.04f); glVertex2f(0.975f,0.04f); glVertex2f(0.975f,0.02f);
    glVertex2f(0.930f,0.04f); glVertex2f(0.930f,0.06f); glVertex2f(0.970f,0.06f); glVertex2f(0.970f,0.04f);
    glVertex2f(0.935f,0.06f); glVertex2f(0.935f,0.08f); glVertex2f(0.965f,0.08f); glVertex2f(0.965f,0.06f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glColor3f(0.063f, 0.510f, 0.063f);
    glVertex2f(0.940f,0.08f); glVertex2f(0.950f,0.12f); glVertex2f(0.960f,0.08f);
    glEnd();

    // ── Tree 4 (far right) ──
    glBegin(GL_QUADS);
    glColor3f(0.42f, 0.235f, 0.012f);
    glVertex2f(0.890f,0.00f); glVertex2f(0.890f,0.02f);
    glVertex2f(0.893f,0.02f); glVertex2f(0.893f,0.00f);
    glBegin(GL_QUADS);
    glColor3f(0.063f, 0.510f, 0.063f);
    glVertex2f(0.915f,0.02f); glVertex2f(0.915f,0.04f); glVertex2f(0.865f,0.04f); glVertex2f(0.865f,0.02f);
    glVertex2f(0.910f,0.04f); glVertex2f(0.910f,0.06f); glVertex2f(0.870f,0.06f); glVertex2f(0.870f,0.04f);
    glVertex2f(0.905f,0.06f); glVertex2f(0.905f,0.08f); glVertex2f(0.875f,0.08f); glVertex2f(0.875f,0.06f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glColor3f(0.063f, 0.510f, 0.063f);
    glVertex2f(0.900f,0.08f); glVertex2f(0.890f,0.12f); glVertex2f(0.880f,0.08f);
    glEnd();

    // ╔══════════════════════════════════════════════════════════════════════╗
    // ║                    SAMSUL — END (Trees in display2)                 ║
    // ╚══════════════════════════════════════════════════════════════════════╝


    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — TRANSPARENT LAUNCH TUBE SUPPORT MARKERS
    //  Two T-shaped white markers flank the rocket tube base (visual reference).
    // ─────────────────────────────────────────────────────────────────────────
    glLineWidth(2);
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);
    // Right marker
    glVertex2f(0.45f, 0.20f); glVertex2f(0.45f, 0.27f);
    glVertex2f(0.44f, 0.27f); glVertex2f(0.46f, 0.27f);
    glVertex2f(0.44f, 0.27f); glVertex2f(0.46f, 0.27f);  // top crossbar (repeated for thickness)
    // Left marker
    glVertex2f(0.36f, 0.20f); glVertex2f(0.36f, 0.27f);
    glVertex2f(0.35f, 0.27f); glVertex2f(0.37f, 0.27f);
    glVertex2f(0.35f, 0.27f); glVertex2f(0.37f, 0.27f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────────
    //  RAKA — BUS ANIMATION
    //  Two buses travel right across the road (Position3 driven by update3()).
    //  Bus 1 is red; Bus 2 is blue, offset +0.4 from Bus 1.
    //  Each bus: body + window strip + 7 windows + door + side mirror + 2 tires.
    // ─────────────────────────────────────────────────────────────────────────
    glPushMatrix();
    glTranslatef(Position3, 0.0f, 0.0f);    // horizontal travel driven by update3()

    // ── Bus 1 (red) ──
    glBegin(GL_QUADS);
    glColor3f(0.780f, 0.137f, 0.204f);     // red body
    glVertex2f(0.14f, 0.175f); glVertex2f(0.14f, 0.225f);
    glVertex2f(0.27f, 0.225f); glVertex2f(0.27f, 0.175f);
    // Front nose extension
    glVertex2f(0.270f, 0.175f); glVertex2f(0.273f, 0.175f);
    glVertex2f(0.273f, 0.200f); glVertex2f(0.270f, 0.205f);
    // Green window frame strip
    glColor3f(0.031f, 0.125f, 0.059f);
    glVertex2f(0.143f, 0.205f); glVertex2f(0.143f, 0.223f);
    glVertex2f(0.268f, 0.223f); glVertex2f(0.268f, 0.205f);
    // Windows 1–7 (glass blue-grey)
    glColor3f(0.655f, 0.725f, 0.725f);
    glVertex2f(0.1445f,0.207f); glVertex2f(0.1445f,0.221f); glVertex2f(0.157f,0.221f); glVertex2f(0.157f,0.207f);
    glVertex2f(0.159f, 0.207f); glVertex2f(0.159f, 0.221f); glVertex2f(0.173f,0.221f); glVertex2f(0.173f,0.207f);
    glVertex2f(0.175f, 0.207f); glVertex2f(0.175f, 0.221f); glVertex2f(0.189f,0.221f); glVertex2f(0.189f,0.207f);
    glVertex2f(0.191f, 0.207f); glVertex2f(0.191f, 0.221f); glVertex2f(0.205f,0.221f); glVertex2f(0.205f,0.207f);
    glVertex2f(0.207f, 0.207f); glVertex2f(0.207f, 0.221f); glVertex2f(0.222f,0.221f); glVertex2f(0.222f,0.207f);
    glVertex2f(0.245f, 0.207f); glVertex2f(0.245f, 0.221f); glVertex2f(0.258f,0.221f); glVertex2f(0.258f,0.207f);
    glVertex2f(0.260f, 0.207f); glVertex2f(0.260f, 0.221f); glVertex2f(0.266f,0.221f); glVertex2f(0.266f,0.207f);
    // Door panel
    glVertex2f(0.225f, 0.175f); glVertex2f(0.225f, 0.217f);
    glVertex2f(0.243f, 0.217f); glVertex2f(0.243f, 0.175f);
    glEnd();
    // Door centre line
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.234f, 0.175f); glVertex2f(0.234f, 0.219f);
    // Side mirror stem
    glColor3f(0.780f, 0.137f, 0.204f);
    glVertex2f(0.270f, 0.220f); glVertex2f(0.277f, 0.220f);
    glEnd();
    // Side mirror glass (thick black line)
    glLineWidth(6);
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(0.277f, 0.220f); glVertex2f(0.277f, 0.205f);
    glEnd();
    // Bus 1 right tire (outer + alloy)
    GLfloat Ak50=0.257f, Pk50=0.175f, Ok50=0.008f;
    glColor3f(0.0f,0.0f,0.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak50,Pk50);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak50+Ok50*cos(i*twicePi/triangleAmount), Pk50+Ok50*sin(i*twicePi/triangleAmount));
    glEnd();
    GLfloat Ak51=0.257f, Pk51=0.175f, Ok51=0.004f;
    glColor3f(1.0f,1.0f,1.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak51,Pk51);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak51+Ok51*cos(i*twicePi/triangleAmount), Pk51+Ok51*sin(i*twicePi/triangleAmount));
    glEnd();
    // Bus 1 left tire
    GLfloat Ak52=0.166f, Pk52=0.175f, Ok52=0.008f;
    glColor3f(0.0f,0.0f,0.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak52,Pk52);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak52+Ok52*cos(i*twicePi/triangleAmount), Pk52+Ok52*sin(i*twicePi/triangleAmount));
    glEnd();
    GLfloat Ak53=0.166f, Pk53=0.175f, Ok53=0.004f;
    glColor3f(1.0f,1.0f,1.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak53,Pk53);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak53+Ok53*cos(i*twicePi/triangleAmount), Pk53+Ok53*sin(i*twicePi/triangleAmount));
    glEnd();

    // ── Bus 2 (blue, offset +0.4 in X within same glPushMatrix) ──
    glPushMatrix();
    glTranslatef(+0.4f, 0.0f, 0.0f);   // Bus 2 is 0.4 units ahead of Bus 1

    glBegin(GL_QUADS);
    glColor3f(0.114f, 0.180f, 0.612f);  // blue body
    glVertex2f(0.14f,0.175f); glVertex2f(0.14f,0.225f);
    glVertex2f(0.27f,0.225f); glVertex2f(0.27f,0.175f);
    // Front nose extension
    glVertex2f(0.270f,0.175f); glVertex2f(0.273f,0.175f);
    glVertex2f(0.273f,0.200f); glVertex2f(0.270f,0.205f);
    // Green window frame strip
    glColor3f(0.031f, 0.125f, 0.059f);
    glVertex2f(0.143f,0.205f); glVertex2f(0.143f,0.223f);
    glVertex2f(0.268f,0.223f); glVertex2f(0.268f,0.205f);
    // Windows 1–7
    glColor3f(0.655f, 0.725f, 0.725f);
    glVertex2f(0.1445f,0.207f); glVertex2f(0.1445f,0.221f); glVertex2f(0.157f,0.221f); glVertex2f(0.157f,0.207f);
    glVertex2f(0.159f, 0.207f); glVertex2f(0.159f, 0.221f); glVertex2f(0.173f,0.221f); glVertex2f(0.173f,0.207f);
    glVertex2f(0.175f, 0.207f); glVertex2f(0.175f, 0.221f); glVertex2f(0.189f,0.221f); glVertex2f(0.189f,0.207f);
    glVertex2f(0.191f, 0.207f); glVertex2f(0.191f, 0.221f); glVertex2f(0.205f,0.221f); glVertex2f(0.205f,0.207f);
    glVertex2f(0.207f, 0.207f); glVertex2f(0.207f, 0.221f); glVertex2f(0.222f,0.221f); glVertex2f(0.222f,0.207f);
    glVertex2f(0.245f, 0.207f); glVertex2f(0.245f, 0.221f); glVertex2f(0.258f,0.221f); glVertex2f(0.258f,0.207f);
    glVertex2f(0.260f, 0.207f); glVertex2f(0.260f, 0.221f); glVertex2f(0.266f,0.221f); glVertex2f(0.266f,0.207f);
    // Door
    glVertex2f(0.225f,0.175f); glVertex2f(0.225f,0.217f);
    glVertex2f(0.243f,0.217f); glVertex2f(0.243f,0.175f);
    glEnd();
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glColor3f(0.0f,0.0f,0.0f);
    glVertex2f(0.234f,0.175f); glVertex2f(0.234f,0.219f);
    glColor3f(0.114f,0.180f,0.612f);
    glVertex2f(0.270f,0.220f); glVertex2f(0.277f,0.220f);
    glEnd();
    glLineWidth(6);
    glBegin(GL_LINES);
    glColor3f(0.0f,0.0f,0.0f);
    glVertex2f(0.277f,0.220f); glVertex2f(0.277f,0.205f);
    glEnd();
    // Bus 2 right tire
    GLfloat Ak54=0.257f, Pk54=0.175f, Ok54=0.008f;
    glColor3f(0.0f,0.0f,0.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak54,Pk54);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak54+Ok54*cos(i*twicePi/triangleAmount), Pk54+Ok54*sin(i*twicePi/triangleAmount));
    glEnd();
    GLfloat Ak55=0.257f, Pk55=0.175f, Ok55=0.004f;
    glColor3f(1.0f,1.0f,1.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak55,Pk55);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak55+Ok55*cos(i*twicePi/triangleAmount), Pk55+Ok55*sin(i*twicePi/triangleAmount));
    glEnd();
    // Bus 2 left tire
    GLfloat Ak56=0.166f, Pk56=0.175f, Ok56=0.008f;
    glColor3f(0.0f,0.0f,0.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak56,Pk56);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak56+Ok56*cos(i*twicePi/triangleAmount), Pk56+Ok56*sin(i*twicePi/triangleAmount));
    glEnd();
    GLfloat Ak57=0.166f, Pk57=0.175f, Ok57=0.004f;
    glColor3f(1.0f,1.0f,1.0f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(Ak57,Pk57);
    for (int i=0; i<=triangleAmount; i++)
        glVertex2f(Ak57+Ok57*cos(i*twicePi/triangleAmount), Pk57+Ok57*sin(i*twicePi/triangleAmount));
    glEnd();

    glPopMatrix();  // end Bus 2 offset
    glPopMatrix();  // end Bus 1 + Bus 2 travel transform

    // ╔══════════════════════════════════════════════════════════════════════╗
    // ║                      RAKA — END (in display2)                       ║
    // ╚══════════════════════════════════════════════════════════════════════╝


    // ╔══════════════════════════════════════════════════════════════════════╗
    // ║                    SAMSUL — START (Pools in display2)               ║
    // ║  Draws 3 circular pools on the field with outlines.                 ║
    // ╚══════════════════════════════════════════════════════════════════════╝

    // ─────────────────────────────────────────────────────────────────────────
    //  SAMSUL — POOLS (3 circular water pools on the grass field)
    // ─────────────────────────────────────────────────────────────────────────

    // ── Pool 2 outline first (drawn before fill so fill is on top) ──
    GLfloat spp21=0.485f, spp11=0.023f, op11=0.040f;
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(spp21, spp11);
    for (i = 0; i <= lineAmount; i++)
        glVertex2f(spp21 + op11*cos(i*twicePi/lineAmount),
                   spp11 + op11*sin(i*twicePi/lineAmount));
    glEnd();

    // ── Pool 1 (left pool — largest, radius 0.07) ──
    GLfloat sp14=0.38f, sp15=0.021f, o13=0.07f;
    glColor3f(0.443f, 0.780f, 0.898f);   // light blue water
    glBegin(GL_TRIANGLE_FAN); glVertex2f(sp14, sp15);
    for (i=0; i<=triangleAmount; i++)
        glVertex2f(sp14+o13*cos(i*twicePi/triangleAmount), sp15+o13*sin(i*twicePi/triangleAmount));
    glEnd();

    // ── Pool 2 (centre pool — small, radius 0.04) ──
    GLfloat sp21=0.485f, sp11=0.023f, o11=0.04f;
    glColor3f(0.443f, 0.780f, 0.898f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(sp21, sp11);
    for (i=0; i<=triangleAmount; i++)
        glVertex2f(sp21+o11*cos(i*twicePi/triangleAmount), sp11+o11*sin(i*twicePi/triangleAmount));
    glEnd();

    // ── Pool 3 (right pool — largest, radius 0.08) ──
    GLfloat sp12=0.60f, sp13=0.023f, o12=0.08f;
    glColor3f(0.443f, 0.780f, 0.898f);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(sp12, sp13);
    for (i=0; i<=triangleAmount; i++)
        glVertex2f(sp12+o12*cos(i*twicePi/triangleAmount), sp13+o12*sin(i*twicePi/triangleAmount));
    glEnd();

    // ── Pool 1 outline ──
    GLfloat spp14=0.38f, spp15=0.021f, op13=0.07f;
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP); glVertex2f(spp14, spp15);
    for (i=0; i<=lineAmount; i++)
        glVertex2f(spp14+op13*cos(i*twicePi/lineAmount), spp15+op13*sin(i*twicePi/lineAmount));
    glEnd();

    // ── Pool 3 outline ──
    GLfloat spp12=0.60f, spp13=0.023f, op12=0.08f;
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP); glVertex2f(spp12, spp13);
    for (i=0; i<=lineAmount; i++)
        glVertex2f(spp12+op12*cos(i*twicePi/lineAmount), spp13+op12*sin(i*twicePi/lineAmount));
    glEnd();

    // ── Pool connecting channel (rectangular water strip between pools) ──
    glBegin(GL_QUADS);
    glColor3f(0.443f, 0.780f, 0.898f);
    glVertex2f(0.35f, 0.039f); glVertex2f(0.35f, 0.000f);
    glVertex2f(0.67f, 0.000f); glVertex2f(0.67f, 0.039f);
    glEnd();

    // ── Pool overflow trickle detail (blue + sand-coloured lines) ──
    glLineWidth(3);
    glBegin(GL_LINES);
    glColor3f(0.443f, 0.780f, 0.898f);
    glVertex2f(0.670f, 0.024f); glVertex2f(0.679f, 0.024f);  // water continuation
    glColor3f(0.898f, 0.651f, 0.502f);
    glVertex2f(0.681f, 0.024f); glVertex2f(0.690f, 0.024f);  // sandy bank edge
    glEnd();

    // ╔══════════════════════════════════════════════════════════════════════╗
    // ║                    SAMSUL — END (Pools in display2)                 ║
    // ╚══════════════════════════════════════════════════════════════════════╝

    glFlush();  // flush all queued OpenGL commands to the display
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         RAKA  —  END                                        ║
// ╚══════════════════════════════════════════════════════════════════════════════╝


// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         SAMSUL  —  START                                    ║
// ║  Responsibilities:                                                           ║
// ║    • display5() — full Moon Landing scene (stars, earth, moon ground,        ║
// ║                   craters, 3D rocket descent with retro-fire)               ║
// ║    • updateMoonLanding() — moon descent physics timer                        ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

// ─────────────────────────────────────────────────────────────────────────────
//  SAMSUL — display5(): Moon Landing Scene
//  Black space background with stars, shaped star polygons, Earth globe,
//  moon surface with craters, and the 3D rocket descending from orbit.
//
//  Controls (from handleKeypressM / SpecialInputs):
//    '2'  → enters this scene; rocket auto-starts descent
//    UP   → resume descent,  DOWN → pause descent,  R → reset to top
// ─────────────────────────────────────────────────────────────────────────────
void display5()
{
    // Clear to black (space)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int     triAmt = 150;           // high segment count for smooth moon/earth circles
    GLfloat tp     = 2.0f * PI;     // full revolution

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — STARS (point-cloud background)
    //  Three passes: size 3, 4, 5, 6 — white and grey-white stars.
    // ─────────────────────────────────────────────────────────────────────
    glPointSize(3.0f); glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    // Batch 1 — small white stars
    glVertex2f(0.9f,0.9f);  glVertex2f(0.7f,0.6f);  glVertex2f(0.5f,0.5f);  glVertex2f(0.4f,0.5f);
    glVertex2f(0.4f,0.9f);  glVertex2f(0.5f,0.7f);  glVertex2f(0.6f,0.4f);  glVertex2f(0.8f,0.5f);
    glPointSize(4.0f);
    glVertex2f(0.6f,0.9f);  glVertex2f(0.6f,0.7f);  glVertex2f(0.3f,0.5f);  glVertex2f(0.6f,0.4f);
    glVertex2f(0.4f,0.9f);  glVertex2f(0.9f,0.5f);  glVertex2f(0.1f,0.46f); glVertex2f(0.8f,0.6f);
    glPointSize(5.0f);
    glVertex2f(0.7f,0.8f);  glVertex2f(0.65f,0.4f); glVertex2f(0.2f,0.7f);  glVertex2f(0.15f,0.45f);
    glVertex2f(0.43f,0.69f);glVertex2f(0.55f,0.75f);glVertex2f(0.05f,0.56f);glVertex2f(0.86f,0.96f);
    // Batch 2 — grey-white stars (smaller / dimmer)
    glPointSize(3.0f); glColor3f(0.89f, 0.867f, 0.875f);
    glVertex2f(0.04f,0.04f);glVertex2f(0.04f,0.30f);glVertex2f(0.04f,0.55f);glVertex2f(0.08f,0.018f);
    glVertex2f(0.08f,0.18f);glVertex2f(0.14f,0.20f);glVertex2f(0.14f,0.50f);glVertex2f(0.25f,0.50f);
    glVertex2f(0.20f,0.35f);glVertex2f(0.06f,0.62f);
    glPointSize(4.0f);
    glVertex2f(0.5f,0.62f); glVertex2f(0.5f,0.18f); glVertex2f(0.4f,0.09f); glVertex2f(0.3f,0.13f);
    glVertex2f(0.22f,0.08f);glVertex2f(0.35f,0.80f);glVertex2f(0.49f,0.83f);glVertex2f(0.3f,0.65f);
    glVertex2f(0.3f,0.90f); glVertex2f(0.45f,0.94f);glVertex2f(0.45f,0.80f);
    glPointSize(5.0f);
    glVertex2f(0.53f,0.75f);glVertex2f(0.63f,0.50f);glVertex2f(0.63f,0.75f);glVertex2f(0.69f,0.80f);
    glVertex2f(0.73f,0.95f);glVertex2f(0.94f,0.95f);glVertex2f(0.86f,0.98f);glVertex2f(0.80f,0.43f);
    glVertex2f(0.75f,0.48f);glVertex2f(0.70f,0.28f);glVertex2f(0.70f,0.63f);glVertex2f(0.45f,0.28f);
    glVertex2f(0.60f,0.28f);glVertex2f(0.50f,0.46f);glVertex2f(0.08f,0.66f);
    glPointSize(6.0f);
    glVertex2f(0.20f,0.60f);glVertex2f(0.40f,0.52f);glVertex2f(0.28f,0.26f);glVertex2f(0.40f,0.70f);
    glVertex2f(0.56f,0.06f);glVertex2f(0.60f,0.90f);glVertex2f(0.84f,0.84f);glVertex2f(0.12f,0.40f);
    glVertex2f(0.32f,0.40f);glVertex2f(0.90f,0.83f);glVertex2f(0.80f,0.60f);glVertex2f(0.80f,0.90f);
    glVertex2f(0.95f,0.86f);glVertex2f(0.90f,0.66f);glVertex2f(0.97f,0.76f);glVertex2f(0.88f,0.56f);
    glVertex2f(0.98f,0.60f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — SHAPED STAR POLYGONS (3 decorative 10-vertex star shapes)
    //  Each uses glScalef+glTranslatef to position and scale a unit star.
    // ─────────────────────────────────────────────────────────────────────
    // Star 1 (lower-centre area)
    glPushMatrix(); glScalef(0.15f, 0.15f, 0); glTranslatef(2.50f, 3.20f, 0);
    glBegin(GL_POLYGON); glColor3f(1,1,1);
    glVertex2f(.030f,.040f);glVertex2f(.050f,.030f);glVertex2f(.040f,.050f);glVertex2f(.057f,.060f);
    glVertex2f(.037f,.060f);glVertex2f(.030f,.080f);glVertex2f(.023f,.060f);glVertex2f(.003f,.060f);
    glVertex2f(.020f,.050f);glVertex2f(.010f,.030f);
    glEnd(); glPopMatrix();

    // Star 2 (mid-left area)
    glPushMatrix(); glScalef(0.15f, 0.15f, 0); glTranslatef(2.40f, 4.60f, 0);
    glBegin(GL_POLYGON); glColor3f(1,1,1);
    glVertex2f(.030f,.040f);glVertex2f(.050f,.030f);glVertex2f(.040f,.050f);glVertex2f(.057f,.060f);
    glVertex2f(.037f,.060f);glVertex2f(.030f,.080f);glVertex2f(.023f,.060f);glVertex2f(.003f,.060f);
    glVertex2f(.020f,.050f);glVertex2f(.010f,.030f);
    glEnd(); glPopMatrix();

    // Star 3 (upper-right area)
    glPushMatrix(); glScalef(0.15f, 0.15f, 0); glTranslatef(3.30f, 6.00f, 0);
    glBegin(GL_POLYGON); glColor3f(1,1,1);
    glVertex2f(.030f,.040f);glVertex2f(.050f,.030f);glVertex2f(.040f,.050f);glVertex2f(.057f,.060f);
    glVertex2f(.037f,.060f);glVertex2f(.030f,.080f);glVertex2f(.023f,.060f);glVertex2f(.003f,.060f);
    glVertex2f(.020f,.050f);glVertex2f(.010f,.030f);
    glEnd(); glPopMatrix();

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — EARTH GLOBE (top-left corner, partially off-screen)
    //  Blue ocean base + 6 green land-mass patches.
    // ─────────────────────────────────────────────────────────────────────
    glColor3f(0.043f, 0.678f, 0.937f);   // ocean blue
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.9f);
    for (int k=0; k<=triAmt; k++)
        glVertex2f(0.0f + 0.27f*cosf(k*tp/triAmt), 0.9f + 0.27f*sinf(k*tp/triAmt));
    glEnd();

    // Land masses (6 green patches at varying positions/radii)
    float eg[][3] = {
        {0.00f,0.84f,.11f},{0.10f,0.85f,.09f},{0.15f,1.00f,.05f},
        {-0.08f,1.05f,.05f},{-0.15f,0.95f,.07f},{0.00f,1.03f,.09f}
    };
    for (int k=0; k<6; k++) {
        glColor3f(0.620f, 0.827f, 0.271f);  // grass green land
        glBegin(GL_TRIANGLE_FAN); glVertex2f(eg[k][0], eg[k][1]);
        for (int j=0; j<=triAmt; j++)
            glVertex2f(eg[k][0]+eg[k][2]*cosf(j*tp/triAmt), eg[k][1]+eg[k][2]*sinf(j*tp/triAmt));
        glEnd();
    }

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — MOON GROUND (white lumpy surface at the bottom of screen)
    //  Five overlapping circles form the uneven moon terrain.
    // ─────────────────────────────────────────────────────────────────────
    glColor3f(1.0f, 1.0f, 1.0f);
    // Terrain mound 1 (far left)
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0.10f,0.20f);
    for (int k=0; k<=triAmt; k++) glVertex2f(0.10f+0.17f*cosf(k*tp/triAmt), 0.20f+0.17f*sinf(k*tp/triAmt));
    glEnd();
    // Mound 2
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0.25f,0.25f);
    for (int k=0; k<=triAmt; k++) glVertex2f(0.25f+0.07f*cosf(k*tp/triAmt), 0.25f+0.07f*sinf(k*tp/triAmt));
    glEnd();
    // Mound 3 (centre — large, main landing area)
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0.45f,0.08f);
    for (int k=0; k<=triAmt; k++) glVertex2f(0.45f+0.27f*cosf(k*tp/triAmt), 0.08f+0.27f*sinf(k*tp/triAmt));
    glEnd();
    // Mound 4
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0.76f,0.17f);
    for (int k=0; k<=triAmt; k++) glVertex2f(0.76f+0.20f*cosf(k*tp/triAmt), 0.17f+0.20f*sinf(k*tp/triAmt));
    glEnd();
    // Mound 5 (far right)
    glBegin(GL_TRIANGLE_FAN); glVertex2f(0.97f,0.27f);
    for (int k=0; k<=triAmt; k++) glVertex2f(0.97f+0.07f*cosf(k*tp/triAmt), 0.27f+0.07f*sinf(k*tp/triAmt));
    glEnd();

    // Flat ground fill (white top strip → grey bottom — gives moon surface depth)
    glBegin(GL_QUADS);
    glColor3f(1.0f,1.0f,1.0f);         glVertex2f(0.0f,0.30f); glVertex2f(0.765f,0.30f);
    glColor3f(0.573f,0.596f,0.616f);   glVertex2f(0.765f,0.00f);glVertex2f(0.0f,  0.00f);
    glEnd();

    // ─────────────────────────────────────────────────────────────────────
    //  SAMSUL — MOON CRATERS (9 craters, each drawn as 3 concentric ellipses)
    //  Outer ellipse = bright rim, middle = shadow, inner = deepest point.
    // ─────────────────────────────────────────────────────────────────────
    float craters[][4] = {
        {0.05f,0.35f,.055f,.025f},{0.01f,0.10f,.065f,.025f},{0.20f,0.20f,.050f,.025f},
        {0.50f,0.32f,.060f,.025f},{0.44f,0.12f,.057f,.025f},{0.37f,0.24f,.037f,.015f},
        {0.20f,0.01f,.067f,.025f},{0.76f,0.33f,.057f,.020f},{0.66f,0.09f,.057f,.020f}
    };
    for (int k=0; k<9; k++) {
        glColor3f(0.839f,0.839f,0.839f); drawEllipse(craters[k][0],craters[k][1],craters[k][2],  craters[k][3],  100);
        glColor3f(0.510f,0.510f,0.510f); drawEllipse(craters[k][0],craters[k][1],craters[k][2]-.005f,craters[k][3]-.005f,100);
        glColor3f(0.518f,0.518f,0.518f); drawEllipse(craters[k][0],craters[k][1],craters[k][2]-.010f,craters[k][3]-.010f,100);
    }

    // Right-side moon ground (mirror of left flat fill)
    glBegin(GL_QUADS);
    glColor3f(1.0f,1.0f,1.0f);         glVertex2f(1.00f,0.30f);glVertex2f(0.76f,0.30f);
    glColor3f(0.573f,0.596f,0.616f);   glVertex2f(0.76f, 0.00f);glVertex2f(1.00f,0.00f);
    glEnd();
    // Single extra crater on right section
    glColor3f(0.647f,0.647f,0.647f); drawEllipse(0.90f,0.07f,0.057f,0.020f,100);
    glColor3f(0.510f,0.510f,0.510f); drawEllipse(0.90f,0.07f,0.052f,0.015f,100);
    glColor3f(0.518f,0.518f,0.518f); drawEllipse(0.90f,0.07f,0.047f,0.010f,100);

    // ─────────────────────────────────────────────────────────────────────────
    //  RAFI — MOON PAGE 3D ROCKET (descends from orbit toward moon surface)
    //
    //  The rocket is drawn inline here (not via draw3DRocket) to apply the
    //  glScalef(1.2, 1.2) zoom that makes it appear larger in the moon scene.
    //  Descent offset: positions goes 1.0 (off-screen top) → -0.60 (landed).
    //  Y-axis spin continues (rocketAngleY) for a 3D orbital approach effect.
    //  Retro-fire plumes show while !moonLanded; extinguished on touchdown.
    // ─────────────────────────────────────────────────────────────────────────
    {
        int segs = 36;

        glPushMatrix();
        glScalef(1.2f, 1.2f, 1.0f);                  // zoom rocket in moon scene
        glTranslatef(0.25f, positions, 0.0f);          // descent: 1.0→-0.60

        // Apply Y-axis spin around rocket centre
        glTranslatef(RCX,  0.375f, 0.0f);
        glRotatef(rocketAngleY, 0.0f, 1.0f, 0.0f);
        glTranslatef(-RCX, -0.375f, 0.0f);

        // ── Nozzle collar ──
        drawCylinderStrip(RCX,0.23f,0.25f,0.0f, RHW,RHD,segs,
                          0.455f,0.525f,0.553f, 0.345f,0.416f,0.424f);
        drawDisk(RCX,0.25f,0.0f, RHW,RHD,segs, 0.42f,0.49f,0.51f);
        drawDisk(RCX,0.23f,0.0f, RHW*0.92f,RHD*0.92f,segs, 0.22f,0.28f,0.30f);

        // ── Lower booster body (orange) ──
        drawCylinderStrip(RCX,0.25f,0.34f,0.0f, RHW,RHD,segs,
                          0.976f,0.475f,0.133f, 0.835f,0.388f,0.09f);
        drawCylinderStrip(RCX,0.34f,0.35f,0.0f, RHW+0.003f,RHD+0.003f,segs,
                          0.588f,0.647f,0.663f, 0.45f,0.50f,0.51f);
        drawOvalRim(RCX,0.35f,0.0f, RHW+0.003f,RHD+0.003f,segs, 0.55f,0.62f,0.64f,1.0f);

        // ── Main body (blue) ──
        drawCylinderStrip(RCX,0.35f,0.50f,0.0f, RHW,RHD,segs,
                          0.224f,0.490f,0.886f, 0.125f,0.420f,0.776f);
        drawCylinderStrip(RCX,0.449f,0.452f,0.0f, RHW+0.002f,RHD+0.002f,segs,
                          0.05f,0.05f,0.05f, 0.05f,0.05f,0.05f);

        // ── Shoulder ring ──
        drawCylinderStrip(RCX,0.50f,0.51f,0.0f, RHW+0.004f,RHD+0.004f,segs,
                          0.478f,0.537f,0.565f, 0.349f,0.420f,0.435f);
        drawOvalRim(RCX,0.51f,0.0f, RHW+0.004f,RHD+0.004f,segs, 0.40f,0.46f,0.48f,1.0f);

        // ── Tapered upper section (orange frustum) ──
        {
            float rxTop=RHW*0.86f, rzTop=RHD*0.86f;
            float rxBot=RHW+0.004f, rzBot=RHD+0.004f;
            glBegin(GL_QUAD_STRIP);
            for (int k=0; k<=segs; k++) {
                float t=twicePi*k/segs, lit=0.50f+0.50f*cosf(t);
                glColor3f(0.925f*lit,0.431f*lit,0.075f*lit);
                glVertex3f(RCX+rxTop*cosf(t),0.56f,rzTop*sinf(t));
                glColor3f(0.976f*lit,0.475f*lit,0.133f*lit);
                glVertex3f(RCX+rxBot*cosf(t),0.51f,rzBot*sinf(t));
            }
            glEnd();
        }

        // ── Nose cone ──
        drawCone(RCX,0.59f,0.56f,0.0f, RHW*0.86f,RHD*0.86f,segs,
                 0.52f,0.58f,0.60f, 0.471f,0.529f,0.557f);
        drawDisk(RCX,0.59f,0.0f, 0.004f,0.004f,12, 0.28f,0.32f,0.33f);
        drawOvalRim(RCX,0.56f,0.0f, RHW*0.86f,RHD*0.86f,segs, 0.40f,0.46f,0.48f,1.0f);

        // ── Porthole window ──
        {
            float wX=RCX, wY=0.45f, wR=0.015f;
            float fZ=RHD+0.002f, bZ=-RHD-0.002f;
            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.976f,0.482f,0.125f); glVertex3f(wX,wY,fZ);
            for (int k=0; k<=segs; k++) {
                float t=twicePi*k/segs, lit=0.70f+0.30f*cosf(t);
                glColor3f(0.976f*lit,0.482f*lit,0.125f*lit);
                glVertex3f(wX+wR*cosf(t),wY+wR*sinf(t),fZ);
            }
            glEnd();
            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.55f,0.92f,1.0f); glVertex3f(wX,wY,fZ+0.002f);
            for (int k=0; k<=segs; k++) {
                float t=twicePi*k/segs, lit=0.60f+0.40f*cosf(t-0.5f);
                glColor3f(0.314f*lit,0.863f*lit,0.98f*lit);
                glVertex3f(wX+wR*0.65f*cosf(t),wY+wR*0.65f*sinf(t),fZ+0.002f);
            }
            glEnd();
            glBegin(GL_TRIANGLE_FAN);
            glColor3f(1.0f,1.0f,1.0f); glVertex3f(wX-0.003f,wY+0.004f,fZ+0.004f);
            for (int k=0; k<=12; k++) {
                float t=twicePi*k/12;
                glColor3f(0.88f,0.94f,1.0f);
                glVertex3f(wX-0.003f+0.003f*cosf(t),wY+0.004f+0.003f*sinf(t),fZ+0.004f);
            }
            glEnd();
            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.07f,0.09f,0.11f); glVertex3f(wX,wY,bZ);
            for (int k=0; k<=segs; k++) {
                float t=twicePi*k/segs;
                glColor3f(0.04f,0.06f,0.08f);
                glVertex3f(wX+wR*cosf(t),wY+wR*sinf(t),bZ);
            }
            glEnd();
        }

        // ── Side boosters ──
        {
            float bRX=0.005f, bRZ=0.004f;
            for (int side=0; side<2; side++) {
                float bX=(side==0)?0.440f:0.370f;
                drawCylinderStrip(bX,0.23f,0.39f,0.0f,bRX,bRZ,segs,
                                  0.196f,0.510f,0.984f, 0.150f,0.400f,0.800f);
                drawCone(bX,0.42f,0.39f,0.0f,bRX,bRZ,segs,
                         0.99f,0.62f,0.12f, 0.99f,0.55f,0.10f);
                drawOvalRim(bX,0.39f,0.0f,bRX,bRZ,segs, 0.18f,0.48f,0.90f,1.0f);
            }
        }

        // ── Wings ──
        {
            float sinA=sinf(rocketAngleY*3.14159f/180.0f), wT=0.008f;
            float rw[4][2]={{0.434f,0.36f},{0.48f,0.24f},{0.49f,0.23f},{0.43f,0.25f}};
            float lw[4][2]={{0.376f,0.36f},{0.33f,0.24f},{0.32f,0.23f},{0.38f,0.25f}};
            drawWingFace(rw[0][0],rw[0][1],rw[1][0],rw[1][1],rw[2][0],rw[2][1],rw[3][0],rw[3][1],
                          wT,0.224f,0.494f,0.882f,0.62f+0.38f*sinA);
            drawWingFace(rw[3][0],rw[3][1],rw[2][0],rw[2][1],rw[1][0],rw[1][1],rw[0][0],rw[0][1],
                         -wT,0.125f,0.420f,0.776f,0.38f+0.22f*sinA);
            float rLit=(0.55f+0.45f*sinA)*0.90f;
            glColor3f(0.18f*rLit,0.45f*rLit,0.88f*rLit);
            glBegin(GL_QUADS);
            glVertex3f(rw[0][0],rw[0][1],wT);  glVertex3f(rw[3][0],rw[3][1],wT);
            glVertex3f(rw[3][0],rw[3][1],-wT); glVertex3f(rw[0][0],rw[0][1],-wT);
            glEnd();
            glColor3f(0.10f,0.33f,0.70f);
            glBegin(GL_QUADS);
            glVertex3f(rw[1][0],rw[1][1],wT);  glVertex3f(rw[2][0],rw[2][1],wT);
            glVertex3f(rw[2][0],rw[2][1],-wT); glVertex3f(rw[1][0],rw[1][1],-wT);
            glEnd();
            drawWingFace(lw[0][0],lw[0][1],lw[1][0],lw[1][1],lw[2][0],lw[2][1],lw[3][0],lw[3][1],
                          wT,0.224f,0.494f,0.882f,0.62f-0.38f*sinA);
            drawWingFace(lw[3][0],lw[3][1],lw[2][0],lw[2][1],lw[1][0],lw[1][1],lw[0][0],lw[0][1],
                         -wT,0.125f,0.420f,0.776f,0.38f-0.22f*sinA);
            float lLit=(0.55f-0.35f*sinA)*0.90f;
            glColor3f(0.18f*lLit,0.45f*lLit,0.88f*lLit);
            glBegin(GL_QUADS);
            glVertex3f(lw[0][0],lw[0][1],wT);  glVertex3f(lw[3][0],lw[3][1],wT);
            glVertex3f(lw[3][0],lw[3][1],-wT); glVertex3f(lw[0][0],lw[0][1],-wT);
            glEnd();
            glColor3f(0.10f,0.33f,0.70f);
            glBegin(GL_QUADS);
            glVertex3f(lw[1][0],lw[1][1],wT);  glVertex3f(lw[2][0],lw[2][1],wT);
            glVertex3f(lw[2][0],lw[2][1],-wT); glVertex3f(lw[1][0],lw[1][1],-wT);
            glEnd();
        }

        // ── Base trapezoid slab ──
        {
            float d=RHD;
            glBegin(GL_QUADS);
            glColor3f(0.52f,0.50f,0.48f);
            glVertex3f(0.37f,0.23f,d); glVertex3f(0.32f,0.20f,d);
            glVertex3f(0.49f,0.20f,d); glVertex3f(0.44f,0.23f,d);
            glColor3f(0.26f,0.25f,0.24f);
            glVertex3f(0.44f,0.23f,-d); glVertex3f(0.49f,0.20f,-d);
            glVertex3f(0.32f,0.20f,-d); glVertex3f(0.37f,0.23f,-d);
            glColor3f(0.62f,0.60f,0.58f);
            glVertex3f(0.37f,0.23f,d);  glVertex3f(0.44f,0.23f,d);
            glVertex3f(0.44f,0.23f,-d); glVertex3f(0.37f,0.23f,-d);
            glColor3f(0.34f,0.33f,0.32f);
            glVertex3f(0.32f,0.20f,d);  glVertex3f(0.49f,0.20f,d);
            glVertex3f(0.49f,0.20f,-d); glVertex3f(0.32f,0.20f,-d);
            glEnd();
        }

        // ── Retro-fire plumes (braking thrust — shown while descending, off when landed) ──


        glPopMatrix();  // end moon-scene rocket transform
    }

    glFlush();  // send all commands to display
}

// ─────────────────────────────────────────────────────────────────────────────
//  Rafi — MOON LANDING DESCENT TIMER (called every 16ms ≈ 60fps)
//  Smoothly decrements positions by speeds each frame until the rocket
//  visually rests on the moon surface (positions == -0.60).
//  On touchdown: speeds is zeroed and moonLanded=true kills retro-fire.
// ─────────────────────────────────────────────────────────────────────────────
void updateMoonLanding(int value)
{
    if (!moonLanded && speeds > 0.0f) {
        positions -= speeds;            // descend by one speed unit
        if (positions <= -0.60f) {
            positions  = -0.60f;        // clamp exactly at landing height
            speeds     =  0.0f;         // stop descent
            moonLanded =  true;         // trigger landing state — kills fire
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, updateMoonLanding, 0);    // re-schedule for next frame
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         SAMSUL  —  END                                      ║
// ╚══════════════════════════════════════════════════════════════════════════════╝


// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         RAFI  —  START (Timer callbacks & input)            ║
// ╚══════════════════════════════════════════════════════════════════════════════╝

// ─────────────────────────────────────────────────────────────────────────────
//  Samsul — CLOUD DRIFT TIMER (update1 — 100ms interval)
//  Moves cloud groups left by Speed1 per tick.
//  Wraps around: when Position1 > 2.0, reset to -1.0 (loop effect).
// ─────────────────────────────────────────────────────────────────────────────
void update1(int value)
{
    if (Position1 > 2.0f)
        Position1 = -1.0f;      // wrap cloud groups back to right edge
    Position1 += Speed1;
    glutPostRedisplay();
    glutTimerFunc(100, update1, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — ROCKET LAUNCH TIMER (update2 — 16ms interval ≈ 60fps)
//  Implements smooth ease-in acceleration:
//    - When launching: launchSpeed increases by 0.00005 per frame (capped at 0.004)
//    - When paused (m pressed): launchSpeed decays by ×0.88 per frame (ease-out)
//  Position2 is clamped at 2.0 (rocket reaches sky boundary and stops).
// ─────────────────────────────────────────────────────────────────────────────
void update2(int value)
{
    if (rocketLaunching) {
        launchSpeed += 0.00005f;                          // gradual acceleration (thrust build-up)
        if (launchSpeed > 0.004f) launchSpeed = 0.004f;  // terminal velocity cap
    } else {
        launchSpeed *= 0.88f;                             // smooth deceleration when thrust cut
        if (launchSpeed < 0.0001f) launchSpeed = 0.0f;   // snap to zero to avoid drift
    }
    Position2 += launchSpeed;
    if (Position2 > 2.0f) Position2 = 2.0f;              // sky limit — prevent rocket leaving scene
    glutPostRedisplay();
    glutTimerFunc(16, update2, 0);                        // 60fps smooth motion
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — 3D ROCKET Y-AXIS SPIN TIMER (updateRocketSpin — 16ms interval ≈ 60fps)
//  Spin is dormant (rocketSpinActive=false) until 'n' first pressed.
//  Target spin speed: 3.5°/frame while launching, 0.6°/frame when idle.
//  Speed is smoothly eased toward target with an exponential filter (factor 0.05).
// ─────────────────────────────────────────────────────────────────────────────
void updateRocketSpin(int value)
{
    if (rocketSpinActive) {
        float targetSpin = rocketLaunching ? 3.5f : 0.6f;   // fast spin during launch, slow while idle
        rocketSpinSpeed += (targetSpin - rocketSpinSpeed) * 0.05f; // exponential ease
        rocketAngleY    += rocketSpinSpeed;
        if (rocketAngleY >= 360.0f) rocketAngleY -= 360.0f; // wrap 360°
    }
    glutPostRedisplay();
    glutTimerFunc(16, updateRocketSpin, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAKA — BUS TRAVEL TIMER (update3 — 100ms interval)
//  Moves both buses right by Speed3 per tick.
//  Wraps around when Position3 > 2.0 (buses loop off-screen right → re-enter left).
// ─────────────────────────────────────────────────────────────────────────────
void update3(int value)
{
    if (Position3 > 2.0f)
        Position3 = -1.0f;      // wrap bus back to left entry point
    Position3 += Speed3;
    glutPostRedisplay();
    glutTimerFunc(100, update3, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — KEYBOARD HANDLER (handleKeypressM)
//  Handles all regular key presses for scene control and animation.
//
//  Key map:
//    v → start cloud drift (left) + bus travel (right)
//    b → stop all moving objects
//    n → ignite rocket + enable 3D spin
//    m → cut rocket thrust (spin continues at idle speed)
//    r → full reset of all positions, speeds, and states
//    1 → switch to display2 (day/campus scene) + play background music
//    2 → switch to display5 (moon landing scene) + auto-start descent
// ─────────────────────────────────────────────────────────────────────────────
void handleKeypressM(unsigned char key, int x, int y)
{
    switch (key)
    {
        // ── Cloud + Bus control ──
        case 'v':
            Speed1 = -0.005f;   // clouds drift left
            Speed3 =  0.015f;   // buses move right
            break;
        case 'b':
            Speed1 = 0.0f;      // stop clouds
            Speed3 = 0.0f;      // stop buses
            break;

        // ── Rocket launch control ──
        case 'n':
            rocketLaunching  = true;    // ignite engines
            rocketSpinActive = true;    // enable 3D Y-spin from first press
            break;
        case 'm':
            rocketLaunching  = false;   // cut thrust; rocket decelerates (ease-out)
            break;

        // ── Full scene reset ──
        case 'r':
            rocketLaunching  = false;
            rocketSpinActive = false;
            launchSpeed      = 0.0f;
            Position2        = 0.0f;    // rocket back to launch pad height
            rocketAngleY     = 0.0f;    // reset spin angle
            rocketSpinSpeed  = 0.0f;
            speeds           = 0.0f;    // moon lander stopped
            positions        = 1.0f;    // moon lander back above screen
            moonLanded       = false;
            break;

        // ── Scene switches ──
        case '1':
            // Switch to day/campus scene and start background audio
            PlaySound(TEXT("E:\\music.wav"), NULL, SND_FILENAME | SND_ASYNC);
            glutDisplayFunc(display2);
            sndPlaySound("Sound-2.wav", SND_ASYNC);
            glutPostRedisplay();
            break;

        case '2':
            // Switch to moon landing scene
            positions        = 1.0f;    // rocket starts above screen
            speeds           = 0.002f;  // auto-start descent on entering moon page
            moonLanded       = false;
            rocketSpinActive = true;    // Y-spin on for orbital approach effect
            glutDisplayFunc(display5);
            glutPostRedisplay();
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  RAFI — SPECIAL KEY HANDLER (SpecialInputs — arrow keys)
//  Used exclusively in display5 (moon landing page).
//    UP   → resume / start descent (speeds = 0.002)
//    DOWN → pause descent (speeds = 0)
// ─────────────────────────────────────────────────────────────────────────────
void SpecialInputs(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        speeds           = 0.002f;  // start / resume descent
        moonLanded       = false;   // re-arm landing state
        rocketSpinActive = true;    // spin on while active
        break;
    case GLUT_KEY_DOWN:
        speeds           = 0.0f;    // pause descent
        rocketSpinActive = false;   // stop spin while paused
        break;
    }
    glutPostRedisplay();
}

// ╔══════════════════════════════════════════════════════════════════════════════╗
// ║                         RAFI  —  END (Timer callbacks & input)              ║
// ╚══════════════════════════════════════════════════════════════════════════════╝


// ─────────────────────────────────────────────────────────────────────────────
//  MAIN ENTRY POINT
//  Sets up GLUT window, registers all callbacks, starts timer chains, and
//  enters the GLUT main loop.
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH); // single-buffer + RGB + depth
    glutInitWindowSize(1400, 800);
    glutCreateWindow("Project - Moon Land");
    glutPositionWindow(80, 50);

    init();     // set up orthographic projection + depth test

    // Register input handlers
    glutKeyboardFunc(handleKeypressM);
    glutSpecialFunc(SpecialInputs);

    // Start background audio loop
    PlaySound(TEXT("E:\\music.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    // Default display: day/campus scene
    glutDisplayFunc(display2);
    glutFullScreen();

    // Start all animation timers
    glutTimerFunc(100, update1,          0);    // cloud drift  (100ms)
    glutTimerFunc(16,  update2,          0);    // rocket launch (16ms ≈ 60fps)
    glutTimerFunc(100, update3,          0);    // bus travel   (100ms)
    glutTimerFunc(16,  updateRocketSpin, 0);    // 3D spin      (16ms ≈ 60fps)
    glutTimerFunc(200, updateMoonLanding,0);    // moon descent (200ms)

    glutIdleFunc(Idle);     // continuous redraw on idle
    glutMainLoop();         // enter GLUT event loop (never returns)
    return 0;
}
