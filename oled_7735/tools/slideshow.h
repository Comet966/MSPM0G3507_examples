#ifndef SLIDESHOW_H_
#define SLIDESHOW_H_
#include <stdint.h>
#include "img_00.h"
#include "img_01.h"
#include "img_02.h"
#include "img_03.h"
#include "img_04.h"
#include "img_05.h"
#include "img_06.h"
#include "img_07.h"
#include "img_08.h"
#include "img_09.h"
#include "img_10.h"
#include "img_11.h"
#include "img_12.h"
#include "img_13.h"
#include "img_14.h"
#include "img_15.h"
#include "img_16.h"

#define SLIDESHOW_COUNT  17
#define SLIDESHOW_W      48
#define SLIDESHOW_H      60

static const uint16_t * const slideshow_images[SLIDESHOW_COUNT] = {
    img_00,  /* avg_2027_wang.png */
    img_01,  /* char_007_closre_1.png */
    img_02,  /* char_1035_wisdel_game#9.png */
    img_03,  /* char_1035_wisdel_sale#14.png */
    img_04,  /* char_1038_whitw2_sale#15.png */
    img_05,  /* char_1041_angel2_iteration#6.png */
    img_06,  /* char_1042_phatm2_2.png */
    img_07,  /* char_1043_leizi2_2.png */
    img_08,  /* char_1044_hsgma2_2.png */
    img_09,  /* char_1045_svash2_2.png */
    img_10,  /* char_1046_sbell2.png */
    img_11,  /* char_1050_chen3_2.png */
    img_12,  /* char_1052_kalts2_2.png */
    img_13,  /* char_2025_shu_nian#11.png */
    img_14,  /* char_2026_yu.png */
    img_15,  /* char_4064_mlynar_epoque#28.png */
    img_16,  /* char_4087_ines_2.png */
};

#endif /* SLIDESHOW_H_ */
