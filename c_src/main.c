#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "tim.h"

u16 arctan_c(s32 dx, s32 dy);
u16 rope_calculate_flags(struct RopeData *rope, int param_2, int param_3);
bool calculate_line_intersection_helper(s16 a, s16 b, s16 c);
bool calculate_line_intersection(const struct Line *a, const struct Line *b, struct ShortVec *out);
bool part_image_size(enum PartType type, u16 index, struct ShortVec *size_out);

static inline s16 utos(u16 v) {
    // Unsigned->signed conversion for integers with the same number of bits is undefined in C.
    // But we often like to do it for stuff like calculating angles.
    // Manually wrap around as two's complement
    if (v < 0x8000) {
        return v;
    } else {
        s32 x = 0x10000 - (s32)v;
        return (s16)(-x);
    }
}

static inline u16 uneg(u16 v) {
    // Do the same as casting v to two's complement, then negating the value.
    return (v ^ 0xFFFF) + 1;
}

static inline s32 mul32(s32 a, s32 b) {
    // Convenience, because explicit casting is ugly and hard to read.
    return a * b;
}

/* TIMWIN: 10a8:1e46 */
static inline void insert_part_into_root(struct Part *part, struct Part *root) {
    struct Part *curpart = root;

    while (1) {
        if (!curpart->next) {
            break;
        }

        if (root == &PARTS_BIN_ROOT) {
            if (part_order(part->type) < part_order(curpart->next->type)) {
                break;
            }
        } else if (part_mass(part->type) == part_mass(curpart->next->type)) {
            if (curpart->type == P_ROPE_SEVERED_END || curpart->type == P_ROPE) {
                break;
            }
            if (part->type == P_ROPE_SEVERED_END || part->type == P_ROPE) {
                break;
            }
            if (curpart->next->original_pos_y < part->original_pos_y) {
                break;
            } else if (curpart->next->original_pos_y == part->original_pos_y && curpart->next->original_pos_x < part->original_pos_x) {
                break;
            }
        } else if (root == &MOVING_PARTS_ROOT) {
            if (part_mass(part->type) < part_mass(curpart->next->type)) {
                break;
            }
        } else {
            break;
        }

        curpart = curpart->next;
    }

    part->next = curpart->next;
    part->prev = curpart;
    curpart->next = part;
    if (part->next) {
        part->next->prev = part;
    }
}

/* Partial from TIMWIN: 10a8:1e46 */
void insert_part_into_static_parts(struct Part *part) {
    return insert_part_into_root(part, &STATIC_PARTS_ROOT);
}

/* Partial from TIMWIN: 10a8:1e46 */
void insert_part_into_moving_parts(struct Part *part) {
    return insert_part_into_root(part, &MOVING_PARTS_ROOT);
}

/* Partial from TIMWIN: 10a8:1e46 */
void insert_part_into_parts_bin(struct Part *part) {
    return insert_part_into_root(part, &PARTS_BIN_ROOT);
}

/* Partial from TIMWIN: 10b0:02a5 */
void initialize_llamas() {
    LLAMA_1 = 0;
    LLAMA_2 = 0;
    for (int i = 0; i < 20; i++) {
        struct Llama *o = malloc(sizeof(struct Llama));
        o->next = LLAMA_1;
        LLAMA_1 = o;
    }
}

/* TIMWIN: 10a8:0290
   Note: double checked for accuracy */
void four_points_adjust_p1_by_one(struct Line *points) {
    if (points->p1.x < points->p0.x) {
        points->p1.x -= 1;
    } else if (points->p1.x > points->p0.x) {
        points->p1.x += 1;
    }

    if (points->p1.y < points->p0.y) {
        points->p1.y -= 1;
    } else if (points->p1.y > points->p0.y) {
        points->p1.y += 1;
    }
}

static inline void calculate_border_normal_segment(struct BorderPoint *a, struct BorderPoint *b) {
    struct Line p = { { a->x, a->y }, { b->x, b->y } };
    four_points_adjust_p1_by_one(&p);
    u16 angle = arctan_c(p.p1.x - p.p0.x, p.p1.y - p.p0.y);

    /*
    Transform angle so that
    Right: 0
    Up: 25% of 65536
    Left: 50% of 65536
    Down: 75% of 65536
    */
    a->normal_angle = 0xc000 - angle;
}

/* TIMWIN: 10a8:26b5 */
void part_calculate_border_normals(struct Part *part) {
    // Calculate between first and last points...
    for (u16 i = 0; i < part->num_borders-1; i++) {
        struct BorderPoint *a = part->borders_data + i;
        struct BorderPoint *b = part->borders_data + i + 1;
        calculate_border_normal_segment(a, b);
    }

    // Then calculate between last and first point
    calculate_border_normal_segment(part->borders_data + part->num_borders-1, part->borders_data);
}

void part_set_size(struct Part *part);

/* TIMWIN: 10a8:2485 */
struct Part* get_first_part(int choice) {
    if (STATIC_PARTS_ROOT.next && (choice & CHOOSE_STATIC_PART)) {
        return STATIC_PARTS_ROOT.next;
    }
    if (MOVING_PARTS_ROOT.next && (choice & CHOOSE_MOVING_PART)) {
        return MOVING_PARTS_ROOT.next;
    }
    if (PARTS_BIN_ROOT.next && (choice & CHOOSE_FROM_PARTS_BIN)) {
        return PARTS_BIN_ROOT.next;
    }
    return 0;
}

/* TIMWIN: 10a8:24d8 */
struct Part* next_part_or_fallback(struct Part *part, int choice) {
    if (part->next) {
        return part->next;
    }
    if (ANY_FLAGS(part->flags1, 0x2000)) {
        return get_first_part(choice);
    }
    if (ANY_FLAGS(part->flags1, 0x1000) && (choice & CHOOSE_FROM_PARTS_BIN)) {
        return PARTS_BIN_ROOT.next;
    }
    return 0;
}

/* TIMWIN: 1078:1402 */
void part_free(struct Part *part) {
    if (!part) return;

    if (part->borders_data) {
        free(part->borders_data);
    }
    if (part->belt_data && NO_FLAGS(part->flags2, 0x0001)) {
        free(part->belt_data);
    }
    if (part->rope_data[0] && (part->type == P_PULLEY || part->type == P_ROPE)) {
        // Only free rope_data[0] if the part owns it (only pulleys and ropes can own it)
        // rope_data[1] is always shared.
        free(part->rope_data[0]);
    }
    free(part);
}

struct Part* part_alloc() {
    struct Part *part = malloc(sizeof(struct Part));
    if (!part) return 0;

    memset(part, 0, sizeof(struct Part));
    return part;
}

struct BeltData* belt_data_alloc() {
    struct BeltData *belt = malloc(sizeof(struct BeltData));
    if (!belt) return 0;

    memset(belt, 0, sizeof(struct BeltData));
    return belt;
}

struct RopeData* rope_data_alloc() {
    struct RopeData *rope = malloc(sizeof(struct RopeData));
    if (!rope) return 0;

    memset(rope, 0, sizeof(struct RopeData));
    return rope;
}

/* TIMWIN: 1078:00f2 */
struct Part* part_new(enum PartType type) {
    struct Part *part = part_alloc();
    if (!part) {
        goto error;
    }

    part->type = type;
    part->flags1 = part_data30_flags1(type);
    part->flags3 = part_data30_flags3(type);
    part->size_something2 = part_data30_size_something2(type);
    part->size = part_data30_size(type);
    part->num_borders = part_data31_num_borders(type);
    part->original_pos_x = -1;
    part->original_pos_y = -1;

    int res = part_create_func(type, part);
    if (res == 1) {
        goto error;
    }

    part->original_flags2 = part->flags2;
    part_set_size(part);
    part->size_something = part->size;

    return part;

error:
    part_free(part);
    return 0;
}

// Only used this for debugging purposes
size_t debug_part_size() {
    return sizeof(struct Part);
}

/* TIMWIN: 10a8:1e18 */
void remove_part_from_linked_list(struct Part *part) {
    part->prev->next = part->next;
    if (part->next) {
        part->next->prev = part->prev;
    }
}

/* TIMWIN: 10a8:44d5 */
static inline void move_llama2_to_beginning_of_llama1() {
    if (!LLAMA_2) return;

    struct Llama *last_llama_2 = LLAMA_2;

    for (struct Llama *cur = LLAMA_2->next; cur != 0; cur = cur->next) {
        last_llama_2 = cur;
    }

    last_llama_2->next = LLAMA_1;
    LLAMA_1 = LLAMA_2;
    LLAMA_2 = 0;
}

/* TIMWIN: 10a8:25d9 */
void part_set_size(struct Part *part) {
    if (part->type == P_BELT || part->type == P_ROPE) {
        part->size.x = 0;
        part->size.y = 0;
        return;
    }

    if (ANY_FLAGS(part->flags1, 0x0040)) {
        part->size = part->size_something2;
        return;
    }

    struct ShortVec size;

    if (part_explicit_size(part->type, part->state1, &size)) {
        part->size = size;
        return;
    }

    if (part_image_size(part->type, part->state1, &size)) {
        part->size = size;
        return;
    }

    part->size.x = 0;
    part->size.y = 0;
}

/* TIMWIN: 10a8:252b */
void part_set_size_and_pos_render(struct Part *part) {
    part->pos_render = part->pos;
    u16 state1 = part->state1;
    u16 flags2 = part->flags2;
    part_set_size(part);

    struct SByteVec *offsets = part_data31_render_pos_offsets(part->type);
    if (!offsets) return;

    struct SByteVec *v = offsets + state1;

    if (NO_FLAGS(flags2, F2_FLIP_HORZ)) {
        part->pos_render.x += v->x;
    } else {
        part->pos_render.x += part->size_something.x - v->x - part->size.x;
    }

    if (NO_FLAGS(flags2, F2_FLIP_VERT)) {
        part->pos_render.y += v->y;
    } else {
        part->pos_render.y += part->size_something.y - v->y - part->size.y;
    }
}

/* TIMWIN: 1090:0240 */
void adjust_part_position(struct Part *part) {
    part->pos_x_hi_precision += part->vel_hi_precision.x;
    part->pos_y_hi_precision += part->vel_hi_precision.y;

    if (ANY_FLAGS(part->flags1, 0x0001)) {
        if (part_acceleration(part->type) <= 0) {
            part->pos_y_hi_precision -= 2 * 512;
        } else {
            part->pos_y_hi_precision += 2 * 512;
        }
    }

    part->pos.x = part->pos_x_hi_precision / 512;
    part->pos.y = part->pos_y_hi_precision / 512;
    if (part->pos.x < -1000) {
        part->pos.x = -1000;
        part->pos_x_hi_precision = part->pos.x * 512;
    } else if (part->pos.x > 6000) {
        part->pos.x = 6000;
        part->pos_x_hi_precision = part->pos.x * 512;
    }

    if (part->pos.y < -1000) {
        part->pos.y = -1000;
        part->pos_y_hi_precision = part->pos.y * 512;
    } else if (part->pos.y > 6000) {
        part->pos.y = 6000;
        part->pos_y_hi_precision = part->pos.y * 512;
    }

    part_set_size_and_pos_render(part);
}

/* TIMWIN: 1090:012d */
void part_clamp_to_terminal_velocity(struct Part *part) {
    const s16 tv = part_terminal_velocity(part->type);

    if (tv < part->vel_hi_precision.x) {
        part->vel_hi_precision.x = tv;
    } else if (part->vel_hi_precision.x < -tv) {
        part->vel_hi_precision.x = -tv;
    }

    if (tv < part->vel_hi_precision.y) {
        part->vel_hi_precision.y = tv;
    } else if (part->vel_hi_precision.y < -tv) {
        part->vel_hi_precision.y = -tv;
    }
}

/* TIMWIN: 1090:01b0 */
void part_update_vel_and_force(struct Part *part) {
    part->vel_hi_precision.y += part_acceleration(part->type);
    part_clamp_to_terminal_velocity(part);
    part->force = mul32(abs(part->vel_hi_precision.x) + abs(part->vel_hi_precision.y), part->mass);
}

/* TIMWIN: 1050:0221
   Returns 0 to 3. */
u16 quadrant_from_angle(u16 angle) {
    if (angle == 0x2000) {
        return 0;
    }
    if (angle == 0xa000) {
        return 2;
    }
    return ((angle + 0x2000) >> 14) & 3;
}

/* TIMWIN: 1050:01e7 */
u16 part_get_movement_delta_angle(struct Part *part) {
    return arctan_c(part->pos_prev1.x - part->pos.x, part->pos.y - part->pos_prev1.y);
}

/* TIMWIN: 10a8:176f */
void stub_10a8_176f(struct RopeData *rope) {
    if (!rope->part1) return;

    struct Part *part1 = rope->part1;
    rope->ends_pos[0].x = part1->pos_render.x + part1->rope_loc[rope->part1_rope_slot].x;
    rope->ends_pos[0].y = part1->pos_render.y + part1->rope_loc[rope->part1_rope_slot].y;

    if (rope->part2) {
        struct Part *part2 = rope->part2;
        rope->ends_pos[1].x = part2->pos_render.x + part2->rope_loc[rope->part2_rope_slot].x;
        rope->ends_pos[1].y = part2->pos_render.y + part2->rope_loc[rope->part2_rope_slot].y;
    }

    struct Part *part = part1->links_to[rope->part1_rope_slot];
    while (part && part->type == P_PULLEY) {
        for (int i = 0; i < 2; i++) {
            part->rope_data[0]->ends_pos[i].x = part->pos.x + part->rope_loc[i].x;
            part->rope_data[0]->ends_pos[i].y = part->pos.y + part->rope_loc[i].y;
        }
        part = part->links_to[0];
    }

    if (LEVEL_STATE != SIMULATION_MODE) {
        rope->rope_or_pulley_part->extra1 = approximate_hypot_of_rope(rope, ROPETIME_CURRENT, ROPE_FROM_FIRST);
        rope->rope_or_pulley_part->extra2 = approximate_hypot_of_rope(rope, ROPETIME_CURRENT, ROPE_FROM_LAST);
    }
}

/* TIMWIN: 10a8:166a */
void belt_set_four_pos(struct BeltData *belt) {
    struct Part *part1 = belt->part1;
    struct Part *part2 = belt->part2;

    belt->pos1.x = part1->pos_render.x + part1->belt_loc.x;
    belt->pos1.y = part1->pos_render.y + part1->belt_loc.y;

    belt->pos2.x = part2->pos_render.x + part2->belt_loc.x;
    belt->pos2.y = part2->pos_render.y + part2->belt_loc.y;

    s16 delta_x = abs(belt->pos1.x - belt->pos2.x);
    s16 delta_y = abs(belt->pos1.y - belt->pos2.y);

    s16 pos1_x_off, pos1_y_off;
    s16 pos2_x_off, pos2_y_off;
    s16 pos3_x_off, pos3_y_off;
    s16 pos4_x_off, pos4_y_off;

    if (delta_x < delta_y) {
        pos2_x_off = 0;
        pos1_x_off = 0;
        pos3_x_off = part1->belt_width;
        pos3_y_off = part1->belt_width / 2;
        pos4_x_off = part2->belt_width;
        pos4_y_off = part2->belt_width / 2;
        pos2_y_off = part2->belt_width / 2;
        pos1_y_off = part1->belt_width / 2;
    } else {
        pos2_y_off = 0;
        pos1_y_off = 0;
        pos3_y_off = part1->belt_width;
        pos3_x_off = part1->belt_width / 2;
        pos4_y_off = part2->belt_width;
        pos4_x_off = part2->belt_width / 2;
        pos2_x_off = part2->belt_width / 2;
        pos1_x_off = part1->belt_width / 2;
    }

    belt->pos3.x = belt->pos1.x + pos3_x_off;
    belt->pos3.y = belt->pos1.y + pos3_y_off;
    belt->pos4.x = belt->pos2.x + pos4_x_off;
    belt->pos4.y = belt->pos2.y + pos4_y_off;
    belt->pos1.x += pos1_x_off;
    belt->pos1.y += pos1_y_off;
    belt->pos2.x += pos2_x_off;
    belt->pos2.y += pos2_y_off;
}

/* TIMWIN: 10a8:4794 */
void restore_parts_state_from_design() {
    // Not using EACH_STATIC_THEN_MOVING_PART macro, because I'm unsure of the side-effects of the loop body and next_part_or_fallback.
    // Might be okay, but not sure yet.
    for (struct Part *part = get_first_part(CHOOSE_STATIC_OR_ELSE_MOVING_PART); part != 0; ) {
        struct Part *nextpart = next_part_or_fallback(part, CHOOSE_MOVING_PART);

        if (NO_FLAGS(part->flags1, 0x0010)) {
            part->flags1 &= ~(0x0008 | 0x0004 | 0x0002 | 0x0001);
            part->flags2 = part->original_flags2;
            part->flags3 &= ~(0x0400 | 0x0200 | 0x0100 | 0x0080 | 0x0010);

            part->pos_prev2.x = part->original_pos_x;
            part->pos_prev1.x = part->original_pos_x;
            part->pos.x       = part->original_pos_x;
            part->pos_prev2.y = part->original_pos_y;
            part->pos_prev1.y = part->original_pos_y;
            part->pos.y       = part->original_pos_y;

            part->pos_x_hi_precision = part->pos.x * 512;
            part->pos_y_hi_precision = part->pos.y * 512;

            part->state1       = part->original_state1;
            part->state1_prev1 = part->original_state1;
            part->state1_prev2 = part->original_state1;

            part_set_size(part);
            part->size_something = part->size;

            part_set_size_and_pos_render(part);
            part->pos_render_prev1 = part->pos_render;
            part->pos_render_prev2 = part->pos_render;
            part->size_prev1 = part->size;
            part->size_prev2 = part->size;
            part->mass = part_mass(part->type);
            part->bounce_part = 0;

            part->state2 = part->original_state2;
            part->vel_hi_precision.x = 0;
            part->vel_hi_precision.y = 0;
            part->extra1_prev2 = 0;
            part->extra1_prev1 = 0;
            part->extra1       = 0;
            part->extra2_prev2 = 0;
            part->extra2_prev1 = 0;
            part->extra2       = 0;

            if (part->type != P_GEAR) {
                for (int i = 0; i < 2; i++) {
                    part->links_to[i] = part->links_to_design[i];
                }
            }

            part_reset(part);
            part->original_state1 = part->state1;
        } else {
            remove_part_from_linked_list(part);
            part_free(part);
        }

        part = nextpart;
    }

    EACH_STATIC_THEN_MOVING_PART(part) {
        if (part->type == P_BELT) {
            belt_set_four_pos(part->belt_data);
        } else if (part->type == P_ROPE) {
            struct RopeData *rope = part->rope_data[0];
            rope->part1 = rope->original_part1;
            rope->part2 = rope->original_part2;
            
            rope->part1_rope_slot = rope->original_part1_rope_slot;
            rope->part2_rope_slot = rope->original_part2_rope_slot;

            rope->part1->rope_data[rope->part1_rope_slot] = rope;
            rope->part2->rope_data[rope->part2_rope_slot] = rope;

            struct Part *curpart = rope->part1;
            struct Part *nextpart = curpart->links_to[rope->part1_rope_slot];
            while (curpart) {
                if (curpart->type == P_PULLEY) {
                    curpart->rope_data[1] = rope;
                }

                if (rope->part2 == curpart) {
                    curpart = 0;
                } else {
                    curpart = nextpart;
                    nextpart = nextpart->links_to[0];
                }
            }

            stub_10a8_176f(rope);

            s16 v;
            v = approximate_hypot_of_rope(rope, ROPETIME_CURRENT, ROPE_FROM_FIRST);
            part->extra1_prev2 = v;
            part->extra1_prev1 = v;
            part->extra1       = v;

            v = approximate_hypot_of_rope(rope, ROPETIME_CURRENT, ROPE_FROM_LAST);
            part->extra2_prev2 = v;
            part->extra2_prev1 = v;
            part->extra2       = v;

            rope->rope_unknown_prev2 = 0;
            rope->rope_unknown_prev1 = 0;
            rope->rope_unknown       = 0;
        }
    }
}

/* TIMWIN: 1050:001e
   Note: I double-checked it for accuracy */
void tmp_3a6c_update_vars() {
    TMP_X2_3a6c = PART_3a6c->pos.x;
    TMP_Y2_3a6c = PART_3a6c->pos.y;

    TMP_X_CENTER_3a6c = PART_3a6c->pos.x + PART_3a6c->size_something2.x/2;
    TMP_Y_CENTER_3a6c = PART_3a6c->pos.y + PART_3a6c->size_something2.y/2;

    TMP_X_DELTA_3a6c = PART_3a6c->pos.x - PART_3a6c->pos_prev1.x;
    TMP_Y_DELTA_3a6c = PART_3a6c->pos.y - PART_3a6c->pos_prev1.y;

    TMP_X_LEFTMOST_3a6c = MIN(PART_3a6c->pos_prev1.x, PART_3a6c->pos.x);
    TMP_Y_TOPMOST_3a6c = MIN(PART_3a6c->pos_prev1.y, PART_3a6c->pos.y);

    TMP_X_RIGHT_3a6c = PART_3a6c->pos.x + PART_3a6c->size.x + abs(TMP_X_DELTA_3a6c);
    TMP_Y_BOTTOM_3a6c = PART_3a6c->pos.y + PART_3a6c->size.y + abs(TMP_Y_DELTA_3a6c);
}

/* TIMWIN: 1050:00a8
   Note: I double-checked it for accuracy */
void tmp_3a6a_update_vars() {
    TMP_X_3a6a = PART_3a6a->pos.x;
    TMP_Y_3a6a = PART_3a6a->pos.y;

    TMP_X_CENTER_3a6a = PART_3a6a->pos.x + PART_3a6a->size_something2.x/2;
    TMP_Y_CENTER_3a6a = PART_3a6a->pos.y + PART_3a6a->size_something2.y/2;

    TMP_X_RIGHT_3a6a = PART_3a6a->pos.x + PART_3a6a->size.x;
    TMP_Y_BOTTOM_3a6a = PART_3a6a->pos.y + PART_3a6a->size.y;
}

/* TIMWIN: 1050:00f0
   Note: double-checked for accuracy */
int stub_1050_00f0(u16 angle) {
    if (!PART_3a68) return 0;
    if (quadrant_from_angle(angle) != TMP_QUADRANT) return 0;

    bool flag = 0;

    u16 new_angle = angle + 0x2000;
    u16 tmp_3a90_new_angle = TMP_BOUNCE_ANGLE_3a6c + 0x2000;

    if (utos(new_angle) < 0 || utos(new_angle) > 0x4000 || utos(tmp_3a90_new_angle) < 0 || utos(tmp_3a90_new_angle) > 0x4000) {
        new_angle = angle + 0xa000;
        tmp_3a90_new_angle = TMP_BOUNCE_ANGLE_3a6c + 0xa000;

        if (!(utos(new_angle) < 0 || utos(new_angle) > 0x4000 || utos(tmp_3a90_new_angle) < 0 || utos(tmp_3a90_new_angle) > 0x4000)) {
            flag = 1;
        }
    } else {
        flag = 1;
    }

    if (flag) {
        if (angle == TMP_BOUNCE_ANGLE_3a6c) return 1;
        if (new_angle == 0x2000 || tmp_3a90_new_angle == 0x2000) return 1;
        if (utos(new_angle) < 0x2000 && utos(tmp_3a90_new_angle) < 0x2000) return 1;
        if (utos(new_angle) > 0x2000 && utos(tmp_3a90_new_angle) > 0x2000) return 1;
    }
    return 0;
}

/* TIMWIN: 1050:025e
   Note: We changed the signature slightly, to pass bounce_field_0x86 directly.
   Note: I double-checked this for accuracy.
*/
void stub_1050_025e(struct Line *line, s16 x, byte *bounce_field_0x86) {
    if (calculate_line_intersection_helper(x, line->p0.x, line->p1.x) != 0) {
        bounce_field_0x86[0] = 1;
        bounce_field_0x86[1] = 1;
    }
    if (line->p0.x < line->p1.x) {
        if (x >= line->p0.x) {
            bounce_field_0x86[0] = 1;
        } else {
            bounce_field_0x86[1] = 1;
        }
    } else {
        if (x < line->p1.x) {
            bounce_field_0x86[0] = 1;
        } else {
            bounce_field_0x86[1] = 1;
        }
    }
}

/* TIMWIN: 1050:0550
   Very similar to stub_1050_08fe
   Note: I double-checked this for accuracy. */
int stub_1050_0550(bool some_bool) {
    struct BorderPoint* borders_data = PART_3a6a->borders_data;
    s16 b0x = TMP_X_3a6a + borders_data[0].x;
    s16 b0y = TMP_Y_3a6a + borders_data[0].y;
    
    s16 b1x = TMP_X_3a6a + borders_data[1].x;
    s16 b1y = TMP_Y_3a6a + borders_data[1].y;

    u16 b0angle = borders_data[0].normal_angle;

    s16 somex = b0x;
    s16 somey = b0y;

    u16 border_idx = 1;
    bool found = 0;

    while (borders_data) {
        u16 quad = quadrant_from_angle(b0angle);

        if (TMP_MOVEMENT_ANGLE_3a6c - b0angle != 0xC000 && utos(TMP_MOVEMENT_ANGLE_3a6c - b0angle + 0x4000) >= 0) {
            struct BorderPoint *borders_data_3a6c = PART_3a6c->borders_data;
            u16 angle_3a6c = borders_data_3a6c->normal_angle;
            borders_data_3a6c += 1;
            int border_3a6c_idx = 1;

            while (borders_data_3a6c) {
                u16 ivar5 = angle_3a6c - b0angle + 0x8000;

                if (utos(ivar5) >= 0 || ivar5 == 0x8000) {
                    u16 ivar5 = borders_data_3a6c->normal_angle - b0angle;
                    if (ivar5 == 0x8000 || utos(ivar5 + 0x8000) < 0) {
                        if (TMP_X_DELTA_3a6c != 0 || TMP_Y_DELTA_3a6c != 0) {
                            struct Line line1, line2;

                            line1.p0.x = PART_3a6c->pos_prev1.x + borders_data_3a6c->x - somex;
                            line1.p0.y = PART_3a6c->pos_prev1.y + borders_data_3a6c->y - somey;
                            line1.p1.x = line1.p0.x + TMP_X_DELTA_3a6c;
                            line1.p1.y = line1.p0.y + TMP_Y_DELTA_3a6c;
                            line2.p0.x = 0;
                            line2.p0.y = 0;
                            line2.p1.x = b1x - somex;
                            line2.p1.y = b1y - somey;

                            s16 saved_line1_p1_x = line1.p1.x;
                            s16 saved_line1_p1_y = line1.p1.y;

                            four_points_adjust_p1_by_one(&line2);

                            struct ShortVec point;
                            int intersects = calculate_line_intersection(&line1, &line2, &point);

                            if (intersects && !VEC_EQ(point, line2.p1)) {
                                if (some_bool) {
                                    return 1;
                                }
                                /* TIMWIN: 1108:0c26 */
                                static const s16 dat_0c26[4] = { 0, -1, 0, 1 };
                                /* TIMWIN: 1108:0c2e */
                                static const s16 dat_0c2e[4] = { -1, 0, 1, 0 };

                                line2.p0.x = dat_0c26[quad];
                                line2.p0.y = dat_0c2e[quad];
                                line2.p1.x += dat_0c26[quad];
                                line2.p1.y += dat_0c2e[quad];

                                struct Part *ppvar4;
                                ppvar4 = PART_3a6c;

                                if (stub_1050_00f0(b0angle) == 0) {
                                    ppvar4 = PART_3a6c;
                                    if (calculate_line_intersection(&line1, &line2, &point)) {
                                        PART_3a6c->pos.x += point.x - saved_line1_p1_x;
                                        PART_3a6c->pos.y += point.y - saved_line1_p1_y;
                                    } else {
                                        PART_3a6c->pos = PART_3a6c->pos_prev1;
                                    }
                                } else {
                                    s16 local_22 = (line2.p1.y - line2.p0.y)*line2.p0.x - (line2.p1.x - line2.p0.x)*line2.p0.y;
                                    s16 ivar5 = -(line2.p1.x - line2.p0.x);
                                    if (ivar5 == 0) {
                                        PART_3a6c->pos = PART_3a6c->pos_prev1;
                                    } else {
                                        point.y = (local_22 - (line2.p1.y - line2.p0.y)*line1.p1.x) / ivar5;
                                        PART_3a6c->pos.y += point.y - saved_line1_p1_y;
                                    }
                                }

                                part_set_size_and_pos_render(PART_3a6c);
                                // Changes:
                                // TMP_X2_3a6c, TMP_Y2_3a6c, TMP_X_CENTER_3a6c, TMP_Y_CENTER_3a6c,
                                // TMP_X_DELTA_3a6c, TMP_Y_DELTA_3a6c, TMP_X_LEFTMOST_3a6c, TMP_Y_TOPMOST_3a6c, TMP_X_RIGHT_3a6c, TMP_Y_BOTTOM_3a6c
                                tmp_3a6c_update_vars();

                                PART_3a6c->flags1 &= ~(0x0004 | 0x0002);

                                if (PART_3a6a->type == P_EIGHTBALL) {
                                    PART_3a6c->flags1 |= 0x0004;
                                } else {
                                    if (NO_FLAGS(PART_3a6c->flags2, 0x8000) && NO_FLAGS(PART_3a6a->flags2, 0x8000) && NO_FLAGS(PART_3a6a->flags1, 0x4000)) {
                                        PART_3a6c->flags1 |= 0x0004;
                                    } else {
                                        PART_3a6c->flags1 |= 0x0002;
                                    }
                                }

                                PART_3a6c->bounce_part = PART_3a6a;
                                PART_3a6c->bounce_angle = b0angle;
                                PART_3a6c->bounce_border_index = border_idx - 1;

                                stub_1050_025e(&line2, TMP_X_CENTER_3a6c - somex, PART_3a6c->bounce_field_0x86);
                                found = 1;
                            }
                        }
                    }
                }

                border_3a6c_idx += 1;
                if (PART_3a6c->num_borders < border_3a6c_idx) {
                    borders_data_3a6c = 0;
                } else {
                    angle_3a6c = borders_data_3a6c->normal_angle;
                    if (PART_3a6c->num_borders == border_3a6c_idx) {
                        borders_data_3a6c = PART_3a6c->borders_data;
                    } else {
                        borders_data_3a6c += 1;
                    }
                }
            }
        }
        border_idx += 1;
        if (PART_3a6a->num_borders < border_idx) {
            borders_data = 0;
        } else {
            somex = b1x;
            somey = b1y;
            b0angle = borders_data[1].normal_angle;
            if (PART_3a6a->num_borders == border_idx) {
                b1x = b0x;
                b1y = b0y;
            } else {
                b1x = TMP_X_3a6a + borders_data[2].x;
                b1y = TMP_Y_3a6a + borders_data[2].y;
            }
            borders_data += 1;
        }
    }

    return found;
}

/* TIMWIN: 10a8:03ac */
bool should_parts_skip_collision(enum PartType a, enum PartType b) {
    // Checks if the two part types are a set of any two specific parts, regardless of order.

    #define CHK(x, y) ((a == x && b == y) || (b == x && a == y))

    if (CHK(P_POKEY_THE_CAT, P_MORT_THE_MOUSE)) return 1;
    if (CHK(P_MORT_THE_MOUSE, P_CHEESE)) return 1;
    if (CHK(P_MEL_SCHLEMMING, P_MELS_HOUSE)) return 1;
    if (CHK(P_MEL_SCHLEMMING, P_MEL_SCHLEMMING)) return 1;

    #undef CHK

    return 0;
}

/* TIMWIN: 1090:158b */
bool bucket_contains(struct Part *bucket, struct Part *contains) {
    if (bucket->type != P_BUCKET) return 0;

    EACH_INTERACION(bucket, curpart) {
        if (curpart == contains) {
            return 1;
        }
    }
    return 0;
}

/* TIMWIN: 1050:08fe
   Very similar to stub_1050_0550.
   Note: I double-checked this for accuracy. */
int stub_1050_08fe(bool some_bool) {
    struct BorderPoint *borders_data = PART_3a6c->borders_data;
    s16 b0x = TMP_X2_3a6c + borders_data[0].x;
    s16 b0y = TMP_Y2_3a6c + borders_data[0].y;

    s16 b1x = TMP_X2_3a6c + borders_data[1].x;
    s16 b1y = TMP_Y2_3a6c + borders_data[1].y;

    u16 b0angle = borders_data[0].normal_angle;

    s16 somex = b0x;
    s16 somey = b0y;

    u16 border_idx = 1;
    bool found = 0;

    while (borders_data) {
        u16 quad = quadrant_from_angle(b0angle + 0x8000);

        if ((TMP_MOVEMENT_ANGLE_3a6c - b0angle != 0x4000) && utos(TMP_MOVEMENT_ANGLE_3a6c - b0angle + 0xC000) >= 0) {
            struct BorderPoint *borders_data_3a6a = PART_3a6a->borders_data;
            u16 angle_3a6a = borders_data_3a6a->normal_angle;
            borders_data_3a6a += 1;
            u16 border_3a6a_idx = 1;

            while (borders_data_3a6a) {
                u16 ivar5 = angle_3a6a - b0angle + 0x8000;
                if (utos(ivar5) >= 0 || ivar5 == 0x8000) {
                    u16 ivar5 = borders_data_3a6a->normal_angle - b0angle;
                    if (ivar5 == 0x8000 || utos(ivar5 + 0x8000) < 0) {
                        if (TMP_X_DELTA_3a6c != 0 || TMP_Y_DELTA_3a6c != 0) {
                            struct Line line1, line2;

                            line1.p1.x = PART_3a6a->pos.x + borders_data_3a6a->x - somex;
                            line1.p1.y = PART_3a6a->pos.y + borders_data_3a6a->y - somey;
                            line1.p0.x = line1.p1.x + TMP_X_DELTA_3a6c;
                            line1.p0.y = line1.p1.y + TMP_Y_DELTA_3a6c;
                            line2.p0.x = 0;
                            line2.p0.y = 0;
                            line2.p1.x = b1x - somex;
                            line2.p1.y = b1y - somey;

                            s16 saved_line1_p1_x = line1.p1.x;
                            s16 saved_line1_p1_y = line1.p1.y;

                            four_points_adjust_p1_by_one(&line2);

                            struct ShortVec point;
                            int intersects = calculate_line_intersection(&line1, &line2, &point);

                            if (intersects && !VEC_EQ(point, line2.p1)) {
                                if (some_bool) {
                                    return 1;
                                }
                                /* TIMWIN: 1108:0c26 */
                                static const s16 dat_0c26[4] = { 0, -1, 0, 1 };
                                /* TIMWIN: 1108:0c2e */
                                static const s16 dat_0c2e[4] = { -1, 0, 1, 0 };

                                line2.p0.x = -dat_0c26[quad];
                                line2.p0.y = -dat_0c2e[quad];
                                line2.p1.x += -dat_0c26[quad];
                                line2.p1.y += -dat_0c2e[quad];

                                struct Part *ppvar4;
                                ppvar4 = PART_3a6c;

                                if (stub_1050_00f0(b0angle + 0x8000) == 0) {
                                    ppvar4 = PART_3a6c;
                                    if (calculate_line_intersection(&line1, &line2, &point)) {
                                        PART_3a6c->pos.x -= point.x - saved_line1_p1_x;
                                        PART_3a6c->pos.y -= point.y - saved_line1_p1_y;
                                    } else {
                                        PART_3a6c->pos = PART_3a6c->pos_prev1;
                                    }
                                } else {
                                    s16 local_22 = (line2.p1.y - line2.p0.y)*line2.p0.x - (line2.p1.x - line2.p0.x)*line2.p0.y;
                                    s16 ivar5 = -(line2.p1.x - line2.p0.x);
                                    if (ivar5 == 0) {
                                        PART_3a6c->pos = PART_3a6c->pos_prev1;
                                    } else {
                                        point.y = (local_22 - (line2.p1.y - line2.p0.y)*line1.p1.x) / ivar5;
                                        PART_3a6c->pos.y -= point.y - saved_line1_p1_y;
                                    }
                                }

                                s16 local_1e = TMP_X_CENTER_3a6c - somex;

                                part_set_size_and_pos_render(PART_3a6c);
                                // Changes:
                                // TMP_X2_3a6c, TMP_Y2_3a6c, TMP_X_CENTER_3a6c, TMP_Y_CENTER_3a6c,
                                // TMP_X_DELTA_3a6c, TMP_Y_DELTA_3a6c, TMP_X_LEFTMOST_3a6c, TMP_Y_TOPMOST_3a6c, TMP_X_RIGHT_3a6c, TMP_Y_BOTTOM_3a6c
                                tmp_3a6c_update_vars();

                                PART_3a6c->flags1 &= ~(0x0004 | 0x0002);

                                if (PART_3a6a->type == P_EIGHTBALL) {
                                    PART_3a6c->flags1 |= 0x0004;
                                } else {
                                    if (NO_FLAGS(PART_3a6c->flags2, 0x8000) && NO_FLAGS(PART_3a6a->flags2, 0x8000) && NO_FLAGS(PART_3a6a->flags1, 0x4000)) {
                                        PART_3a6c->flags1 |= 0x0004;
                                    } else {
                                        PART_3a6c->flags1 |= 0x0002;
                                    }
                                }

                                PART_3a6c->bounce_part = PART_3a6a;
                                PART_3a6c->bounce_angle = b0angle + 0x8000;

                                if (b1x < somex) {
                                    if (point.x < local_1e) {
                                        PART_3a6c->bounce_field_0x86[0] = 1;
                                    } else {
                                        PART_3a6c->bounce_field_0x86[1] = 1;
                                    }
                                } else {
                                    if (point.x < local_1e) {
                                        PART_3a6c->bounce_field_0x86[1] = 1;
                                    } else {
                                        PART_3a6c->bounce_field_0x86[0] = 1;
                                    }
                                }

                                PART_3a6c->bounce_border_index = border_3a6a_idx - 1;
                                found = 1;
                            }
                        }
                    }
                }

                border_3a6a_idx += 1;
                if (PART_3a6a->num_borders < border_3a6a_idx) {
                    borders_data_3a6a = 0;
                } else {
                    angle_3a6a = borders_data_3a6a->normal_angle;
                    if (PART_3a6a->num_borders == border_3a6a_idx) {
                        borders_data_3a6a = PART_3a6a->borders_data;
                    } else {
                        borders_data_3a6a += 1;
                    }
                }
            }
        }
        border_idx += 1;
        if (PART_3a6c->num_borders < border_idx) {
            borders_data = 0;
        } else {
            somex = b1x;
            somey = b1y;
            b0angle = borders_data[1].normal_angle;
            if (PART_3a6c->num_borders == border_idx) {
                b1x = b0x;
                b1y = b0y;
            } else {
                b1x = TMP_X2_3a6c + borders_data[2].x;
                b1y = TMP_Y2_3a6c + borders_data[2].y;
            }
            borders_data += 1;
        }
    }

    return found;
}

/* TIMWIN: 1050:02db
   Note: I double-checked it for accuracy */
int stub_1050_02db(struct Part *part) {
    PART_3a6c = part;
    if (!part->borders_data) return 0;

    PART_3a68 = part->bounce_part;
    if (part->bounce_part) {
        TMP_BOUNCE_ANGLE_3a6c = part->bounce_angle;
        TMP_QUADRANT = quadrant_from_angle(part->bounce_angle);
    }

    part->bounce_field_0x86[0] = 0;
    part->bounce_field_0x86[1] = 0;

    TMP_MOVEMENT_ANGLE_3a6c = part_get_movement_delta_angle(part);

    // Changes:
    // TMP_X2_3a6c, TMP_Y2_3a6c, TMP_X_CENTER_3a6c, TMP_Y_CENTER_3a6c,
    // TMP_X_DELTA_3a6c, TMP_Y_DELTA_3a6c, TMP_X_LEFTMOST_3a6c, TMP_Y_TOPMOST_3a6c, TMP_X_RIGHT_3a6c, TMP_Y_BOTTOM_3a6c
    tmp_3a6c_update_vars();
    
    bool has_found_angle = 0;

    if (part->bounce_part && !bucket_contains(part, part->bounce_part)) {
        PART_3a6a = PART_3a68;
        if (PART_3a68->borders_data && NO_FLAGS(PART_3a68->flags2, F2_DISAPPEARED)) {
            // Changes: TMP_X_3a6a, TMP_Y_3a6a, TMP_X_CENTER_3a6a, TMP_Y_CENTER_3a6a, TMP_X_RIGHT_3a6a, TMP_Y_BOTTOM_3a6a
            tmp_3a6a_update_vars();

            if (TMP_X_3a6a < TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c < TMP_X_RIGHT_3a6a
                && TMP_Y_3a6a < TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c < TMP_Y_BOTTOM_3a6a) {
                int result = stub_1050_0550(0);
                if (result != 0) {
                    has_found_angle = 1;
                    TMP_MOVEMENT_ANGLE_3a6c = part_get_movement_delta_angle(PART_3a6c);
                }
            }
            // unsure about side-effects from stub_1050_0550
            // which is why i haven't merged the below if statement with the above one
            if (TMP_X_3a6a < TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c < TMP_X_RIGHT_3a6a
                && TMP_Y_3a6a < TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c < TMP_Y_BOTTOM_3a6a) {
                int result = stub_1050_08fe(0);
                if (result != 0) {
                    has_found_angle = 1;
                    TMP_MOVEMENT_ANGLE_3a6c = part_get_movement_delta_angle(PART_3a6c);
                }
            }
        }
    }

    // Wasn't sure if PART_3a6a got mutated, so I decided not to use EACH_STATIC_THEN_MOVING_PART macro
    PART_3a6a = get_first_part(CHOOSE_STATIC_OR_ELSE_MOVING_PART);
    while (PART_3a6a) {
        if (!bucket_contains(PART_3a6c, PART_3a6a)) {
            if (PART_3a6c != PART_3a6a && PART_3a68 != PART_3a6a) {
                if (PART_3a6a->borders_data) {
                    if (NO_FLAGS(PART_3a6a->flags2, F2_DISAPPEARED)) {
                        if (!should_parts_skip_collision(PART_3a6c->type, PART_3a6a->type)) {
                            // Changes: TMP_X_3a6a, TMP_Y_3a6a, TMP_X_CENTER_3a6a, TMP_Y_CENTER_3a6a, TMP_X_RIGHT_3a6a, TMP_Y_BOTTOM_3a6a
                            tmp_3a6a_update_vars();

                            if (TMP_X_3a6a < TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c < TMP_X_RIGHT_3a6a
                                && TMP_Y_3a6a < TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c < TMP_Y_BOTTOM_3a6a) {
                                int result = stub_1050_0550(0);
                                if (result != 0) {
                                    has_found_angle = 1;
                                    TMP_MOVEMENT_ANGLE_3a6c = part_get_movement_delta_angle(PART_3a6c);
                                }
                            }

                            // The <= is intentional.
                            if (TMP_X_3a6a <= TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c <= TMP_X_RIGHT_3a6a
                                && TMP_Y_3a6a <= TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c <= TMP_Y_BOTTOM_3a6a) {
                                int result = stub_1050_08fe(0);
                                if (result != 0) {
                                    has_found_angle = 1;
                                    TMP_MOVEMENT_ANGLE_3a6c = part_get_movement_delta_angle(PART_3a6c);
                                }
                            }
                        }
                    }
                }
            }
        }

        PART_3a6a = next_part_or_fallback(PART_3a6a, CHOOSE_MOVING_PART);
    }
    PART_3a6a = 0;

    if (has_found_angle == 0) {
        part->bounce_part = 0;
    } else {
        if (stub_1050_00f0(part->bounce_angle) != 0) {
            PART_3a6c->flags1 |= 0x0001;
        }
    }
    return has_found_angle;
}

/* TIMWIN: 10a8:42e6
   Returns absoulute length between the two rope points.
   out_x and out_y are the signed x/y deltas between the two rope points.
*/
s16 distance_to_rope_link(struct RopeData *rope, struct Part *part, s16 *out_x, s16 *out_y) {
    s16 rope_x_1, rope_y_1;
    s16 rope_x_2, rope_y_2;
    if (rope->part1 == part) {
        int rope_slot = rope->part1_rope_slot;
        rope_x_1 = part->pos_render.x + part->rope_loc[rope_slot].x;
        rope_y_1 = part->pos_render.y + part->rope_loc[rope_slot].y;

        struct Part *links_to = part->links_to[rope_slot];
        int index = part_get_rope_link_index(part, links_to);
        if (links_to->type == P_PULLEY) {
            rope_x_2 = links_to->rope_data[0]->ends_pos[1 - index].x;
            rope_y_2 = links_to->rope_data[0]->ends_pos[1 - index].y;
        } else {
            rope_x_2 = links_to->pos_render.x + links_to->rope_loc[index].x;
            rope_y_2 = links_to->pos_render.y + links_to->rope_loc[index].y;
        }
    } else {
        int rope_slot = rope->part2_rope_slot;
        rope_x_1 = rope->part2->pos_render.x + rope->part2->rope_loc[rope_slot].x;
        rope_y_1 = rope->part2->pos_render.y + rope->part2->rope_loc[rope_slot].y;

        struct Part *links_to = rope->part2->links_to[rope_slot];
        int index = part_get_rope_link_index(rope->part2, links_to);
        if (links_to->type == P_PULLEY) {
            rope_x_2 = links_to->rope_data[0]->ends_pos[1 - index].x;
            rope_y_2 = links_to->rope_data[0]->ends_pos[1 - index].y;
        } else {
            rope_x_2 = links_to->pos_render.x + links_to->rope_loc[index].x;
            rope_y_2 = links_to->pos_render.y + links_to->rope_loc[index].y;
        }
    }

    *out_x = rope_x_1 - rope_x_2;
    *out_y = rope_y_1 - rope_y_2;

    return approx_hypot(abs(rope_x_1 - rope_x_2), abs(rope_y_1 - rope_y_2));
}

/* TIMWIN: 10a8:4509 */
int stub_10a8_4509(struct Part *part_a, struct Part *part_b) {
    s32 force = part_a->force;

    for (struct Llama *llama2 = LLAMA_2; llama2 != 0; llama2 = llama2->next) {
        if (llama2->part == part_b && llama2->force >= force) {
            return 0;
        }
    }

    struct Llama *llama1_head = LLAMA_1;

    if (LLAMA_2 && LLAMA_2->force >= force) {
        struct Llama *llama2_insert_point = LLAMA_2;
        
        struct Llama *cur = LLAMA_2->next;
        while (cur) {
            if (cur->force < force) break;
            llama2_insert_point = cur;
            cur = cur->next;
        }

        // Pop head of LLAMA_1, insert it AFTER llama2_insert_point.

        struct Llama *tmp = LLAMA_1;
        LLAMA_1 = LLAMA_1->next;
        tmp->next = llama2_insert_point->next;
        llama2_insert_point->next = tmp;
    } else {
        // Pop head of LLAMA_1, prepend to LLAMA_2

        struct Llama *tmp = LLAMA_1;
        LLAMA_1 = LLAMA_1->next;
        tmp->next = LLAMA_2;
        LLAMA_2 = tmp;
    }

    llama1_head->part = part_b;
    llama1_head->force = force;
    return 1;
}

/* TIMWIN: 10a8:2bea */
void stub_10a8_2bea(struct ShortVec *pos, struct ShortVec *size, u8 param3, u8 param4, s16 param5) {
    // doesn't update parts or datas. might be related to drawing. could maybe omit in the port?
    // do nothing, and let's see how that works out.
}

/* TIMWIN: 10a8:28f6 */
void stub_10a8_28f6(struct Part *part, int _unused) {
    // Called when ropes are used.
    // I think it's related to drawing. Could maybe omit in the port?
    // do nothing, and let's see how that works out.
}

/* TIMWIN: 10a8:3c6d */
static inline void stub_10a8_3c6d(struct Part *part, byte param2, byte param3, u16 param4) {
    if ((param4 & 1) != 0) {
        part->vel_hi_precision.x = (part->pos.x - part->pos_prev1.x) << (9 - (param2 % 32));
    }
    if ((param4 & 2) != 0) {
        part->vel_hi_precision.y = (part->pos.y - part->pos_prev1.y) << (9 - (param3 % 32));
    }
    part_clamp_to_terminal_velocity(part);
}

/* TIMWIN: 10a8:3cc1 */
int stub_10a8_3cc1(struct Part *part) {
    enum PartType links_to_0_type = part->links_to[0]->type;

    int part_cmp_pos_y;
    if (part->pos.y < part->pos_prev1.y) {
        // Went up
        part_cmp_pos_y = 0;
    } else if (part->pos.y > part->pos_prev1.y) {
        // Went down
        part_cmp_pos_y = 1;
    } else {
        // Didn't change
        part_cmp_pos_y = -1;
    }

    struct RopeData *rope = part->rope_data[0];
    struct Part *rope_or_pulley_part = rope->rope_or_pulley_part;
    struct Part *other_part = rope_get_other_part(part, rope);

    byte rope_slot;
    s16 thing1, thing2, thing3;
    if (rope->part1 != part) {
        rope_slot = rope->part1_rope_slot;
        thing1 = rope_or_pulley_part->extra2;
        thing2 = rope_or_pulley_part->extra1;
        thing3 = 1;
    } else {
        rope_slot = rope->part2_rope_slot;
        thing1 = rope_or_pulley_part->extra1;
        thing2 = rope_or_pulley_part->extra2;
        thing3 = 0;
    }

    s16 delta_other_x, delta_other_y;
    s16 delta_part_x, delta_part_y;
    s16 distance_other_part = distance_to_rope_link(rope, other_part, &delta_other_x, &delta_other_y);
    s16 distance_part       = distance_to_rope_link(rope, part,       &delta_part_x, &delta_part_y);

    s16 adj_distance_part = distance_part - thing1;

    if (part->type != P_ROPE_SEVERED_END && adj_distance_part > 0 && (s16)(distance_other_part - thing2) < 0) {
        s16 v = adj_distance_part + distance_other_part - thing2;
        s16 tmp;
        if (v <= 0) {
            adj_distance_part = 0;
            tmp = v;
        } else {
            adj_distance_part = v;
            tmp = 0;
        }

        if (rope->part1 == part) {
            thing1 = distance_part - adj_distance_part;
            rope_or_pulley_part->extra1 = thing1;
            thing2 = distance_other_part - tmp;
            rope_or_pulley_part->extra2 = thing2;
        } else {
            thing1 = distance_part - adj_distance_part;
            rope_or_pulley_part->extra2 = thing1;
            thing2 = distance_other_part - tmp;
            rope_or_pulley_part->extra1 = thing2;
        }
    }

    if (adj_distance_part > 0 && ANY_FLAGS(other_part->flags1, 0x1000)) {
        if (other_part->type != P_ROPE_SEVERED_END && part->type != P_ROPE_SEVERED_END && other_part->mass < part->mass) {
            s16 mass_delta = part->mass - other_part->mass;
            s16 tmp = abs(adj_distance_part);
            s32 tmp2 = (s32)tmp * (s32)mass_delta;
            s16 tmp3 = (tmp2 + part->mass) / (part->mass);
            tmp = abs(thing2);
            mass_delta = tmp3;
            if (mass_delta < 2) {
                mass_delta = 1;
            }
            if (mass_delta < abs(thing2)) {
                if (tmp3 < 2) {
                    tmp3 = 1;
                }
            } else {
                tmp3 = abs(thing2);
            }
            if (tmp3 != 0) {
                if (rope->part1 == part) {
                    rope_or_pulley_part->extra2 -= tmp3;
                    thing2 = rope_or_pulley_part->extra2;

                    stub_10a8_3cc1(other_part);
                    other_part->flags1 &= ~(0x000f);
                    stub_1050_02db(other_part);
                    distance_other_part = distance_to_rope_link(rope, other_part, &delta_other_x, &delta_other_y);
                    mass_delta = distance_other_part - thing2;
                    if (mass_delta != 0) {
                        rope_or_pulley_part->extra2 += mass_delta;
                        tmp3 -= mass_delta;
                    }
                    if (tmp3 != 0) {
                        rope_or_pulley_part->extra1 += tmp3;
                        thing1 = rope_or_pulley_part->extra1;
                        adj_distance_part = distance_part - thing1;
                    }
                } else {
                    rope_or_pulley_part->extra1 -= tmp3;
                    thing2 = rope_or_pulley_part->extra1;

                    stub_10a8_3cc1(other_part);
                    other_part->flags1 &= ~(0x0008 | 0x0004 | 0x0002 | 0x0001);
                    stub_1050_02db(other_part);
                    distance_other_part = distance_to_rope_link(rope, other_part, &delta_other_x, &delta_other_y);
                    mass_delta = distance_other_part - thing2;
                    if (mass_delta != 0) {
                        rope_or_pulley_part->extra1 += mass_delta;
                        tmp3 -= mass_delta;
                    }
                    if (tmp3 != 0) {
                        rope_or_pulley_part->extra2 += tmp3;
                        thing1 = rope_or_pulley_part->extra2;
                        adj_distance_part = distance_part - thing1;
                    }
                }
            }
        }
    }

    if (adj_distance_part > 0) {
        if (other_part->type == P_ROPE_SEVERED_END && part->type != P_ROPE_SEVERED_END) {
            if (links_to_0_type == P_PULLEY) {
                if (rope->part1 == other_part) {
                    rope_or_pulley_part->extra1 -= adj_distance_part;
                    if (rope_or_pulley_part->extra1 < 0) {
                        adj_distance_part += rope_or_pulley_part->extra1;

                        enum LevelState prev_level_state = LEVEL_STATE;
                        LEVEL_STATE = DESIGN_MODE;
                        stub_10a8_28f6(rope_or_pulley_part, 3);
                        LEVEL_STATE = prev_level_state;

                        struct Part *a_part = other_part->links_to[rope->part1_rope_slot];
                        struct Part *b_part = a_part->links_to[0];
                        int rope_link_idx = part_get_rope_link_index(a_part, b_part);
                        other_part->links_to[rope->part1_rope_slot] = b_part;
                        b_part->links_to[rope_link_idx] = other_part;
                        a_part->links_to[0] = 0;
                        a_part->links_to[1] = 0;

                        rope->rope_or_pulley_part->extra1 = approximate_hypot_of_rope(rope, ROPETIME_CURRENT, ROPE_FROM_FIRST);
                    }
                    rope_or_pulley_part->extra2 += adj_distance_part;
                } else {
                    rope_or_pulley_part->extra2 -= adj_distance_part;
                    if (rope_or_pulley_part->extra2 < 0) {
                        adj_distance_part += rope_or_pulley_part->extra2;

                        enum LevelState prev_level_state = LEVEL_STATE;
                        LEVEL_STATE = DESIGN_MODE;
                        stub_10a8_28f6(rope_or_pulley_part, 3);
                        LEVEL_STATE = prev_level_state;

                        struct Part *a_part = other_part->links_to[rope->part2_rope_slot];
                        struct Part *b_part = a_part->links_to[1];
                        int rope_link_idx = part_get_rope_link_index(a_part, b_part);
                        other_part->links_to[rope->part2_rope_slot] = b_part;
                        b_part->links_to[rope_link_idx] = other_part;
                        a_part->links_to[0] = 0;
                        a_part->links_to[1] = 0;

                        rope->rope_or_pulley_part->extra2 = approximate_hypot_of_rope(rope, ROPETIME_CURRENT, ROPE_FROM_LAST);
                    }
                    rope_or_pulley_part->extra1 += adj_distance_part;
                }
            }
            
            return 0;
        } else {
            part->pos.x += (((s32)delta_part_x * (s32)thing1) / distance_part) - delta_part_x;
            part->pos_x_hi_precision = part->pos.x * 512;

            part->pos.y += (((s32)delta_part_y * (s32)thing1) / distance_part) - delta_part_y;
            part->pos_y_hi_precision = part->pos.y * 512;

            part_set_size_and_pos_render(part);
            stub_10a8_3c6d(part, 0, 0, 1);

            if (part->pos.x == part->pos_prev1.x) {
                part->vel_hi_precision.y = 0;
            }

            if (part->type != P_ROPE_SEVERED_END && part_cmp_pos_y != -1) {
                u16 rope_flags = rope_calculate_flags(rope, thing3, part_cmp_pos_y == 0 ? 0 : 1);

                if (other_part->type == P_TEETER_TOTTER) {
                    s16 state2 = 0;
                    if ((rope_flags & 0x0004) == 0) {
                        if (rope_slot == 0) {
                            if (other_part->state1 < 2) {
                                state2 = 1;
                            }
                        } else {
                            if (other_part->state1 > 0) {
                                state2 = -1;
                            }
                        }
                    } else {
                        if (rope_slot == 0) {
                            if (other_part->state1 > 0) {
                                state2 = -1;
                            }
                        } else {
                            if (other_part->state1 < 2) {
                                state2 = 1;
                            }
                        }
                    }

                    int res = stub_10a8_4509(part, other_part);

                    if (res != 0) {
                        other_part->state2 = state2;
                        other_part->force = part->force;
                    }
                } else {
                    part_rope(other_part->type, part, other_part, 0, rope_flags, part_mass(part->type), part->force);
                }
            }

            return 1;
        }
    }

    return 0;
}

/* TIMWIN: 1080:1777
   Note: I double-checked this for accuracy. */
void stub_1080_1777(struct Part *part) {
    if (ANY_FLAGS(part->flags2, F2_DISAPPEARED)) return;

    if (part->type != P_TEAPOT) {
        part_run(part);
    }

    adjust_part_position(part);
    part->flags1 &= ~(0x0008 | 0x0004 | 0x0002 | 0x0001);

    stub_1050_02db(part);

    if (part->rope_data[0]) {
        int v = stub_10a8_3cc1(part);
        if (v == 0) {
            struct Part *saved_84 = part->bounce_part;
            byte saved_86_0 = part->bounce_field_0x86[0];
            byte saved_86_1 = part->bounce_field_0x86[1];
            part->bounce_part = 0;

            stub_1050_02db(part);

            if (!part->bounce_part) {
                part->bounce_part = saved_84;
                part->bounce_field_0x86[0] = saved_86_0;
                part->bounce_field_0x86[1] = saved_86_1;
            }
        } else {
            part->flags1 &= ~(0x0008 | 0x0004 | 0x0002 | 0x0001);
            stub_1050_02db(part);
        }
    }
}

/* TIMWIN: 1090:1480 */
void bucket_handle_contained_parts(struct Part *bucket) {
    if (bucket->type != P_BUCKET) {
        return;
    }

    bucket->interactions = 0;

    EACH_MOVING_PART(curpart) {
        if (bucket == curpart) continue;
        if (ANY_FLAGS(curpart->flags2, F2_DISAPPEARED)) continue;
        if (curpart->type == P_CAGE) continue;

        s16 curpart_x_center = curpart->pos_prev1.x + curpart->size.x/2;
        s16 curpart_y_bottom = curpart->pos_prev1.y + curpart->size.y;
        if (curpart->type == P_ROCKET) {
            curpart_y_bottom -= 12;
        }

        bool in_x = BETWEEN_EXCL(bucket->pos_prev1.x + 4, curpart_x_center, bucket->pos_prev1.x + 32);
        bool in_y = BETWEEN_EXCL(bucket->pos_prev1.y + 20, curpart_y_bottom, bucket->pos_prev1.y + bucket->size.y + 4);
        bool in_bucket = in_x && (((curpart->bounce_part == bucket) && (curpart->vel_hi_precision.y > 0)) || in_y);

        if (in_bucket) {
            curpart->interactions = 0;
            bucket->interactions = curpart;
            curpart->flags3 |= 0x0010;

            curpart->vel_hi_precision = bucket->vel_hi_precision;

            curpart->extra1 = bucket->pos.y + 20;
        }
    }
}

/* TIMWIN: 10a8:45f8 */
void bucket_add_mass(struct Part *bucket, struct Part *part) {
    s32 sum = bucket->mass + part->mass;
    if (sum > 32000) {
        sum = 32000;
    }
    bucket->mass = (s16)sum;
}

/* TIMWIN: 10a8:45c6 */
void bucket_add_mass_of_contained(struct Part *bucket) {
    EACH_INTERACION(bucket, curpart) {
        bucket_add_mass(bucket, curpart);
    }
}

/* TIMWIN: 1090:15c8 */
void bucket_move_contained(struct Part *bucket) {
    if (bucket->type != P_BUCKET) return;

    s16 dx = bucket->pos.x - bucket->pos_prev1.x;
    s16 dy = bucket->pos.y - bucket->pos_prev1.y;
    if ((dx != 0) || (dy != 0)) {
        // the bucket moved
        EACH_INTERACION(bucket, part) {
            part->pos.x += dx;
            part->pos.y += dy;
            part_set_size_and_pos_render(part);
            part->pos_x_hi_precision = part->pos.x * 512;
            part->pos_y_hi_precision = part->pos.y * 512;
        }
    }
}

/* TIMWIN: 1090:05f8 */
static inline void check_play_bowling_ball_impact_sound(struct Part *part) {
    if (part->type == P_BOWLING_BALL) {
        if (abs(part->vel_hi_precision.x) + abs(part->vel_hi_precision.y) > 0x1000) {
            play_sound(0x14);
        }
    }
}

/* TIMWIN: 10a8:0328 */
static inline u16 stub_10a8_0328(struct Part *a, struct Part *b) {
    return arctan_c((a->pos.x + a->size.x/2) - (b->pos.x + b->size.x/2),
                    (b->pos.y + b->size.y/2) - (a->pos.y + a->size.y/2));
}

/* TIMWIN: 1090:0809 */
void stub_1090_0809(struct Part *part) {
    // I think this function is called on bounce impact?

    check_play_bowling_ball_impact_sound(part);

    struct Part *bounce_part = part->bounce_part;
    part->flags1 |= 0x0008;
    bounce_part->flags1 |= 0x0008;

    s16 part_m = part_mass(part->type);
    s16 bounce_part_m = part_mass(bounce_part->type);

    s16 x1 = part->vel_hi_precision.x;
    s16 y1 = part->vel_hi_precision.y;
    s16 x2 = bounce_part->vel_hi_precision.x;
    s16 y2 = bounce_part->vel_hi_precision.y;

    u16 angle = stub_10a8_0328(part, bounce_part) + 0xC000;

    rotate_point_c(&x1, &y1, angle);
    rotate_point_c(&x2, &y2, angle);

    s32 total_mass = part_m + bounce_part_m;

    s32 lvar4 = part_m*x1;
    s32 lvar5 = bounce_part_m*x2;
    s32 lvar6 = bounce_part_m*x1;
    s32 lvar7 = part_m*x2;

    x1 = ((lvar4 + lvar5*2) - lvar6) / total_mass;
    x2 = ((lvar5 + lvar4*2) - lvar7) / total_mass;
    rotate_point_c(&x1, &y1, uneg(angle));
    rotate_point_c(&x2, &y2, uneg(angle));

    part->vel_hi_precision.x = x1/2;
    part->vel_hi_precision.y = y1/2;
    bounce_part->vel_hi_precision.x = x2/2;
    bounce_part->vel_hi_precision.y = y2/2;

    bool somebool = 0;

    if (abs(part->vel_hi_precision.x) < 0x100 && abs(bounce_part->vel_hi_precision.x) < 0x100) {
        somebool = 1;
    }

    if (ANY_FLAGS(part->flags1, 0x0001)) {
        somebool = 1;
    }

    if (abs(part->vel_hi_precision.y) < abs(part->vel_hi_precision.x)) {
        somebool = 0;
    }

    if (ANY_FLAGS(part->flags3, 0x0010)) {
        somebool = 1;
    }

    if (somebool) {
        if ((part->pos.x + part->size.x/2) < (bounce_part->pos.x + bounce_part->size.x/2)) {
            if (part->vel_hi_precision.x > -0x400) {
                part->vel_hi_precision.x = -0x400;
            }
            if (NO_FLAGS(part->flags3, 0x0010) && bounce_part->vel_hi_precision.x < 0x400) {
                bounce_part->vel_hi_precision.x = 0x400;
            }
        } else {
            if (part->vel_hi_precision.x < 0x400) {
                part->vel_hi_precision.x = 0x400;
            }
            if (NO_FLAGS(part->flags3, 0x0010) && (bounce_part->vel_hi_precision.x > -0x400)) {
                bounce_part->vel_hi_precision.x = -0x400;
            }
        }
    }

    part_clamp_to_terminal_velocity(part);
    part_clamp_to_terminal_velocity(bounce_part);

    if (part->vel_hi_precision.x < 0) {
        part->pos_x_hi_precision = part->pos.x * 512;
    } else {
        part->pos_x_hi_precision = (part->pos.x + 1)*512 - 1;
    }

    if (part_acceleration(part->type) < 0) {
        part->pos_y_hi_precision = part->pos.y * 512;
    } else {
        part->pos_y_hi_precision = (part->pos.y + 1)*512 - 1;
    }

    if (bounce_part->vel_hi_precision.x < 0) {
        bounce_part->pos_x_hi_precision = bounce_part->pos.x * 512;
    } else {
        bounce_part->pos_x_hi_precision = (bounce_part->pos.x + 1)*512 - 1;
    }

    if (part_acceleration(bounce_part->type) < 0) {
        bounce_part->pos_y_hi_precision = bounce_part->pos.y * 512;
    } else {
        bounce_part->pos_y_hi_precision = (bounce_part->pos.y + 1)*512 - 1;
    }
}

/* TIMWIN: 1090:0644 */
void stub_1090_0644(struct Part *part) {
    check_play_bowling_ball_impact_sound(part);

    u16 angle = part->bounce_angle;
    if (angle == 0 || angle == 0x8000) {
        if (part->bounce_field_0x86[0] == 0) {
            angle += 0x1000;
        } else if (part->bounce_field_0x86[1] == 0) {
            angle -= 0x1000;
        }
    }

    s16 x1 = part->vel_hi_precision.x;
    s16 y1 = part->vel_hi_precision.y;
    rotate_point_c(&x1, &y1, angle);

    s16 part_bou = MIN(part_bounciness(part->bounce_part->type), part_bounciness(part->type));

    if (part->type == P_SUPER_BALL && part_bou > 0x7f) {
        part_bou = part_bounciness(part->type);
    }

    if (part_friction(part->type) == 0) {
        y1 = -y1;
    } else {
        s32 v = -((s32)(y1 * part_bou) / 256);

        if (v < 0) {
            y1 = v + 0x40;
            if (y1 >= 0) {
                y1 = 0;
            }
        } else {
            y1 = v - 0x40;
            if (y1 <= 0) {
                y1 = 0;
            }
        }
    }

    rotate_point_c(&x1, &y1, uneg(angle));
    part->vel_hi_precision.x = x1;
    part->vel_hi_precision.y = y1;
    part_clamp_to_terminal_velocity(part);

    if (x1 < 0) {
        part->pos_x_hi_precision = part->pos.x * 512;
    } else {
        part->pos_x_hi_precision = (part->pos.x + 1)*512 - 1;
    }

    if (part_acceleration(part->type) < 0) {
        part->pos_y_hi_precision = part->pos.y * 512;
    } else {
        part->pos_y_hi_precision = (part->pos.y + 1)*512 - 1;
    }
}

/* TIMWIN: 1090:033f */
void stub_1090_033f(struct Part *part) {
    struct Part *bounce_part = part->bounce_part;

    s16 part_accel = part_acceleration(part->type);
    u16 angle = part->bounce_angle;
    if (angle == 0 || angle == 0x8000) {
        if (part->bounce_field_0x86[0] == 0) {
            angle += 0x1000;
        } else if (part->bounce_field_0x86[1] == 0) {
            angle -= 0x1000;
        }
    }

    s16 friction;
    if (bounce_part->type == P_CONVEYOR && bounce_part->state2 != 0) {
        friction = 0x100;
    } else {
        friction = part_friction(part->type);
        if (part_friction(bounce_part->type) >= friction) {
            friction = part_friction(bounce_part->type);
        }
    }

    s16 c = cosine_c(uneg(angle));
    s16 s = sine_c(uneg(angle));

    s16 tmp2;
    if ((part->vel_hi_precision.x <= 0 || utos(angle) <= 0) && (part->vel_hi_precision.x >= 0 || utos(angle) >= 0)) {
        tmp2 = 0;
    } else {
        tmp2 = abs(((s32)s*(s32)part->vel_hi_precision.x) >> 0xe);
    }

    s32 tmp4 = abs((((s32)c * (s32)abs(part_accel)) >> 0xe) + tmp2);
    tmp4 = tmp4 * friction;
    tmp4 = c * (s16)(((u32)tmp4) >> 8);

    s16 tmp6;
    if (NO_FLAGS(part->flags1, 0x0020)) {
        tmp6 = abs(tmp4 >> 0xe) + 2;
    } else {
        tmp6 = abs(tmp4 >> 0xe) + 32;
    }

    tmp4 = (s16)((s*abs(part_accel)) >> 0xe) * abs(c);

    s16 part_vel_x_hi = part->vel_hi_precision.x + (s16)(tmp4 >> 0xe);
    if (part_vel_x_hi < 0) {
        part_vel_x_hi += tmp6;
        if (part_vel_x_hi >= 0) {
            part_vel_x_hi = 0;
        }
    } else {
        part_vel_x_hi -= tmp6;
        if (part_vel_x_hi <= 0) {
            part_vel_x_hi = 0;
        }
    }

    part->vel_hi_precision.x = part_vel_x_hi;

    if (((angle + 0x4000) & 0x8000) == 0) {
        part->vel_hi_precision.y = (sine_c(uneg(angle)) * part_vel_x_hi) >> 0xe;
    } else {
        part->vel_hi_precision.y = (sine_c(uneg(angle)) * -part_vel_x_hi) >> 0xe;
    }

    part_clamp_to_terminal_velocity(part);

    if (part_accel < 0) {
        part->pos_y_hi_precision = part->pos.y * 512;
    } else {
        part->pos_y_hi_precision = (part->pos.y + 1)*512 - 1;
    }
}

struct GDIRect {
    s16 left;
    s16 top;
    s16 right;
    s16 bottom;
};

/* TIMWIN: 1020:02ba */
bool calculate_intersecting_rect(struct GDIRect *out, struct GDIRect *a, struct GDIRect *b) {
    out->left   = MAX(a->left,   b->left);
    out->right  = MIN(a->right,  b->right);
    out->top    = MAX(a->top,    b->top);
    out->bottom = MIN(a->bottom, b->bottom);

    return (out->left < out->right) && (out->top < out->bottom);
}

int stub_10a8_1329(struct BeltData *belt) {
    UNIMPLEMENTED;
    return 0;
}

/* TIMWIN: 10a8:21cb */
void stub_10a8_21cb(struct Part *part, u8 c) {
    if (SQUIRREL != 0) return;

    if (part->type != P_ROPE_SEVERED_END) {
        part->field_0x14 = c;
    }

    if (part->type == P_PULLEY) {
        struct RopeData *rope = part->rope_data[1];
        if (rope) {
            rope->rope_or_pulley_part->field_0x14 = c;
        }
    } else {
        struct BeltData *belt = part->belt_data;
        if (belt) {
            if (LEVEL_STATE == DESIGN_MODE) {
                belt_set_four_pos(belt);
                int result = stub_10a8_1329(belt);
                if (result != 0) {
                    belt->belt_part->field_0x14 = c;
                }
            } else {
                belt->belt_part->field_0x14 = c;
            }
        }

        if (LEVEL_STATE == SIMULATION_MODE) {
            struct RopeData *rope = part->rope_data[0];
            if (rope && rope->rope_or_pulley_part->field_0x14 == 0) {
                rope->rope_or_pulley_part->field_0x14 = c;
                stub_10a8_176f(rope);
            }
            rope = part->rope_data[1];
            if (rope && rope->rope_or_pulley_part->field_0x14 == 0) {
                rope->rope_or_pulley_part->field_0x14 = c;
                stub_10a8_176f(rope);
            }
        } else {
            for (int i = 0; i < 2; i++) {
                struct RopeData *rope = part->rope_data[i];
                if (rope) {
                    rope->rope_or_pulley_part->field_0x14 = c;
                    stub_10a8_176f(rope);
                }
            }
        }
    }
}

void stub_10a8_28a5(struct Part *part, int _unused) {
    // doesn't do anything except call stub_10a8_2bea
    // stub_10a8_2bea doesn't update parts or datas. might be related to drawing. could maybe omit in the port?
    
    UNIMPLEMENTED;
}

/* TIMWIN: 10a8:280a */
void stub_10a8_280a(struct Part *part, int c) {
    if (part->type == P_PULLEY) {
        if (part->rope_data[1]) {
            stub_10a8_28f6(part->rope_data[1]->rope_or_pulley_part, c);
        }
    } else {
        if (part->type != P_BELT && part->type != P_ROPE) {
            if (LEVEL_STATE != SIMULATION_MODE && part->belt_data) {
                stub_10a8_28a5(part->belt_data->belt_part, c);
            }
            if (part->rope_data[0]) {
                stub_10a8_28f6(part->rope_data[0]->rope_or_pulley_part, c);
            }
            if (part->rope_data[1]) {
                stub_10a8_28f6(part->rope_data[1]->rope_or_pulley_part, c);
            }
        }
    }
}

/* TIMWIN: 1040:197d */
bool is_low_res_and_specific_part(enum PartType type) {
    if (VALUES_PER_PIXEL > 256) return 0;

    // I'm not really sure what's so special about these parts.
    switch (type) {
        P_BRICK_WALL:
        P_INCLINE:
        P_MORT_THE_MOUSE_CAGE:
        P_CONVEYOR:
        P_PULLEY:
        P_LIGHT_SWITCH_OUTLET:
        P_EYE_HOOK:
        P_FAN:
        P_MAGNIFYING_GLASS:
        P_SOLAR_PANELS:
        P_PIPE_STRAIGHT:
        P_PIPE_CURVED:
        P_WOOD_WALL:
        P_ELECTRIC_ENGINE:
        P_NAIL:
        P_DIRT_WALL:
        P_PINBALL_BUMPER:
            return 1;

        default:
            return 0;
    }
}

/* TIMWIN: 10a8:2b6d */
void stub_10a8_2b6d(struct Part *part, int c) {
    if (SQUIRREL != 0) return;

    if (part->type == P_BELT) {
        stub_10a8_28a5(part, c);
    } else if (part->type == P_ROPE) {
        stub_10a8_28f6(part, c);
    } else {
        if (!(is_low_res_and_specific_part(part->type) && LEVEL_STATE == SIMULATION_MODE)) {
            stub_10a8_2bea(&part->pos_render_prev1, &part->size_prev1, 1, 1, 0);
        }
    }
}

/* TIMWIN: 10a8:4690 */
void part_set_prev_vars(struct Part *part) {
    part->pos_prev2 = part->pos_prev1;
    part->pos_prev1 = part->pos;

    part->pos_render_prev2 = part->pos_render_prev1;
    part->pos_render_prev1 = part->pos_render;

    part->size_prev2 = part->size_prev1;
    part->size_prev1 = part->size;

    part->state1_prev2 = part->state1_prev1;
    part->state1_prev1 = part->state1;

    if (part->type == P_BELT && LEVEL_STATE == DESIGN_MODE) {
        struct BeltData *belt = part->belt_data;

        belt->pos1_prev2 = belt->pos1_prev1;
        belt->pos1_prev1 = belt->pos1;

        belt->pos2_prev2 = belt->pos2_prev1;
        belt->pos2_prev1 = belt->pos2;

        belt->pos3_prev2 = belt->pos3_prev1;
        belt->pos3_prev1 = belt->pos3;

        belt->pos4_prev2 = belt->pos4_prev1;
        belt->pos4_prev1 = belt->pos4;
    }

    if (part->type == P_ROPE || part->type == P_PULLEY) {
        struct RopeData *rope = part->rope_data[0];

        rope->rope_unknown_prev2 = rope->rope_unknown_prev1;
        rope->rope_unknown_prev1 = rope->rope_unknown;

        rope->ends_pos_prev2[0] = rope->ends_pos_prev1[0];
        rope->ends_pos_prev1[0] = rope->ends_pos[0];

        rope->ends_pos_prev2[1] = rope->ends_pos_prev1[1];
        rope->ends_pos_prev1[1] = rope->ends_pos[1];
    }

    part->extra1_prev2 = part->extra1_prev1;
    part->extra1_prev1 = part->extra1;

    part->extra2_prev2 = part->extra2_prev1;
    part->extra2_prev1 = part->extra2;
}

/* TIMWIN: 10a8:4645 */
void all_parts_set_prev_vars() {
    if (SELECTED_PART) {
        part_set_prev_vars(SELECTED_PART);
    }

    EACH_STATIC_THEN_MOVING_PART(part) {
        if (part != SELECTED_PART) {
            part_set_prev_vars(part);
        }
    }
}

/* TIMWIN: 10a8:36f0 */
void stub_10a8_36f0(struct Part *part) {
    stub_10a8_21cb(part, 1);
    stub_10a8_280a(part, 1);
    if (part->type != P_ROPE_SEVERED_END) {
        stub_10a8_2b6d(part, 1);
    }
}

/* TIMWIN: 10a8:078e */
void stub_10a8_078e(struct RopeData *rope) {
    struct Part *part = rope->part1->links_to[rope->part1_rope_slot];

    while (part && part->type == P_PULLEY) {
        SWAP(part->links_to[0], part->links_to[1]);

        part->links_to_design[0] = part->links_to[0];
        part->links_to_design[1] = part->links_to[1];

        SWAP(part->rope_loc[0], part->rope_loc[1]);

        struct RopeData *rd0 = part->rope_data[0];
        SWAP(rd0->ends_pos[0], rd0->ends_pos[1]);
        SWAP(rd0->ends_pos_prev1[0], rd0->ends_pos_prev1[1]);
        SWAP(rd0->ends_pos_prev2[0], rd0->ends_pos_prev2[1]);

        part = part->links_to[1];
    }

    SWAP(rope->part1, rope->part2);
    rope->original_part1 = rope->part1;
    rope->original_part2 = rope->part2;

    SWAP(rope->part1_rope_slot, rope->part2_rope_slot);
    rope->original_part1_rope_slot = rope->part1_rope_slot;
    rope->original_part2_rope_slot = rope->part2_rope_slot;

    stub_10a8_2b6d(rope->rope_or_pulley_part, 3);
}

/* TIMWIN: 10a8:0880 */
struct Part* stub_10a8_0880(struct Part *a, struct Part *b) {
    UNIMPLEMENTED;
    return 0;
}

/* TIMWIN: 10a8:0ab8 */
struct Part* stub_10a8_0ab8(struct Part *part) {
    if (part && NO_FLAGS(part->flags3, 0x0040)) {
        struct Part *p = stub_10a8_0880(part, part);
        if (p) {
            return p;
        }
    }

    struct Part *anotherpart = 0;

    EACH_STATIC_THEN_MOVING_PART(curpart) {
        struct Part *somepart = stub_10a8_0880(part, curpart);

        if (part && ANY_FLAGS(somepart->flags1, 0x8000)) {
            continue;
        }

        if (somepart) {
            anotherpart = somepart;
            if (NO_FLAGS(somepart->flags1, 0x8000) && NO_FLAGS(somepart->flags3, 0x0040)) {
                return somepart;
            }
        }
    }

    if (anotherpart) {
        return anotherpart;
    }

    if (SELECTED_PART && SELECTED_PART->type == P_ROPE) {
        return 0;
    }

    return part;
}

/* TIMWIN: 1080:1464 */
void advance_parts() {
    EACH_STATIC_PART(part) {
        part->flags2 &= ~(0x0400 | 0x0200 | 0x0040);
    }

    for (struct Llama *p = LLAMA_2; p != 0; p = p->next) {
        struct Part *part = p->part;
        if (NO_FLAGS(part->flags2, 0x0040)) {
            part_run(part);
        }
    }

    move_llama2_to_beginning_of_llama1();

    EACH_STATIC_PART(part) {
        if (ANY_FLAGS(part->flags2, 0x0800) && NO_FLAGS(part->flags2, F2_DISAPPEARED | 0x0040)) {
            part_run(part);
        }
    }

    EACH_STATIC_PART(part) {
        if (part->type == P_GEAR && NO_FLAGS(part->flags2, F2_DISAPPEARED | 0x0040)) {
            part_run(part);
        }
    }

    EACH_STATIC_PART(part) {
        if (NO_FLAGS(part->flags2, F2_DISAPPEARED | 0x0800 | 0x0040)) {
            part_run(part);
        }
    }

    EACH_MOVING_PART(part) {
        if (part->type == P_TEAPOT) {
            part_run(part);
        }
    }

    EACH_MOVING_PART(part) {
        if (NO_FLAGS(part->flags2, F2_DISAPPEARED)) {
            part_update_vel_and_force(part);
        }
        part->mass = part_mass(part->type);
        part->flags3 &= ~(0x0010);
    }

    EACH_MOVING_PART(part) {
        if (part->type != P_BUCKET) {
            // collission is eventually handled here
            stub_1080_1777(part);
        }
    }

    EACH_MOVING_PART(part) {
        if (part->type == P_BUCKET) {
            bucket_handle_contained_parts(part);
            bucket_add_mass_of_contained(part);
        }
    }

    EACH_MOVING_PART(part) {
        if (part->type == P_BUCKET) {
            bucket_handle_contained_parts(part);
            stub_1080_1777(part);
        }
    }

    EACH_MOVING_PART(part) {
        if (part->type == P_BUCKET) {
            bucket_handle_contained_parts(part);
            bucket_move_contained(part);
        }
    }

    EACH_MOVING_PART(part) {
        if (NO_FLAGS(part->flags1, 0x0008) && NO_FLAGS(part->flags2, F2_DISAPPEARED)) {
            if (ANY_FLAGS(part->flags1, 0x0004 | 0x0002) && part->type == P_MEL_SCHLEMMING) {
                MEL_JUMPY(part);
            }
            if (NO_FLAGS(part->flags1, 0x0002)) {
                if (ANY_FLAGS(part->flags1, 0x0004)) {
                    int res = part_bounce(part->bounce_part->type, part);
                    if (res != 0) {
                        stub_1090_0809(part);
                    }
                }
            } else {
                int res = part_bounce(part->bounce_part->type, part);
                if (res != 0) {
                    if (NO_FLAGS(part->flags1, 0x0001) || part->type == P_MEL_SCHLEMMING) {
                        // Seems to get called when the part has to bounce off a surface
                        stub_1090_0644(part);
                    } else {
                        // Seems to get called when the part is resting on a surface
                        stub_1090_033f(part);
                    }
                }
            }
        }
    }

    EACH_STATIC_THEN_MOVING_PART(part) {
        if (ANY_FLAGS(part->flags2, F2_DISAPPEARED)) continue;

        if (VEC_EQ(part->pos, part->pos_prev1) && part->state1 == part->state1_prev1) {
            if (!VEC_EQ(part->pos, part->pos_prev1) || part->state1 != part->state1_prev1) {
                // The previous predicate is exactly the same as the negation as the one before it.
                // This is dead code, but it's in the TIMWIN decompilation.
                // TODO - make sure we should remove it.
                for (int i = 0; i < 2; i++) {
                    struct RopeData *rd = part->rope_data[i];
                    if (rd) {
                        rd->ends_pos[0] = rd->ends_pos_prev1[0];
                        rd->ends_pos[1] = rd->ends_pos_prev1[1];
                    }
                }
            }
        } else {
            stub_10a8_36f0(part);
        }
    }

    // OpenTIM - manually reset this field for all parts
    // Rope positions won't update properly unless we do
    EACH_STATIC_THEN_MOVING_PART(part) {
        part->field_0x14 = 0;
    }
}

#if ENABLE_TEST_SUITE
TEST_SUITE(helpers) {
    TEST("swap, ByteVec") {
        struct ByteVec a = { 5, 10 };
        struct ByteVec b = { 35, 255 };

        SWAP(a, b);

        ASSERT_EQ(a.x, 35);
        ASSERT_EQ(a.y, 255);
        ASSERT_EQ(b.x, 5);
        ASSERT_EQ(b.y, 10);

        SWAP(a, b);

        ASSERT_EQ(a.x, 5);
        ASSERT_EQ(a.y, 10);
        ASSERT_EQ(b.x, 35);
        ASSERT_EQ(b.y, 255);
    }
}
#endif
