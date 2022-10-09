#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "zigzag.h"

/*
    Fonction qui prend en paramètre une matrice 8x8 et un vecteur de taille 64
    et remplie ce vecteur en effectuant le parcours en zigzag de la matrice 
*/
void zigzag(int16_t **matrice, int16_t vecteur[64])
{
    vecteur[0] = matrice[0][0];

    vecteur[1] = matrice[0][1];
    vecteur[2] = matrice[1][0];

    vecteur[3] = matrice[2][0];
    vecteur[4] = matrice[1][1];
    vecteur[5] = matrice[0][2];

    vecteur[6] = matrice[0][3];
    vecteur[7] = matrice[1][2];
    vecteur[8] = matrice[2][1];
    vecteur[9] = matrice[3][0];

    vecteur[10] = matrice[4][0];
    vecteur[11] = matrice[3][1];
    vecteur[12] = matrice[2][2];
    vecteur[13] = matrice[1][3];
    vecteur[14] = matrice[0][4];

    vecteur[15] = matrice[0][5];
    vecteur[16] = matrice[1][4];
    vecteur[17] = matrice[2][3];
    vecteur[18] = matrice[3][2];
    vecteur[19] = matrice[4][1];
    vecteur[20] = matrice[5][0];

    vecteur[21] = matrice[6][0];
    vecteur[22] = matrice[5][1];
    vecteur[23] = matrice[4][2];
    vecteur[24] = matrice[3][3];
    vecteur[25] = matrice[2][4];
    vecteur[26] = matrice[1][5];
    vecteur[27] = matrice[0][6];

    vecteur[28] = matrice[0][7];
    vecteur[29] = matrice[1][6];
    vecteur[30] = matrice[2][5];
    vecteur[31] = matrice[3][4];
    vecteur[32] = matrice[4][3];
    vecteur[33] = matrice[5][2];
    vecteur[34] = matrice[6][1];
    vecteur[35] = matrice[7][0];

    vecteur[36] = matrice[7][1];
    vecteur[37] = matrice[6][2];
    vecteur[38] = matrice[5][3];
    vecteur[39] = matrice[4][4];
    vecteur[40] = matrice[3][5];
    vecteur[41] = matrice[2][6];
    vecteur[42] = matrice[1][7];
    vecteur[43] = matrice[2][7];
    vecteur[44] = matrice[3][6];
    vecteur[45] = matrice[4][5];
    vecteur[46] = matrice[5][4];
    vecteur[47] = matrice[6][3];
    vecteur[48] = matrice[7][2];
    vecteur[49] = matrice[7][3];
    vecteur[50] = matrice[6][4];
    vecteur[51] = matrice[5][5];
    vecteur[52] = matrice[4][6];
    vecteur[53] = matrice[3][7];
    vecteur[54] = matrice[4][7];
    vecteur[55] = matrice[5][6];
    vecteur[56] = matrice[6][5];
    vecteur[57] = matrice[7][4];
    vecteur[58] = matrice[7][5];
    vecteur[59] = matrice[6][6];
    vecteur[60] = matrice[5][7];
    vecteur[61] = matrice[6][7];
    vecteur[62] = matrice[7][6];
    
    vecteur[63] = matrice[7][7];
}

/*
    Fonction qui affiche les valeurs d'un vecteur zigzagué.
*/
void show_zigzag_vector(int16_t vect_zigzag[64])
{
    for (uint8_t i = 0; i < 64; i += 8)
        {
            for (uint8_t j = 0; j < 8; j++)
            {
                printf("%04" PRIx16 " ", (uint16_t)vect_zigzag[i + j]);
            }
            printf("\n");
        }
}
