#include "tim.h"

struct BeltData* belt_data_alloc();
struct RopeData* rope_data_alloc();
void part_alloc_borders(struct Part *part, u16 length);
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

// From Rust
void bob_the_fish_break_bowl(struct Part *part);
void mort_the_mouse_cage_start(struct Part *part);

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

/* Partial from TIMWIN: 1048:06f4 */
void detach_rope_from_part(struct Part *part) {
    // only if there's a rope there
    if (part->rope_data[0]) {
        struct Part *severed_part = part_new(P_ROPE_SEVERED_END);
        insert_part_into_moving_parts(severed_part);
        severed_part->flags1 |= 0x0010;
        severed_part->rope_data[0] = part->rope_data[0];
        severed_part->links_to[0] = part->links_to[0];
        struct Part *links_to = severed_part->links_to[0];

        int i = part_get_rope_link_index(part, links_to);
        if (i != -1) {
            links_to->links_to[i] = severed_part;
        }
        if (part->rope_data[0]->part1 == part) {
            part->rope_data[0]->part1 = severed_part;
            part->rope_data[0]->part1_rope_slot = 0;
            severed_part->pos = part->rope_data[0]->ends_pos[0];
        } else {
            part->rope_data[0]->part2 = severed_part;
            part->rope_data[0]->part2_rope_slot = 0;
            severed_part->pos = part->rope_data[0]->ends_pos[1];
        }
        severed_part->pos_x_hi_precision = severed_part->pos.x * 512;
        severed_part->pos_y_hi_precision = severed_part->pos.y * 512;
        part_set_size_and_pos_render(severed_part);
        part->rope_data[0] = 0;
        part->links_to[0] = 0;
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

/* TIMWIN: 10a8:376e
   Note: I double-checked this for accuracy */
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
int teeter_totter_bounce(struct Part *part) {
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
void teeter_totter_run(struct Part *part) {
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
            teeter_totter_reset(part);
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

/* TIMWIN: 10d0:04fe */
int teeter_totter_rope(struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_mass, s32 p1_force) {
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

/* TIMWIN: 1048:06f4 */
void balloon_run(struct Part *part) {
    if (part->state2 == 0) return;

    part->flags2 |= 0x0040;

    if (part->state1 == 6) {
        stub_10a8_2b6d(part, 3);
        part->flags2 |= F2_DISAPPEARED;
        return;
    }

    if (part->state2 == 1) {
        // popped ballon with a rope attached to it
        detach_rope_from_part(part);
    }
    if (part->state1 == 0) {
        play_sound(0xe);
    }
    part->state1 += 1;
    part_set_size_and_pos_render(part);
}

/* TIMWIN: 1048:082b */
int balloon_rope(struct Part *p1, struct Part *p2, int rope_slot, u16 flags, s16 p1_pass, s32 p1_force) {
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

/* TIMWIN: 1090:1094 */
void search_for_interactions(struct Part *part, enum GetPartsFlags choice, s16 search_x_min, s16 search_x_max, s16 search_y_min, s16 search_y_max) {
    part->interactions = 0;

    for (struct Part *curpart = get_first_part(choice); curpart != 0; curpart = next_part_or_fallback(curpart, choice & CHOOSE_MOVING_PART)) {
        if (part == curpart) continue;
        if (ANY_FLAGS(curpart->flags2, F2_DISAPPEARED)) continue;

        s16 somex = curpart->pos.x + curpart->size.x - part->pos.x;

        if (somex >= search_x_min) {
            s16 field_0x7a = somex >= 0 ? -1 : somex;
            s16 x = curpart->pos.x - (part->pos.x + part->size.x);
            if (x <= search_x_max) {
                if (abs(x) < abs(somex)) {
                    field_0x7a = x <= 0 ? 1 : x;
                }
                s16 somey = curpart->pos.y + curpart->size.y - part->pos.y;
                if (somey >= search_y_min) {
                    s16 field_0x7c = somey >= 0 ? -1 : somey;

                    s16 y = curpart->pos.y - (part->pos.y + part->size.y);
                    if (y <= search_y_max) {
                        if (abs(y) < abs(somey)) {
                            field_0x7c = y <= 0 ? 1 : y;
                        }

                        // Prepare curpart to part->interactions linked list.
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

/* TIMWIN: 1048:14cf */
void pokey_the_cat_run(struct Part *part) {
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
                        search_for_interactions(part, CHOOSE_STATIC_OR_ELSE_MOVING_PART, -240, 0, 0, 0);
                    } else {
                        search_for_interactions(part, CHOOSE_STATIC_OR_ELSE_MOVING_PART, 0, 240, 0, 0);
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
            // Pokey's touching a surface. Put Pokey upright again
            part->flags2 &= ~(F2_FLIP_VERT);
            part->state1 = 0;
        }
    }

    if (part->state1 != part->state1_prev1) {
        part_set_size_and_pos_render(part);
    }
}
