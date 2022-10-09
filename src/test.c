#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <getopt.h>
#include <ctype.h>

#include "decoupage.h"
#include "dct.h"
#include "quantization.h"
#include "zigzag.h"
#include "qtables.h"
#include "downsampling.h"

/*
    Fonction utilisée pour vérifieer le bon fonctionnement de différents modules ".c" et ".h"
    écrits lors de notre travail sur ce projet.
*/

int old_main(void)
{
    // FILE *ppm_image;
    FILE *test_file;
    test_file = fopen("./images/test.bla", "w");
    /* on choisit une image pour faire les tests et debugger */
    const char *filename = "./images/zig-zag.jpg";
    /* Pour voir l'image il suffit de l'ouvrir sous vscode et changer le magic number */
    struct ppm *ppm = ppm_create(filename, 1, 1);
    uint16_t hauteur = ppm->hauteur;
    uint16_t largeur = ppm->largeur;
    uint16_t new_height = hauteur + ppm->v_remainder;
    uint16_t new_width = largeur + ppm->h_remainder;
    fprintf(test_file, "Magic number : %s\n", ppm->magic_number);
    fprintf(test_file, "Largeur : %u, Hauteur : %u\n", largeur, hauteur);
    fprintf(test_file, "Color_range : %u\n", ppm->color_range);
    fprintf(test_file, "Valeurs en rgb : \n");

    //Test
    printf("Magic number : %s\n", ppm->magic_number);
    printf("Largeur : %u, Hauteur : %u\n", largeur, hauteur);
    printf("Color_range : %u\n", ppm->color_range);

    for (uint16_t i = 0; i < new_height; i++)
    {
        for (uint16_t j = 0; j < new_width; j++)
        {
            struct rgb rgb_color = ppm->rgb_values[i][j];
            fprintf(test_file, "%02" PRIx16 " ", (uint8_t)rgb_color.red);
            // fprintf(test_file, "%hx ", rgb_color.red);
        }
        fprintf(test_file, "\n");
    }
    fprintf(test_file, "\n");
    fprintf(test_file, "\n");
    bool gray_scale = is_gray_scale(ppm);
    struct YCbCr **ycbcr_values = convert(ppm->rgb_values, new_height, new_width, gray_scale);
    // for (uint16_t i = 0; i < hauteur; i++)
    // {
    //     for (uint16_t j = 0; j < largeur; j++)
    //     {
    //         struct YCbCr ycbcr_color = ycbcr_values[i][j];
    //         printf("%u %u %u\n", ycbcr_color.Y, ycbcr_color.Cb, ycbcr_color.Cr);
    //     }
    // }
    struct bloc **blocs = decoupe_blocs(ycbcr_values, new_height, new_width);
    uint16_t hauteur_blocs = hauteur / 8;
    uint16_t largeur_blocs = largeur / 8;
    for (uint16_t i = 0; i < hauteur_blocs; i++)
    {
        for (uint16_t j = 0; j < largeur_blocs; j++)
        {
            struct bloc bloc1 = blocs[i][j];
            fprintf(test_file, "Valeurs en YCbCr : \n");
            uint8_t **Y = matrice_Y(&bloc1);
            uint8_t **Cb = matrice_Cb(&bloc1);
            for (uint8_t s = 0; s < 8; s++)
            {
                for (uint8_t t = 0; t < 8; t++)
                {
                    fprintf(test_file, "%02" PRIx16 " ", (uint8_t)Y[s][t]);
                }
                fprintf(test_file, "\n");
            }
            fprintf(test_file, "\n");
            for (uint8_t s = 0; s < 8; s++)
            {
                for (uint8_t t = 0; t < 8; t++)
                {
                    fprintf(test_file, "%02" PRIx16 " ", (uint8_t)Cb[s][t]);
                }
                fprintf(test_file, "\n");
            }
            fprintf(test_file, "\n");
            int16_t **dct_values = dct(Y, "naive");
            int16_t **dct_Cb = dct(Cb, "naive");
            matrix_free(Y);
            matrix_free(Cb);
            fprintf(test_file, "TCD : \n");
            for (uint8_t s = 0; s < 8; s++)
            {
                for (uint8_t t = 0; t < 8; t++)
                {
                    fprintf(test_file, "%04" PRIx16 " ", (uint16_t)dct_values[s][t]);
                }
                fprintf(test_file, "\n");
            }
            fprintf(test_file, "\n");
            for (uint8_t s = 0; s < 8; s++)
            {
                for (uint8_t t = 0; t < 8; t++)
                {
                    fprintf(test_file, "%04" PRIx16 " ", (uint16_t)dct_Cb[s][t]);
                }
                fprintf(test_file, "\n");
            }
            fprintf(test_file, "\n");
            fprintf(test_file, "ZZ : \n");
            int16_t vecteur[64] = {0};
            int16_t vecteur_Cb[64] = {0};
            zigzag(dct_values, vecteur);
            zigzag(dct_Cb, vecteur_Cb);
            for (uint8_t i = 0; i < 64; i += 8)
            {
                for (uint8_t j = 0; j < 8; j++)
                {
                    fprintf(test_file, "%04" PRIx16 " ", (uint16_t)vecteur[i + j]);
                }
                fprintf(test_file, "\n");
            }
            fprintf(test_file, "\n");
            for (uint8_t i = 0; i < 64; i += 8)
            {
                for (uint8_t j = 0; j < 8; j++)
                {
                    fprintf(test_file, "%04" PRIx16 " ", (uint16_t)vecteur_Cb[i + j]);
                }
                fprintf(test_file, "\n");
            }
            dct_free(dct_values);
            dct_free(dct_Cb);
            fprintf(test_file, "\n");
            fprintf(test_file, "\n");
            fprintf(test_file, "Quantification : \n");
            int16_t *quantified = quantization(vecteur, quantification_table_Y);
            int16_t *quantified_Cb = quantization(vecteur, quantification_table_CbCr);
            for (uint8_t i = 0; i < 64; i += 8)
            {
                for (uint8_t j = 0; j < 8; j++)
                {
                    fprintf(test_file, "%04" PRIx16 " ", (uint16_t)quantified[i + j]);
                }
                fprintf(test_file, "\n");
            }
            fprintf(test_file, "\n");
            for (uint8_t i = 0; i < 64; i += 8)
            {
                for (uint8_t j = 0; j < 8; j++)
                {
                    fprintf(test_file, "%04" PRIx16 " ", (uint16_t)quantified_Cb[i + j]);
                }
                fprintf(test_file, "\n");
            }
            free(quantified);
            free(quantified_Cb);
        }
    }
    fprintf(test_file, "\n");
    fprintf(test_file, "\n");
    // struct mcu **mcus = decoupe_mcus(blocs, largeur_blocs, hauteur_blocs, 2, 2);
    // for (uint16_t i = 0; i < hauteur_blocs / 2; i++)
    // {
    //     for (uint16_t j = 0; j < largeur_blocs / 2; j++)
    //     {
    //         struct bloc **mcu_bloc = mcus[i][j].blocs;
    //         for (uint8_t k = 0; k < 2; k++)
    //         {
    //             for (uint8_t l = 0; l < 2; l++)
    //             {
    //                 struct bloc current = mcu_bloc[k][l];
    //                 bloc_print(&current, test_file);
    //             }
    //         }
    //         fprintf(test_file, "\n");
    //     }
    //     fprintf(test_file, "\n");
    // }
    fclose(test_file);
    ppm_free(ppm);
    ycbcr_free(ycbcr_values, hauteur);
    blocs_free(blocs, hauteur_blocs, largeur_blocs);
    // mcu_free(mcus, hauteur_blocs / 2, largeur_blocs / 2);
    // FILE *jpeg_image;
    // char *new_name;
    // new_name = change_extentension(filename, new_name);
    // jpeg_image = fopen(new_name, "w");
    // printf("%s\n", new_name);
    // fclose(jpeg_image);
    // struct jpeg *test = jpeg_create();
    // jpeg_set_ppm_filename(test, filename);
    // char *jpeg_name = jpeg_get_ppm_filename(test);
    // printf("%s\n", jpeg_name);
    return EXIT_SUCCESS;
}
