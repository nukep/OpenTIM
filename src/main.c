#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "tim.h"

u16 arctan(s32 dx, s32 dy);

/* Partial from TIMWIN: 10a8:1e46 */
void insert_part_into_static_parts(struct Part *part) {
    // UNIMPLEMENTED
}

/* Partial from TIMWIN: 10a8:1e46 */
void insert_part_into_moving_parts(struct Part *part) {
    // UNIMPLEMENTED
}

/* TIMWIN: 10a8:4d2d */
void play_sound(int id) {
    // UNIMPLEMENTED
}

/* TIMWIN: 10a8:0290 */
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
    u16 angle = arctan(p.p1.x - p.p0.x, p.p1.y - p.p0.y);

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
    for (int i = 0; i < part->num_borders-1; i++) {
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
    if (STATIC_PARTS_START && (choice & CHOOSE_STATIC_PART)) {
        return STATIC_PARTS_START;
    }
    if (MOVING_PARTS_START && (choice & CHOOSE_MOVING_PART)) {
        return MOVING_PARTS_START;
    }
    if (PARTS_BIN_START && (choice & CHOOSE_FROM_PARTS_BIN)) {
        return PARTS_BIN_START;
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
        return PARTS_BIN_START;
    }
    return 0;
}

/* TIMWIN: 1078:1402 */
void EXPORT part_free(struct Part *part) {
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

/* TIMWIN: 1078:00f2 */
struct Part* EXPORT part_new(enum PartType type) {
    struct Part *part = malloc(sizeof(struct Part));
    if (!part) {
        goto error;
    }

    memset(part, 0, sizeof(struct Part));
    part->type = type;
    part->flags1 = part_data30_flags1(type);
    part->flags2 = part_data30_flags3(type);
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
size_t EXPORT debug_part_size() {
    return sizeof(struct Part);
}

void stub_10a8_44d5() {
    // UNIMPLEMENTED
    // TODO:
    /*
    if (_g_1108_3be8 == 0) return;

    void *DI = _g_1108_3be8;
    void *SI = *_g_1108_3be8;
    while (SI != 0) {
        DI = SI;
        SI = *SI;
    }
    *DI = _g_1108_3be6;
    _g_1108_3be6 = _g_1108_3be8;
    _g_1108_3be8 = 0;
    */
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

    struct ShortVec *v = part_data31_field_0x1a(part->type);
    if (v) {
        part->size = v[part->state1];
        return;
    }

    struct Data31Field0x14 **vv = part_data31_field_0x14(part->type);
    if (vv) {
        struct Data31Field0x14 *ptr = vv[part->state1];
        part->size = ptr->size;
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

    struct SByteVec *v = part_data31_field_0x18(part->type);
    if (!v) return;

    v += state1;    // pointer arithmetic
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
    if (ANY_FLAGS(part->flags1, 0x0001)) {
        if (part_acceleration(part->type) <= 0) {
            part->pos_y_hi_precision -= 1024;
        } else {
            part->pos_y_hi_precision += 1024;
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
}

/* TIMWIN: 1050:0221 */
// Returns 0 to 3.
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
    return arctan(-(part->pos.x - part->pos_prev1.x), part->pos.y - part->pos_prev1.y);
}

/* TIMWIN: 1050:001e */
void stub_1050_001e() {
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

/* TIMWIN: 1050:00a8 */
void stub_1050_00a8() {
    TMP_X_3a6a = PART_3a6a->pos.x;
    TMP_Y_3a6a = PART_3a6a->pos.y;

    TMP_X_CENTER_3a6a = PART_3a6a->pos.x + PART_3a6a->size_something2.x/2;
    TMP_Y_CENTER_3a6a = PART_3a6a->pos.y + PART_3a6a->size_something2.y/2;

    TMP_X_RIGHT_3a6a = PART_3a6a->pos.x + PART_3a6a->size.x;
    TMP_Y_BOTTOM_3a6a = PART_3a6a->pos.y + PART_3a6a->size.y;
}

/* TIMWIN: 10a8:02dc */
static inline int calculate_line_intersection_helper(s16 a, s16 b, s16 c) {
    if (c < b) {
        // swap
        s16 tmp = b;
        b = c;
        c = tmp;
    }

    if (a < b) return 0;

    return (a - b) <= (c - b);
}

/* TIMWIN: 10a8:00c1 */
// Given two lines "a" and "b", assign the point where the two lines meet to "out".
// Function returns false (0) if there's no intersection within the lines.
bool calculate_line_intersection(const struct Line *a, const struct Line *b, struct ShortVec *out) {
    s16 aDx = a->p0.x - a->p1.y;
    s16 aDy = a->p0.y - a->p1.y;
    s16 bDx = b->p1.x - b->p0.x;
    s16 bDy = b->p1.y - b->p0.y;

    s16 ivar3 = aDy*a->p1.x - aDx*a->p1.y;
    s16 ivar4 = bDy*b->p0.x - bDx*b->p0.y;

    s16 ivar5 = bDy*aDx - bDx*aDy;

    if (ivar5 != 0) {
        out->x = (ivar4*aDx - ivar3*bDx) / ivar5;
        out->y = (ivar4*aDy - ivar3*bDy) / ivar5;
    } else if (bDy*a->p0.x + bDx*a->p0.y == 0) {
        out->x = a->p1.x;
        out->y = a->p1.y;
    } else {
        out->x = 0;
        out->y = 0;
    }

    // "out" is the intersection, assuming the lines are infinite length.
    // We need to check the bounds of the lines.

    if (calculate_line_intersection_helper(out->x, a->p0.x, a->p1.x)) return 0;
    if (calculate_line_intersection_helper(out->x, b->p0.x, b->p1.x)) return 0;
    if (calculate_line_intersection_helper(out->y, a->p0.y, a->p1.y)) return 0;
    if (calculate_line_intersection_helper(out->y, b->p0.y, b->p1.y)) return 0;

    // Return "true" if there's an intersection
    return 1;
}

/* TIMWIN: 1050:00f0 */
int stub_1050_00f0(u16 angle) {
    if (!PART_3a68) return 0;
    if (quadrant_from_angle(angle) != TMP_3a8e) return 0;

    bool flag = 0;

    // Angles are signed!
    s16 new_angle = (s16)angle + 0x2000;
    s16 tmp_3a90_new_angle = (s16)TMP_3a90 + 0x2000;

    if (new_angle < 0 || new_angle > 0x4000 || tmp_3a90_new_angle < 0 || tmp_3a90_new_angle > 0x4000) {
        new_angle = angle + 0xa0000;
        tmp_3a90_new_angle = TMP_3a90 + 0xa0000;

        if (!((new_angle < 0 || new_angle > 0x4000 || tmp_3a90_new_angle < 0 || tmp_3a90_new_angle > 0x4000))) {
            flag = 1;
        }
    } else {
        flag = 1;
    }

    if (flag) {
        if (angle == TMP_3a90) return 1;
        if (new_angle == 0x2000 || tmp_3a90_new_angle == 0x2000) return 1;
        if (new_angle < 0x2000 && tmp_3a90_new_angle < 0x2000) return 1;
        if (new_angle > 0x2000 && tmp_3a90_new_angle > 0x2000) return 1;
    }
    return 0;
}

/* TIMWIN: 1050:025e
   Note: We changed the signature slightly, to pass bounce_field_0x86 directly
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

int stub_1050_0550(bool some_bool) {
    struct BorderPoint* borders_data = PART_3a6a->borders_data;
    s16 b0x = TMP_X_3a6a + borders_data[0].x;
    s16 b0y = TMP_Y_3a6a + borders_data[0].y;
    
    s16 b1x = TMP_X_3a6a + borders_data[1].x;
    s16 b1y = TMP_Y_3a6a + borders_data[1].y;

    s16 b0angle = borders_data[0].normal_angle;

    s16 somex = b0x;
    s16 somey = b0y;

    int border_idx = 1;
    bool found = 0;

    while (borders_data) {
        u16 quad = quadrant_from_angle(b0angle);
        if (TMP_3a92 - b0angle != -0x4000 && (TMP_3a92 - b0angle) + 0x4000 >= 0) {
            struct BorderPoint *borders_data_3a6c = PART_3a6c->borders_data;
            s16 angle_3a6c = borders_data_3a6c->normal_angle;
            borders_data_3a6c += 1;
            int border_3a6c_idx = 1;

            while (borders_data_3a6c) {
                s16 ivar5 = angle_3a6c - b0angle - 0x8000;

                if (ivar5 >= 0 || ivar5 == -0x8000) {
                    ivar5 = borders_data_3a6c->normal_angle - b0angle;
                    if (ivar5 == -0x8000 || ivar5 - 0x8000 < 0) {
                        if (TMP_X_DELTA_3a6c != 0 || TMP_Y_DELTA_3a6c != 0) {
                            struct Line line1, line2;
                            struct ShortVec point;

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
                            if (calculate_line_intersection(&line1, &line2, &point) && !VEC_EQ(point, line2.p1)) {
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
                                    s16 local_20 = line1.p1.x;
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
                                stub_1050_001e();

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
int should_parts_skip_collision(enum PartType a, enum PartType b) {
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

    EACH_INTERACION(bucket, curpart, {
        if (curpart == contains) {
            return 1;
        }
    })
    return 0;
}

/* TIMWIN: 1050:08fe */
int stub_1050_08fe(bool some_bool) {
    // UNIMPLEMENTED
}

/* TIMWIN: 1050:02db */
int stub_1050_02db(struct Part *part) {
    PART_3a6c = part;
    if (!part->borders_data) return 0;

    PART_3a68 = part->bounce_part;
    if (part->bounce_part) {
        TMP_3a90 = part->bounce_angle;
        TMP_3a8e = quadrant_from_angle(part->bounce_angle);
    }

    part->bounce_field_0x86[0] = 0;
    part->bounce_field_0x86[1] = 0;

    TMP_3a92 = part_get_movement_delta_angle(part);

    // Changes:
    // TMP_X2_3a6c, TMP_Y2_3a6c, TMP_X_CENTER_3a6c, TMP_Y_CENTER_3a6c,
    // TMP_X_DELTA_3a6c, TMP_Y_DELTA_3a6c, TMP_X_LEFTMOST_3a6c, TMP_Y_TOPMOST_3a6c, TMP_X_RIGHT_3a6c, TMP_Y_BOTTOM_3a6c
    stub_1050_001e();
    
    bool has_found_angle = 0;

    if (part->bounce_part && !bucket_contains(part, part->bounce_part)) {
        PART_3a6a = PART_3a68;
        if (PART_3a68->borders_data && NO_FLAGS(PART_3a68->flags2, 0x2000)) {
            // Changes: TMP_X_3a6a, TMP_Y_3a6a, TMP_X_CENTER_3a6a, TMP_Y_CENTER_3a6a, TMP_X_RIGHT_3a6a, TMP_Y_BOTTOM_3a6a
            stub_1050_00a8();

            if (TMP_X_3a6a < TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c < TMP_X_RIGHT_3a6a
                && TMP_Y_3a6a < TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c < TMP_Y_BOTTOM_3a6a) {
                if (stub_1050_0550(0) != 0) {
                    has_found_angle = 1;
                    TMP_3a92 = part_get_movement_delta_angle(PART_3a6c);
                }
            }
            // unsure about side-effects from stub_1050_0550
            // which is why i haven't merged the below if statement with the above one
            if (TMP_X_3a6a < TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c < TMP_X_RIGHT_3a6a
                && TMP_Y_3a6a < TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c < TMP_Y_BOTTOM_3a6a) {
                if (stub_1050_08fe(0) != 0) {
                    has_found_angle = 1;
                    TMP_3a92 = part_get_movement_delta_angle(PART_3a6c);
                }
            }
        }

        EACH_STATIC_THEN_MOVING_PART(curpart, {
            PART_3a6a = curpart;
            if (!bucket_contains(PART_3a6c, curpart)) {
                if (PART_3a6c != curpart && PART_3a68 != curpart) {
                    if (curpart->borders_data) {
                        if (NO_FLAGS(curpart->flags2, 0x2000)) {
                            if (should_parts_skip_collision(PART_3a6c->type, curpart->type) == 0) {
                                // Changes: TMP_X_3a6a, TMP_Y_3a6a, TMP_X_CENTER_3a6a, TMP_Y_CENTER_3a6a, TMP_X_RIGHT_3a6a, TMP_Y_BOTTOM_3a6a
                                stub_1050_00a8();

                                if (TMP_X_3a6a < TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c < TMP_X_RIGHT_3a6a
                                    && TMP_Y_3a6a < TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c < TMP_Y_BOTTOM_3a6a) {
                                    if (stub_1050_0550(0) != 0) {
                                        has_found_angle = 1;
                                        TMP_3a92 = part_get_movement_delta_angle(PART_3a6c);
                                    }
                                }

                                // unsure about side-effects from stub_1050_0550
                                // which is why i haven't merged the below if statement with the above one
                                if (TMP_X_3a6a <= TMP_X_RIGHT_3a6c && TMP_X_LEFTMOST_3a6c <= TMP_X_RIGHT_3a6a
                                    && TMP_Y_3a6a <= TMP_Y_BOTTOM_3a6c && TMP_Y_TOPMOST_3a6c <= TMP_Y_BOTTOM_3a6a) {
                                    if (stub_1050_08fe(0) != 0) {
                                        has_found_angle = 1;
                                        TMP_3a92 = part_get_movement_delta_angle(PART_3a6c);
                                    }
                                }

                            }
                        }
                    }
                }
            }
        })
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
    return 0;
}

/* TIMWIN: 10a8:3cc1 */
int stub_10a8_3cc1(struct Part *part) {
    // UNIMPLEMENTED
}

/* TIMWIN: 1080:1777 */
void stub_1080_1777(struct Part *part) {
    if (ANY_FLAGS(part->flags2, 0x2000)) return;

    if (part->type != P_TEAPOT) {
        part_run(part);
    }

    adjust_part_position(part);
    part->flags1 &= 0xfff0;

    stub_1050_02db(part);

    if (part->rope_data[0]) {
        int v = stub_10a8_3cc1(part);
        if (v == 0) {
            struct Part *saved_84 = part->bounce_part;
            byte saved_86_0 = part->bounce_field_0x86[0];
            byte saved_86_1 = part->bounce_field_0x86[1];

            stub_1050_02db(part);

            if (part->bounce_part == 0) {
                part->bounce_part = saved_84;
                part->bounce_field_0x86[0] = saved_86_0;
                part->bounce_field_0x86[1] = saved_86_1;
            }
        } else {
            part->flags1 &= 0xfff0;
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

    EACH_MOVING_PART(curpart, {
        if (bucket == curpart) continue;
        if (ANY_FLAGS(curpart->flags2, 0x2000)) continue;
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
    });
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
    EACH_INTERACION(bucket, curpart, {
        bucket_add_mass(bucket, curpart);
    })
}

/* TIMWIN: 1090:15c8 */
void bucket_move_contained(struct Part *bucket) {
    if (bucket->type != P_BUCKET) return;

    s16 dx = bucket->pos.x - bucket->pos_prev1.x;
    s16 dy = bucket->pos.y - bucket->pos_prev1.y;
    if ((dx != 0) || (dy != 0)) {
        // the bucket moved
        EACH_INTERACION(bucket, part, {
            part->pos.x += dx;
            part->pos.y += dy;
            part_set_size_and_pos_render(part);
            part->pos_x_hi_precision = part->pos.x * 512;
            part->pos_y_hi_precision = part->pos.y * 512;
        })
    }
}

/* TIMWIN: 1090:0809 */
void stub_1090_0809(struct Part *part) {
    // UNIMPLEMENTED
}

/* TIMWIN: 1090:0644 */
void stub_1090_0644(struct Part *part) {
    // UNIMPLEMENTED
}

/* TIMWIN: 1090:033f */
void stub_1090_033f(struct Part *part) {
    // UNIMPLEMENTED
}

/* TIMWIN: 10a8:21cb */
void stub_10a8_21cb(struct Part *part, int c) {
    // UNIMPLEMENTED
}
/* TIMWIN: 10a8:280a */
void stub_10a8_280a(struct Part *part, int c) {
    // UNIMPLEMENTED
}
/* TIMWIN: 10a8:2b6d */
void stub_10a8_2b6d(struct Part *part, int c) {
    // UNIMPLEMENTED
}

/* TIMWIN: 10a8:36f0 */
void stub_10a8_36f0(struct Part *part) {
    stub_10a8_21cb(part, 1);
    stub_10a8_280a(part, 1);
    if (part->type != P_ROPE_SEVERED_END) {
        stub_10a8_2b6d(part, 1);
    }
}

/* TIMWIN: 1080:1464 */
void advance_parts() {
    struct Part *part;

    EACH_STATIC_PART(part, {
        part->flags2 &= 0xf9bf;
    })

    for (struct Llama *p = LLAMA; p != 0; p = p->next) {
        part = p->part;
        if (NO_FLAGS(part->flags2, 0x0040)) {
            part_run(part);
        }
    }

    stub_10a8_44d5();

    EACH_STATIC_PART(part, {
        if (ANY_FLAGS(part->flags2, 0x0800) && NO_FLAGS(part->flags2, 0x2000 | 0x0040)) {
            part_run(part);
        }
    })

    EACH_STATIC_PART(part, {
        if (part->type == P_GEAR && NO_FLAGS(part->flags2, 0x2000 | 0x0040)) {
            part_run(part);
        }
    })

    EACH_STATIC_PART(part, {
        if (NO_FLAGS(part->flags2, 0x2840)) {
            part_run(part);
        }
    })

    EACH_MOVING_PART(part, {
        if (part->type == P_TEAPOT) {
            part_run(part);
        }
    })

    EACH_MOVING_PART(part, {
        if (NO_FLAGS(part->flags2, 0x2000)) {
            part_update_vel_and_force(part);
            part->force = (s32)(abs(part->vel_hi_precision.x) + abs(part->vel_hi_precision.y)) * (s32)part->mass;
        }
        part->mass = part_mass(part->type);
        part->flags3 &= 0xffef;
    })

    EACH_MOVING_PART(part, {
        if (part->type != P_BUCKET) {
            stub_1080_1777(part);
        }
    })

    EACH_MOVING_PART(part, {
        if (part->type == P_BUCKET) {
            bucket_handle_contained_parts(part);
            bucket_add_mass_of_contained(part);
        }
    })

    EACH_MOVING_PART(part, {
        if (part->type == P_BUCKET) {
            bucket_handle_contained_parts(part);
            stub_1080_1777(part);
        }
    })

    EACH_MOVING_PART(part, {
        if (part->type == P_BUCKET) {
            bucket_handle_contained_parts(part);
            bucket_move_contained(part);
        }
    })

    EACH_MOVING_PART(part, {
        if (NO_FLAGS(part->flags1, 0x0008) && NO_FLAGS(part->flags2, 0x2000)) {
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
                        stub_1090_0644(part);
                    } else {
                        stub_1090_033f(part);
                    }
                }
            }
        }
    })

    EACH_STATIC_THEN_MOVING_PART(part, {
        if (ANY_FLAGS(part->flags2, 0x2000)) continue;

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
    })
}