#ifndef _DOWNSAMPLING_H_
#define _DOWNSAMPLING_H_

#include <stdlib.h>
#include <stdint.h>

/*
    Downsampling horizontal pour 2 blocs
*/
extern uint8_t **horizontal_downsampling(uint8_t **bloc1, uint8_t **bloc2);

/*
    Downsampling vertical pour 2 blocs
*/
extern uint8_t **horizontal_vertical_downsampling(uint8_t **bloc1, uint8_t **bloc2);

/*
    Downsampling horizontal pour 3 blocs
*/
extern uint8_t **horizontal_downsampling_triple(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3);

/*
    Downsampling vertical pour 3 blocs
*/
extern uint8_t **horizontal_vertical_downsampling_triple(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3);

/*
    Downsampling horizontal pour 4 blocs
*/
extern uint8_t **horizontal_downsampling_4(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3, uint8_t **bloc4);

/*
    Downsampling vertical pour 4 blocs
*/
extern uint8_t **horizontal_vertical_downsampling_4(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3, uint8_t **bloc4);

#endif /* DOWNSAMPLING_H */