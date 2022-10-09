#ifndef _DCT_H
#define _DCT_H

#include <stdint.h>

/*
    Retranche 128 de chaque valeur de la matrice
*/
extern int16_t **offset(uint8_t **matrice);

/*
    Calcul de la dct d'un bloc
*/
extern int16_t **dct_naive(uint8_t **matrice);

/*
    Calcul de la dct pour un bloc en suivant la méthode de loeffler
*/
extern int16_t **dct_loeffler(uint8_t **bloc);

/*
    Calcul de la dct d'un bloc
*/
extern int16_t **dct(uint8_t **matrice, char *dct_used);

/*
    Libère l'espace mémoire associé à un matrice uint8_t
*/
extern void dct_free(int16_t **dct_values);

#endif
