// This file is both used to declare extern'd global variables, and to declare them in globals.c.

// Technique for naming global variables:
// * If we don't know what it does, give it a codename. It's easier to remember (and more fun) than a number.

#ifdef TIM_GLOBALS_C
#   define GLOBAL(decl, init) decl = init;
#else
#   define GLOBAL(decl, init) extern decl;
#endif

/* Codename. TIMWIN: 1108:3e53 */
GLOBAL(struct Part *GOOBER_ARRAY[6], { 0 })

/* Codename. TIMWIN: 1108:0c5e */
GLOBAL(void (*MEL_JUMPY)(struct Part *), 0)

/* Codename. TIMWIN: 1108:3be8 */
GLOBAL(struct Llama *LLAMA, 0)

/* Codename. TIMWIN: 1108:3bfb */
GLOBAL(uint RESIZE_GOPHER, 0);

/* TIMWIN: 1108:3bfd */
GLOBAL(enum LevelState LEVEL_STATE, 0)

GLOBAL(struct Part *STATIC_PARTS_START, 0)
GLOBAL(struct Part *MOVING_PARTS_START, 0)
GLOBAL(struct Part *PARTS_BIN_START, 0)
