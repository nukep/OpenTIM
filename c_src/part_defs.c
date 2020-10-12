#include "tim.h"

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

    u16 density;               // TIMWIN offset: 00
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
    struct ShortVec *explicit_sizes;

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

struct BeltData* belt_data_alloc();
struct RopeData* rope_data_alloc();
void part_calculate_border_normals(struct Part *);
void part_set_size_and_pos_render(struct Part *part);
void stub_10a8_280a(struct Part *part, int c);
void stub_10a8_2b6d(struct Part *part, int c);
void stub_10a8_21cb(struct Part *part, u8 c);
int stub_1050_02db(struct Part *part);
int stub_10a8_4509(struct Part *part_a, struct Part *part_b);
bool calculate_line_intersection(const struct Line *a, const struct Line *b, struct ShortVec *out);
struct Part* part_new(enum PartType type);
void insert_part_into_static_parts(struct Part *part);
void insert_part_into_moving_parts(struct Part *part);
bool part_collides_with_playfield_part(struct Part *part);

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

void part_flip(struct Part *part, int flag) {
    struct PartDef *def = part_def(part->type);
    if (!def) {
        TRACE_ERROR("part_flip - def not found");
        return 0;
    }

    if (def->flip_func) {
        def->flip_func(part, flag);
    }
}

void part_resize(struct Part *part) {
    struct PartDef *def = part_def(part->type);
    if (!def) {
        TRACE_ERROR("part_resize - def not found");
        return 0;
    }

    if (def->resize_func) {
        def->resize_func(part);
    }
}

int part_rope(enum PartType type, struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_mass, s32 p1_force) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_rope - def not found");
        return 0;
    }

    return def->rope_func(p1, p2, rope_slot, flags, p1_mass, p1_force);
}

u16 part_density(enum PartType type) {
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

/* TIMWIN: 1090:0dca */
int default_bounce(struct Part *part) {
    return 1;
}

/* TIMWIN: 1090:0de2 */
void default_run(struct Part *part) {
    return;
}

/* TIMWIN: 1090:0e0c */
void default_flip(struct Part *part, int flag) {
    return;
}

/* TIMWIN: 1090:0df7 */
void default_reset(struct Part *part) {
    return;
}

/* TIMWIN: 1090:0e21 */
void default_resize(struct Part *part) {
    return;
}

/* TIMWIN: 1090:0e36 */
int default_rope(struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_pass, s32 p1_force) {
    return 0;
}

#define ALLOC_BORDERS(part) { if (part->num_borders == 0) { part->borders_data = 0; } else { part->borders_data = malloc(part->num_borders * 4); if (!part->borders_data) { return 1; } }  }

int part_alloc_borders_and_reset(struct Part *part) {
    // Used by level-loading
    struct PartDef *def = part_def(part->type);
    if (!def) {
        TRACE_ERROR("part_alloc_borders_and_reset - def not found");
        return 1;
    }

    part->num_borders = def->borders;
    ALLOC_BORDERS(part);

    part_reset(part);
}

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

/* TIMWIN: 1080:1388 */
void reset_incline(struct Part *part) {
    /* TIMWIN: 1108:0DC0 */
    static const u8 DAT_0DC0[4][4][2] = {
        { {0, 0}, {15, 15}, {15, 31}, {0, 16} },
        { {0, 0}, {31, 15}, {31, 31}, {0, 16} },
        { {0, 0}, {47, 15}, {47, 31}, {0, 16} },
        { {0, 0}, {63, 15}, {63, 31}, {0, 16} }
    };
    /* TIMWIN: 1108:0DE8 */
    static const DAT_0DE8[4][4][2] = {
        { {0, 15}, {15, 0}, {15, 16}, {0, 31} },
        { {0, 15}, {31, 0}, {31, 16}, {0, 31} },
        { {0, 15}, {47, 0}, {47, 16}, {0, 31} },
        { {0, 15}, {63, 0}, {63, 16}, {0, 31} }
    };

    if (NO_FLAGS(part->flags2, F2_FLIP_HORZ)) {
        for (int i = 0; i < 4; i++) {
            part->borders_data[i].x = DAT_0DC0[part->state1][i][0];
            part->borders_data[i].y = DAT_0DC0[part->state1][i][1];
        }
    } else {
        for (int i = 0; i < 4; i++) {
            part->borders_data[i].x = DAT_0DE8[part->state1][i][0];
            part->borders_data[i].y = DAT_0DE8[part->state1][i][1];
        }
    }

    part_calculate_border_normals(part);
}

/* TIMWIN: 1078:02cc */
int create_incline(struct Part *part) {
    part->flags1 |= 0x0400 | 0x0200;
    part->flags2 |= 0x0080;
    part->state1 = 1;
    part->original_state1 = 1;

    ALLOC_BORDERS(part);
    reset_incline(part);
    return 0;
}

/* TIMWIN: 1080:1427 */
void flip_incline(struct Part *part, int flag)  {
    part->flags2 ^= F2_FLIP_HORZ;
    reset_incline(part);
    stub_10a8_2b6d(part, 3);
    stub_10a8_21cb(part, 2);
}

/* TIMWIN: 1080:13ec */
void resize_incline(struct Part *part) {
    part->size = part->size_something2;
    part->state1 = part->size.x/16 - 1;
    part->original_state1 = part->state1;
    reset_incline(part);
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

/* TIMWIN: 1090:11dc */
void part_find_interactions(struct Part *part, enum GetPartsFlags choice, s16 hitbox_left, s16 hitbox_right, s16 hitbox_top, s16 hitbox_bottom) {
    part->interactions = 0;

    s16 x = part->pos.x;
    s16 y = part->pos.y;

    for (struct Part *curpart = get_first_part(choice); curpart != 0; curpart = next_part_or_fallback(curpart, choice & CHOOSE_MOVING_PART)) {
        if (curpart == part) continue;
        if (ANY_FLAGS(curpart->flags2, F2_DISAPPEARED)) continue;

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

        // VANILLAFIX: avoid null pointer dereference on part without borders (such as P_ROPE_SEVERED_END)
        if (!border_point) continue;

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

/* TIMWIN: 10d0:0000 */
int bounce_teeter_totter(struct Part *part) {
    if (ANY_FLAGS(part->bounce_part->flags2, 0x0200)) {
        return 1;
    }

    u16 idx = part->bounce_border_index;
    s16 new_state2 = 1;

    if (idx == 0) {
        s16 ivar3 = part->pos.x + part->size.x/2 - part->bounce_part->pos.x;
        if (ivar3 < 44) {
            if (ivar3 >= 37) {
                return 1;
            }
            if (part->bounce_part->state1 == 0) {
                return 1;
            }
            new_state2 = -1;
        } else {
            if (part->bounce_part->state1 == 2) {
                return 1;
            }
            new_state2 = 1;
        }
    } else if (idx == 2) {
        if (part->bounce_part->state1 == 0) {
            return 1;
        }
        new_state2 = -1;
    } else if (idx == 6) {
        if (part->bounce_part->state1 == 2) {
            return 1;
        }
        new_state2 = 1;
    } else {
        return 1;
    }

    if (stub_10a8_4509(part, part->bounce_part) == 0) {
        return 1;
    }

    part->bounce_part->state2 = new_state2;
    part->bounce_part->force = part->force;
    part->bounce_part = 0;

    return 0;
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

                    part->flags2 |= F2_DISAPPEARED;
                    stub_1050_02db(curpart);
                    part->flags2 &= ~F2_DISAPPEARED; 

                    curpart->pos_prev1.y = curpart->pos.y;
                    curpart->pos_y_hi_precision = curpart->pos.y * 512;
                } else {
                    curpart->pos_prev1.y = curpart->pos.y + 16;
                    stub_1050_02db(curpart);
                    curpart->pos_prev1.y = curpart->pos.y - 16;

                    part->flags2 |= F2_DISAPPEARED;
                    stub_1050_02db(curpart);
                    part->flags2 &= ~F2_DISAPPEARED;

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

/* TIMWIN: 10d0:01db */
void flip_teeter_totter(struct Part *part, int flags) {
    if (part->state1 == 0) {
        part->state1 = 2;
    } else {
        part->state1 = 0;
    }

    part->original_state1 = part->state1;
    reset_teeter_totter(part);
    part_set_size_and_pos_render(part);
    stub_10a8_280a(part, 3);
    stub_10a8_2b6d(part, 3);
    stub_10a8_21cb(part, 2);
}

/* TIMWIN: 10d0:04fe */
int rope_teeter_totter(struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_mass, s32 p1_force) {
    struct RopeData *rd = p2->rope_data[rope_slot];

    if (((flags & 7) != 1) && rd->rope_unknown != 0) {
        if ((flags & 0x8000) == 0) {
            rd->rope_unknown -= 1;
        }
        return 0;
    }

    s16 old_p2_state2 = p2->state2;

    int a = 0;
    s16 b = 0;

    if ((flags & 7) == 4) {
        if (rope_slot == 0) {
            if (p2->state1 == 0) {
                a = 1;
            } else {
                b = -1;
            }
        } else {
            if (p2->state1 == 2) {
                a = 1;
            } else {
                b = 1;
            }
        }
    } else if ((flags & 7) == 2) {
        if (rope_slot == 0) {
            if (p2->state1 == 2) {
                a = 1;
            } else {
                b = 1;
            }
        } else {
            if (p2->state1 == 0) {
                a = 1;
            } else {
                b = -1;
            }
        }
    }

    if (a == 0 && (flags & 7) != 1) {
        p2->state2 = b;
        a = teeter_totter_helper_2(p1, p2, flags & 0x8000, p1_mass, p1_force);

        if ((flags & 0x8000) == 0) {
            if (a == 0) {
                p2->flags2 |= 0x0400;
            }
        } else {
            p2->state2 = old_p2_state2;
        }
    }

    if (a != 0) {
        p2->flags2 |= 0x0200;
    }
    if ((p2->flags2 & 0x0200) != 0) {
        return 1;
    }
    if ((flags & (0x8000 | 0x0008 | 0x0004 | 0x0002 | 0x0001)) == 1) {
        rd->rope_unknown += 1;
    }

    return 0;
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
        part->flags2 |= F2_DISAPPEARED;
        return;
    }

    if (part->state2 == 1 && part->rope_data[0]) {
        // popped ballon with a rope attached to it

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

/* TIMWIN: 1078:0480 */
int create_pulley(struct Part *part) {
    part->flags2 |= 0x0004;
    part->rope_loc[0].x = 0;
    part->rope_loc[0].y = 8;
    part->rope_loc[1].x = 15;
    part->rope_loc[1].y = 8;

    struct RopeData *rope = rope_data_alloc();
    part->rope_data[0] = rope;
    if (!rope) {
        return 1;
    }
    rope->rope_or_pulley_part = part;
    return 0;
}

/* TIMWIN: 1078:0554 */
int create_rope(struct Part *part) {
    struct RopeData *rope = rope_data_alloc();
    part->rope_data[0] = rope;
    if (!rope) {
        return 1;
    }
    rope->rope_or_pulley_part = part;
    return 0;
}

/* TIMWIN: 1088:178c */
int bounce_nail(struct Part *part) {
    if (part->type == P_BALLOON) {
        u16 angle = part->bounce_angle;
        if (angle == 0x5e00 || angle == 0x8000 || angle == 0x9c90) {
            part->state2 = 1;
        }
    }

    return 1;
}

/* TIMWIN: 1088:17ca */
void reset_nail(struct Part *part) {
    // TIMWIN: 1108:0e5e
    static const struct ByteVec NAIL_BORDERS[] = {
        { 0, 0 }, { 12, 0 }, { 6, 16 }
    };
    set_border(part, NAIL_BORDERS);
}

/* TIMWIN: 1078:1087 */
int create_nail(struct Part *part) {
    ALLOC_BORDERS(part);
    reset_nail(part);
    return 0;
}

/* TIMWIN: 1090:1094 */
void stub_1090_1094(struct Part *part, enum GetPartsFlags param2, s16 param3, s16 param4, s16 param5, s16 param6) {
    part->interactions = 0;

    for (struct Part *curpart = get_first_part(param2); curpart != 0; curpart = next_part_or_fallback(curpart, param2 & CHOOSE_MOVING_PART)) {
        if (part == curpart) continue;
        if (ANY_FLAGS(curpart->flags2, F2_DISAPPEARED)) continue;

        s16 somex = curpart->pos.x + curpart->size.x - part->pos.x;

        if (somex >= param3) {
            s16 field_0x7a = somex >= 0 ? -1 : somex;
            s16 x = curpart->pos.x - (part->pos.x + part->size.x);
            if (x <= param4) {
                s16 tmp1 = x <= 0 ? 1 : x;

                x = abs(x);
                somex = abs(somex);
                if (x < somex) {
                    field_0x7a = tmp1;
                }
                x = curpart->pos.y + curpart->size.y - part->pos.y;
                if (x >= param5) {
                    s16 field_0x7c = x >= 0 ? -1 : x;

                    s16 y = curpart->pos.y - (part->pos.y + part->size.y);
                    if (y <= param6) {
                        tmp1 = y <= 0 ? 1 : y;

                        x = abs(x);
                        y = abs(y);

                        if (y < x) {
                            field_0x7c = tmp1;
                        }

                        curpart->interactions = part->interactions;
                        part->interactions = curpart;
                        curpart->field_0x7A = field_0x7a;
                        curpart->field_0x7C = field_0x7c;
                    }
                }
            }
        }
    }
}

struct PartDef BOWLING_BALL = {
    .flags1 = 0x0800,
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

    .field_0x16 = 0,
    .render_pos_offsets = 0,
    .explicit_sizes = 0,

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

    .field_0x16 = 0,
    .render_pos_offsets = 0,
    .explicit_sizes = 0,

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

struct PartDef INCLINE = {
    .flags1 = 0x4800,
    .flags3 = 0x0000,
    .size_something2 = { 32, 32 },
    .size = { 32, 32 },
    .create_func = create_incline,

    .density = 1510,
    .mass = 1000,
    .bounciness = 1024,
    .friction = 16,

    .field_0x0c = 0x0040,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x0010,
    .field_0x12 = 0x00f0,

    .field_0x16 = 0,
    .render_pos_offsets = 0,
    .explicit_sizes = 0,

    .goobers = { 4, 0xff },
    .borders = 4,
    .part_order = 44,

    .bounce_func = default_bounce,
    .run_func = default_run,
    .reset_func = reset_incline,
    .flip_func = flip_incline,
    .resize_func = resize_incline,
    .rope_func = default_rope,
};

struct PartDef TEETER_TOTTER = {
    .flags1 = 0x4800,
    .flags3 = 0x0000,
    .size_something2 = { 80, 32 },
    .size = { 80, 32 },
    .create_func = create_teeter_totter,

    .density = 1888,
    .mass = 1000,
    .bounciness = 1024,
    .friction = 16,

    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .field_0x16 = 0,
     // TIMWIN: 1108:2059. 3 states.
    .render_pos_offsets = (struct SByteVec[3]){ {0, 0}, {0, 12}, {0, 0} },
    .explicit_sizes = 0,

    .goobers = { 4, 0xff },
    .borders = 8,
    .part_order = 6,

    .bounce_func = bounce_teeter_totter,
    .run_func = run_teeter_totter,
    .reset_func = reset_teeter_totter,
    .flip_func = flip_teeter_totter,
    .resize_func = default_resize,
    .rope_func = rope_teeter_totter,
};

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

    .field_0x16 = 0,
    // TIMWIN: 1108:205F. 7 states.
    .render_pos_offsets = (struct SByteVec[7]){ {0, 0}, {-15, -9}, {-20, -5}, {-28, 8}, {-30, 25}, {-26, 41}, {-26, 56} },
    .explicit_sizes = 0,

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

struct PartDef PULLEY = {
    .flags1 = 0x4800,
    .flags3 = 0x0008,
    .size_something2 = { 16, 16 },
    .size = { 16, 16 },
    .create_func = create_pulley,

    .density = 3776,
    .mass = 1000,
    .bounciness = 0,
    .friction = 0,

    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .field_0x16 = 0,
    .render_pos_offsets = 0,
    .explicit_sizes = 0,

    .goobers = { 2, 0xff },
    .borders = 0,
    .part_order = 17,

    .bounce_func = default_bounce,
    .run_func = default_run,
    .reset_func = default_reset,
    .flip_func = default_flip,
    .resize_func = default_resize,
    .rope_func = default_rope,
};

struct PartDef ROPE = {
    .flags1 = 0x4800,
    .flags3 = 0x0000,
    .size_something2 = { 0, 0 },
    .size = { 0, 0 },
    .create_func = create_rope,

    .density = 1600,
    .mass = 1000,
    .bounciness = 0,
    .friction = 0,

    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .field_0x16 = 0,
    .render_pos_offsets = 0,
    .explicit_sizes = 0,

    .goobers = { 1, 0xff },
    .borders = 0,
    .part_order = 15,

    .bounce_func = default_bounce,
    .run_func = default_run,
    .reset_func = default_reset,
    .flip_func = default_flip,
    .resize_func = default_resize,
    .rope_func = default_rope,
};

/* TIMWIN: 1048:1434 */
void reset_pokey_the_cat(struct Part *part) {
    // TIMWIN: 1108:0bf8
    static const struct ByteVec POKEY_LEFT_BORDERS[5] = {
        { 0, 7 }, { 10, 0 }, { 36, 26 }, { 36, 37 }, { 10, 40 }
    };
    // TIMWIN: 1108:0c02
    static const struct ByteVec POKEY_RIGHT_BORDERS[5] = {
        { 3, 26 }, { 29, 0 }, { 39, 10 }, { 29, 40 }, { 3, 37 }
    };
    if (NO_FLAGS(part->flags2, F2_FLIP_HORZ)) {
        set_border(part, POKEY_LEFT_BORDERS);
    } else {
        set_border(part, POKEY_RIGHT_BORDERS);
    }

    part_calculate_border_normals(part);
}

/* TIMWIN: 1078:05e5 */
int create_pokey_the_cat(struct Part *part) {
    part->flags1 |= 0x0400;
    part->flags2 |= 0x8000;
    ALLOC_BORDERS(part);
    reset_pokey_the_cat(part);
    return 0;
}

/* TIMWIN: 1048:148a */
int bounce_pokey_the_cat(struct Part *part) {
    struct Part *cat = part->bounce_part;
    if (cat->state1 == 0) {
        cat->state1 = 1;
        cat->extra1 = 0;
        part_set_size_and_pos_render(cat);
        play_sound(0x07);
    }
    return 1;
}

/* TIMWIN: 1048:14cf */
void run_pokey_the_cat(struct Part *part) {
    if (NO_FLAGS(part->flags2, F2_FLIP_VERT)) {
        s16 y_delta = abs(part->pos.y - part->pos_prev2.y);

        if (y_delta < 2 || part->state1 > 1) {
            if (part->state1 == 1) {
                part->extra1 += 1;
                if (part->extra1 > 12) {
                    s16 move_x = NO_FLAGS(part->flags2, F2_FLIP_HORZ) ? -32 : 32;

                    part->extra1 = 0;

                    part->pos.x += move_x;
                    part_set_size_and_pos_render(part);

                    if (part_collides_with_playfield_part(part)) {
                        part->pos.x -= move_x*2;
                        part_set_size_and_pos_render(part);

                        if (part_collides_with_playfield_part(part)) {
                            part->pos.x += move_x;
                            part_set_size_and_pos_render(part);

                            part->state1 = 0;
                        } else {
                            part->state1 = 2;
                            part->flags2 ^= F2_FLIP_HORZ;
                        }
                    } else {
                        part->state1 = 2;
                    }

                    part->pos_x_hi_precision = part->pos.x * 512;
                }
            } else {
                bool someflag = part->state1 != 0;

                if (part->state1 != 0) {
                    part->state1 += 1;
                    if (part->state1 == 10) {
                        part->state1 = 0;
                    }
                }

                if (part->state1 == 0 || part->state1 > 7) {
                    if (NO_FLAGS(part->flags2, F2_FLIP_HORZ)) {
                        stub_1090_1094(part, 0x3000, -240, 0, 0, 0);
                    } else {
                        stub_1090_1094(part, 0x3000, 0, 240, 0, 0);
                    }

                    EACH_INTERACION(part, curpart) {
                        s16 move_x = 0;
                        if (curpart->type == P_BOB_THE_FISH) {
                            if (curpart->state1 < 11) {
                                move_x = 0x60;
                            } else {
                                move_x = 0x124;
                            }
                        } else if (curpart->type == P_MORT_THE_MOUSE) {
                            move_x = curpart->pos.x - part->pos.x + 16;
                            s16 tmp = curpart->pos.y - part->pos.y;
                            if (move_x <= 0 || move_x > 55 || tmp <= 0 || tmp > 39) {
                                move_x = someflag ? 192 : 128;
                            } else {
                                part->flags3 |= 0x0080;
                                stub_10a8_2b6d(curpart, 3);
                                curpart->flags2 |= F2_DISAPPEARED;
                                curpart->flags3 |= 0x0200;
                                play_sound(0x0D);
                                move_x = -1;
                            }
                        } else {
                            move_x = -1;
                        }

                        if (abs(curpart->field_0x7A) < move_x && part->state1 == 0) {
                            move_x = NO_FLAGS(part->flags2, F2_FLIP_HORZ) ? -32 : 32;
                            part->extra1 = 0;
                            part->pos.x += move_x;
                            part_set_size_and_pos_render(part);
                            if (!part_collides_with_playfield_part(part)) {
                                part->state1 = 2;
                            } else {
                                part->pos.x -= move_x;
                                part_set_size_and_pos_render(part);
                                part->state1 = 0;
                            }
                            part->pos_x_hi_precision = part->pos.x * 512;
                            break;
                        }
                    }
                }
            }
        } else if (part->extra1 < 5) {
            part->extra1 += 1;
        } else {
            part->flags2 |= F2_FLIP_VERT;
            part->state1 = 1;
            part->extra1 = 0;
        }

    } else {
        if (ANY_FLAGS(part->flags1, 0x0002)) {
            part->flags2 &= ~(F2_FLIP_VERT);
            part->state1 = 0;
        }
    }

    if (part->state1 != part->state1_prev1) {
        part_set_size_and_pos_render(part);
    }
}

/* TIMWIN: 1048:1771 */
void flip_pokey_the_cat(struct Part *part, int flag) {
    part->flags2 ^= F2_FLIP_HORZ;
    reset_pokey_the_cat(part);
    part_set_size_and_pos_render(part);
    stub_10a8_2b6d(part, 3);
    stub_10a8_21cb(part, 2);
}

struct PartDef POKEY_THE_CAT = {
    .flags1 = 0x0800,
    .flags3 = 0x0008,
    .size_something2 = { 40, 41 },
    .size = { 40, 39 },
    .create_func = create_pokey_the_cat,

    .density = 2000,
    .mass = 120,
    .bounciness = 0,
    .friction = 64,

    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .field_0x16 = 0,
    // TIMWIN: 1108:206D. 10 states.
    .render_pos_offsets = (struct SByteVec[10]){ {0,0}, {-6,-16}, {19,-1}, {14,0}, {10,0}, {8,0}, {6,-2}, {-1,-2}, {-5,-2}, {-9,-3} },
    .explicit_sizes = 0,

    .goobers = { 3, 0xff },
    .borders = 5,
    .part_order = 35,

    .bounce_func = bounce_pokey_the_cat,
    .run_func = run_pokey_the_cat,
    .reset_func = reset_pokey_the_cat,
    .flip_func = flip_pokey_the_cat,
    .resize_func = default_resize,
    .rope_func = default_rope,
};

/* TIMWIN: 1078:0f7b */
int create_rope_severed_end(struct Part *part) {
    part->flags2 |= 0x0004;
    part->rope_loc[0].x = 0;
    part->rope_loc[0].y = 0;
    return 0;
}

struct PartDef ROPE_SEVERED_END = {
    .flags1 = 0x0800,
    .flags3 = 0x0008,
    .size_something2 = { 0, 0 },
    .size = { 0, 0 },
    .create_func = create_rope_severed_end,

    .density = 1600,
    .mass = 1000,
    .bounciness = 0,
    .friction = 0,

    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .field_0x16 = 0,
    .render_pos_offsets = 0,
    .explicit_sizes = 0,

    .goobers = { 4, 0xff },
    .borders = 0,
    .part_order = 50,

    .bounce_func = default_bounce,
    .run_func = default_run,
    .reset_func = default_reset,
    .flip_func = default_flip,
    .resize_func = default_resize,
    .rope_func = default_rope,
};

struct PartDef NAIL = {
    .flags1 = 0x4800,
    .flags3 = 0x0008,
    .size_something2 = { 14, 17 },
    .size = { 14, 17 },
    .create_func = create_nail,

    .density = 7552,
    .mass = 20,
    .bounciness = 1024,
    .friction = 2,

    .field_0x0c = 0x0000,
    .field_0x0e = 0x0000,
    .field_0x10 = 0x00f0,
    .field_0x12 = 0x00f0,

    .field_0x16 = 0,
    .render_pos_offsets = 0,
    .explicit_sizes = 0,

    .goobers = { 4, 0xff },
    .borders = 3,
    .part_order = 52,

    .bounce_func = bounce_nail,
    .run_func = default_run,
    .reset_func = reset_nail,
    .flip_func = default_flip,
    .resize_func = default_resize,
    .rope_func = rope_balloon,
};

struct PartDef *part_def(enum PartType type) {
    switch (type) {
        case P_BOWLING_BALL: return &BOWLING_BALL;
        case P_BRICK_WALL: return &BRICK_WALL;
        case P_INCLINE: return &INCLINE;
        case P_TEETER_TOTTER: return &TEETER_TOTTER;
        case P_BALLOON: return &BALLOON;
        case P_PULLEY: return &PULLEY;
        case P_ROPE: return &ROPE;
        case P_POKEY_THE_CAT: return &POKEY_THE_CAT;
        case P_ROPE_SEVERED_END: return &ROPE_SEVERED_END;
        case P_NAIL: return &NAIL;
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

struct SByteVec* part_data31_render_pos_offsets(enum PartType type) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_data31_render_pos_offsets - def not found");
        return 0;
    }

    return def->render_pos_offsets;
}

bool part_explicit_size(enum PartType type, u16 index, struct ShortVec *size_out) {
    struct PartDef *def = part_def(type);
    if (!def) {
        TRACE_ERROR("part_explicit_size - def not found");
        return 0;
    }

    if (!def->explicit_sizes) {
        return 0;
    }

    *size_out = def->explicit_sizes[index];

    return 1;
}