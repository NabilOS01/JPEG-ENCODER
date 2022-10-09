#ifndef _ENCODE_AC_DC_H_
#define _ENCODE_AC_DC_H_

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "my_huffman.h"
#include "my_bitstream.h"
#include "htables.h"
#include "my_jpeg_writer.h"

/*
    Calcule le coefficient DC assossié au coefficient "coeff"
    et l'écrit dans le bitstream "stream".
*/
extern void encode_DC(struct bitstream *stream, struct huff_table *table_dc, int16_t coeff);

/*
    Code tous les coefficients non nuls dans le vecteur quantifiée, vecteue_quantifie,
    et les écrits dans le bitsteam stream.
*/
extern void encode_vect_AC(struct bitstream *stream, struct huff_table *ht, int16_t *vecteur_quantifie);

#endif /* _ENCODE_AD_DC_H_ */
