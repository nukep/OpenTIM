#include "tim.h"

// From TIMWIN - Segment 29
static const int ARCTAN_LOOKUP[512] = {
    0, 1, 2, 3, 5, 6, 7, 8, 10, 11, 12, 14, 15, 16, 17, 19, 20, 21, 22, 24, 25, 26, 27, 29, 30, 31, 33, 34, 35, 36, 38, 39, 40, 41, 43, 44, 45, 47, 48, 49, 50, 52, 53, 54, 55, 57, 58, 59, 60, 62, 63, 64, 65, 67, 68, 69, 71, 72, 73, 74, 76, 77, 78, 79, 81, 82, 83, 84, 86, 87, 88, 89, 91, 92, 93, 94, 96, 97, 98, 99, 101, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 117, 118, 119, 120, 122, 123, 124, 125, 126, 128, 129, 130, 131, 133, 134, 135, 136, 137, 139, 140, 141, 142, 144, 145, 146, 147, 148, 150, 151, 152, 153, 154, 156, 157, 158, 159, 160, 162, 163, 164, 165, 166, 168, 169, 170, 171, 172, 174, 175, 176, 177, 178, 179, 181, 182, 183, 184, 185, 186, 188, 189, 190, 191, 192, 193, 195, 196, 197, 198, 199, 200, 202, 203, 204, 205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 216, 218, 219, 220, 221, 222, 223, 224, 226, 227, 228, 229, 230, 231, 232, 233, 234, 236, 237, 238, 239, 240, 241, 242, 243, 244, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 375, 376, 377, 378, 379, 380, 381, 382, 383, 383, 384, 385, 386, 387, 388, 389, 390, 390, 391, 392, 393, 394, 395, 396, 396, 397, 398, 399, 400, 401, 402, 402, 403, 404, 405, 406, 407, 407, 408, 409, 410, 411, 412, 412, 413, 414, 415, 416, 417, 417, 418, 419, 420, 421, 421, 422, 423, 424, 425, 425, 426, 427, 428, 429, 429, 430, 431, 432, 433, 433, 434, 435, 436, 437, 437, 438, 439, 440, 440, 441, 442, 443, 444, 444, 445, 446, 447, 447, 448, 449, 450, 450, 451, 452, 453, 453, 454, 455, 456, 456, 457, 458, 459, 459, 460, 461, 462, 462, 463, 464, 464, 465, 466, 467, 467, 468, 469, 470, 470, 471, 472, 472, 473, 474, 475, 475, 476, 477, 477, 478, 479, 479, 480, 481, 482, 482, 483, 484, 484, 485, 486, 486, 487, 488, 488, 489, 490, 490, 491, 492, 493, 493, 494, 495, 495, 496, 497, 497, 498, 499, 499, 500, 500, 501, 502, 502, 503, 504, 504, 505, 506, 506, 507, 508, 508, 509, 510, 510, 511
};

/* TIMWIN: 1040:1442

  Get the arc tangent of the coordinates.
  Returns an angle from 0 to 65535 inclusive. Angles are clockwise.
  Down  (0, +) returns 0.
  Left  (-, 0) returns 0x4000 (25% of 65536)
  Up    (0, -) returns 0x8000 (50% of 65536)
  Right (+, 0) returns 0xC000 (75% of 65536)

  dx and dy range from -0x400000 to +0x3FFFFF (-4,194,304 to 4,194,303)
  */
u16 arctan(s32 dx, s32 dy) {
    bool negx = dx < 0;
    bool negy = dy < 0;
    dx = abs(dx);
    dy = abs(dy);

    u16 val;

    if (dx == 0) {
        val = 0x400;
    } else if (dy == 0) {
        val = 0;
    } else if (dx == dy) {
        val = 0x0200;
    } else if (dx < dy) {
        val = 0x0400 - ARCTAN_LOOKUP[(dx * 512) / dy];
    } else {
        val = ARCTAN_LOOKUP[(dy * 512) / dx];
    }

    if (negx) {
        val = 0x0800 - val;
    }
    if (negy) {
        val = 0x1000 - val;
    }

    return (val - 0x400) * 16;
}

#if ENABLE_TEST_SUITE

// unit-test helper
#define T(x, y, expected) ASSERT_EQ(arctan(x*i, y*i), expected)

// #include <math.h>

TEST_SUITE(arctan) {
    TEST("0, 0") {
        ASSERT_EQ(arctan(0, 0), 0);
    }

    TEST("Straight and diagonal lines") {
        for (s32 i = 1; i <= 10000; i++) {
            T( 0,  1, 0x0000);
            T(-1,  1, 0x2000);
            T(-1,  0, 0x4000);
            T(-1, -1, 0x6000);
            T( 0, -1, 0x8000);
            T( 1, -1, 0xA000);
            T( 1,  0, 0xC000);
            T( 1,  1, 0xE000);
        }
    }

    TEST("30-degree intervals") {
        for (s32 i = 1; i <= 10000; i++) {
            T(-1,  2, 0x12E0);
            T(-2,  1, 0x2D20);
            T(-2, -1, 0x52E0);
            T(-1, -2, 0x6D20);
            T( 1, -2, 0x92E0);
            T( 2, -1, 0xAD20);
            T( 2,  1, 0xD2E0);
            T( 1,  2, 0xED20);
        }
    }

    TEST("Sample of random coordinates") {
        // arctan (ours) and atan2 (C) are both clockwise, but their "0" is different.
        // arctan(): x=0,y=1 (down) is 0 degrees
        // atan2():  x=1,y=0 (right) is 0 degrees

        // for (int i = 0; i < 30; i++) {
        //     s16 x = rand() & 0xffff;
        //     s16 y = rand() & 0xffff;

        //     u16 result = arctan(x, y);

        //     float c_result = (atan2f(y, x) / (M_PI*2.0f)) - 0.25f;
        //     while (c_result < 0) {
        //         c_result += 1.0f;
        //     }
        //     c_result *= 65536.0f;
        //     c_result = roundf(c_result);

        //     float error = (float)result - c_result;

        //     printf("ASSERT_EQ(arctan(%6d, %6d), %6d); // Accurate: %6.0f (error = %+3.0f, %+.02f%%, %+.2f deg)\n", x, y, result, c_result, error, error*100.0f/c_result, error*360.0f/65536.0f);
        // }

        ASSERT_EQ(arctan( 17767,   9158),  54096); // Accurate:  54116 (error = -20, -0.04%, -0.11 deg)
        ASSERT_EQ(arctan(-26519,  18547),  10032); // Accurate:  10018 (error = +14, +0.14%, +0.08 deg)
        ASSERT_EQ(arctan( -9135,  23807),   3808); // Accurate:   3822 (error = -14, -0.37%, -0.08 deg)
        ASSERT_EQ(arctan(-27574,  22764),   9200); // Accurate:   9186 (error = +14, +0.15%, +0.08 deg)
        ASSERT_EQ(arctan(  7977,  31949),  63008); // Accurate:  62984 (error = +24, +0.04%, +0.13 deg)
        ASSERT_EQ(arctan( 22714, -10325),  44720); // Accurate:  44702 (error = +18, +0.04%, +0.10 deg)
        ASSERT_EQ(arctan( 16882,   7931),  53712); // Accurate:  53733 (error = -21, -0.04%, -0.12 deg)
        ASSERT_EQ(arctan(-22045,  -7866),  19936); // Accurate:  19959 (error = -23, -0.12%, -0.13 deg)
        ASSERT_EQ(arctan(   124,  25282),  65504); // Accurate:  65485 (error = +19, +0.03%, +0.10 deg)
        ASSERT_EQ(arctan(  2132,  10232),  63408); // Accurate:  63393 (error = +15, +0.02%, +0.08 deg)
        ASSERT_EQ(arctan(  8987,  -5656),  43312); // Accurate:  43293 (error = +19, +0.04%, +0.10 deg)
        ASSERT_EQ(arctan(-12825,  17293),   6640); // Accurate:   6656 (error = -16, -0.24%, -0.09 deg)
        ASSERT_EQ(arctan(  3958,   9562),  61472); // Accurate:  61443 (error = +29, +0.05%, +0.16 deg)
        ASSERT_EQ(arctan( -1746,  29283),    608); // Accurate:    621 (error = -13, -2.09%, -0.07 deg)
        ASSERT_EQ(arctan(-15821, -10337),  22400); // Accurate:  22421 (error = -21, -0.09%, -0.12 deg)
        ASSERT_EQ(arctan(-15159,   1946),  15072); // Accurate:  15052 (error = +20, +0.13%, +0.11 deg)
        ASSERT_EQ(arctan( -1178,  23858),    496); // Accurate:    515 (error = -19, -3.69%, -0.10 deg)
        ASSERT_EQ(arctan( 20493, -10313),  44304); // Accurate:  44289 (error = +15, +0.03%, +0.08 deg)
        ASSERT_EQ(arctan(-17871,  -7080),  20288); // Accurate:  20318 (error = -30, -0.15%, -0.16 deg)
        ASSERT_EQ(arctan( 12451,  -9894),  42160); // Accurate:  42148 (error = +12, +0.03%, +0.07 deg)
        ASSERT_EQ(arctan( 24869, -30371),  39920); // Accurate:  39925 (error =  -5, -0.01%, -0.03 deg)
        ASSERT_EQ(arctan(-20219, -23785),  25424); // Accurate:  25419 (error =  +5, +0.02%, +0.03 deg)
        ASSERT_EQ(arctan(-22440,  23273),   7984); // Accurate:   8002 (error = -18, -0.22%, -0.10 deg)
        ASSERT_EQ(arctan(-31650, -22316),  22784); // Accurate:  22790 (error =  -6, -0.03%, -0.03 deg)
        ASSERT_EQ(arctan(-16981, -29518),  27344); // Accurate:  27323 (error = +21, +0.08%, +0.12 deg)
        ASSERT_EQ(arctan(-12083,  -7994),  22464); // Accurate:  22480 (error = -16, -0.07%, -0.09 deg)
        ASSERT_EQ(arctan( 30363, -24908),  42000); // Accurate:  41986 (error = +14, +0.03%, +0.08 deg)
        ASSERT_EQ(arctan(  9300, -31215),  35776); // Accurate:  35788 (error = -12, -0.03%, -0.07 deg)
        ASSERT_EQ(arctan(-15346,   7554),  11616); // Accurate:  11613 (error =  +3, +0.03%, +0.02 deg)
        ASSERT_EQ(arctan( -1932, -31167),  32144); // Accurate:  32122 (error = +22, +0.07%, +0.12 deg)
    }
}

#undef T
#endif