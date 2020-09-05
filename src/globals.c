#define TIM_GLOBALS_C
#include "tim.h"

// Expose functions for the WebAssembly build
struct Part* EXPORT static_parts_start() { return STATIC_PARTS_ROOT.next; }
struct Part* EXPORT moving_parts_start() { return MOVING_PARTS_ROOT.next; }
