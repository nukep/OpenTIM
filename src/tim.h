#ifndef TIM_H
#define TIM_H

#include "testing.h"

#include <stdlib.h>

// Used by Emscripten
#define EXPORT __attribute__((used))

// Do nothing
#define TRACE_ERROR(message) 0

/* TIM was originally written for 16-bit architectures.
   Our primary "int" type is therefore 16-bit. */
typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef int bool;

struct ByteVec {
    byte x, y;
};

struct SByteVec {
    sbyte x, y;
};

struct ShortVec {
    s16 x, y;
};

struct LongVec {
    s32 x, y;
};

struct BorderPoint {
    byte x, y;
    u16 normal_angle;
};

enum Flags1_Flags {
    // TODO - put flags here
    F1_placeholder
};

enum Flags2_Flags {
    F2_FLIP_HORZ = 0x0010,
    F2_FLIP_VERT = 0x0020,
};

enum Flags3_Flags {
    // TODO - put flags here
    F3_placeholder
};

#include "generated/structs.h"
#include "parttype.h"

enum GetPartsFlags {
    CHOOSE_FROM_PARTS_BIN=0x800,
    CHOOSE_MOVING_PART=0x1000,
    CHOOSE_STATIC_PART=0x2000,
    CHOOSE_STATIC_OR_ELSE_MOVING_PART=0x3000
};

struct Part* get_first_part(int choice);
struct Part* next_part_or_fallback(struct Part *part, int choice);

#define EACH_STATIC_PART(varname, body) { struct Part *varname = STATIC_PARTS_START; for (; varname != 0; varname = varname->next) body }
#define EACH_MOVING_PART(varname, body) { struct Part *varname = MOVING_PARTS_START; for (; varname != 0; varname = varname->next) body }
#define EACH_STATIC_THEN_MOVING_PART(varname, body) { \
    struct Part *varname = get_first_part(CHOOSE_STATIC_OR_ELSE_MOVING_PART); \
    for (; varname != 0; varname = next_part_or_fallback(varname, CHOOSE_MOVING_PART)) body \
}
#define EACH_INTERACION(part, varname, body) for (struct Part *varname = part->interactions; varname != 0; varname = varname->interactions) body

#define VEC_EQ(a, b) ((a).x == (b).x && (a).y == (b).y)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Return true if b is between a and c (exclusive).
#define BETWEEN_EXCL(a, b, c) (((a) < (b)) && ((b) < (c)))
// Return true if b is between a and c (inclusive).
#define BETWEEN_INCL(a, b, c) (((a) <= (b)) && ((b) <= (c)))

#define NO_FLAGS(v, f)  (((v) & (f)) == 0)
#define ANY_FLAGS(v, f) (((v) & (f)) != 0)
#define ALL_FLAGS(v, f) (((v) & (f)) == f)

struct Data31Field0x14 {
    s16 field_0x00;        // TIMWIN offset: 00
    struct ShortVec size;   // TIMWIN offset: 04
};

struct PartDef {
    // Fields henceforth correlate to Segment 30 in TIMWIN

    u16 flags1;                        // TIMWIN offset: 00
    u16 flags3;                        // TIMWIN offset: 02
    struct ShortVec size_something2;    // TIMWIN offset: 04
    struct ShortVec size;               // TIMWIN offset: 08
    int (*create_func)(struct Part *);  // TIMWIN offset: 0A

    // Fields henceforth correlate to Segment 31 in TIMWIN

    s16 density;               // TIMWIN offset: 00
    s16 mass;                  // TIMWIN offset: 02
    s16 bounciness;            // TIMWIN offset: 04
    s16 friction;              // TIMWIN offset: 06

    // calculated:
    // OpenTIM - removed
    // TIM would modify this data structure whenver the air pressure or gravity changed, to pre-calculate these properties.
    // To keep the implementation simple, we'll recalculate these each time.
    // s16 acceleration;          // TIMWIN offset: 08
    // s16 terminal_velocity;     // TIMWIN offset: 0A

    s16 field_0x0c;
    s16 field_0x0e;
    s16 field_0x10;
    s16 field_0x12;

    // field_0x14 is initialized later.
    struct Data31Field0x14 **field_0x14;
    s16 field_0x16;
    struct SByteVec *field_0x18;
    struct ShortVec *field_0x1a;

    // "Goobers" is a codename.
    byte goobers[2];            // TIMWIN offset: 0x1c

    u16 borders;               // TIMWIN offset: 0x1e
    u16 part_order;            // TIMWIN offset: 0x20

    int  (*bounce_func)(struct Part *);
    void (*run_func)   (struct Part *);
    void (*reset_func) (struct Part *);
    void (*flip_func)  (struct Part *, int);
    void (*resize_func)(struct Part *);
    int  (*rope_func)  (struct Part *p1, struct Part *p2, int rope_slot, int flags, s16 p1_mass, s32 p1_force);
};

// Part Def accessors

int part_create_func(enum PartType type, struct Part *part);
u16 part_data30_flags1(enum PartType type);
u16 part_data30_flags3(enum PartType type);
struct ShortVec part_data30_size_something2(enum PartType type);
struct ShortVec part_data30_size(enum PartType type);
u16 part_data31_num_borders(enum PartType type);
void part_run(struct Part *part);
int part_bounce(enum PartType type, struct Part *part);
s16 part_mass(enum PartType type);
s16 part_acceleration(enum PartType type);
s16 part_terminal_velocity(enum PartType type);
struct Data31Field0x14** part_data31_field_0x14(enum PartType type);
struct SByteVec* part_data31_field_0x18(enum PartType type);
struct ShortVec* part_data31_field_0x1a(enum PartType type);


enum LevelState {
    OBJECTIVE_SCREEN      = 0x0000,
    unknown_0004          = 0x0004,
    unknown_0010          = 0x0010,
    unknown_0080          = 0x0080,
    unknown_0100          = 0x0100,
    BEAT_THE_LEVEL        = 0x0200,
    unknown_0400          = 0x0400,
    unknown_0800          = 0x0800,
    DESIGN_MODE           = 0x1000,
    SIMULATION_MODE       = 0x2000,
    unknown_4000          = 0x4000,
    REPLAY_OR_ADVANCE_BOX = 0x8000
};

// Codename
struct Llama {
    struct Llama *next;         // TIMWIN offset: 0x00
    struct Part *part;          // TIMWIN offset: 0x02
};

struct Line {
    struct ShortVec p0, p1;
};

void play_sound(int id);

static const u16 DEFAULT_GRAVITY = 272;
static const u16 DEFAULT_AIR_PRESSURE = 67;
#define MAX_GRAVITY 512
#define MAX_AIR_PRESSURE 128

#include "globals.h"

#endif