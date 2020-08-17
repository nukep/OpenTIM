#include "tim.h"

enum RopeTime {
    PREV2 = 1,
    PREV1 = 2,
    CURRENT = 3
};

/* TIMWIN: 10a8:396f */
sint approximate_hypot_of_rope(struct RopeData *rope_data, enum RopeTime time, int first_or_last) {
    struct RopeData *a, *b;
    int i_a, i_b;

    if (first_or_last == 0) {
        struct Part *v = rope_data->part1->links_to[rope_data->part1_rope_slot];

        if (rope_data->part2 == v) {
            // o ---- o   Part1 -> Part2
            a = rope_data; i_a = 0;
            b = rope_data; i_b = 1;
        } else {
            // o ---- u   Part1 -> Pulley (into left side)
            a = rope_data;       i_a = 0;
            b = v->rope_data[0]; i_b = 0;
        }
    } else {
        // From the last part
        if (rope_data->part2 == 0) {
            return 0;
        }
        
        struct Part *v = rope_data->part2->links_to[rope_data->part2_rope_slot];

        if (rope_data->part1 == v) {
            // o ---- o   Part2 -> Part1
            a = rope_data; i_a = 1;
            b = rope_data; i_b = 0;
        } else {
            // o ---- u   Part2 -> Pulley (into right side)
            a = rope_data;       i_a = 1;
            b = v->rope_data[0]; i_b = 1;
        }
    }

    if (b == 0) {
        return 0;
    }

    sint dx, dy;

    switch (time) {
        case PREV2: dx = abs(a->ends_pos_prev2[i_a].x - b->ends_pos_prev2[i_b].x);
                    dy = abs(a->ends_pos_prev2[i_a].y - b->ends_pos_prev2[i_b].y);
                    break;
        case PREV1: dx = abs(a->ends_pos_prev1[i_a].x - b->ends_pos_prev1[i_b].x);
                    dy = abs(a->ends_pos_prev1[i_a].y - b->ends_pos_prev1[i_b].y);
                    break;
        default:    dx = abs(a->ends_pos      [i_a].x - b->ends_pos      [i_b].x);
                    dy = abs(a->ends_pos      [i_a].y - b->ends_pos      [i_b].y);
                    break;
    }

    if (dx < dy) {
        return dx/4 + dx/8 + dy;
    } else {
        return dy/4 + dy/8 + dx;
    }
}

/* TIMWIN: 10a8:3b05 */
sint calculate_rope_sag(struct Part *part, struct RopeData *rope_data, enum RopeTime time) {
    struct Part *nextpart;
    if (part->type == P_PULLEY) {
        nextpart = part->links_to[0];
    } else {
        nextpart = part->links_to[rope_data->part1_rope_slot];
    }

    struct Part *rope_part;

    rope_part = rope_data->rope_or_pulley_part;

    if (rope_data->part1 == part) {
        sint v;
        switch (time) {
            case PREV2: v = rope_part->extra1_prev2; break;
            case PREV1: v = rope_part->extra1_prev1; break;
            default:    v = rope_part->extra1; break;
        }
        return v - approximate_hypot_of_rope(rope_data, time, 0);
    } else {
        if (nextpart == 0) {
            return 0;
        }
        if (rope_data->part2 != nextpart) {
            return 0;
        }

        sint v;
        switch (time) {
            case PREV2: v = rope_part->extra2_prev2; break;
            case PREV1: v = rope_part->extra2_prev1; break;
            default:    v = rope_part->extra2; break;
        }

        return v - approximate_hypot_of_rope(rope_data, time, 1);
    }
}