// This file is both used to declare extern'd global variables, and to declare them in globals.c.

// Technique for naming global variables:
// * If we don't know what it does, give it a codename. It's easier to remember (and more fun) than a number.

#ifdef TIM_GLOBALS_C
#   define GLOBAL(declaration, init) declaration = init;
#else
#   define GLOBAL(declaration, init) extern declaration;
#endif

// The original game has an abundance of short-lived, temporary global variables (only used across a few function calls).
// Until we have a hollistic sense of how they're actually used, we have to recreate them.
// Refactoring these away at some point would probably be ideal.

/* TIMWIN: 1108:3a6c */
GLOBAL(struct Part *PART_3a6c, 0)
/* TIMWIN: 1108:3a6a */
GLOBAL(struct Part *PART_3a6a, 0)
/* TIMWIN: 1108:3a68 */
GLOBAL(struct Part *PART_3a68, 0)
/* TIMWIN: 1108:3a8e */
GLOBAL(u16 TMP_3a8e, 0);
/* TIMWIN: 1108:3a90 */
GLOBAL(u16 TMP_3a90, 0);
/* TIMWIN: 1108:3a92 */
GLOBAL(u16 TMP_3a92, 0);

GLOBAL(s16 TMP_X2_3a6c, 0)
GLOBAL(s16 TMP_Y2_3a6c, 0)
GLOBAL(s16 TMP_X_CENTER_3a6c, 0)
GLOBAL(s16 TMP_Y_CENTER_3a6c, 0)
GLOBAL(s16 TMP_X_DELTA_3a6c, 0)
GLOBAL(s16 TMP_Y_DELTA_3a6c, 0)
GLOBAL(s16 TMP_X_LEFTMOST_3a6c, 0)
GLOBAL(s16 TMP_Y_TOPMOST_3a6c, 0)
GLOBAL(s16 TMP_X_RIGHT_3a6c, 0)
GLOBAL(s16 TMP_Y_BOTTOM_3a6c, 0)

GLOBAL(s16 TMP_X_3a6a, 0)
GLOBAL(s16 TMP_Y_3a6a, 0)
GLOBAL(s16 TMP_X_CENTER_3a6a, 0)
GLOBAL(s16 TMP_Y_CENTER_3a6a, 0)
GLOBAL(s16 TMP_X_RIGHT_3a6a, 0)
GLOBAL(s16 TMP_Y_BOTTOM_3a6a, 0)

/* TIMWIN: 1108:3e47 */
// AIR_PRESSURE ranges from 0 to 128 inclusive.
GLOBAL(u16 AIR_PRESSURE, DEFAULT_AIR_PRESSURE)

/* TIMWIN: 1108:3e49 */
// GRAVITY ranges from 0 to 512 inclusive.
GLOBAL(u16 GRAVITY, DEFAULT_GRAVITY)

/* TIMWIN: 1108:3e43 */
GLOBAL(u16 BONUS_1, 0)

/* TIMWIN: 1108:3e45 */
GLOBAL(u16 BONUS_2, 0)

/* TIMWIN: 1108:3e4f */
GLOBAL(u16 MUSIC_TRACK, 0x03e9)

/* Codename. TIMWIN: 1108:3e53 */
GLOBAL(struct Part *GOOBER_ARRAY[6], { 0 })

/* Codename. TIMWIN: 1108:0c5e */
GLOBAL(void (*MEL_JUMPY)(struct Part *), 0)

/* Codename. TIMWIN: 1108:3be6 */
GLOBAL(struct Llama *LLAMA_1, 0)

/* Codename. TIMWIN: 1108:3be8 */
GLOBAL(struct Llama *LLAMA_2, 0)

/* Codename. TIMWIN: 1108:3bfb */
GLOBAL(u16 RESIZE_GOPHER, 0);

/* TIMWIN: 1108:3bfd */
GLOBAL(enum LevelState LEVEL_STATE, OBJECTIVE_SCREEN)

GLOBAL(struct Part *STATIC_PARTS_START, 0)
GLOBAL(struct Part *MOVING_PARTS_START, 0)
GLOBAL(struct Part *PARTS_BIN_START, 0)
