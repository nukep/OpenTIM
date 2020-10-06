#ifndef TIM_H
#define TIM_H

#include "testing.h"

#include <stdlib.h>

// Used by Emscripten
#define EXPORT __attribute__((used))

// Do nothing
#define TRACE_ERROR(message) 0

#include "int.h"

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
    F1_EPHEMERAL = 0x0010,         // part despawns when we quit the simulation (e.g. bullets, rope fragments)
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

#define EACH_STATIC_PART(varname) for (struct Part *varname = STATIC_PARTS_ROOT.next; varname != 0; varname = varname->next)
#define EACH_MOVING_PART(varname) for (struct Part *varname = MOVING_PARTS_ROOT.next; varname != 0; varname = varname->next)
#define EACH_STATIC_THEN_MOVING_PART(varname) \
    for (struct Part *varname = get_first_part(CHOOSE_STATIC_OR_ELSE_MOVING_PART); varname != 0; varname = next_part_or_fallback(varname, CHOOSE_MOVING_PART))

#define EACH_INTERACION(part, varname) for (struct Part *varname = part->interactions; varname != 0; varname = varname->interactions)

#define VEC_EQ(a, b) ((a).x == (b).x && (a).y == (b).y)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// This one doesn't work in MSVC
// #define SWAP(a, b) { typeof(a) tmp = a; a = b; b = tmp; }

#define SWAP(x, y) \
    { \
        unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp,&y,sizeof(x)); \
        memcpy(&y,&x,       sizeof(x)); \
        memcpy(&x,swap_temp,sizeof(x)); \
    }

// Return true if b is between a and c (exclusive).
#define BETWEEN_EXCL(a, b, c) (((a) < (b)) && ((b) < (c)))
// Return true if b is between a and c (inclusive).
#define BETWEEN_INCL(a, b, c) (((a) <= (b)) && ((b) <= (c)))

#define NO_FLAGS(v, f)  (((v) & (f)) == 0)
#define ANY_FLAGS(v, f) (((v) & (f)) != 0)
#define ALL_FLAGS(v, f) (((v) & (f)) == f)

// struct Data31Field0x14 {
//     s16 field_0x00;        // TIMWIN offset: 00
//     struct ShortVec size;   // TIMWIN offset: 04
// };

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
    // Number of elements is the number of states (state1).
    // OpenTIM - removed
    // we get the image size (the only important part I'm aware of) via part_image_size
    // struct Data31Field0x14 **field_0x14;

    s16 field_0x16;
    // Number of elements is the number of states (state1).
    struct SByteVec *render_pos_offsets;
    // Number of elements is the number of states (state1).
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
    int  (*rope_func)  (struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_mass, s32 p1_force);
};

// Part Def accessors

int part_create_func(enum PartType type, struct Part *part);
u16 part_data30_flags1(enum PartType type);
u16 part_data30_flags3(enum PartType type);
struct ShortVec part_data30_size_something2(enum PartType type);
struct ShortVec part_data30_size(enum PartType type);
u16 part_data31_num_borders(enum PartType type);
void part_run(struct Part *part);
void part_reset(struct Part *part);
int part_bounce(enum PartType type, struct Part *part);
int part_rope(enum PartType type, struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_mass, s32 p1_force);
s16 part_mass(enum PartType type);
s16 part_bounciness(enum PartType type);
s16 part_friction(enum PartType type);
u16 part_order(enum PartType type);
s16 part_acceleration(enum PartType type);
s16 part_terminal_velocity(enum PartType type);
struct SByteVec* part_data31_render_pos_offsets(enum PartType type);
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
    s32 force;                  // TIMWIN offset: 0x04
};

struct Line {
    struct ShortVec p0, p1;
};

void play_sound(int id);

#define DEFAULT_GRAVITY 272
#define DEFAULT_AIR_PRESSURE 67
#define MAX_GRAVITY 512
#define MAX_AIR_PRESSURE 128

static inline s16 approx_hypot(s16 x, s16 y) {
    if (x < y) {
        return x/4 + x/8 + y;
    } else {
        return y/4 + y/8 + x;
    }
}


/* TIMWIN: 10a8:3935 */
static inline struct Part* rope_get_other_part(struct Part *part, struct RopeData *rope) {
    if (!rope) {
        return 0;
    } else if (rope->part1 == part) {
        return rope->part2;
    } else {
        return rope->part1;
    }
}

/* TIMWIN: 10a8:38fc */
static inline int part_get_rope_link_index(struct Part *target, struct Part *from) {
    if (from->links_to[0] == target) return 0;
    if (from->links_to[1] == target) return 1;
    return -1;
}


/* draw_rope.c */
enum RopeTime {
    ROPETIME_PREV2 = 1,
    ROPETIME_PREV1 = 2,
    ROPETIME_CURRENT = 3
};

enum RopeFirstOrLast {
    ROPE_FROM_FIRST = 0,
    ROPE_FROM_LAST = 1
};

s16 approximate_hypot_of_rope(struct RopeData *rope_data, enum RopeTime time, enum RopeFirstOrLast first_or_last);
/* */


/* math.rs */
s16 sine_c(u16 angle);
s16 cosine_c(u16 angle);
void rotate_point_c(s16 *x, s16 *y, u16 angle);
/* */

void unimplemented();
void output_c(const char *);
void output_part_c(struct Part *);
void output_int_c(int64_t);

#include "globals.h"

#endif