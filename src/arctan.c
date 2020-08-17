#include "tim.h"

// From TIMWIN - Segment 29
static const int ARCTAN_LOOKUP[512] = {
    0, 1, 2, 3, 5, 6, 7, 8, 10, 11, 12, 14, 15, 16, 17, 19, 20, 21, 22, 24, 25, 26, 27, 29, 30, 31, 33, 34, 35, 36, 38, 39, 40, 41, 43, 44, 45, 47, 48, 49, 50, 52, 53, 54, 55, 57, 58, 59, 60, 62, 63, 64, 65, 67, 68, 69, 71, 72, 73, 74, 76, 77, 78, 79, 81, 82, 83, 84, 86, 87, 88, 89, 91, 92, 93, 94, 96, 97, 98, 99, 101, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 117, 118, 119, 120, 122, 123, 124, 125, 126, 128, 129, 130, 131, 133, 134, 135, 136, 137, 139, 140, 141, 142, 144, 145, 146, 147, 148, 150, 151, 152, 153, 154, 156, 157, 158, 159, 160, 162, 163, 164, 165, 166, 168, 169, 170, 171, 172, 174, 175, 176, 177, 178, 179, 181, 182, 183, 184, 185, 186, 188, 189, 190, 191, 192, 193, 195, 196, 197, 198, 199, 200, 202, 203, 204, 205, 206, 207, 208, 210, 211, 212, 213, 214, 215, 216, 218, 219, 220, 221, 222, 223, 224, 226, 227, 228, 229, 230, 231, 232, 233, 234, 236, 237, 238, 239, 240, 241, 242, 243, 244, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 375, 376, 377, 378, 379, 380, 381, 382, 383, 383, 384, 385, 386, 387, 388, 389, 390, 390, 391, 392, 393, 394, 395, 396, 396, 397, 398, 399, 400, 401, 402, 402, 403, 404, 405, 406, 407, 407, 408, 409, 410, 411, 412, 412, 413, 414, 415, 416, 417, 417, 418, 419, 420, 421, 421, 422, 423, 424, 425, 425, 426, 427, 428, 429, 429, 430, 431, 432, 433, 433, 434, 435, 436, 437, 437, 438, 439, 440, 440, 441, 442, 443, 444, 444, 445, 446, 447, 447, 448, 449, 450, 450, 451, 452, 453, 453, 454, 455, 456, 456, 457, 458, 459, 459, 460, 461, 462, 462, 463, 464, 464, 465, 466, 467, 467, 468, 469, 470, 470, 471, 472, 472, 473, 474, 475, 475, 476, 477, 477, 478, 479, 479, 480, 481, 482, 482, 483, 484, 484, 485, 486, 486, 487, 488, 488, 489, 490, 490, 491, 492, 493, 493, 494, 495, 495, 496, 497, 497, 498, 499, 499, 500, 500, 501, 502, 502, 503, 504, 504, 505, 506, 506, 507, 508, 508, 509, 510, 510, 511
};

/* TIMWIN: 1040:1442 */
uint EXPORT arctan(slong dx, slong dy) {
    bool negx = dx < 0;
    bool negy = dy < 0;
    dx = abs(dx);
    dy = abs(dy);

    uint val;

    if (dx == 0) {
        val = 0x400;
    } else if (dy == 0) {
        val = 0;
    } else if (dx == dy) {
        val = 0x200;
    } else if (dx < dy) {
        val = 0x400 - ARCTAN_LOOKUP[(dx * 512) / dy];
    } else {
        val = ARCTAN_LOOKUP[(dy * 512) / dx];
    }

    if (negx) {
        val = 0x800 - val;
    }
    if (negy) {
        val = 0x1000 - val;
    }

    return (val - 0x400) * 16;
}
