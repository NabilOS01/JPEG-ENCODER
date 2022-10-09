#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#define _USE_MATH_DEFINES
#define PI 3.141592653



/*
    Retranche 128 de chaque valeur de la matrice
*/
int16_t **offset(uint8_t **matrice)
{
    int16_t **offset = malloc(8 * sizeof(int16_t *));
    for (uint32_t i = 0; i < 8; i++)
    {
        offset[i] = malloc(8 * sizeof(int16_t));
        for (uint32_t j = 0; j < 8; j++)
        {
            offset[i][j] = (int16_t)matrice[i][j] - 128;
        }
    }
    return offset;
}

/*
    Calcul de la dct pour un bloc en utilisant la méthode naive
*/
int16_t **dct_naive(uint8_t **bloc)
{
    int16_t **matrice = offset(bloc);
    int16_t **bloc_dct = malloc(8 * sizeof(int16_t *));
    double **cosinus = malloc(8 * sizeof(double *));

    cosinus[0] = malloc(8 * sizeof(double));
    for (uint8_t j = 0; j < 8; j++)
    {
        cosinus[0][j] = 1 / sqrt(8);
    }

    for (uint8_t i = 1; i < 8; i++)
    {
        cosinus[i] = malloc(8 * sizeof(double));
        for (uint8_t j = 0; j < 8; j++)
        {
            cosinus[i][j] = 0.5 * cos((2 * j + 1) * i * PI / 16);
        }
    }

    double **inter = malloc(8 * sizeof(double *));
    for (uint8_t i = 0; i < 8; i++)
    {
        inter[i] = malloc(8 * sizeof(double));
        for (uint8_t j = 0; j < 8; j++)
        {
            inter[i][j] = 0;
            for (uint8_t k = 0; k < 8; k++)
            {
                inter[i][j] += cosinus[i][k] * matrice[k][j];
            }
        }
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        bloc_dct[i] = malloc(8 * sizeof(int16_t));
        for (uint8_t j = 0; j < 8; j++)
        {

            double resultat = 0;
            for (uint8_t k = 0; k < 8; k++)
            {
                resultat += inter[i][k] * cosinus[j][k];
            }
            bloc_dct[i][j] = (int16_t)resultat;
        }
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        free(cosinus[i]);
        free(inter[i]);
        free(matrice[i]);
    }
    free(cosinus);
    free(inter);
    free(matrice);

    return bloc_dct;
}


/*
    Calcul de la dct pour un bloc en suivant la méthode de loeffler
*/
int16_t **dct_loeffler(uint8_t **bloc)
{
    int16_t **matrice = offset(bloc);
    int16_t **dct_loeffler = malloc(8 * sizeof(int16_t *));
    // Définition des constantes:
    double c1, c3, c6, s1, s3, s6, squar;
    c1 = 0.9807852804;   //cos(PI / 16);
    c3 = 0.8314696123;   //cos(3 * PI / 16);
    c6 = 0.3826834324;   //cos(6 * PI / 16);
    s1 = 0.195090322;    //sin(PI / 16);
    s3 = 0.555570233;    //sin(3 * PI / 16);
    s6 = 0.9238795325;   //sin(6 * PI / 16);
    squar = 1.414213562; //sqrt(2);

    // DCT 1D sur les lignes.
    // double **auxi1 = malloc(8 * sizeof(double *));
    double auxi1[8][8];
    for(uint8_t i = 0; i < 8; i++){
        // auxi1[i] = malloc(8 * sizeof(double));

        // Étape 1:
        double *vect1 = malloc(8 * sizeof(double));
        vect1[0] = matrice[i][0] + matrice[i][7];
        vect1[1] = matrice[i][0] - matrice[i][7];
        vect1[2] = matrice[i][1] + matrice[i][6];
        vect1[3] = matrice[i][1] - matrice[i][6];
        vect1[4] = matrice[i][2] + matrice[i][5];
        vect1[5] = matrice[i][2] - matrice[i][5];
        vect1[6] = matrice[i][3] + matrice[i][4];
        vect1[7] = matrice[i][3] - matrice[i][4];

        // Étape 2:
        double *vect2 = malloc(8 * sizeof(double));
        vect2[0] = vect1[0] + vect1[6];
        vect2[1] = vect1[0] - vect1[6];
        vect2[2] = vect1[2] + vect1[4];
        vect2[3] = vect1[2] - vect1[4];
        vect2[4] = vect1[7] * c3 + vect1[1] * s3;
        vect2[5] = vect1[1] * c3 - vect1[7] * s3;
        vect2[6] = vect1[5] * c1 + vect1[3] * s1;
        vect2[7] = vect1[3] * c1 - vect1[5] * s1;

        // Étape 3:
        double *vect3 = malloc(8 * sizeof(double));
        vect3[0] = vect2[0] + vect2[2];
        vect3[1] = vect2[0] - vect2[2];
        vect3[2] = squar * c6 * vect2[3] + squar * s6 * vect2[1];
        vect3[3] = -squar * s6 * vect2[1] + squar * c6 * vect2[3];
        vect3[4] = vect2[4] + vect2[7];
        vect3[5] = vect2[4] - vect2[7];
        vect3[6] = vect2[5] - vect2[6];
        vect3[7] = vect2[5] + vect2[6];

        // Étape 4:
        auxi1[i][0] = vect3[0];
        auxi1[i][4] = vect3[1];
        auxi1[i][2] = vect3[2];
        auxi1[i][6] = vect3[3];
        auxi1[i][7] = vect3[7] - vect3[4];
        auxi1[i][1] = vect3[7] + vect3[4];
        auxi1[i][3] = squar * vect3[6];
        auxi1[i][5] = squar * vect3[5];
    }

    // DCT 1D sur les collonnes de la mtrice auxi1 => DCT 2D.
    double auxi2[8][8];
    for(uint8_t j = 0; j < 8; j++){

        // Étape 1:
        double *vect1 = malloc(8 * sizeof(double));
        vect1[0] = auxi1[0][j] + auxi1[7][j];
        vect1[1] = auxi1[0][j] - auxi1[7][j];
        vect1[2] = auxi1[1][j] + auxi1[6][j];
        vect1[3] = auxi1[1][j] - auxi1[6][j];
        vect1[4] = auxi1[2][j] + auxi1[5][j];
        vect1[5] = auxi1[2][j] - auxi1[5][j];
        vect1[6] = auxi1[3][j] + auxi1[4][j];
        vect1[7] = auxi1[3][j] - auxi1[4][j];

        // Étape 2:
        double *vect2 = malloc(8 * sizeof(double));
        vect2[0] = vect1[0] + vect1[6];
        vect2[1] = vect1[0] - vect1[6];
        vect2[2] = vect1[2] + vect1[4];
        vect2[3] = vect1[2] - vect1[4];
        vect2[4] = vect1[7] * c3 + vect1[1] * s3;
        vect2[5] = vect1[1] * c3 - vect1[7] * s3;
        vect2[6] = vect1[5] * c1 + vect1[3] * s1;
        vect2[7] = vect1[3] * c1 - vect1[5] * s1;

        // Étape 3:
        double *vect3 = malloc(8 * sizeof(double));
        vect3[0] = vect2[0] + vect2[2];
        vect3[1] = vect2[0] - vect2[2];
        vect3[2] = squar * c6 * vect2[3] + squar * s6 * vect2[1];
        vect3[3] = -squar * s6 * vect2[1] + squar * c6 * vect2[3];
        vect3[4] = vect2[4] + vect2[7];
        vect3[5] = vect2[4] - vect2[7];
        vect3[6] = vect2[5] - vect2[6];
        vect3[7] = vect2[5] + vect2[6];

        // Étape 4:
        auxi2[0][j] = vect3[0] / 8;
        auxi2[4][j] = vect3[1] / 8;
        auxi2[2][j] = vect3[2] / 8;
        auxi2[6][j] = vect3[3] / 8;
        auxi2[7][j] = (vect3[7] - vect3[4]) / 8;
        auxi2[1][j] = (vect3[7] + vect3[4]) / 8;
        auxi2[3][j] = (squar * vect3[6]) / 8;
        auxi2[5][j] = (squar * vect3[5]) / 8;
    }

    for(uint8_t i = 0; i < 8; i++){
        dct_loeffler[i] = malloc(8 * sizeof(double));
        for(uint8_t j = 0; j < 8; j++){
            dct_loeffler[i][j] = (int16_t) auxi2[i][j];
        }
    }

    return dct_loeffler;
}

/*
    Calcul de la dct d'un bloc en précisant la méthode à suivre dans la paramètre *dct_used.
*/
int16_t **dct(uint8_t **bloc, char *dct_used)
{
    if (strcmp(dct_used, "loeffler") == 0)
    {
        return dct_loeffler(bloc);
    }
    return dct_naive(bloc);
}

/*
    Libère l'espace mémoire associé à un matrice uint8_t
*/
void dct_free(int16_t **dct_values)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        free(dct_values[i]);
    }
    free(dct_values);
}
