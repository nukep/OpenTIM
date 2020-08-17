#include "tim.h"

void part_calculate_border_normals(struct Part *);

int default_bounce(struct Part *part) {
    return 1;
}

void default_run(struct Part *part) {
    return;
}

void default_flip(struct Part *part, int flag) {
    return;
}

void default_reset(struct Part *part) {
    return;
}

void default_resize(struct Part *part) {
    return;
}

int default_rope(struct Part *p1, struct Part *p2, int rope_slot, int flags, sint p1_pass, slong p1_force) {
    return 0;
}

#define ALLOC_BORDERS(part) { if (part->num_borders == 0) { part->borders_data = 0; } else { part->borders_data = malloc(part->num_borders * 4); if (!part->borders_data) { return 1; } }  }

static struct ByteVec BOWLING_BALL_BORDERS[] = {
    { 8, 0 }, { 23, 0 }, { 31, 8 }, { 31, 23 }, { 23, 31 }, { 8, 31 }, { 0, 23 }, { 0, 8 }
};

static inline void set_border(struct Part *part, struct ByteVec *src) {
    for (int i = 0; i < part->num_borders; i++) {
        part->borders_data[i].x = src[i].x;
        part->borders_data[i].y = src[i].y;
    }
    part_calculate_border_normals(part);
}

/* TIMWIN: 1048:051c */
void reset_bowling_ball_and_basketball(struct Part *part) {
    set_border(part, BOWLING_BALL_BORDERS);
}

/* TIMWIN: 1078:023d */
int create_bowling_ball(struct Part *part) {
    ALLOC_BORDERS(part);
    reset_bowling_ball_and_basketball(part);
    return 0;
}

/* TIMWIN: 10d0:0e7a */
// Used for brick wall, pipe wall, wood wall, dirt wall
void reset_wall(struct Part *part) {
    part->borders_data[0].x = 0;
    part->borders_data[0].y = 0;
    part->borders_data[1].x = part->size.x - 1;
    part->borders_data[1].y = 0;
    part->borders_data[2].x = part->size.x - 1;
    part->borders_data[2].y = part->size.y - 1;
    part->borders_data[3].x = 0;
    part->borders_data[3].y = part->size.y - 1;
    part_calculate_border_normals(part);
}

/* TIMWIN: 10d0:0ed8 */
// Used for brick wall, pipe wall, wood wall, dirt wall
void resize_wall(struct Part *part) {
    switch (RESIZE_GOPHER) {
        case 0x8003:
        case 0x8004:
            part->size_something2.y = 16;
            break;

        case 0x8005:
        case 0x8006:
            part->size_something2.x = 16;
            break;
    }

    part->size = part->size_something2;
    part->borders_data[1].x = part->size.x - 1;
    part->borders_data[2].x = part->size.x - 1;

    part->borders_data[2].y = part->size.y - 1;
    part->borders_data[3].y = part->size.y - 1;
}

/* TIMWIN: 1078:0280 */
// Used for brick wall, pipe wall, wood wall, dirt wall
int create_wall(struct Part *part) {
    part->flags1 |= 0x0040;
    part->flags2 |= 0x0180;
    ALLOC_BORDERS(part);
    reset_wall(part);
}

struct PartDef BOWLING_BALL = {
    .flags1 = 0x8000,
    .flags3 = 0x0008,
    .size_something2 = { 32, 32 },
    .size = { 32, 32 },
    .create_func = create_bowling_ball,
    .density = 2832,
    .mass = 200,
    .bounciness = 128,
    .friction = 16,

    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .goobers = { 3, 0xff },
    .borders = 8,
    .part_order = 0,

    .bounce_func = default_bounce,
    .run_func = default_run,
    .reset_func = reset_bowling_ball_and_basketball,
    .flip_func = default_flip,
    .resize_func = default_resize,
    .rope_func = default_rope,
};

struct PartDef BRICK_WALL = {
    .flags1 = 0x4800,
    .flags3 = 0x0000,
    .size_something2 = { 32, 16 },
    .size = { 32, 16 },
    .create_func = create_wall,

    .density = 4153,
    .mass = 1000,
    .bounciness = 1024,
    .friction = 24,
    
    .field_0x0c = 0x00f0,
    .field_0x0e = 0x00f0,
    .field_0x10 = 0x0010,
    .field_0x12 = 0x0010,

    .goobers = { 4, 0xff },
    .borders = 4,
    .part_order = 40,

    .bounce_func = default_bounce,
    .run_func = default_run,
    .reset_func = reset_wall,
    .flip_func = default_flip,
    .resize_func = resize_wall,
    .rope_func = default_rope,
};

struct PartDef *part_def(enum PartType type) {
    switch (type) {
        case P_BOWLING_BALL: return &BOWLING_BALL;
        case P_BRICK_WALL: return &BRICK_WALL;
        default: return 0;
    }
}

int part_create_func(enum PartType type, struct Part *part) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_create_func - def not found");
        return 1;
    }

    return def->create_func(part);
}

uint part_data30_flags1(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data30_flags1 - def not found");
        return 0;
    }

    return def->flags1;
}
uint part_data30_flags3(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data30_flags3 - def not found");
        return 0;
    }

    return def->flags3;
}
struct ShortVec part_data30_size_something2(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data30_size_something2 - def not found");
        return (struct ShortVec){ 0, 0 };
    }

    return def->size_something2;
}
struct ShortVec part_data30_size(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data30_size - def not found");
        return (struct ShortVec){ 0, 0 };
    }

    return def->size;
}
uint part_data31_num_borders(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data31_num_borders - def not found");
        return 0;
    }

    return def->borders;
}

void part_run(struct Part *part) {
    struct PartDef *def = part_def(part->type);
    if (!def) {
        TRACE_ERROR("part_run - def not found");
        return;
    }

    def->run_func(part);
}

int part_bounce(enum PartType type, struct Part *part) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_bounce - def not found");
        return 0;
    }

    return def->bounce_func(part);
}

sint part_mass(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_mass - def not found");
        return 0;
    }

    return def->mass;
}

sint part_acceleration(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_acceleration - def not found");
        return 0;
    }

    return def->acceleration;
}

sint part_terminal_velocity(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_terminal_velocity - def not found");
        return 0;
    }

    return def->terminal_velocity;
}

struct Data31Field0x14** part_data31_field_0x14(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data31_field_0x14 - def not found");
        return 0;
    }

    return def->field_0x14;
}

struct SByteVec* part_data31_field_0x18(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data31_field_0x18 - def not found");
        return 0;
    }

    return def->field_0x18;
}

struct ShortVec* part_data31_field_0x1a(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data31_field_0x1a - def not found");
        return 0;
    }

    return def->field_0x1a;
}