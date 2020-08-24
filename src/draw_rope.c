#include "tim.h"

enum RopeTime {
    PREV2 = 1,
    PREV1 = 2,
    CURRENT = 3
};

static inline s16 approx_hypot(s16 x, s16 y) {
    if (x < y) {
        return x/4 + x/8 + y;
    } else {
        return y/4 + y/8 + x;
    }
}

/* TIMWIN: 10a8:396f */
s16 approximate_hypot_of_rope(struct RopeData *rope_data, enum RopeTime time, int first_or_last) {
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

    switch (time) {
        case PREV2:
        return approx_hypot(abs(a->ends_pos_prev2[i_a].x - b->ends_pos_prev2[i_b].x),
                            abs(a->ends_pos_prev2[i_a].y - b->ends_pos_prev2[i_b].y));
        
        case PREV1:
        return approx_hypot(abs(a->ends_pos_prev1[i_a].x - b->ends_pos_prev1[i_b].x),
                            abs(a->ends_pos_prev1[i_a].y - b->ends_pos_prev1[i_b].y));
        
        default:
        return approx_hypot(abs(a->ends_pos      [i_a].x - b->ends_pos      [i_b].x),
                            abs(a->ends_pos      [i_a].y - b->ends_pos      [i_b].y));
    }
}

/* TIMWIN: 10a8:3b05 */
s16 calculate_rope_sag(struct Part *part, struct RopeData *rope_data, enum RopeTime time) {
    struct Part *nextpart;
    if (part->type == P_PULLEY) {
        nextpart = part->links_to[0];
    } else {
        nextpart = part->links_to[rope_data->part1_rope_slot];
    }

    struct Part *rope_part;

    rope_part = rope_data->rope_or_pulley_part;

    if (rope_data->part1 == part) {
        s16 v;
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

        s16 v;
        switch (time) {
            case PREV2: v = rope_part->extra2_prev2; break;
            case PREV1: v = rope_part->extra2_prev1; break;
            default:    v = rope_part->extra2; break;
        }

        return v - approximate_hypot_of_rope(rope_data, time, 1);
    }
}

#ifdef ENABLE_TEST_SUITE
#if 0
#include <math.h>

void generate_hypot_samples(int n, int max_val) {
    for (int i = 0; i < n; i++) {
        s16 result = -1;
        s16 x;
        s16 y;
        do {
            x = rand() % max_val;
            y = rand() % max_val;
            result = approx_hypot(x, y);
            // some results overflow
        } while (result < 0);

        float c_result = sqrtf((float)x*(float)x + (float)y*(float)y);
        float error = (float)result - c_result;
        printf("ASSERT_EQ(approx_hypot(%5d, %5d), %5d); // Accurate: %8.2f (error = %+.2f%%)\n", x, y, result, c_result, error*100.0f/c_result);
    }
}
#endif
#endif

TEST_SUITE(draw_rope, {
    TEST("approx_hypot", {
        ASSERT_EQ(approx_hypot(0, 0), 0);
        ASSERT_EQ(approx_hypot(256, 256), 352);

        // The largest hypoteneuse of equal arms possible with this function.
        ASSERT_EQ(approx_hypot(23831, 23831), 32766);
    })

    TEST("approx_hypot, random samples", {
        // generate_hypot_samples(16, 64);
        // generate_hypot_samples(8, 512);
        // generate_hypot_samples(8, 32767);
        ASSERT_EQ(approx_hypot(   39,     6),    40); // Accurate:    39.46 (error = +1.37%)
        ASSERT_EQ(approx_hypot(   41,    51),    66); // Accurate:    65.44 (error = +0.86%)
        ASSERT_EQ(approx_hypot(   17,    63),    69); // Accurate:    65.25 (error = +5.74%)
        ASSERT_EQ(approx_hypot(   10,    44),    47); // Accurate:    45.12 (error = +4.16%)
        ASSERT_EQ(approx_hypot(   41,    13),    45); // Accurate:    43.01 (error = +4.62%)
        ASSERT_EQ(approx_hypot(   58,    43),    73); // Accurate:    72.20 (error = +1.11%)
        ASSERT_EQ(approx_hypot(   50,    59),    77); // Accurate:    77.34 (error = -0.44%)
        ASSERT_EQ(approx_hypot(   35,     6),    36); // Accurate:    35.51 (error = +1.38%)
        ASSERT_EQ(approx_hypot(   60,     2),    60); // Accurate:    60.03 (error = -0.06%)
        ASSERT_EQ(approx_hypot(   20,    56),    63); // Accurate:    59.46 (error = +5.95%)
        ASSERT_EQ(approx_hypot(   27,    40),    49); // Accurate:    48.26 (error = +1.53%)
        ASSERT_EQ(approx_hypot(   39,    13),    43); // Accurate:    41.11 (error = +4.60%)
        ASSERT_EQ(approx_hypot(   54,    26),    63); // Accurate:    59.93 (error = +5.12%)
        ASSERT_EQ(approx_hypot(   46,    35),    58); // Accurate:    57.80 (error = +0.34%)
        ASSERT_EQ(approx_hypot(   51,    31),    61); // Accurate:    59.68 (error = +2.21%)
        ASSERT_EQ(approx_hypot(    9,    26),    29); // Accurate:    27.51 (error = +5.40%)

        ASSERT_EQ(approx_hypot(  358,   306),   472); // Accurate:   470.96 (error = +0.22%)
        ASSERT_EQ(approx_hypot(   13,   439),   443); // Accurate:   439.19 (error = +0.87%)
        ASSERT_EQ(approx_hypot(   49,    88),   106); // Accurate:   100.72 (error = +5.24%)
        ASSERT_EQ(approx_hypot(  163,   346),   406); // Accurate:   382.47 (error = +6.15%)
        ASSERT_EQ(approx_hypot(  293,   349),   458); // Accurate:   455.69 (error = +0.51%)
        ASSERT_EQ(approx_hypot(  261,   279),   376); // Accurate:   382.05 (error = -1.58%)
        ASSERT_EQ(approx_hypot(   88,   233),   266); // Accurate:   249.06 (error = +6.80%)
        ASSERT_EQ(approx_hypot(   94,   212),   246); // Accurate:   231.91 (error = +6.08%)
        
        ASSERT_EQ(approx_hypot(10192, 18558), 22380); // Accurate: 21172.54 (error = +5.70%)
        ASSERT_EQ(approx_hypot( 2108, 24418), 25208); // Accurate: 24508.82 (error = +2.85%)
        ASSERT_EQ(approx_hypot(21627,  8866), 24951); // Accurate: 23373.77 (error = +6.75%)
        ASSERT_EQ(approx_hypot(28064,  5736), 30215); // Accurate: 28644.19 (error = +5.48%)
        ASSERT_EQ(approx_hypot(12833, 13754), 18566); // Accurate: 18811.12 (error = -1.30%)
        ASSERT_EQ(approx_hypot(10316,  5954), 12548); // Accurate: 11910.92 (error = +5.35%)
        ASSERT_EQ(approx_hypot(17783,  9170), 21221); // Accurate: 20008.10 (error = +6.06%)
        ASSERT_EQ(approx_hypot(13844, 11637), 18207); // Accurate: 18085.25 (error = +0.67%)
    })
})