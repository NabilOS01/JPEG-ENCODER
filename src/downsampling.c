#include <stdlib.h>
#include <stdint.h>
#include "downsampling.h"
#include "decoupage.h"

/*
    Downsampling horizontal pour 2 blocs
*/
uint8_t **horizontal_downsampling(uint8_t **bloc1, uint8_t **bloc2)
{
    uint8_t **nv_matrice = calloc(8, sizeof(uint8_t *));
    for (uint8_t i = 0; i < 8; i++)
    {
        nv_matrice[i] = malloc(8 * sizeof(uint8_t));
        for (uint8_t j = 0; j < 4; j++)
        {
            nv_matrice[i][j] = (bloc1[i][j * 2] + bloc1[i][(j * 2) + 1]) / 2; // la moyenne pour chaque pair de pixels
        }
        for (uint8_t j = 0; j < 4; j++)
        {
            nv_matrice[i][j + 4] = (bloc2[i][j * 2] + bloc2[i][(j * 2) + 1]) / 2; // la moyenne pour chaque pair de pixels
        }
    }
    return nv_matrice;
}

/*
    Downsampling vertical pour 2 blocs
*/
uint8_t **horizontal_vertical_downsampling(uint8_t **bloc1, uint8_t **bloc2)
{
    uint8_t **nv_matrice = calloc(8, sizeof(uint8_t *));
    for (uint8_t j = 0; j < 8; j++)
    {
        nv_matrice[j] = malloc(8 * sizeof(uint8_t));
    }
    
    for (uint8_t j = 0; j < 8; j++)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            nv_matrice[i][j] = (bloc1[i * 2][j] + bloc1[(i * 2) + 1][j]) / 2; // la moyenne pour chaque pair de pixels
        }
        for (uint8_t i = 0; i < 4; i++)
        {
            nv_matrice[i + 4][j] = (bloc2[i * 2][j] + bloc2[i * 2 + 1][j]) / 2; // la moyenne pour chaque pair de pixels
        }
    }
    return nv_matrice;
}

/*
    Downsampling horizontal pour 3 blocs
*/
uint8_t **horizontal_downsampling_triple(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3){
    uint8_t **nv_matrice = calloc(8, sizeof(uint8_t *));
    for (uint8_t i = 0; i < 8; i++)
    {
        nv_matrice[i] = malloc(8 * sizeof(uint8_t));
        
        nv_matrice[i][0] = (bloc1[i][0] + bloc1[i][1] + bloc1[i][2]) / 3; // la moyenne pour chaque pair de pixels
        nv_matrice[i][1] = (bloc1[i][3] + bloc1[i][4] + bloc1[i][5]) / 3;
        nv_matrice[i][2] = (bloc1[i][6] + bloc1[i][7] + bloc2[i][0]) / 3;
        nv_matrice[i][3] = (bloc2[i][1] + bloc2[i][2] + bloc2[i][3]) / 3;
        nv_matrice[i][4] = (bloc2[i][4] + bloc2[i][5] + bloc2[i][6]) / 3;
        nv_matrice[i][5] = (bloc2[i][7] + bloc3[i][0] + bloc3[i][1]) / 3;
        nv_matrice[i][6] = (bloc3[i][2] + bloc3[i][3] + bloc3[i][4]) / 3;
        nv_matrice[i][7] = (bloc3[i][5] + bloc3[i][6] + bloc3[i][7]) / 3;

    }
    return nv_matrice;
}

/*
    Downsampling vertical pour 3 blocs
*/
uint8_t **horizontal_vertical_downsampling_triple(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3)
{
    uint8_t **nv_matrice = calloc(8, sizeof(uint8_t *));
    for (uint8_t j = 0; j < 8; j++)
    {
        nv_matrice[j] = malloc(8 * sizeof(uint8_t));
    }
    for (uint8_t j = 0; j < 8; j++)
    {
        nv_matrice[0][j] = (bloc1[0][j] + bloc1[1][j] + bloc1[2][j]) / 3; // la moyenne pour chaque pair de pixels
        nv_matrice[1][j] = (bloc1[3][j] + bloc1[4][j] + bloc1[5][j]) / 3;
        nv_matrice[2][j] = (bloc1[6][j] + bloc1[7][j] + bloc2[0][j]) / 3;
        nv_matrice[3][j] = (bloc2[1][j] + bloc2[2][j] + bloc2[3][j]) / 3;
        nv_matrice[4][j] = (bloc2[4][j] + bloc2[5][j] + bloc2[6][j]) / 3;
        nv_matrice[5][j] = (bloc2[7][j] + bloc3[0][j] + bloc3[1][j]) / 3;
        nv_matrice[6][j] = (bloc3[2][j] + bloc3[3][j] + bloc3[4][j]) / 3;
        nv_matrice[7][j] = (bloc3[5][j] + bloc3[6][j] + bloc3[7][j]) / 3;

    }
    return nv_matrice;
}

/*
    Downsampling horizontal pour 4 blocs
*/
uint8_t **horizontal_downsampling_4(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3, uint8_t **bloc4)
{
    uint8_t **matrice1 = horizontal_downsampling(bloc1, bloc2);
    uint8_t **matrice2 = horizontal_downsampling(bloc3, bloc4);
    uint8_t **nv_matrice = horizontal_downsampling(matrice1, matrice2);
    matrix_free(matrice1);
    matrix_free(matrice2);
    return nv_matrice;
}

/*
    Downsampling vertical pour 4 blocs
*/
uint8_t **horizontal_vertical_downsampling_4(uint8_t **bloc1, uint8_t **bloc2, uint8_t **bloc3, uint8_t **bloc4)
{
    uint8_t **matrice1 = horizontal_vertical_downsampling(bloc1, bloc2);
    uint8_t **matrice2 = horizontal_vertical_downsampling(bloc3, bloc4);
    uint8_t **nv_matrice = horizontal_vertical_downsampling(matrice1, matrice2);
    matrix_free(matrice1);
    matrix_free(matrice2);
    return nv_matrice;
}