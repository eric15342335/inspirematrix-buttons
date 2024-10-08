/// @brief Button calibration values from ADC_read()
#pragma once

#include "funconfig.h"

// Joypad calibration values
#define JOY_N 197  // joypad UP
#define JOY_NE 259 // joypad UP + RIGHT
#define JOY_E 90   // joypad RIGHT
#define JOY_SE 388 // joypad DOWN + RIGHT
#define JOY_S 346  // joypad DOWN
#define JOY_SW 616 // joypad DOWN + LEFT
#define JOY_W 511  // joypad LEFT
#define JOY_NW 567 // JOYPAD UP + LEFT
#define JOY_DEV 20 // deviation

#ifndef NUM_BUTTONS
#define NUM_BUTTONS NUM_LEDS
#endif

#ifdef INTERNAL_INSPIRE_MATRIX

#define BUTTON_NONE 40
#define BUTTON_DEVIATION 2
#define BUTTON_DEVIATION_UPPER_HALF 2
// PD2 PIN as for now
#define BUTTON_0 686
#define BUTTON_1 653
#define BUTTON_2 625
#define BUTTON_3 597
#define BUTTON_4 572
#define BUTTON_5 549
#define BUTTON_6 527
#define BUTTON_7 506
#define BUTTON_8 488
#define BUTTON_9 471
#define BUTTON_10 455
#define BUTTON_11 439
#define BUTTON_12 424
#define BUTTON_13 410
#define BUTTON_14 398
#define BUTTON_15 385
#define BUTTON_16 374
#define BUTTON_17 362
#define BUTTON_18 352
#define BUTTON_19 342
#define BUTTON_20 332
#define BUTTON_21 323
#define BUTTON_22 314
#define BUTTON_23 306
#define BUTTON_24 298
#define BUTTON_25 290
#define BUTTON_26 282
#define BUTTON_27 275
#define BUTTON_28 266
#define BUTTON_29 260
#define BUTTON_30 255
#define BUTTON_31 246
// PD3 PIN as for now
#define BUTTON_32 243
#define BUTTON_33 236
#define BUTTON_34 230
#define BUTTON_35 224
#define BUTTON_36 218
#define BUTTON_37 212
#define BUTTON_38 207
#define BUTTON_39 200
#define BUTTON_40 196
#define BUTTON_41 190
#define BUTTON_42 184
#define BUTTON_43 179
#define BUTTON_44 174
#define BUTTON_45 168
#define BUTTON_46 163
#define BUTTON_47 158
#define BUTTON_48 152
#define BUTTON_49 0 // faulty
#define BUTTON_50 142
#define BUTTON_51 136
#define BUTTON_52 131
#define BUTTON_53 125
#define BUTTON_54 120
#define BUTTON_55 114
#define BUTTON_56 108
#define BUTTON_57 102
#define BUTTON_58 96
#define BUTTON_59 90
#define BUTTON_60 84
#define BUTTON_61 76
#define BUTTON_62 68
#define BUTTON_63 62
#define BUTTON_X 20
#define BUTTON_Y 10
#define BUTTON_UP 46
#define BUTTON_DOWN 29
#define BUTTON_LEFT 55
#define BUTTON_RIGHT 37 /// @note When using battery power source without laptop grounding,
// right button seems cannot reliably detected.
#define SPECIAL_BUTTON_DEVIATION 2

/// @brief Array of buttons corresponding to the ADC values, for linear searching
static const int buttons[NUM_BUTTONS] = {BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4,
    BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8, BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12,
    BUTTON_13, BUTTON_14, BUTTON_15, BUTTON_16, BUTTON_17, BUTTON_18, BUTTON_19,
    BUTTON_20, BUTTON_21, BUTTON_22, BUTTON_23, BUTTON_24, BUTTON_25, BUTTON_26,
    BUTTON_27, BUTTON_28, BUTTON_29, BUTTON_30, BUTTON_31, BUTTON_32, BUTTON_33,
    BUTTON_34, BUTTON_35, BUTTON_36, BUTTON_37, BUTTON_38, BUTTON_39, BUTTON_40,
    BUTTON_41, BUTTON_42, BUTTON_43, BUTTON_44, BUTTON_45, BUTTON_46, BUTTON_47,
    BUTTON_48, BUTTON_49, BUTTON_50, BUTTON_51, BUTTON_52, BUTTON_53, BUTTON_54,
    BUTTON_55, BUTTON_56, BUTTON_57, BUTTON_58, BUTTON_59, BUTTON_60, BUTTON_61,
    BUTTON_62, BUTTON_63};

#else

#define BUTTON_NONE 10
#define BUTTON_DEVIATION 5
#define BUTTON_0 1019
#define BUTTON_1 995
#define BUTTON_2 970
#define BUTTON_3 946
#define BUTTON_4 923
#define BUTTON_5 900
#define BUTTON_6 875
#define BUTTON_7 856
#define BUTTON_8 835
#define BUTTON_9 814
#define BUTTON_10 794
#define BUTTON_11 775
#define BUTTON_12 756
#define BUTTON_13 737
#define BUTTON_14 719
#define BUTTON_15 701
#define BUTTON_16 684
#define BUTTON_17 666
#define BUTTON_18 650
#define BUTTON_19 633
#define BUTTON_20 617
#define BUTTON_21 601
#define BUTTON_22 586
#define BUTTON_23 570
#define BUTTON_24 555
#define BUTTON_25 538
#define BUTTON_26 524
#define BUTTON_27 511
#define BUTTON_28 494
#define BUTTON_29 480
#define BUTTON_30 467
#define BUTTON_31 452
#define BUTTON_32 439
#define BUTTON_33 425
#define BUTTON_34 412
#define BUTTON_35 399
#define BUTTON_36 387
#define BUTTON_37 372
#define BUTTON_38 360
#define BUTTON_39 346
#define BUTTON_40 332
#define BUTTON_41 320
#define BUTTON_42 308
#define BUTTON_43 295
#define BUTTON_44 282
#define BUTTON_45 269
#define BUTTON_46 256
#define BUTTON_47 244
#define BUTTON_48 230
#define BUTTON_49 216
#define BUTTON_50 203
#define BUTTON_51 190
#define BUTTON_52 176
#define BUTTON_53 161
#define BUTTON_54 149
#define BUTTON_55 136
#define BUTTON_56 121
#define BUTTON_57 107
#define BUTTON_58 92
#define BUTTON_59 77
#define BUTTON_60 62
#define BUTTON_61 48
#define BUTTON_62 32
#define BUTTON_63 20

/// @brief Array of buttons corresponding to the ADC values, for linear searching
static const int buttons[NUM_BUTTONS] = {BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4,
    BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8, BUTTON_9, BUTTON_10, BUTTON_11, BUTTON_12,
    BUTTON_13, BUTTON_14, BUTTON_15, BUTTON_16, BUTTON_17, BUTTON_18, BUTTON_19,
    BUTTON_20, BUTTON_21, BUTTON_22, BUTTON_23, BUTTON_24, BUTTON_25, BUTTON_26,
    BUTTON_27, BUTTON_28, BUTTON_29, BUTTON_30, BUTTON_31, BUTTON_32, BUTTON_33,
    BUTTON_34, BUTTON_35, BUTTON_36, BUTTON_37, BUTTON_38, BUTTON_39, BUTTON_40,
    BUTTON_41, BUTTON_42, BUTTON_43, BUTTON_44, BUTTON_45, BUTTON_46, BUTTON_47,
    BUTTON_48, BUTTON_49, BUTTON_50, BUTTON_51, BUTTON_52, BUTTON_53, BUTTON_54,
    BUTTON_55, BUTTON_56, BUTTON_57, BUTTON_58, BUTTON_59, BUTTON_60, BUTTON_61,
    BUTTON_62, BUTTON_63};

#endif
