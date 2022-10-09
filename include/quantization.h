#ifndef _QUANTIZATION_H_
#define _QUANTIZATION_H_
#include <stdlib.h>
#include <stdint.h>

/*
    Fonction qui réalise la quantification d'un vecteur donné en paramètre
    selon une table de quantification
*/
extern int16_t *quantization(int16_t vecteur[64], uint8_t quantification_table[64]);

/*
    Fonction qui affiche les valeurs d'un vecteur quantifié.
*/
extern void show_quantified_vector(int16_t vecteur_quantifie[64]);

#endif /* _QUANTIZATION_H_ */
