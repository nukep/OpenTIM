#include "tim.h"

void part_calculate_border_normals(struct Part *);
void part_set_size_and_pos_render(struct Part *part);
void stub_10a8_2b6d(struct Part *part, int c);
int stub_1050_02db(struct Part *part);
bool calculate_line_intersection(const struct Line *a, const struct Line *b, struct ShortVec *out);
struct Part* part_new(enum PartType type);
void insert_part_into_static_parts(struct Part *part);
void insert_part_into_moving_parts(struct Part *part);

struct PartDef *part_def(enum PartType type);


u16 part_data30_flags1(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data30_flags1 - def not found");
        return 0;
    }

    return def->flags1;
}
u16 part_data30_flags3(enum PartType type) {
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
u16 part_data31_num_borders(enum PartType type) {
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

void part_reset(struct Part *part) {
    struct PartDef *def = part_def(part->type);
    if (!def) {
        TRACE_ERROR("part_reset - def not found");
        return;
    }

    def->reset_func(part);
}

int part_bounce(enum PartType type, struct Part *part) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_bounce - def not found");
        return 0;
    }

    return def->bounce_func(part);
}

int part_rope(enum PartType type, struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_mass, s32 p1_force) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_rope - def not found");
        return 0;
    }

    return def->rope_func(p1, p2, rope_slot, flags, p1_mass, p1_force);
}

s16 part_density(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_density - def not found");
        return 0;
    }

    return def->density;
}

s16 part_mass(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_mass - def not found");
        return 0;
    }

    return def->mass;
}

s16 part_bounciness(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_bounciness - def not found");
        return 0;
    }

    return def->bounciness;
}

s16 part_friction(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_friction - def not found");
        return 0;
    }

    return def->friction;
}

u16 part_order(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_order - def not found");
        return 0;
    }

    return def->part_order;
}

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

int default_rope(struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_pass, s32 p1_force) {
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
    return 0;
}

/* TIMWIN: 10d0:0114 */
void reset_teeter_totter(struct Part *part) {
    /* TIMWIN: 1108:2e9e */
    static byte dat_2e9e[3][2] = { { 0x05, 0x1b }, { 0x04, 0x02 }, { 0x06, 0x03 } };
    /* TIMWIN: 1108:2eaa */
    static byte dat_2eaa[3][2] = { { 0x49, 0x03 }, { 0x4b, 0x02 }, { 0x4a, 0x1b } };

    /* TIMWIN: 1108:2eb6 */
    static byte teeter_borders[3][8][2] = {
        { { 0x00, 0x20 }, { 0x4f, 0x03 }, { 0x4f, 0x08 }, { 0x2c, 0x15 }, { 0x2c, 0x22 }, { 0x24, 0x22 }, { 0x24, 0x18 }, { 0x00, 0x24 } },
        { { 0x00, 0x11 }, { 0x4F, 0x11 }, { 0x4F, 0x15 }, { 0x2C, 0x15 }, { 0x2C, 0x22 }, { 0x24, 0x22 }, { 0x24, 0x15 }, { 0x00, 0x15 } },
        { { 0x00, 0x03 }, { 0x4F, 0x20 }, { 0x4F, 0x24 }, { 0x2C, 0x18 }, { 0x2C, 0x22 }, { 0x24, 0x22 }, { 0x24, 0x15 }, { 0x00, 0x08 } }
    };

    part->rope_loc[0].x = dat_2e9e[part->state1][0];
    part->rope_loc[0].y = dat_2e9e[part->state1][1];
    part->rope_loc[1].x = dat_2eaa[part->state1][0];
    part->rope_loc[1].y = dat_2eaa[part->state1][1];

    // Set Teeter-Totter's border based on its state.
    if (part->state1 < 3) {
        for (int i = 0; i < 8; i++) {
            part->borders_data[i].x = teeter_borders[part->state1][i][0];
            part->borders_data[i].y = teeter_borders[part->state1][i][1];
        }
    }

    part_calculate_border_normals(part);
}

/* TIMWIN: 1078:0323 */
int create_teeter_totter(struct Part *part) {
    part->flags1 |= 0x0400;
    part->flags2 |= 0x000c;
    part->borders_data = malloc(part->num_borders * 4);
    if (!part->borders_data) {
        return 1;
    }

    reset_teeter_totter(part);
    return 0;
}

/* TIMWIN: 1058:1591 */
void reset_unknown37(struct Part *part) {
    byte x1 = 0;
    byte y1 = 0;
    byte x2 = 0x54;
    byte y2 = part->size.y - 1;

    if (part->state1 != 1) {
        part->borders_data[0].x = x1;
        part->borders_data[0].y = y1;
        part->borders_data[1].x = x2;
        part->borders_data[1].y = y1;
        part->borders_data[2].x = x2;
        part->borders_data[2].y = y2;
        part->borders_data[3].x = x1;
        part->borders_data[3].y = y2;
    }

    part_calculate_border_normals(part);
    part->rope_loc[0].x = part->size.x / 2;
    part->rope_loc[0].y = 0;
}

/* TIMWIN: 1058:1591 */
void reset_unknown39(struct Part *part) {
    byte x1, y1, x2, y2;
    if (part->state1 == 8) {
        x1 = 0;
        y1 = 10;
        x2 = 0x69;
        y2 = part->size.y - 1;
    } else if (part->state1 == 0) {
        x1 = 0x6e;
        y1 = 0;
        x2 = 0x6f;
        y2 = 1;
    } else {
        x1 = 0;
        y1 = 0;
        x2 = part->size.x - 1;
        y2 = part->size.y - 1;
    }

    part->borders_data[0].x = x1;
    part->borders_data[0].y = y1;
    part->borders_data[1].x = x2;
    part->borders_data[1].y = y1;
    part->borders_data[2].x = x2;
    part->borders_data[2].y = y2;
    part->borders_data[3].x = x1;
    part->borders_data[3].y = y2;

    part_calculate_border_normals(part);
    part->rope_loc[0].x = part->size.x / 2;
    part->rope_loc[0].y = 0;
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

/* TIMWIN: 1090:11dc */
void part_find_interactions(struct Part *part, enum GetPartsFlags choice, s16 hitbox_left, s16 hitbox_right, s16 hitbox_top, s16 hitbox_bottom) {
    part->interactions = 0;

    s16 x = part->pos.x;
    s16 y = part->pos.y;

    for (struct Part *curpart = get_first_part(choice); curpart != 0; curpart = next_part_or_fallback(curpart, choice)) {
        if (curpart == part) continue;
        if (ANY_FLAGS(curpart->flags2, 0x2000)) continue;

        s16 cx = curpart->pos.x;
        s16 cy = curpart->pos.y;
        s16 csx = cx + curpart->size.x;
        s16 csy = cy + curpart->size.y;

        if (curpart->type == P_TEAPOT) {
            csx = cx + 30;
        }

        if ((csx > x + hitbox_left) && (cx < x + hitbox_right)) {
            if ((csy > y + hitbox_top) && (cy < y + hitbox_bottom)) {
                // Prepend curpart to part->interactions linked list.

                curpart->interactions = part->interactions;
                part->interactions = curpart;
            }
        }
    }
}

/* TIMWIN: 1088:0f4b */
void mort_the_mouse_cage_start(struct Part *part) {
    if (part->state2 == 0) {
        play_sound(0x0d);
    }
    if (NO_FLAGS(part->flags2, F2_FLIP_HORZ)) {
        part->state2 = 1;
    } else {
        part->state2 = -1;
    }
    part->extra1 = 100;
}

/* TIMWIN: 1070:1b36 */
void bob_the_fish_break_bowl(struct Part *part) {
    if (part->state1 < 0xb) {
        part->state1 = 0xb;
        part_set_size_and_pos_render(part);
        play_sound(10);

        part->num_borders = 3;

        part->borders_data[0].x = 8;
        part->borders_data[0].y = 47;

        part->borders_data[1].x = 24;
        part->borders_data[1].y = 44;

        part->borders_data[2].x = 39;
        part->borders_data[2].y = 47;
    }
}

/* TIMWIN: 10d0:07a1 */
void teeter_totter_helper_1(struct Part *part, bool is_bottom, s16 offset_x) {
    s16 x = part->pos.x;
    EACH_INTERACION(part, curpart) {
        s16 ivar2 = (x + offset_x) - curpart->pos.x;

        enum PartType pt = curpart->type;

        if (pt == P_DYNAMITE_WITH_PLUNGER) {
            if (is_bottom) {
                if (NO_FLAGS(curpart->flags2, F2_FLIP_HORZ)) {
                    if (ivar2 > 0x66 && ivar2 < 0x88) {
                        curpart->state2 = 1;
                    }
                } else {
                    if (ivar2 >= 0 && ivar2 < 0x20) {
                        curpart->state2 = 1;
                    }
                }
            }
        } else if (pt == P_MORT_THE_MOUSE_CAGE) {
            mort_the_mouse_cage_start(curpart);
        } else if (pt == P_BOB_THE_FISH) {
            bob_the_fish_break_bowl(curpart);
        } else if (pt == P_BELLOWS) {
            if (NO_FLAGS(curpart->flags2, F2_FLIP_HORZ)) {
                if (ivar2 >= 0 && ivar2 < 9) {
                    curpart->state2 = 1;
                }
            } else {
                if (ivar2 > 0x35 && ivar2 < 0x3d) {
                    curpart->state2 = 1;
                } 
            }
        } else if (pt == P_FLASHLIGHT) {
            if (is_bottom) {
                if (NO_FLAGS(curpart->flags2, F2_FLIP_HORZ)) {
                    if (ivar2 > 4 && ivar2 < 0x11) {
                        curpart->state2 = 1;
                    }
                } else {
                    if (ivar2 > 0xc && ivar2 < 0x19) {
                        curpart->state2 = 1;
                    }
                }
            }
        } else if (pt == P_SCISSORS) {
            if (NO_FLAGS(curpart->flags2, F2_FLIP_HORZ)) {
                if (ivar2 >= 0 && ivar2 < 0xd) {
                    curpart->state2 = 1;
                }
            } else {
                if (ivar2 > 0x18 && ivar2 < 0x26) {
                    curpart->state2 = 1;
                } 
            }
        }
    }
}

/*
enum RopeFlags {
    0x01,

    // Y-related
    0x02,
    0x04,

    // X-related
    0x08,
    0x10
};
*/

/* TIMWIN: 10a8:376e */
u16 rope_calculate_flags(struct RopeData *rope, int param_2, int param_3) {
    struct Part *local14;
    byte bvar1, bvar2;
    struct Part *ppvar4;

    if (param_2 == 0) {
        local14 = rope->part1;
        bvar1 = rope->part1_rope_slot;
        ppvar4 = rope->part2;
        bvar2 = rope->part2_rope_slot;
    } else {
        local14 = rope->part2;
        bvar1 = rope->part2_rope_slot;
        ppvar4 = rope->part1;
        bvar2 = rope->part1_rope_slot;
    }

    u16 local6 = bvar1;
    u16 local8 = bvar2;
    struct Part *ppvar3 = local14->links_to[local6];

    struct RopeData *local10;
    struct RopeData *local12;
    if (ppvar3 == ppvar4) {
        local12 = rope;
        local10 = rope;
    } else {
        local10 = ppvar3->rope_data[0];
        local12 = ppvar4->links_to[local8]->rope_data[0];
    }

    u16 flags;

    if (local12->ends_pos[param_2].x < rope->ends_pos[1 - param_2].x) {
        flags = 0x0008;
    } else {
        flags = 0x0010;
    }

    if (param_3 != 0) {
        if (rope->ends_pos[param_2].y < local10->ends_pos[1 - param_2].y) {
            return 0x0001;
        } else if (rope->ends_pos[1 - param_2].y < local12->ends_pos[param_2].y) {
            return flags | 0x0004;
        } else {
            return flags | 0x0002;
        }
    } else {
        if (rope->ends_pos[param_2].y > local10->ends_pos[1 - param_2].y) {
            return 0x0001;
        } else if (rope->ends_pos[1 - param_2].y > local12->ends_pos[param_2].y) {
            return flags | 0x0002;
        } else {
            return flags | 0x0004;
        }
    }
}

/* TIMWIN: 10d0:0627 */
int teeter_totter_helper_2(struct Part *exclude_part, struct Part *part, u16 flags, s16 mass, s32 force) {
    if (ANY_FLAGS(part->flags2, 0x0200)) {
        return 1;
    }

    for (int i = 0; i < 2; i++) {
        struct RopeData *rpd = part->rope_data[i];
        if (!rpd) continue;

        struct Part *other_part = rope_get_other_part(part, rpd);
        if (other_part == exclude_part) continue;

        byte bvar1, bvar2;

        if (rpd->part1 != part) {
            bvar1 = rpd->part2_rope_slot;
            bvar2 = rpd->part1_rope_slot;
        } else {
            bvar1 = rpd->part1_rope_slot;
            bvar2 = rpd->part2_rope_slot;
        }

        int rope_slot = bvar2;
        int ivar4;

        if (part->state2 < 1) {
            ivar4 = bvar1 == 0 ? 1 : 0;
        } else {
            ivar4 = bvar1 == 0 ? 0 : 1;
        }

        u16 new_flags = rope_calculate_flags(rpd, rpd->part1 == part ? 0 : 1, ivar4);

        int res = part_rope(other_part->type, part, other_part, rope_slot, new_flags | flags, mass, force);
        if (res != 0) {
            return res;
        }
    }

    return 0;
}

/* TIMWIN: 10d0:0731 */
s16 teeter_totter_helper_get_part_speed(struct Part *part) {
    s16 mass = part_mass(part->type);
    if (mass < 2) {
        return 0x1c00;
    }
    if (mass < 6) {
        return 0x1a00;
    }
    if (mass < 10) {
        return 0x1800;
    }
    if (mass < 21) {
        return 0x1600;
    }
    if (mass < 121) {
        return 0x1400;
    }
    if (mass < 151) {
        return 0x1200;
    }
    return 0x1000;
}

/* TIMWIN: 1090:1289 */
void stub_1090_1289(struct Part *teeter_part, enum GetPartsFlags choice, const struct Line *line) {
    teeter_part->interactions = 0;
    for (struct Part *curpart = get_first_part(choice); curpart != 0; curpart = next_part_or_fallback(curpart, choice & CHOOSE_MOVING_PART)) {
        int border_idx = 1;
        struct BorderPoint *border_point = curpart->borders_data;
        s16 cx1 = curpart->pos.x + border_point[0].x;
        s16 cy1 = curpart->pos.y + border_point[0].y;
        s16 cx2 = curpart->pos.x + border_point[1].x;
        s16 cy2 = curpart->pos.y + border_point[1].y;

        s16 x, y;
        x = cx1;
        y = cy1;

        while (border_point) {
            struct Line line2 = { { x   - teeter_part->pos.x, y   - teeter_part->pos.y },
                                  { cx2 - teeter_part->pos.x, cy2 - teeter_part->pos.y } };
            struct ShortVec _point;

            if (calculate_line_intersection(line, &line2, &_point)) {
                curpart->interactions = teeter_part->interactions;
                teeter_part->interactions = curpart;
                border_idx = curpart->num_borders;
            }
            border_idx += 1;
            if (curpart->num_borders < border_idx) {
                border_point = 0;
            } else {
                x = cx2;
                y = cy2;
                if (curpart->num_borders == border_idx) {
                    cx2 = cx1;
                    cy2 = cy1;
                } else {
                    cx2 = curpart->pos.x + border_point[2].x;
                    cy2 = curpart->pos.y + border_point[2].y;
                }
                border_point += 1;
            }
        }
    }
}

/* TIMWIN: 10d0:0240 */
void run_teeter_totter(struct Part *part) {
    if (part->state2 != 0) {
        part->flags2 |= 0x0040;
        if (NO_FLAGS(part->flags2, 0x0400)) {
            int res = teeter_totter_helper_2(0, part, 0x8000, 1000, part->force);
            if (res == 0) {
                teeter_totter_helper_2(0, part, 0, 1000, part->force);
                part->state1 += part->state2;
            } else {
                part->flags2 |= 0x0200;
            }
        } else {
            part->state1 += part->state2;
        }
        if (part->state1 != part->state1_prev1) {
            reset_teeter_totter(part);
            if (part->state1_prev1 == 0 || part->state1_prev1 == 2) {
                play_sound(0x12);
            }
            part_set_size_and_pos_render(part);
            s16 teeter_center_x = part->pos.x + part->size.x/2;

            /* TIMWIN: 1108:2f16 */
            static const struct Line dat_1108_2f16[3] = {
                { { 0, 32 }, { 79, 3 } },
                { { 0, 17 }, { 79, 17 } },
                { { 0, 3 },  { 79, 32 } }
            };

            stub_1090_1289(part, CHOOSE_MOVING_PART, &dat_1108_2f16[part->state1]);

            EACH_INTERACION(part, curpart) {
                s16 curpart_center_x = curpart->pos.x + curpart->size.x / 2;
                s16 speed = teeter_totter_helper_get_part_speed(curpart);
                if (part->state2 == -1) {
                    if (curpart_center_x < teeter_center_x) {
                        curpart->vel_hi_precision.x = -speed / 4;
                        curpart->vel_hi_precision.y = speed;
                    } else {
                        curpart->vel_hi_precision.x = speed / 4;
                        curpart->vel_hi_precision.y = -speed;
                    }
                } else if (part->state2 == 1) {
                    if (curpart_center_x < teeter_center_x) {
                        curpart->vel_hi_precision.x = -speed / 4;
                        curpart->vel_hi_precision.y = -speed;
                    } else {
                        curpart->vel_hi_precision.x = speed / 4;
                        curpart->vel_hi_precision.y = speed;
                    }
                }
                stub_10a8_2b6d(curpart, 3);
                if (curpart->vel_hi_precision.y < 0) {
                    curpart->pos_prev1.y = curpart->pos.y - 16;
                    stub_1050_02db(curpart);
                    curpart->pos_prev1.y = curpart->pos.y + 16;

                    part->flags2 |= 0x2000;
                    stub_1050_02db(curpart);
                    part->flags2 &= ~0x2000; 

                    curpart->pos_prev1.y = curpart->pos.y;
                    curpart->pos_y_hi_precision = curpart->pos.y * 512;
                } else {
                    curpart->pos_prev1.y = curpart->pos.y + 16;
                    stub_1050_02db(curpart);
                    curpart->pos_prev1.y = curpart->pos.y - 16;

                    part->flags2 |= 0x2000;
                    stub_1050_02db(curpart);
                    part->flags2 &= ~0x2000;

                    curpart->pos_prev1.y = curpart->pos.y;
                    curpart->pos_y_hi_precision = (curpart->pos.y + 1) * 512 - 1;
                }
            }
        }
        part->state2 = 0;
        part->force = 0;
    }
    if (part->state1_prev1 == 0 && part->state1_prev2 != 0) {
        //      |--x 0
        // 1 x--|
        part_find_interactions(part, CHOOSE_STATIC_PART, 74, 79, -2, 2);
        teeter_totter_helper_1(part, 0, 74);
        part_find_interactions(part, CHOOSE_STATIC_PART, 0, 6, 32, 36);
        teeter_totter_helper_1(part, 1, 0);
    } else if (part->state1_prev1 == 2 && part->state1_prev2 != 2) {
        // 0 x--|
        //      |--x 1
        part_find_interactions(part, CHOOSE_STATIC_PART, 74, 79, 32, 36);
        teeter_totter_helper_1(part, 1, 74);
        part_find_interactions(part, CHOOSE_STATIC_PART, 0, 6, -2, 2);
        teeter_totter_helper_1(part, 0, 0);
    }
}

/* TIMWIN: 1048:067e */
void reset_balloon(struct Part *part) {
    /* TIMWIN: 1108:0b18 */
    static struct ByteVec balloon_borders[8] = {
        { 0x00, 0x0a }, { 0x0c, 0x00 }, { 0x16, 0x00 }, { 0x1f, 0x0a }, { 0x1f, 0x1c }, { 0x13, 0x2b }, { 0x0b, 0x2b }, { 0x00, 0x1d }
    };

    set_border(part, balloon_borders);
}

/* TIMWIN: 1078:036f */
int create_balloon(struct Part *part) {
    part->flags1 |= 0x20;
    part->flags2 |= 0x04;
    part->rope_loc[0].x = 16;
    part->rope_loc[0].y = 47;
    ALLOC_BORDERS(part);
    reset_balloon(part);
    return 0;
}

/* TIMWIN: 1048:06f4 */
void run_balloon(struct Part *part) {
    if (part->state2 == 0) return;

    part->flags2 |= 0x0040;

    if (part->state1 == 6) {
        stub_10a8_2b6d(part, 3);
        part->flags2 |= 0x2000;
        return;
    }

    if (part->state2 == 1 && part->rope_data[0]) {
        struct Part *severed_part = part_new(P_ROPE_SEVERED_END);
        insert_part_into_moving_parts(severed_part);
        severed_part->flags1 |= 0x0010;
        severed_part->rope_data[0] = part->rope_data[0];
        severed_part->links_to[0] = part->links_to[0];
        struct Part *links_to = severed_part->links_to[0];

        int i = part_get_rope_link_index(part, severed_part->links_to[0]);
        if (i != -1) {
            links_to->links_to[i] = severed_part;
        }
        if (part->rope_data[0]->part1 == part) {
            part->rope_data[0]->part1 = severed_part;
            severed_part->pos = part->rope_data[0]->ends_pos[0];
        } else {
            part->rope_data[0]->part2 = severed_part;
            severed_part->pos = part->rope_data[0]->ends_pos[1];
        }
        severed_part->pos_x_hi_precision = severed_part->pos.x * 512;
        severed_part->pos_y_hi_precision = severed_part->pos.y * 512;
        part_set_size_and_pos_render(severed_part);
        part->rope_data[0] = 0;
        part->links_to[0] = 0;
    }
    if (part->state1 == 0) {
        play_sound(0xe);
    }
    part->state1 += 1;
    part_set_size_and_pos_render(part);
}

/* TIMWIN: 1048:082b */
int rope_balloon(struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_pass, s32 p1_force) {
    if (flags == 0x0001) {
        p2->rope_data[0]->rope_unknown += 1;
        return 0;
    }
    if (p1->type == P_TEETER_TOTTER) {
        if (p1_force < p2->force) {
            return 1;
        } else {
            return 0;
        }
    } else {
        if (p1_force < p2->force*2) {
            return 1;
        } else {
            return 0;
        }
    }
}

struct PartDef BALLOON = {
    .flags1 = 0x0800,
    .flags3 = 0x0008,
    .size_something2 = { 32, 48 },
    .size = { 32, 48 },
    .create_func = create_balloon,

    .density = 9,
    .mass = 1,
    .bounciness = 64,
    .friction = 32,
    
    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .goobers = { 3, 0xff },
    .borders = 8,
    .part_order = 5,

    .bounce_func = default_bounce,
    .run_func = run_balloon,
    .reset_func = reset_balloon,
    .flip_func = default_flip,
    .resize_func = default_resize,
    .rope_func = rope_balloon,
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

/* Partial from TIMWIN: 1090:0000 */
// Returns a value from 0 to 1024 inclusive.
static inline u16 calculate_adj_grav(u16 gravity) {
    if (gravity < 140) {
        return gravity/4 + 1;
    } else if (gravity > 278) {
        return gravity*2;
    } else {
        return gravity;
    }
}

/* Partial from TIMWIN: 1090:0000 */
// Returns a value from 0 to 2048 inclusive.
static inline u16 calculate_adj_air(u16 air_pressure) {
    if (air_pressure < 70) {
        return air_pressure/2;
    } else {
        return air_pressure*16;
    }
}

/* Partial from TIMWIN: 1090:0000 */
static inline s16 calculate_acceleration(u16 gravity, u16 air_pressure, s16 density) {
    u16 adj_air = calculate_adj_air(air_pressure);

    if (density == adj_air) {
        return 0;
    }

    u16 adj_grav = calculate_adj_grav(gravity);

    if (density > adj_air) {
        return +(adj_grav - (((s32)adj_air * (s32)adj_grav) / density));
    } else {
        return -(adj_grav - (((s32)density * (s32)adj_grav) / adj_air));
    }
}

/* Partial from TIMWIN: 1090:0000 */
static inline s16 calculate_terminal_velocity(s16 air_pressure) {
    u16 adj_air = calculate_adj_air(air_pressure);
    // If max of adj_air is 2048 (0x0800), then this ranges from 0x1E00 to 0x2600 inclusive.
    return 0x2600 - adj_air;
}

#if ENABLE_TEST_SUITE

/* The following fixtures were taken by inspecting the memory of the Parts table (segment 31) from the original TIMWIN.
   The pairs are: { density, acceleration }
*/

static const s16 EARTH_TEST_FIXTURE[21][2] = {
    {     0,  -272 },
    {     9,  -198 },
    {    11,  -182 },
    {   100,   183 },
    {  1132,   265 },
    {  1300,   266 },
    {  1322,   266 },
    {  1500,   267 },
    {  1510,   267 },
    {  1600,   267 },
    {  1800,   268 },
    {  1888,   268 },
    {  2000,   268 },
    {  2400,   269 },
    {  2832,   269 },
    {  3776,   270 },
    {  4153,   270 },
    {  7552,   271 },
    { 14726,   272 },
    { 18000,   272 },
    { 21428,   272 },
};

static const s16 S2_TEST_FIXTURE[21][2] = {
    {     0,    -1 },
    {     9,    -1 },
    {    11,    -1 },
    {   100,     1 },
    {  1132,     1 },
    {  1300,     1 },
    {  1322,     1 },
    {  1500,     1 },
    {  1510,     1 },
    {  1600,     1 },
    {  1800,     1 },
    {  1888,     1 },
    {  2000,     1 },
    {  2400,     1 },
    {  2832,     1 },
    {  3776,     1 },
    {  4153,     1 },
    {  7552,     1 },
    { 14726,     1 },
    { 18000,     1 },
    { 21428,     1 },
};

static const s16 S3_TEST_FIXTURE[21][2] = {
    {     0, -1024 },
    {     9,  -745 },
    {    11,  -683 },
    {   100,   687 },
    {  1132,   995 },
    {  1300,   999 },
    {  1322,   999 },
    {  1500,  1002 },
    {  1510,  1002 },
    {  1600,  1003 },
    {  1800,  1006 },
    {  1888,  1007 },
    {  2000,  1008 },
    {  2400,  1010 },
    {  2832,  1013 },
    {  3776,  1016 },
    {  4153,  1016 },
    {  7552,  1020 },
    { 14726,  1022 },
    { 18000,  1023 },
    { 21428,  1023 },
};

static const s16 S4_TEST_FIXTURE[21][2] = {
    {     0,     0 },
    {     9,   272 },
    {    11,   272 },
    {   100,   272 },
    {  1132,   272 },
    {  1300,   272 },
    {  1322,   272 },
    {  1500,   272 },
    {  1510,   272 },
    {  1600,   272 },
    {  1800,   272 },
    {  1888,   272 },
    {  2000,   272 },
    {  2400,   272 },
    {  2832,   272 },
    {  3776,   272 },
    {  4153,   272 },
    {  7552,   272 },
    { 14726,   272 },
    { 18000,   272 },
    { 21428,   272 },
};

static const s16 S5_TEST_FIXTURE[21][2] = {
    {     0,  -272 },
    {     9,  -271 },
    {    11,  -271 },
    {   100,  -259 },
    {  1132,  -122 },
    {  1300,  -100 },
    {  1322,   -97 },
    {  1500,   -73 },
    {  1510,   -72 },
    {  1600,   -60 },
    {  1800,   -33 },
    {  1888,   -22 },
    {  2000,    -7 },
    {  2400,    40 },
    {  2832,    76 },
    {  3776,   125 },
    {  4153,   138 },
    {  7552,   199 },
    { 14726,   235 },
    { 18000,   242 },
    { 21428,   247 },
};

static const s16 S6_TEST_FIXTURE[21][2] = {
    {     0,     0 },
    {     9,     1 },
    {    11,     1 },
    {   100,     1 },
    {  1132,     1 },
    {  1300,     1 },
    {  1322,     1 },
    {  1500,     1 },
    {  1510,     1 },
    {  1600,     1 },
    {  1800,     1 },
    {  1888,     1 },
    {  2000,     1 },
    {  2400,     1 },
    {  2832,     1 },
    {  3776,     1 },
    {  4153,     1 },
    {  7552,     1 },
    { 14726,     1 },
    { 18000,     1 },
    { 21428,     1 },
};

static const s16 S7_TEST_FIXTURE[21][2] = {
    {     0, -1024 },
    {     9, -1020 },
    {    11, -1019 },
    {   100,  -974 },
    {  1132,  -458 },
    {  1300,  -374 },
    {  1322,  -363 },
    {  1500,  -274 },
    {  1510,  -269 },
    {  1600,  -224 },
    {  1800,  -124 },
    {  1888,   -80 },
    {  2000,   -24 },
    {  2400,   151 },
    {  2832,   284 },
    {  3776,   469 },
    {  4153,   520 },
    {  7552,   747 },
    { 14726,   882 },
    { 18000,   908 },
    { 21428,   927 },
};

#define T(description, gravity, air_pressure, expected_terminal_velocity, fixture) \
TEST("[" description "] gravity=" #gravity ", air=" #air_pressure ", expected terminal velocity=" #expected_terminal_velocity) { \
    for (int i = 0; i < 21; i++) { \
        s16 density      = fixture[i][0]; \
        s16 acceleration = fixture[i][1]; \
        ASSERT_EQ(calculate_acceleration(gravity, air_pressure, density), acceleration); \
    } \
\
    ASSERT_EQ(calculate_terminal_velocity(air_pressure), expected_terminal_velocity); \
}

TEST_SUITE(acceleration_and_terminal_velocity) {
    // description,             gravity, air, expected-term-vel, fixture
    T("Earth",                  272,     67,  9695,              EARTH_TEST_FIXTURE)
    T("Min gravity, earth air", 0,       67,  9695,              S2_TEST_FIXTURE)
    T("Max gravity, earth air", 512,     67,  9695,              S3_TEST_FIXTURE)
    T("Earth gravity, min air", 272,     0,   9728,              S4_TEST_FIXTURE)
    T("Earth gravity, max air", 272,     128, 7680,              S5_TEST_FIXTURE)
    T("Min gravity, min air",   0,       0,   9728,              S6_TEST_FIXTURE)
    T("Max gravity, max air",   512,     128, 7680,              S7_TEST_FIXTURE)
}

#undef T
#endif

/* Partial from TIMWIN: 1090:0000 */
// Was pre-calculated in TIM each time the air pressure or gravity changed. Now we just recalculate it each time.
// TODO is to memoize this call if performance calls for it.
s16 EXPORT part_acceleration(enum PartType type) {
    if (type == P_GUN_BULLET) {
        return 0;
    }
    if (type == P_EIGHTBALL) {
        return 0;
    }

    return calculate_acceleration(GRAVITY, AIR_PRESSURE, part_density(type));
}

/* Partial from TIMWIN: 1090:0000 */
// Was pre-calculated in TIM each time the air pressure or gravity changed. Now we just recalculate it each time.
// TODO is to memoize this call if performance calls for it.
s16 EXPORT part_terminal_velocity(enum PartType type) {
    if (type == P_GUN_BULLET || type == P_CANNON_BALL) {
        return 0x3000;
    }

    return calculate_terminal_velocity(AIR_PRESSURE);
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