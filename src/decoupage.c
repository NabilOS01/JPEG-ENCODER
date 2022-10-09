#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "decoupage.h"
#include "dct.h"
#include "htables.h"
#include "my_huffman.h"
#include "my_bitstream.h"
#define _USE_MATH_DEFINES
#define PI 3.141592653


/*
    Indique si une image est en niveaux de gris ou en couleurs
*/
bool is_gray_scale(struct ppm *ppm)
{
    if (strcmp(ppm->magic_number, "P5") == 0)
    {
        return true;
    }
    return false;
}

/*
    Crée une structure ppm à partir d'un fichier ppm dont le nom est spécifié en paramètre
*/
struct ppm *ppm_create(const char *ppm_filename, uint8_t h, uint8_t v)
{
    struct ppm *ppm = malloc(sizeof(struct ppm));
    FILE *ppm_file;
    ppm_file = fopen(ppm_filename, "rb");
    fscanf(ppm_file, "%s\n", ppm->magic_number);
    fscanf(ppm_file, "%d %d\n", &(ppm->largeur), &(ppm->hauteur));
    fscanf(ppm_file, "%d\n", &(ppm->color_range));
    /* Stockage des pixels */
    uint8_t pixel;
    uint8_t v_remainder = (8 * v - (ppm->hauteur % (8 * v))) % (8 * v);
    uint8_t h_remainder = (8 * h - (ppm->largeur % (8 * h))) % (8 * h);
    struct rgb **matrice = malloc((ppm->hauteur + v_remainder) * sizeof(struct rgb *));
    uint32_t i = 0; // ligne
    while (i < ppm->hauteur)
    {
        matrice[i] = malloc((ppm->largeur + h_remainder) * sizeof(struct rgb));
        uint32_t j = 0; // colonne
        while (j < ppm->largeur)
        {
            if (strcmp(ppm->magic_number, "P5") == 0)
            {
                fread(&pixel, sizeof(uint8_t), 1, ppm_file);
                matrice[i][j].blue = pixel;  // bleue
                matrice[i][j].green = pixel; // vert
                matrice[i][j].red = pixel;   // rouge
                j++;
            }
            else
            {
                /* Le premier octet est pour le rouge, le second pour le vert et le troisième pour le bleu */
                fread(&pixel, sizeof(uint8_t), 1, ppm_file);
                matrice[i][j].red = pixel;
                fread(&pixel, sizeof(uint8_t), 1, ppm_file);
                matrice[i][j].green = pixel;
                fread(&pixel, sizeof(uint8_t), 1, ppm_file);
                matrice[i][j].blue = pixel;
                j++;
            }
        }
        i++;
    }
    /* On complète les pixels dans le cas d'une troncature */
    for (uint8_t i = 0; i < v_remainder; i++)
    {
        matrice[ppm->hauteur + i] = malloc((ppm->largeur + h_remainder) * sizeof(struct rgb));
    }
    for (uint8_t i = 0; i < v_remainder; i++)
    {
        for (uint32_t j = 0; j < ppm->largeur; j++)
        {
            matrice[ppm->hauteur + i][j] = matrice[ppm->hauteur - 1][j];
        }
    }
    for (uint32_t i = 0; i < ppm->hauteur + v_remainder; i++)
    {
        for (uint8_t j = 0; j < h_remainder; j++)
        {
            matrice[i][ppm->largeur + j] = matrice[i][ppm->largeur - 1];
        }
    }
    ppm->rgb_values = matrice;
    ppm->v_remainder = v_remainder;
    ppm->h_remainder = h_remainder;
    fclose(ppm_file);
    return ppm;
}

/* Convertit une couleur rgb en une couleur YCbCr */
struct YCbCr *RGB2YCbCr(struct rgb *rgb_color, bool gray_scale)
{
    uint8_t R = rgb_color->red;
    uint8_t G = rgb_color->green;
    uint8_t B = rgb_color->blue;
    uint8_t Y, Cb, Cr;
    if (gray_scale)
    {
        Y = R;
        Cb = 128;
        Cr = 128;
    }
    else
    {
        Y = (0.2990 * R + 0.5870 * G + 0.1140 * B);
        Cb = (-0.1687 * R - 0.3313 * G + 0.5000 * B + 128);
        Cr = (0.5000 * R - 0.4187 * G - 0.0813 * B + 128);
    }
    struct YCbCr *ycbcr_color = malloc(sizeof(struct YCbCr));
    ycbcr_color->Y = Y;
    ycbcr_color->Cb = Cb;
    ycbcr_color->Cr = Cr;
    return ycbcr_color;
}

/*
    Convertit les valeurs RGB de rgb_values en YCbCr et les stocke dans une matrice
*/
struct YCbCr **convert(struct rgb **rgb, uint16_t hauteur, uint16_t largeur, bool gray_scale)
{
    struct YCbCr **converted = malloc(hauteur * sizeof(struct YCbCr *));
    for (uint16_t i = 0; i < hauteur; i++)
    {
        converted[i] = malloc(largeur * sizeof(struct YCbCr));
        for (uint16_t j = 0; j < largeur; j++)
        {
            struct rgb rgb_color = rgb[i][j];
            struct YCbCr *ycbcr_color = RGB2YCbCr(&rgb_color, gray_scale);
            converted[i][j] = *ycbcr_color;
            free(ycbcr_color);
        }
    }
    return converted;
}

/*
    Découpe la matrice de couleurs YCbCr en blocs de taille 8x8 et retourne une matrice contenant ces blocs
*/
struct bloc **decoupe_blocs(struct YCbCr **ycbcr, uint16_t hauteur, uint16_t largeur)
{
    uint16_t vertical = (uint16_t)hauteur / 8;
    uint16_t horizontal = (uint16_t)largeur / 8;
    struct bloc **blocs = malloc(vertical * sizeof(struct bloc *));
    uint16_t lines_index = 0;
    uint16_t columns_index = 0;
    while (lines_index < hauteur - 7)
    {
        uint16_t bloc_i = (uint16_t)lines_index / 8;
        blocs[bloc_i] = malloc(horizontal * sizeof(struct bloc));
        while (columns_index < largeur - 7)
        {
            struct bloc *single_bloc = malloc(sizeof(struct bloc));
            single_bloc->y_cb_cr = malloc(8 * sizeof(struct YCbCr *));
            for (uint16_t i = 0; i < 8; i++)
            {
                single_bloc->y_cb_cr[i] = malloc(8 * sizeof(struct YCbCr));
                for (uint16_t j = 0; j < 8; j++)
                {
                    single_bloc->y_cb_cr[i][j] = ycbcr[lines_index + i][columns_index + j];
                }
            }
            uint16_t bloc_j = (uint16_t)columns_index / 8;
            blocs[bloc_i][bloc_j] = *single_bloc;
            columns_index += 8;
            free(single_bloc);
        }
        columns_index = 0;
        lines_index += 8;
    }
    return blocs;
}

/*
    Retourne la matrice de luminance d'un bloc
*/
uint8_t **matrice_Y(struct bloc *bloc)
{
    uint8_t **matrice_Y = malloc(8 * sizeof(uint8_t *));
    for (uint8_t i = 0; i < 8; i++)
    {
        matrice_Y[i] = malloc(8 * sizeof(uint8_t));
        for (uint8_t j = 0; j < 8; j++)
        {
            struct YCbCr ycbcr = bloc->y_cb_cr[i][j];
            matrice_Y[i][j] = ycbcr.Y;
        }
    }
    return matrice_Y;
}

/*
    Retourne la matrice de crominance bleue d'un bloc
*/
uint8_t **matrice_Cb(struct bloc *bloc)
{
    uint8_t **matrice_Cb = malloc(8 * sizeof(uint8_t *));
    for (uint8_t i = 0; i < 8; i++)
    {
        matrice_Cb[i] = malloc(8 * sizeof(uint8_t));
        for (uint8_t j = 0; j < 8; j++)
        {

            struct YCbCr ycbcr = bloc->y_cb_cr[i][j];
            matrice_Cb[i][j] = ycbcr.Cb;
        }
    }
    return matrice_Cb;
}

/*
    Retourne la matrice de chrominance rouge d'un bloc
*/
uint8_t **matrice_Cr(struct bloc *bloc)
{
    uint8_t **matrice_Cr = malloc(8 * sizeof(uint8_t *));
    for (uint8_t i = 0; i < 8; i++)
    {
        matrice_Cr[i] = malloc(8 * sizeof(uint8_t));
        for (uint8_t j = 0; j < 8; j++)
        {

            struct YCbCr ycbcr = bloc->y_cb_cr[i][j];
            matrice_Cr[i][j] = ycbcr.Cr;
        }
    }
    return matrice_Cr;
}


/*
    Prend en entrée une matrice de blocs et la regroupe en mcus de dimension hv chacun
    Pour notre cas simple (photo invaders), on aura h = v = 1
*/

struct mcu **decoupe_mcus(struct bloc **blocs, uint16_t largeur, uint16_t hauteur, uint8_t h, uint8_t v)
{
    struct mcu **mcus = malloc(hauteur * sizeof(struct mcu *));
    uint16_t lines_index = 0;
    uint16_t columns_index = 0;
    while (lines_index < hauteur - v + 1)
    {
        uint16_t mcu_i = (uint16_t)lines_index / v;
        mcus[mcu_i] = malloc(largeur * sizeof(struct mcu));
        while (columns_index < largeur - h + 1)
        {
            struct mcu *single_mcu = malloc(sizeof(struct mcu));
            single_mcu->blocs = malloc(v * sizeof(struct bloc *));
            for (uint16_t i = 0; i < v; i++)
            {
                single_mcu->blocs[i] = malloc(h * sizeof(struct bloc));
                for (uint16_t j = 0; j < h; j++)
                {
                    single_mcu->blocs[i][j] = blocs[lines_index + i][columns_index + j];
                }
            }
            single_mcu->h = h;
            single_mcu->v = v;
            uint16_t mcu_j = (uint16_t)columns_index / h;
            mcus[mcu_i][mcu_j] = *single_mcu;
            columns_index += h;
            free(single_mcu);
        }
        columns_index = 0;
        lines_index += v;
    }
    return mcus;
}

/*
    Fonction pour écrire la matrice Y d'un bloc. Utilisée uniquement pour les tests et debuggage
*/
void bloc_print_Y(struct bloc *to_print, FILE *stream)
{
    uint8_t **Y = matrice_Y(to_print);
    for (uint8_t i = 0; i < 8; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            fprintf(stream, "%02" PRIx16 " ", (uint8_t)Y[i][j]);
        }
        fprintf(stream, "\n");
    }
    matrix_free(Y);
}


void bloc_print_Cb(struct bloc *to_print, FILE *stream)
{
    uint8_t **Cb = matrice_Cb(to_print);
    for (uint8_t i = 0; i < 8; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            fprintf(stream, "%02" PRIx16 " ", (uint8_t)Cb[i][j]);
        }
        fprintf(stream, "\n");
    }
    matrix_free(Cb);
}


void bloc_print_Cr(struct bloc *to_print, FILE *stream)
{
    uint8_t **Cr = matrice_Cr(to_print);
    for (uint8_t i = 0; i < 8; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            fprintf(stream, "%02" PRIx16 " ", (uint8_t)Cr[i][j]);
        }
        fprintf(stream, "\n");
    }
    matrix_free(Cr);
}


/*
    Libère l'espace mémoire associé à struct ppm
*/
void ppm_free(struct ppm *ppm)
{
    for (uint32_t i = 0; i < ppm->hauteur + ppm->v_remainder; i++)
    {
        free(ppm->rgb_values[i]);
    }
    free(ppm->rgb_values);
    free(ppm);
}


/*
    Libère l'espace mémoire associé à une matrice ycbcr
*/
void ycbcr_free(struct YCbCr **ycbcr, uint16_t hauteur)
{
    for (uint16_t i = 0; i < hauteur; i++)
    {
        free(ycbcr[i]);
    }
    free(ycbcr);
}


/*
    Libère l'espace mémoire associé à une matrice uint8_t
*/
void matrix_free(uint8_t **matrice)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        free(matrice[i]);
    }
    free(matrice);
}


/*
    Libère l'espace mémoire associé à une matrice uint8_t
*/
void blocs_free(struct bloc **blocs, uint16_t hauteur, uint16_t largeur)
{
    for (uint16_t i = 0; i < hauteur; i++)
    {
        for (uint16_t j = 0; j < largeur; j++)
        {
            ycbcr_free(blocs[i][j].y_cb_cr, 8);
        }
        free(blocs[i]);
    }
    free(blocs);
}

/*
    Libère l'espace mémoire associé à un matrice uint8_t
*/
void mcu_free(struct mcu **mcus, uint16_t hauteur, uint16_t largeur)
{
    for (uint16_t i = 0; i < hauteur; i++)
    {
        for (uint16_t j = 0; j < largeur; j++)
        {
            blocs_free(mcus[i][j].blocs, mcus[i][j].v, mcus[i][j].h);
        }
        free(mcus[i]);
    }
    free(mcus);
}
