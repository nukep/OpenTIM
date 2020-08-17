#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "tim.h"

uint arctan(slong dx, slong dy);

struct FourPoints {
    struct ShortVec p0, p1;
};

/* TIMWIN: 10a8:0290 */
void four_points_adjust_p1_by_one(struct FourPoints *points) {
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
    struct FourPoints p = { { a->x, a->y }, { b->x, b->y } };
    four_points_adjust_p1_by_one(&p);
    uint angle = arctan(p.p1.x - p.p0.x, p.p1.y - p.p0.y);

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

/* TIMWIN: 10d0:0114 */
void d31_reset_03(struct Part *part) {
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

    part->rope_loc_1.x = dat_2e9e[part->state1][0];
    part->rope_loc_1.y = dat_2e9e[part->state1][1];
    part->rope_loc_2.x = dat_2eaa[part->state1][0];
    part->rope_loc_2.y = dat_2eaa[part->state1][1];

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
int d31_create_03(struct Part *part) {
    part->flags1 |= 0x0400;
    part->flags2 |= 0x000c;
    part->borders_data = malloc(part->num_borders * 4);
    if (!part->borders_data) {
        return 1;
    }

    d31_reset_03(part);
    return 0;
}

/* TIMWIN: 1058:1591 */
void d1_reset_37(struct Part *part) {
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
    part->rope_loc_1.x = part->size.x / 2;
    part->rope_loc_1.y = 0;
}

/* TIMWIN: 1058:1591 */
void d1_reset_39(struct Part *part) {
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
    part->rope_loc_1.x = part->size.x / 2;
    part->rope_loc_1.y = 0;
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
void stub_10a8_252b(struct Part *part) {
    part->pos_render = part->pos;
    uint state1 = part->state1;
    uint flags2 = part->flags2;
    part_set_size(part);

    struct SByteVec *v = part_data31_field_0x18(part->type);
    if (v) {
        v += state1;    // pointer arithmetic
        if (ANY_FLAGS(flags2, F2_FLIP_HORZ)) {
            part->pos_render.x += part->size_something.x - v->x - part->size.x;
        } else {
            part->pos_render.x += v->x;
        }

        if (ANY_FLAGS(flags2, F2_FLIP_VERT)) {
            part->pos_render.y += part->size_something.y - v->y - part->size.y;
            return;
        }
        part->pos_render.y += v->y;
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

    stub_10a8_252b(part);
}

/* TIMWIN: 1090:012d */
void part_clamp_to_terminal_velocity(struct Part *part) {
    const sint tv = part_terminal_velocity(part->type);

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

/* TIMWIN: 1050:02db */
void stub_1050_02db(struct Part *part) {
    // UNIMPLEMENTED
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

        sint curpart_x_center = curpart->pos_prev1.x + curpart->size.x/2;
        sint curpart_y_bottom = curpart->pos_prev1.y + curpart->size.y;
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
    slong sum = bucket->mass + part->mass;
    if (sum > 32000) {
        sum = 32000;
    }
    bucket->mass = (sint)sum;
}

/* TIMWIN: 10a8:45c6 */
void bucket_add_mass_of_contained(struct Part *bucket) {
    for (struct Part *p = bucket->interactions; p != 0; p = p->interactions) {
        bucket_add_mass(bucket, p);
    }
}

/* TIMWIN: 1090:15c8 */
void bucket_move_contained(struct Part *bucket) {
    if (bucket->type != P_BUCKET) return;

    sint dx = bucket->pos.x - bucket->pos_prev1.x;
    sint dy = bucket->pos.y - bucket->pos_prev1.y;
    if ((dx != 0) || (dy != 0)) {
        // the bucket moved
        for (struct Part *part = bucket->interactions; part != 0; part = part->interactions) {
            part->pos.x += dx;
            part->pos.y += dy;
            stub_10a8_252b(part);
            part->pos_x_hi_precision = part->pos.x * 512;
            part->pos_y_hi_precision = part->pos.y * 512;
        }
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
            part->force = (slong)(abs(part->vel_hi_precision.x) + abs(part->vel_hi_precision.y)) * (slong)part->mass;
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