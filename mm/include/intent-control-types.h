#pragma once
#define INTENT_CONTROL_ROLL 1
#define INTENT_CONTROL_JUMP 2
#define INTENT_CONTROL_TALK 3

// Buttons for firing/shooting
#define INTENT_CONTROL_FIRE_BOW 4
#define INTENT_CONTROL_FIRE_HOOKSHOT 5

#define CAROUSEL_RANGE_START 1000

typedef struct CAROUSEL_BUTTONS_TYPE {
    char* useItemName;
    char* swapLeftName;
    char* swapRightName;
    unsigned short carouselNumber;
    unsigned short useItem;
    unsigned short swapRight;
    unsigned short swapLeft;
};

#define MAKE_CAROUSEL_BUTTONS(num) \
 ((struct CAROUSEL_BUTTONS_TYPE) { \
    "Use Carousel Item " #num,\
    "Swap Carousel Left " #num,\
    "Swap Carousel Right " #num,\
    num, \
    CAROUSEL_RANGE_START + num * 3, \
    CAROUSEL_RANGE_START + num * 3 + 1, \
    CAROUSEL_RANGE_START + num * 3 + 2, \
}) \


#define CAROUSEL1 MAKE_CAROUSEL_BUTTONS(1)
#define CAROUSEL2 MAKE_CAROUSEL_BUTTONS(2)
#define CAROUSEL3 MAKE_CAROUSEL_BUTTONS(3)
#define CAROUSEL4 MAKE_CAROUSEL_BUTTONS(4)
#define CAROUSEL5 MAKE_CAROUSEL_BUTTONS(5)

#define INDEXED_CAROUSELS (CAROUSEL_BUTTONS_TYPE[]){CAROUSEL1,CAROUSEL2,CAROUSEL3,CAROUSEL4,CAROUSEL5}