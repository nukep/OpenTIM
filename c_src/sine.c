#include "tim.h"

/* TIMWIN: 10c8:1943 */
static const s16 SINE_LOOKUP_TABLE[2049] = {
     16384,  16383,  16383,  16383,  16383,  16383,  16383,  16383,  16382,  16382,  16382,  16381,  16381,  16380,  16380,  16379,
     16379,  16378,  16377,  16377,  16376,  16375,  16374,  16373,  16372,  16371,  16370,  16369,  16368,  16367,  16366,  16365,
     16364,  16363,  16361,  16360,  16359,  16357,  16356,  16354,  16353,  16351,  16350,  16348,  16346,  16344,  16343,  16341,
     16339,  16337,  16335,  16333,  16331,  16329,  16327,  16325,  16323,  16321,  16319,  16316,  16314,  16312,  16309,  16307,
     16305,  16302,  16300,  16297,  16294,  16292,  16289,  16286,  16284,  16281,  16278,  16275,  16272,  16269,  16266,  16263,
     16260,  16257,  16254,  16251,  16248,  16244,  16241,  16238,  16234,  16231,  16228,  16224,  16221,  16217,  16213,  16210,
     16206,  16202,  16199,  16195,  16191,  16187,  16183,  16179,  16175,  16171,  16167,  16163,  16159,  16155,  16151,  16147,
     16142,  16138,  16134,  16129,  16125,  16120,  16116,  16111,  16107,  16102,  16097,  16093,  16088,  16083,  16078,  16074,
     16069,  16064,  16059,  16054,  16049,  16044,  16039,  16033,  16028,  16023,  16018,  16012,  16007,  16002,  15996,  15991,
     15985,  15980,  15974,  15969,  15963,  15957,  15952,  15946,  15940,  15934,  15928,  15923,  15917,  15911,  15905,  15899,
     15892,  15886,  15880,  15874,  15868,  15861,  15855,  15849,  15842,  15836,  15830,  15823,  15817,  15810,  15803,  15797,
     15790,  15783,  15777,  15770,  15763,  15756,  15749,  15742,  15735,  15728,  15721,  15714,  15707,  15700,  15693,  15685,
     15678,  15671,  15663,  15656,  15649,  15641,  15634,  15626,  15618,  15611,  15603,  15596,  15588,  15580,  15572,  15564,
     15557,  15549,  15541,  15533,  15525,  15517,  15509,  15500,  15492,  15484,  15476,  15468,  15459,  15451,  15443,  15434,
     15426,  15417,  15409,  15400,  15392,  15383,  15374,  15366,  15357,  15348,  15339,  15330,  15322,  15313,  15304,  15295,
     15286,  15277,  15267,  15258,  15249,  15240,  15231,  15221,  15212,  15203,  15193,  15184,  15175,  15165,  15156,  15146,
     15136,  15127,  15117,  15107,  15098,  15088,  15078,  15068,  15058,  15048,  15038,  15028,  15018,  15008,  14998,  14988,
     14978,  14968,  14957,  14947,  14937,  14927,  14916,  14906,  14895,  14885,  14874,  14864,  14853,  14843,  14832,  14821,
     14810,  14800,  14789,  14778,  14767,  14756,  14745,  14734,  14723,  14712,  14701,  14690,  14679,  14668,  14657,  14645,
     14634,  14623,  14611,  14600,  14589,  14577,  14566,  14554,  14543,  14531,  14519,  14508,  14496,  14484,  14473,  14461,
     14449,  14437,  14425,  14413,  14401,  14389,  14377,  14365,  14353,  14341,  14329,  14317,  14304,  14292,  14280,  14267,
     14255,  14243,  14230,  14218,  14205,  14193,  14180,  14167,  14155,  14142,  14129,  14117,  14104,  14091,  14078,  14065,
     14053,  14040,  14027,  14014,  14001,  13988,  13974,  13961,  13948,  13935,  13922,  13908,  13895,  13882,  13868,  13855,
     13842,  13828,  13815,  13801,  13788,  13774,  13760,  13747,  13733,  13719,  13705,  13692,  13678,  13664,  13650,  13636,
     13622,  13608,  13594,  13580,  13566,  13552,  13538,  13524,  13510,  13495,  13481,  13467,  13452,  13438,  13424,  13409,
     13395,  13380,  13366,  13351,  13337,  13322,  13307,  13293,  13278,  13263,  13249,  13234,  13219,  13204,  13189,  13174,
     13159,  13144,  13129,  13114,  13099,  13084,  13069,  13054,  13038,  13023,  13008,  12993,  12977,  12962,  12947,  12931,
     12916,  12900,  12885,  12869,  12854,  12838,  12822,  12807,  12791,  12775,  12760,  12744,  12728,  12712,  12696,  12680,
     12665,  12649,  12633,  12617,  12600,  12584,  12568,  12552,  12536,  12520,  12504,  12487,  12471,  12455,  12438,  12422,
     12406,  12389,  12373,  12356,  12340,  12323,  12307,  12290,  12273,  12257,  12240,  12223,  12207,  12190,  12173,  12156,
     12139,  12122,  12105,  12088,  12072,  12054,  12037,  12020,  12003,  11986,  11969,  11952,  11935,  11917,  11900,  11883,
     11866,  11848,  11831,  11813,  11796,  11779,  11761,  11744,  11726,  11708,  11691,  11673,  11656,  11638,  11620,  11602,
     11585,  11567,  11549,  11531,  11513,  11496,  11478,  11460,  11442,  11424,  11406,  11388,  11370,  11351,  11333,  11315,
     11297,  11279,  11260,  11242,  11224,  11206,  11187,  11169,  11150,  11132,  11114,  11095,  11077,  11058,  11040,  11021,
     11002,  10984,  10965,  10946,  10928,  10909,  10890,  10871,  10853,  10834,  10815,  10796,  10777,  10758,  10739,  10720,
     10701,  10682,  10663,  10644,  10625,  10606,  10586,  10567,  10548,  10529,  10510,  10490,  10471,  10452,  10432,  10413,
     10393,  10374,  10354,  10335,  10315,  10296,  10276,  10257,  10237,  10218,  10198,  10178,  10159,  10139,  10119,  10099,
     10079,  10060,  10040,  10020,  10000,   9980,   9960,   9940,   9920,   9900,   9880,   9860,   9840,   9820,   9800,   9780,
      9759,   9739,   9719,   9699,   9679,   9658,   9638,   9618,   9597,   9577,   9556,   9536,   9516,   9495,   9475,   9454,
      9434,   9413,   9392,   9372,   9351,   9331,   9310,   9289,   9268,   9248,   9227,   9206,   9185,   9165,   9144,   9123,
      9102,   9081,   9060,   9039,   9018,   8997,   8976,   8955,   8934,   8913,   8892,   8871,   8850,   8829,   8807,   8786,
      8765,   8744,   8722,   8701,   8680,   8658,   8637,   8616,   8594,   8573,   8552,   8530,   8509,   8487,   8466,   8444,
      8423,   8401,   8379,   8358,   8336,   8315,   8293,   8271,   8249,   8228,   8206,   8184,   8162,   8141,   8119,   8097,
      8075,   8053,   8031,   8009,   7988,   7966,   7944,   7922,   7900,   7878,   7856,   7833,   7811,   7789,   7767,   7745,
      7723,   7701,   7678,   7656,   7634,   7612,   7590,   7567,   7545,   7523,   7500,   7478,   7456,   7433,   7411,   7388,
      7366,   7343,   7321,   7299,   7276,   7253,   7231,   7208,   7186,   7163,   7141,   7118,   7095,   7073,   7050,   7027,
      7005,   6982,   6959,   6936,   6914,   6891,   6868,   6845,   6822,   6799,   6777,   6754,   6731,   6708,   6685,   6662,
      6639,   6616,   6593,   6570,   6547,   6524,   6501,   6478,   6455,   6432,   6408,   6385,   6362,   6339,   6316,   6293,
      6269,   6246,   6223,   6200,   6176,   6153,   6130,   6106,   6083,   6060,   6036,   6013,   5990,   5966,   5943,   5919,
      5896,   5873,   5849,   5826,   5802,   5779,   5755,   5732,   5708,   5684,   5661,   5637,   5614,   5590,   5566,   5543,
      5519,   5495,   5472,   5448,   5424,   5401,   5377,   5353,   5329,   5306,   5282,   5258,   5234,   5210,   5187,   5163,
      5139,   5115,   5091,   5067,   5043,   5019,   4995,   4972,   4948,   4924,   4900,   4876,   4852,   4828,   4804,   4780,
      4756,   4731,   4707,   4683,   4659,   4635,   4611,   4587,   4563,   4539,   4514,   4490,   4466,   4442,   4418,   4394,
      4369,   4345,   4321,   4297,   4272,   4248,   4224,   4200,   4175,   4151,   4127,   4102,   4078,   4054,   4029,   4005,
      3980,   3956,   3932,   3907,   3883,   3858,   3834,   3810,   3785,   3761,   3736,   3712,   3687,   3663,   3638,   3614,
      3589,   3565,   3540,   3516,   3491,   3467,   3442,   3417,   3393,   3368,   3344,   3319,   3294,   3270,   3245,   3221,
      3196,   3171,   3147,   3122,   3097,   3073,   3048,   3023,   2998,   2974,   2949,   2924,   2900,   2875,   2850,   2825,
      2801,   2776,   2751,   2726,   2701,   2677,   2652,   2627,   2602,   2577,   2553,   2528,   2503,   2478,   2453,   2428,
      2404,   2379,   2354,   2329,   2304,   2279,   2254,   2229,   2204,   2180,   2155,   2130,   2105,   2080,   2055,   2030,
      2005,   1980,   1955,   1930,   1905,   1880,   1855,   1830,   1805,   1780,   1755,   1730,   1705,   1680,   1655,   1630,
      1605,   1580,   1555,   1530,   1505,   1480,   1455,   1430,   1405,   1380,   1355,   1330,   1305,   1280,   1255,   1230,
      1205,   1180,   1155,   1130,   1105,   1079,   1054,   1029,   1004,    979,    954,    929,    904,    879,    854,    829,
       803,    778,    753,    728,    703,    678,    653,    628,    603,    577,    552,    527,    502,    477,    452,    427,
       402,    376,    351,    326,    301,    276,    251,    226,    201,    175,    150,    125,    100,     75,     50,     25,
         0,    -26,    -51,    -76,   -101,   -126,   -151,   -176,   -202,   -227,   -252,   -277,   -302,   -327,   -352,   -377,
      -403,   -428,   -453,   -478,   -503,   -528,   -553,   -578,   -604,   -629,   -654,   -679,   -704,   -729,   -754,   -779,
      -804,   -830,   -855,   -880,   -905,   -930,   -955,   -980,  -1005,  -1030,  -1055,  -1080,  -1106,  -1131,  -1156,  -1181,
     -1206,  -1231,  -1256,  -1281,  -1306,  -1331,  -1356,  -1381,  -1406,  -1431,  -1456,  -1481,  -1506,  -1531,  -1556,  -1581,
     -1606,  -1631,  -1656,  -1681,  -1706,  -1731,  -1756,  -1781,  -1806,  -1831,  -1856,  -1881,  -1906,  -1931,  -1956,  -1981,
     -2006,  -2031,  -2056,  -2081,  -2106,  -2131,  -2156,  -2181,  -2205,  -2230,  -2255,  -2280,  -2305,  -2330,  -2355,  -2380,
     -2405,  -2429,  -2454,  -2479,  -2504,  -2529,  -2554,  -2578,  -2603,  -2628,  -2653,  -2678,  -2702,  -2727,  -2752,  -2777,
     -2802,  -2826,  -2851,  -2876,  -2901,  -2925,  -2950,  -2975,  -2999,  -3024,  -3049,  -3074,  -3098,  -3123,  -3148,  -3172,
     -3197,  -3222,  -3246,  -3271,  -3295,  -3320,  -3345,  -3369,  -3394,  -3418,  -3443,  -3468,  -3492,  -3517,  -3541,  -3566,
     -3590,  -3615,  -3639,  -3664,  -3688,  -3713,  -3737,  -3762,  -3786,  -3811,  -3835,  -3859,  -3884,  -3908,  -3933,  -3957,
     -3981,  -4006,  -4030,  -4055,  -4079,  -4103,  -4128,  -4152,  -4176,  -4201,  -4225,  -4249,  -4273,  -4298,  -4322,  -4346,
     -4370,  -4395,  -4419,  -4443,  -4467,  -4491,  -4515,  -4540,  -4564,  -4588,  -4612,  -4636,  -4660,  -4684,  -4708,  -4732,
     -4757,  -4781,  -4805,  -4829,  -4853,  -4877,  -4901,  -4925,  -4949,  -4973,  -4996,  -5020,  -5044,  -5068,  -5092,  -5116,
     -5140,  -5164,  -5188,  -5211,  -5235,  -5259,  -5283,  -5307,  -5330,  -5354,  -5378,  -5402,  -5425,  -5449,  -5473,  -5496,
     -5520,  -5544,  -5567,  -5591,  -5615,  -5638,  -5662,  -5685,  -5709,  -5733,  -5756,  -5780,  -5803,  -5827,  -5850,  -5874,
     -5897,  -5920,  -5944,  -5967,  -5991,  -6014,  -6037,  -6061,  -6084,  -6107,  -6131,  -6154,  -6177,  -6201,  -6224,  -6247,
     -6270,  -6294,  -6317,  -6340,  -6363,  -6386,  -6409,  -6433,  -6456,  -6479,  -6502,  -6525,  -6548,  -6571,  -6594,  -6617,
     -6640,  -6663,  -6686,  -6709,  -6732,  -6755,  -6778,  -6800,  -6823,  -6846,  -6869,  -6892,  -6915,  -6937,  -6960,  -6983,
     -7006,  -7028,  -7051,  -7074,  -7096,  -7119,  -7142,  -7164,  -7187,  -7209,  -7232,  -7254,  -7277,  -7300,  -7322,  -7344,
     -7367,  -7389,  -7412,  -7434,  -7457,  -7479,  -7501,  -7524,  -7546,  -7568,  -7591,  -7613,  -7635,  -7657,  -7679,  -7702,
     -7724,  -7746,  -7768,  -7790,  -7812,  -7834,  -7857,  -7879,  -7901,  -7923,  -7945,  -7967,  -7989,  -8010,  -8032,  -8054,
     -8076,  -8098,  -8120,  -8142,  -8163,  -8185,  -8207,  -8229,  -8250,  -8272,  -8294,  -8316,  -8337,  -8359,  -8380,  -8402,
     -8424,  -8445,  -8467,  -8488,  -8510,  -8531,  -8553,  -8574,  -8595,  -8617,  -8638,  -8659,  -8681,  -8702,  -8723,  -8745,
     -8766,  -8787,  -8808,  -8830,  -8851,  -8872,  -8893,  -8914,  -8935,  -8956,  -8977,  -8998,  -9019,  -9040,  -9061,  -9082,
     -9103,  -9124,  -9145,  -9166,  -9186,  -9207,  -9228,  -9249,  -9269,  -9290,  -9311,  -9332,  -9352,  -9373,  -9393,  -9414,
     -9435,  -9455,  -9476,  -9496,  -9517,  -9537,  -9557,  -9578,  -9598,  -9619,  -9639,  -9659,  -9680,  -9700,  -9720,  -9740,
     -9760,  -9781,  -9801,  -9821,  -9841,  -9861,  -9881,  -9901,  -9921,  -9941,  -9961,  -9981, -10001, -10021, -10041, -10061,
    -10080, -10100, -10120, -10140, -10160, -10179, -10199, -10219, -10238, -10258, -10277, -10297, -10316, -10336, -10355, -10375,
    -10394, -10414, -10433, -10453, -10472, -10491, -10511, -10530, -10549, -10568, -10587, -10607, -10626, -10645, -10664, -10683,
    -10702, -10721, -10740, -10759, -10778, -10797, -10816, -10835, -10854, -10872, -10891, -10910, -10929, -10947, -10966, -10985,
    -11003, -11022, -11041, -11059, -11078, -11096, -11115, -11133, -11151, -11170, -11188, -11207, -11225, -11243, -11261, -11280,
    -11298, -11316, -11334, -11352, -11371, -11389, -11407, -11425, -11443, -11461, -11479, -11497, -11514, -11532, -11550, -11568,
    -11586, -11603, -11621, -11639, -11657, -11674, -11692, -11709, -11727, -11745, -11762, -11780, -11797, -11814, -11832, -11849,
    -11867, -11884, -11901, -11918, -11936, -11953, -11970, -11987, -12004, -12021, -12038, -12055, -12073, -12089, -12106, -12123,
    -12140, -12157, -12174, -12191, -12208, -12224, -12241, -12258, -12274, -12291, -12308, -12324, -12341, -12357, -12374, -12390,
    -12407, -12423, -12439, -12456, -12472, -12488, -12505, -12521, -12537, -12553, -12569, -12585, -12601, -12618, -12634, -12650,
    -12666, -12681, -12697, -12713, -12729, -12745, -12761, -12776, -12792, -12808, -12823, -12839, -12855, -12870, -12886, -12901,
    -12917, -12932, -12948, -12963, -12978, -12994, -13009, -13024, -13039, -13055, -13070, -13085, -13100, -13115, -13130, -13145,
    -13160, -13175, -13190, -13205, -13220, -13235, -13250, -13264, -13279, -13294, -13308, -13323, -13338, -13352, -13367, -13381,
    -13396, -13410, -13425, -13439, -13453, -13468, -13482, -13496, -13511, -13525, -13539, -13553, -13567, -13581, -13595, -13609,
    -13623, -13637, -13651, -13665, -13679, -13693, -13706, -13720, -13734, -13748, -13761, -13775, -13789, -13802, -13816, -13829,
    -13843, -13856, -13869, -13883, -13896, -13909, -13923, -13936, -13949, -13962, -13975, -13989, -14002, -14015, -14028, -14041,
    -14054, -14066, -14079, -14092, -14105, -14118, -14130, -14143, -14156, -14168, -14181, -14194, -14206, -14219, -14231, -14244,
    -14256, -14268, -14281, -14293, -14305, -14318, -14330, -14342, -14354, -14366, -14378, -14390, -14402, -14414, -14426, -14438,
    -14450, -14462, -14474, -14485, -14497, -14509, -14520, -14532, -14544, -14555, -14567, -14578, -14590, -14601, -14612, -14624,
    -14635, -14646, -14658, -14669, -14680, -14691, -14702, -14713, -14724, -14735, -14746, -14757, -14768, -14779, -14790, -14801,
    -14811, -14822, -14833, -14844, -14854, -14865, -14875, -14886, -14896, -14907, -14917, -14928, -14938, -14948, -14958, -14969,
    -14979, -14989, -14999, -15009, -15019, -15029, -15039, -15049, -15059, -15069, -15079, -15089, -15099, -15108, -15118, -15128,
    -15137, -15147, -15157, -15166, -15176, -15185, -15194, -15204, -15213, -15222, -15232, -15241, -15250, -15259, -15268, -15278,
    -15287, -15296, -15305, -15314, -15323, -15331, -15340, -15349, -15358, -15367, -15375, -15384, -15393, -15401, -15410, -15418,
    -15427, -15435, -15444, -15452, -15460, -15469, -15477, -15485, -15493, -15501, -15510, -15518, -15526, -15534, -15542, -15550,
    -15558, -15565, -15573, -15581, -15589, -15597, -15604, -15612, -15619, -15627, -15635, -15642, -15650, -15657, -15664, -15672,
    -15679, -15686, -15694, -15701, -15708, -15715, -15722, -15729, -15736, -15743, -15750, -15757, -15764, -15771, -15778, -15784,
    -15791, -15798, -15804, -15811, -15818, -15824, -15831, -15837, -15843, -15850, -15856, -15862, -15869, -15875, -15881, -15887,
    -15893, -15900, -15906, -15912, -15918, -15924, -15929, -15935, -15941, -15947, -15953, -15958, -15964, -15970, -15975, -15981,
    -15986, -15992, -15997, -16003, -16008, -16013, -16019, -16024, -16029, -16034, -16040, -16045, -16050, -16055, -16060, -16065,
    -16070, -16075, -16079, -16084, -16089, -16094, -16098, -16103, -16108, -16112, -16117, -16121, -16126, -16130, -16135, -16139,
    -16143, -16148, -16152, -16156, -16160, -16164, -16168, -16172, -16176, -16180, -16184, -16188, -16192, -16196, -16200, -16203,
    -16207, -16211, -16214, -16218, -16222, -16225, -16229, -16232, -16235, -16239, -16242, -16245, -16249, -16252, -16255, -16258,
    -16261, -16264, -16267, -16270, -16273, -16276, -16279, -16282, -16285, -16287, -16290, -16293, -16295, -16298, -16301, -16303,
    -16306, -16308, -16310, -16313, -16315, -16317, -16320, -16322, -16324, -16326, -16328, -16330, -16332, -16334, -16336, -16338,
    -16340, -16342, -16344, -16345, -16347, -16349, -16351, -16352, -16354, -16355, -16357, -16358, -16360, -16361, -16362, -16364,
    -16365, -16366, -16367, -16368, -16369, -16370, -16371, -16372, -16373, -16374, -16375, -16376, -16377, -16378, -16378, -16379,
    -16380, -16380, -16381, -16381, -16382, -16382, -16383, -16383, -16383, -16384, -16384, -16384, -16384, -16384, -16384, -16384,
    -16384
};

/* TIMWIN: 10c8:2946
   Returns value from -0x4000 to +0x4000 inclusive. */
s16 sine(u16 angle) {
    angle = (u16)(angle + 0xC000) / 16;
    if ((angle & 0x0800) != 0) {
        // make angle less than 0x0800.
        angle = 0x1000 - angle;
    }
    // "angle" is one of 2049 possible values (0x000 to 0x800 inclusive)
    return SINE_LOOKUP_TABLE[angle];
}

/* TIMWIN: 10c8:296e
   Returns value from -0x4000 to +0x4000 inclusive. */
s16 cosine(u16 angle) {
    angle = angle / 16;
    if ((angle & 0x0800) != 0) {
        // make angle less than 0x0800.
        angle = 0x1000 - angle;
    }
    // "angle" is one of 2049 possible values (0x000 to 0x800 inclusive)
    return SINE_LOOKUP_TABLE[angle];
}

/* TIMWIN: 10a8:0000
   Modifies x and y by rotating it around 0,0. Rotates clockwise. */
void rotate_point(s16 *x, s16 *y, u16 angle) {
    s32 a = *x;
    s32 b = *y;

    *x = (s32)(a*cosine(angle) - b*sine(angle))   >> 0xe;
    *y = (s32)(a*sine(angle)   + b*cosine(angle)) >> 0xe;
}

#if ENABLE_TEST_SUITE

TEST_SUITE(sine) {
    TEST("sine(x + 0x4000) = cosine(x)") {
        for (s32 i = 0; i < 0x10000; i++) {
            ASSERT_EQ(sine(i + 0x4000), cosine(i));
        }
    }

    TEST("sine(0) = 0") {
        ASSERT_EQ(sine(0), 0);
    }

    TEST("sine(0x2000) = 0x2D41 (45 degrees)") {
        // Note: 0x2D41/0x4000 = approx 0.7071
        ASSERT_EQ(sine(0x2000), 0x2D41);
    }

    TEST("sine(0x4000) = 0x4000 (90 degrees)") {
        ASSERT_EQ(sine(0x4000), 0x4000);
    }

    TEST("sine(0x8000) = 0 (180 degrees)") {
        ASSERT_EQ(sine(0x8000), 0);
    }

    TEST("sine(0xC000) = -0x4000 (270 degrees)") {
        ASSERT_EQ(sine(0xC000), -0x4000);
    }

    TEST("cosine(0) = 0x4000") {
        ASSERT_EQ(cosine(0), 0x4000);
    }
}

#endif