#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "quantization.h"

/*
    Fonction qui réalise la quantification d'un vecteur donné en paramètre
    selon une table de quantification
*/
int16_t *quantization(int16_t vecteur[64], uint8_t quantification_table[64])
{
    int16_t *vecteur_quantifie = malloc(64 * sizeof(int16_t));
    for(uint16_t i = 0; i < 64; i++){
        int16_t facteur = (int16_t) quantification_table[i];
        vecteur_quantifie[i] = vecteur[i]/facteur;
    }
    return vecteur_quantifie;
}

/*
    Fonction qui affiche les valeurs d'un vecteur quantifié.
*/
void show_quantified_vector(int16_t quantified[64])
{
    for (uint8_t i = 0; i < 64; i += 8)
        {
            for (uint8_t j = 0; j < 8; j++)
            {
                printf("%04" PRIx16 " ", (uint16_t)quantified[i + j]);
            }
            printf("\n");
        }
}
