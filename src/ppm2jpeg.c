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
#include "encode_acdc.h"
#include "quantization.h"
#include "zigzag.h"
#include "quantization.h"
#include "dct.h"
#include "encode_acdc.h"
#include "qtables.h"
#include "htables.h"
#include "my_jpeg_writer.h"
#include "my_bitstream.h"
#include "my_huffman.h"
#include "downsampling.h"
#include "ppm2jpeg.h"

/*
    transforme le nom d'un fichier ppm ou pgm en celui d'un fichier portant
    le même nom, mais avec l'extension .jpeg au lieu de .ppm ou .pgm
*/
static char *change_extentension(char *name, char *new_name)
{
    uint8_t length = strlen(name);
    for (uint8_t i = 0; i <= length - 4; i++)
    {
        new_name[i] = name[i];
    }
    new_name[length - 3] = *"j";
    new_name[length - 2] = *"p";
    new_name[length - 1] = *"g";
    new_name[length] = *"\0";
    return new_name;
}

/*
    Vérifie la syntaxe de l'argument sample et enregistre dans une structure arg
*/
static int verify_sample(struct arg *sample, char *arg)
{
    if (strlen(arg) != 11)
    {
        printf("Wrong sampling factors, try --help for more.\n");
        return 0;
    }
    else
    {
        for (uint8_t i = 0; i < 6; i++)
        {
            if (!isdigit(arg[2*i]) || atoi(&arg[2*i]) > 4 || atoi(&arg[2*i]) < 1)
            {
                printf("Sampling factors have to be in the form hxv where h and v are integers between 1 and 4.\n");
                return 0;
            }
        }
        if (arg[1] != 'x' || arg[5] != 'x' || arg[9] != 'x')
        {
            printf("Sampling factors have to be in the form hxv.\n");
            return 0;
        }
        if (arg[3] != ',' || arg[7] != ',')
        {
            printf("Wrong format ! Expected : hxv,h2xv2,h3xv3.\n");
            return 0;
        }
    }
    sample->h1 = atoi(&arg[0]);
    sample->v1 = atoi(&arg[2]);
    sample->h2 = atoi(&arg[4]);
    sample->v2 = atoi(&arg[6]);
    sample->h3 = atoi(&arg[8]);
    sample->v3 = atoi(&arg[10]);
    return 1;
}

/*
    Affiche le menu d'aide
*/
static void print_help(void)
{
    printf("Usage : ./ppm2jpeg [options] image_to_convert.{ppm, pgm}\n");
    printf("options are the following : \n");
    printf("--sample=h1xv1,h2xv2,h3xv3 : Sets horizontal and vertical sampling factors for\n");
    printf("                             Y, Cb and Cr.\n");
    printf("                             Example : --sample=2x2,1x1,1x1\n");
    printf("--outfile=filename : Sets the name of the output file to filename\n");
    printf("--dct={naive, loeffler} : Sets the method used to compute dct\n");
    printf("--help : Prints this help.\n");
}

/*
    Vérifie que le fichier d'entrée est bien un fichier ppm ou pgm
*/
static bool verify_extension(char *input, bool *gray_scale)
{
    char ignore = 0;
    if (input[0] == '.')
    {
        ignore++;
    }
    if (!strcmp(strchr(input + ignore, '.'), ".ppm"))
    {
        *gray_scale = false;
        return true;
    }
    else if (!strcmp(strchr(input + ignore, '.'), ".pgm"))
    {
        *gray_scale = true;
        return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    int option_char, option_index;
    char *ppm_filename;
    struct arg *sample = malloc(sizeof(struct arg));
    sample->h1 = 0;
    sample->h2 = 0;
    sample->h3 = 0;
    sample->v1 = 0;
    sample->v2 = 0;
    sample->v3 = 0;
    sample->outfile = NULL;
    sample->dct = "naive";
    int status;
    static struct option long_options[] = {
        {"sample", required_argument, 0, 's'},
        {"outfile", required_argument, 0, 'o'},
        {"dct", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {NULL, 0, 0, 0}
    };
    while ((option_char = getopt_long(argc, argv, "s:o:d:h", long_options, &option_index)) != -1)
    {
        switch (option_char) {
            case (int)'?':
                printf("Error : Try --help for more information\n");
                exit(1);
                break;
            case (int)'s':
                status = verify_sample(sample, optarg);
                if (status == 0)
                {
                    return EXIT_FAILURE;
                }
                break;
            case (int)'o':
                sample->outfile = optarg;
                printf("%s\n", sample->outfile);
                break;
            case (int)'d':
                sample->dct = optarg;
                break;
            case (int)'h':
                print_help();
                exit(0);
                break;
            default:
                return 1;
        }
    }
    uint8_t h1 = sample->h1 > 0 ? sample->h1 :1;
    uint8_t v1 = sample->v1 > 0 ? sample->v1 :1;
    uint8_t h2 = sample->h2 > 0 ? sample->h2 :1;
    uint8_t v2 = sample->v2 > 0 ? sample->v2 :1;
    uint8_t h3 = sample->h3 > 0 ? sample->h3 :1;
    uint8_t v3 = sample->v3 > 0 ? sample->v3 :1;
    if (h1 > 0 && h2 > 0 && h3 > 0 && v1 > 0 && v2 > 0 && v3 > 0)
    {
        if (h1 * v1 + h2 * v2 + h3 * v3 > 10)
        {
            printf("La somme des hxv doit être <= 10.\n");
            return EXIT_FAILURE;
        }
        if (h1 % h2 != 0 || h1 % h3 != 0 || v1 % v2 != 0 || v1 % v3 != 0)
        {
            printf("Les facteurs d'échantillonage des chrominances doivent diviser ceux de la luminance.\n");
            return EXIT_FAILURE;
        }
    }
    if (optind < argc)
    {
        ppm_filename = argv[optind];
        optind++;
    }
    bool gray_scale;
    if (!verify_extension(ppm_filename, &gray_scale))
    {
        printf("Le fichier que vous avez entré n'est pas un fichier ppm ou pgm.\n");
        return EXIT_FAILURE;
    }
    if (gray_scale)
    {
        if (h1 != 1 || h2 != 1 || h3 != 1 || v1 != 1 || v2 != 1 || v3 != 1)
        {
            printf("Le fichier entré est en niveau de gris, les sampling factors seront remis à 1.\n");
        }
        h1 = h2 = h3 = v1 = v2 = v3 = 1;
    }
    char *dct_used = sample->dct;
    struct ppm *ppm = ppm_create(ppm_filename, h1, v1);
    uint16_t hauteur = ppm->hauteur;
    uint16_t largeur = ppm->largeur;
    uint16_t new_height = hauteur + ppm->v_remainder;
    uint16_t new_width = largeur + ppm->h_remainder;
    // gray_scale = is_gray_scale(ppm);
    
    struct jpeg *jpeg = jpeg_create();
    jpeg_set_ppm_filename(jpeg, ppm_filename);
    
    char *jpeg_filename;
    if (sample->outfile == NULL)
    {
        jpeg_filename = malloc((strlen(ppm_filename)+1)*sizeof(char));
        jpeg_filename = change_extentension(ppm_filename, jpeg_filename);
    }
    else
    {
        jpeg_filename = malloc((strlen(sample->outfile)+1)*sizeof(char));
        strcpy(jpeg_filename, sample->outfile);
    }
    free(sample);
    jpeg_set_jpeg_filename(jpeg, jpeg_filename);
    jpeg_set_image_width(jpeg, largeur);
    jpeg_set_image_height(jpeg, hauteur);
    jpeg_set_nb_components(jpeg, (gray_scale ? 1 :3));
    
    jpeg_set_sampling_factor(jpeg,0,0,h1);
    jpeg_set_sampling_factor(jpeg,1,0,h2);
    jpeg_set_sampling_factor(jpeg,2,0,h3);

    jpeg_set_sampling_factor(jpeg,0,1,v1);
    jpeg_set_sampling_factor(jpeg,1,1,v2);
    jpeg_set_sampling_factor(jpeg,2,1,v3);
 
    //Table de Huffman pour l'échantillon DC et la composante Y
    struct huff_table *htable_DC_Y =  huffman_table_build(htables_nb_symb_per_lengths[0][0], htables_symbols[0][0], htables_nb_symbols[0][0]);
    jpeg_set_huffman_table(jpeg,0,0,htable_DC_Y);

    //Table de Huffman pour l'échantillon DC et la composante Cb
    struct huff_table *htable_DC_Cb =  huffman_table_build(htables_nb_symb_per_lengths[0][1], htables_symbols[0][1], htables_nb_symbols[0][1]);
    jpeg_set_huffman_table(jpeg,0,1,htable_DC_Cb);

    //Table de Huffman pour l'échantillon DC et la composante Cr
    struct huff_table *htable_DC_Cr =  huffman_table_build(htables_nb_symb_per_lengths[0][2], htables_symbols[0][2], htables_nb_symbols[0][2]);
    jpeg_set_huffman_table(jpeg,0,2,htable_DC_Cr);

    //Table de Huffman pour l'échantillon AC et la composante Y
    struct huff_table *htable_AC_Y =  huffman_table_build(htables_nb_symb_per_lengths[1][0], htables_symbols[1][0], htables_nb_symbols[1][0]);
    jpeg_set_huffman_table(jpeg,1,0,htable_AC_Y);

    //Table de Huffman pour l'échantillon AC et la composante Cb
    struct huff_table *htable_AC_Cb =  huffman_table_build(htables_nb_symb_per_lengths[1][1], htables_symbols[1][1], htables_nb_symbols[1][1]);
    jpeg_set_huffman_table(jpeg,1,1,htable_AC_Cb);

    //Table de Huffman pour l'échantillon AC et la composante Cr
    struct huff_table *htable_AC_Cr =  huffman_table_build(htables_nb_symb_per_lengths[1][2], htables_symbols[1][2], htables_nb_symbols[1][2]);
    jpeg_set_huffman_table(jpeg,1,2,htable_AC_Cr);
 
    // Tables de quantifications pour les composantes Y, Cb, Cr
    jpeg_set_quantization_table(jpeg,0,quantification_table_Y);
    jpeg_set_quantization_table(jpeg,1,quantification_table_CbCr);
    jpeg_set_quantization_table(jpeg,2,quantification_table_CbCr);

    jpeg_write_header(jpeg);

    struct bitstream *stream = jpeg_get_bitstream(jpeg);
    
    //Récupération et traitement des données de l'image ppm
    struct YCbCr **ycbcr_values = convert(ppm->rgb_values, new_height, new_width, gray_scale);
    struct bloc **blocs = decoupe_blocs(ycbcr_values, new_height, new_width);
    uint16_t hauteur_blocs = new_height / 8;
    uint16_t largeur_blocs = new_width / 8;
    int16_t predicateur_Y = 0;
    int16_t predicateur_Cb = 0;
    int16_t predicateur_Cr = 0;

    struct mcu **mcus = decoupe_mcus(blocs, largeur_blocs, hauteur_blocs, h1, v1);

     for (uint16_t i = 0; i < hauteur_blocs / v1; i++) 
     {
         for (uint16_t j = 0; j < largeur_blocs / h1; j++)
        {
            struct bloc **mcu_bloc = mcus[i][j].blocs;
            
            //Y
            for (uint8_t i=0; i<v1;i++) {
                for (uint8_t j=0; j<h1 ;j++){
                    uint8_t **bloc_Y = matrice_Y(&(mcu_bloc[i][j]));
                    int16_t **dct_Y = dct(bloc_Y, dct_used);
                    int16_t vecteur_Y[64] = {0};
                    zigzag(dct_Y, vecteur_Y);
                    int16_t *quantified_Y = quantization(vecteur_Y, quantification_table_Y);
                    int16_t coeff_Y = quantified_Y[0] - predicateur_Y;
                    encode_DC(stream, htable_DC_Y, coeff_Y);
                    encode_vect_AC(stream, htable_AC_Y, quantified_Y);
                    predicateur_Y = quantified_Y[0];
                    dct_free(dct_Y);
                    free(quantified_Y);
                    matrix_free(bloc_Y);
                }
            }

            if (!gray_scale){
                // Traitement matrice Cb
                uint8_t **bloc00, **bloc01, **bloc10, **bloc11;
                uint8_t **down1_Cb, **down2_Cb, **vertical_Cb;
                if (h1 > 1 && v1 > 1)
                {
                    if (h2 == h1 && v2 == v1)
                    {
                        for (uint8_t s = 0; s < v2; s++)
                        {
                            for (uint8_t t = 0; t < h2; t++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[s][t]));
                                int16_t **dct_Cb = dct(bloc00, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                            }
                        }
                    }
                    else if ((h1 == 2) && (v1 == 2))
                    {
                        if ((h2 == 1) && (v2 == 1))
                        {
                            bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[0][1]));
                            bloc10 = matrice_Cb(&(mcu_bloc[1][0]));
                            bloc11 = matrice_Cb(&(mcu_bloc[1][1]));
                            down1_Cb = horizontal_downsampling(bloc00, bloc01);
                            down2_Cb = horizontal_downsampling(bloc10, bloc11);
                            vertical_Cb = horizontal_vertical_downsampling(down1_Cb, down2_Cb);
                            int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                            int16_t vecteur_Cb[64] = {0};
                            zigzag(dct_Cb, vecteur_Cb);
                            int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                            int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                            encode_DC(stream, htable_DC_Cb, coeff_Cb);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                            predicateur_Cb = quantified_Cb[0];
                            dct_free(dct_Cb);
                            free(quantified_Cb);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(bloc11);
                            matrix_free(down1_Cb);
                            matrix_free(down2_Cb);
                            matrix_free(vertical_Cb);
                        }
                        else if (h2 == 1)
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[s][0]));
                                bloc01 = matrice_Cb(&(mcu_bloc[s][1]));
                                down1_Cb = horizontal_downsampling(bloc00, bloc01);
                                int16_t **dct_Cb = dct(down1_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(down1_Cb);
                            }
                        }
                        else
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[0][s]));
                                bloc10 = matrice_Cb(&(mcu_bloc[1][s]));
                                vertical_Cb = horizontal_vertical_downsampling(bloc00, bloc10);
                                int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc10);
                                matrix_free(vertical_Cb);
                            }
                        }
                    }
                    else if ((v1 == 3) && (h1 == 2))
                    {
                        if ((h2 == 1) && (v2 == 1))
                        {
                            bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[0][1]));
                            down1_Cb = horizontal_downsampling(bloc00, bloc01);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            bloc00 = matrice_Cb(&(mcu_bloc[1][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[1][1]));
                            down2_Cb = horizontal_downsampling(bloc00, bloc01);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            bloc00 = matrice_Cb(&(mcu_bloc[2][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[2][1]));
                            bloc10 = horizontal_downsampling(bloc00, bloc01);
                            vertical_Cb = horizontal_vertical_downsampling_triple(down1_Cb, down2_Cb, bloc10);
                            int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                            int16_t vecteur_Cb[64] = {0};
                            zigzag(dct_Cb, vecteur_Cb);
                            int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                            int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                            encode_DC(stream, htable_DC_Cb, coeff_Cb);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                            predicateur_Cb = quantified_Cb[0];
                            dct_free(dct_Cb);
                            free(quantified_Cb);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(down1_Cb);
                            matrix_free(down2_Cb);
                            matrix_free(vertical_Cb);
                        }
                        else if ((h2 == 2) && (v2 == 1))
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[0][s]));
                                bloc01 = matrice_Cb(&(mcu_bloc[1][s]));
                                bloc10 = matrice_Cb(&(mcu_bloc[2][s]));
                                vertical_Cb = horizontal_vertical_downsampling_triple(bloc00, bloc01, bloc10);
                                int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(bloc10);
                                matrix_free(vertical_Cb);
                            }
                        }
                        else if ((h2 == 1) && (v2 == 3))
                        {
                            for (uint8_t s = 0; s < 3; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[s][0]));
                                bloc01 = matrice_Cb(&(mcu_bloc[s][1]));
                                vertical_Cb = horizontal_downsampling(bloc00, bloc01);
                                int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(vertical_Cb);
                            }
                        }
                    }
                    else if ((h1 == 3) && (v1 == 2))
                    {
                        if ((h2 == 1) && (v2 == 1))
                        {
                            bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[0][1]));
                            bloc10 = matrice_Cb(&(mcu_bloc[0][2]));
                            down1_Cb = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            bloc00 = matrice_Cb(&(mcu_bloc[1][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[1][1]));
                            bloc10 = matrice_Cb(&(mcu_bloc[1][2]));
                            down2_Cb = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                            vertical_Cb = horizontal_vertical_downsampling(down1_Cb, down2_Cb);
                            int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                            int16_t vecteur_Cb[64] = {0};
                            zigzag(dct_Cb, vecteur_Cb);
                            int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                            int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                            encode_DC(stream, htable_DC_Cb, coeff_Cb);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                            predicateur_Cb = quantified_Cb[0];
                            dct_free(dct_Cb);
                            free(quantified_Cb);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(down1_Cb);
                            matrix_free(down2_Cb);
                            matrix_free(vertical_Cb);
                        }
                        else if ((h2 == 1) && (v2 == 2))
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[s][0]));
                                bloc01 = matrice_Cb(&(mcu_bloc[s][1]));
                                bloc10 = matrice_Cb(&(mcu_bloc[s][2]));
                                vertical_Cb = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                                int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(bloc10);
                                matrix_free(vertical_Cb);
                            }
                        }
                        else if ((h2 == 3) && (v2 == 1))
                        {
                            for (uint8_t s = 0; s < 3; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[0][s]));
                                bloc01 = matrice_Cb(&(mcu_bloc[1][s]));
                                vertical_Cb = horizontal_vertical_downsampling(bloc00, bloc01);
                                int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(vertical_Cb);
                            }
                        }
                    }
                    else if ((h1 == 4) && (v1 == 2))
                    {
                        bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cb(&(mcu_bloc[0][1]));
                        bloc10 = matrice_Cb(&(mcu_bloc[0][2]));
                        bloc11 = matrice_Cb(&(mcu_bloc[0][3]));
                        down1_Cb = horizontal_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        bloc00 = matrice_Cb(&(mcu_bloc[1][0]));
                        bloc01 = matrice_Cb(&(mcu_bloc[1][1]));
                        bloc10 = matrice_Cb(&(mcu_bloc[1][2]));
                        bloc11 = matrice_Cb(&(mcu_bloc[1][3]));
                        down2_Cb = horizontal_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        vertical_Cb = horizontal_vertical_downsampling(down1_Cb, down2_Cb); 
                        int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                        int16_t vecteur_Cb[64] = {0};
                        zigzag(dct_Cb, vecteur_Cb);
                        int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                        int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                        encode_DC(stream, htable_DC_Cb, coeff_Cb);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                        predicateur_Cb = quantified_Cb[0];
                        dct_free(dct_Cb);
                        free(quantified_Cb);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        matrix_free(down1_Cb);
                        matrix_free(down2_Cb);
                        matrix_free(vertical_Cb);                        
                    }
                    else if ((h1 == 2) && (v1 == 4))
                    {
                        bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cb(&(mcu_bloc[1][0]));
                        bloc10 = matrice_Cb(&(mcu_bloc[2][0]));
                        bloc11 = matrice_Cb(&(mcu_bloc[3][0]));
                        down1_Cb = horizontal_vertical_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        bloc00 = matrice_Cb(&(mcu_bloc[0][1]));
                        bloc01 = matrice_Cb(&(mcu_bloc[1][1]));
                        bloc10 = matrice_Cb(&(mcu_bloc[2][1]));
                        bloc11 = matrice_Cb(&(mcu_bloc[3][1]));
                        down2_Cb = horizontal_vertical_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        vertical_Cb = horizontal_downsampling(down1_Cb, down2_Cb); 
                        int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                        int16_t vecteur_Cb[64] = {0};
                        zigzag(dct_Cb, vecteur_Cb);
                        int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                        int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                        encode_DC(stream, htable_DC_Cb, coeff_Cb);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                        predicateur_Cb = quantified_Cb[0];
                        dct_free(dct_Cb);
                        free(quantified_Cb);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        matrix_free(down1_Cb);
                        matrix_free(down2_Cb);
                        matrix_free(vertical_Cb);  
                    }
                }
                else if (h1 > 1)
                {
                    if (h2 == h1)
                    {
                        for (uint8_t k = 0; k < h1; k++)
                        {
                            bloc00 = matrice_Cb(&(mcu_bloc[0][k]));
                            int16_t **dct_Cb = dct(bloc00, dct_used);
                            int16_t vecteur_Cb[64] = {0};
                            zigzag(dct_Cb, vecteur_Cb);
                            int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                            int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                            encode_DC(stream, htable_DC_Cb, coeff_Cb);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                            predicateur_Cb = quantified_Cb[0];
                            dct_free(dct_Cb);
                            free(quantified_Cb);
                            matrix_free(bloc00);
                        }
                    }
                    else if (h1 == 2)
                    {
                        bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cb(&(mcu_bloc[0][1]));
                        down1_Cb = horizontal_downsampling(bloc00, bloc01);
                        int16_t **dct_Cb = dct(down1_Cb, dct_used);
                        int16_t vecteur_Cb[64] = {0};
                        zigzag(dct_Cb, vecteur_Cb);
                        int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                        int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                        encode_DC(stream, htable_DC_Cb, coeff_Cb);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                        predicateur_Cb = quantified_Cb[0];
                        dct_free(dct_Cb);
                        free(quantified_Cb);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(down1_Cb);
                    }
                    else if (h1 == 3)
                    {
                        bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cb(&(mcu_bloc[0][1]));
                        bloc10 = matrice_Cb(&(mcu_bloc[0][2]));
                        down1_Cb = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                        int16_t **dct_Cb = dct(down1_Cb, dct_used);
                        int16_t vecteur_Cb[64] = {0};
                        zigzag(dct_Cb, vecteur_Cb);
                        int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                        int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                        encode_DC(stream, htable_DC_Cb, coeff_Cb);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                        predicateur_Cb = quantified_Cb[0];
                        dct_free(dct_Cb);
                        free(quantified_Cb);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(down1_Cb);
                    }
                    else if (h1 == 4)
                    {
                        if (h2 == 2)
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[0][2*s]));
                                bloc01 = matrice_Cb(&(mcu_bloc[0][2*s + 1]));
                                down1_Cb = horizontal_downsampling(bloc00, bloc01);
                                int16_t **dct_Cb = dct(down1_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(down1_Cb);
                            }
                        }
                        else
                        {
                            bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[0][1]));
                            bloc10 = matrice_Cb(&(mcu_bloc[0][2]));
                            bloc11 = matrice_Cb(&(mcu_bloc[0][3]));
                            down1_Cb = horizontal_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                            int16_t **dct_Cb = dct(down1_Cb, dct_used);
                            int16_t vecteur_Cb[64] = {0};
                            zigzag(dct_Cb, vecteur_Cb);
                            int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                            int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                            encode_DC(stream, htable_DC_Cb, coeff_Cb);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                            predicateur_Cb = quantified_Cb[0];
                            dct_free(dct_Cb);
                            free(quantified_Cb);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(bloc11);
                            matrix_free(down1_Cb);
                        }
                    }
                }
                else if (v1 > 1)
                {
                    if (v2 == v1)
                    {
                        for (uint8_t k = 0; k < v1; k++)
                        {
                            bloc00 = matrice_Cb(&(mcu_bloc[k][0]));
                            int16_t **dct_Cb = dct(bloc00, dct_used);
                            int16_t vecteur_Cb[64] = {0};
                            zigzag(dct_Cb, vecteur_Cb);
                            int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                            int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                            encode_DC(stream, htable_DC_Cb, coeff_Cb);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                            predicateur_Cb = quantified_Cb[0];
                            dct_free(dct_Cb);
                            free(quantified_Cb);
                            matrix_free(bloc00);
                        }
                    }
                    else if (v1 == 2)
                    {
                        bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                        bloc10 = matrice_Cb(&(mcu_bloc[1][0]));
                        vertical_Cb = horizontal_vertical_downsampling(bloc00, bloc10);
                        int16_t **dct_Cb = dct(vertical_Cb, dct_used);
                        int16_t vecteur_Cb[64] = {0};
                        zigzag(dct_Cb, vecteur_Cb);
                        int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                        int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                        encode_DC(stream, htable_DC_Cb, coeff_Cb);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                        predicateur_Cb = quantified_Cb[0];
                        dct_free(dct_Cb);
                        free(quantified_Cb);
                        matrix_free(bloc00);
                        matrix_free(bloc10);
                        matrix_free(vertical_Cb);
                    }
                    else if (v1 == 3)
                    {
                        bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cb(&(mcu_bloc[1][0]));
                        bloc10 = matrice_Cb(&(mcu_bloc[2][0]));
                        down1_Cb = horizontal_vertical_downsampling_triple(bloc00, bloc01, bloc10);
                        int16_t **dct_Cb = dct(down1_Cb, dct_used);
                        int16_t vecteur_Cb[64] = {0};
                        zigzag(dct_Cb, vecteur_Cb);
                        int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                        int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                        encode_DC(stream, htable_DC_Cb, coeff_Cb);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                        predicateur_Cb = quantified_Cb[0];
                        dct_free(dct_Cb);
                        free(quantified_Cb);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(down1_Cb);
                    }
                    else if (v1 == 4)
                    {
                        if (v2 == 2)
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cb(&(mcu_bloc[2*s][0]));
                                bloc01 = matrice_Cb(&(mcu_bloc[2*s + 1][0]));
                                down1_Cb = horizontal_vertical_downsampling(bloc00, bloc01);
                                int16_t **dct_Cb = dct(down1_Cb, dct_used);
                                int16_t vecteur_Cb[64] = {0};
                                zigzag(dct_Cb, vecteur_Cb);
                                int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                                int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                                encode_DC(stream, htable_DC_Cb, coeff_Cb);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                                predicateur_Cb = quantified_Cb[0];
                                dct_free(dct_Cb);
                                free(quantified_Cb);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(down1_Cb);
                            }
                        }
                        else
                        {
                            bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cb(&(mcu_bloc[1][0]));
                            bloc10 = matrice_Cb(&(mcu_bloc[2][0]));
                            bloc11 = matrice_Cb(&(mcu_bloc[3][0]));
                            down1_Cb = horizontal_vertical_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                            int16_t **dct_Cb = dct(down1_Cb, dct_used);
                            int16_t vecteur_Cb[64] = {0};
                            zigzag(dct_Cb, vecteur_Cb);
                            int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                            int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                            encode_DC(stream, htable_DC_Cb, coeff_Cb);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                            predicateur_Cb = quantified_Cb[0];
                            dct_free(dct_Cb);
                            free(quantified_Cb);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(bloc11);
                            matrix_free(down1_Cb);
                        }
                    }
                }
                else
                {
                    bloc00 = matrice_Cb(&(mcu_bloc[0][0]));
                    int16_t **dct_Cb = dct(bloc00, dct_used);
                    int16_t vecteur_Cb[64] = {0};
                    zigzag(dct_Cb, vecteur_Cb);
                    int16_t *quantified_Cb = quantization(vecteur_Cb, quantification_table_CbCr);
                    int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
                    encode_DC(stream, htable_DC_Cb, coeff_Cb);
                    encode_vect_AC(stream, htable_AC_Cb, quantified_Cb);
                    predicateur_Cb = quantified_Cb[0];
                    dct_free(dct_Cb);
                    free(quantified_Cb);
                    matrix_free(bloc00);
                }

                //Traitement matrice Cr
                uint8_t **down1_Cr, **down2_Cr, **vertical_Cr;
                if (h1 > 1 && v1 > 1)
                {
                    if ((h3 == h1) && (v3 == v1))
                    {
                        for (uint8_t s = 0; s < v3; s++)
                        {
                            for (uint8_t t = 0; t < h3; t++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[s][t]));
                                int16_t **dct_Cr = dct(bloc00, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cr, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                            }
                        }
                    }
                    else if ((h1 == 2) && (v1 == 2))
                    {
                        if ((h3 == 1) && (v3 == 1))
                        {
                            bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[0][1]));
                            bloc10 = matrice_Cr(&(mcu_bloc[1][0]));
                            bloc11 = matrice_Cr(&(mcu_bloc[1][1]));
                            down1_Cr = horizontal_downsampling(bloc00, bloc01);
                            down2_Cr = horizontal_downsampling(bloc10, bloc11);
                            vertical_Cr = horizontal_vertical_downsampling(down1_Cr, down2_Cr);
                            int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                            int16_t vecteur_Cr[64] = {0};
                            zigzag(dct_Cr, vecteur_Cr);
                            int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                            int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                            encode_DC(stream, htable_DC_Cr, coeff_Cr);
                            encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                            predicateur_Cr = quantified_Cr[0];
                            dct_free(dct_Cr);
                            free(quantified_Cr);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(bloc11);
                            matrix_free(down1_Cr);
                            matrix_free(down2_Cr);
                            matrix_free(vertical_Cr);
                        }
                        else if (h3 == 1)
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[s][0]));
                                bloc01 = matrice_Cr(&(mcu_bloc[s][1]));
                                down1_Cr = horizontal_downsampling(bloc00, bloc01);
                                int16_t **dct_Cr = dct(down1_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cr, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(down1_Cr);
                            }
                        }
                        else
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[0][s]));
                                bloc10 = matrice_Cr(&(mcu_bloc[1][s]));
                                vertical_Cr = horizontal_vertical_downsampling(bloc00, bloc10);
                                int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cr, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc10);
                                matrix_free(vertical_Cr);
                            }
                        }
                    }
                    else if ((v1 == 3) && (h1 == 2))
                    {
                        if ((h3 == 1) && (v3 == 1))
                        {
                            bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[0][1]));
                            down1_Cr = horizontal_downsampling(bloc00, bloc01);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            bloc00 = matrice_Cr(&(mcu_bloc[1][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[1][1]));
                            down2_Cr = horizontal_downsampling(bloc00, bloc01);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            bloc00 = matrice_Cr(&(mcu_bloc[2][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[2][1]));
                            bloc10 = horizontal_downsampling(bloc00, bloc01);
                            vertical_Cr = horizontal_vertical_downsampling_triple(down1_Cr, down2_Cr, bloc00);
                            int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                            int16_t vecteur_Cr[64] = {0};
                            zigzag(dct_Cr, vecteur_Cr);
                            int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                            int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                            encode_DC(stream, htable_DC_Cb, coeff_Cr);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                            predicateur_Cr = quantified_Cr[0];
                            dct_free(dct_Cr);
                            free(quantified_Cr);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(down1_Cr);
                            matrix_free(down2_Cr);
                            matrix_free(vertical_Cr);
                        }
                        else if ((h3 == 2) && (v3 == 1))
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[0][s]));
                                bloc01 = matrice_Cr(&(mcu_bloc[1][s]));
                                bloc10 = matrice_Cr(&(mcu_bloc[2][s]));
                                vertical_Cr = horizontal_vertical_downsampling_triple(bloc00, bloc01, bloc10);
                                int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cb, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(bloc10);
                                matrix_free(vertical_Cr);
                            }
                        }
                        else if ((h3 == 1) && (v3 == 3))
                        {
                            for (uint8_t s = 0; s < 3; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[s][0]));
                                bloc01 = matrice_Cr(&(mcu_bloc[s][1]));
                                vertical_Cr = horizontal_downsampling(bloc00, bloc01);
                                int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cb, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(vertical_Cr);
                            }
                        }
                    }
                    else if ((h1 == 3) && (v1 == 2))
                    {
                        if ((h3 == 1) && (v3 == 1))
                        {
                            bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[0][1]));
                            bloc10 = matrice_Cr(&(mcu_bloc[0][2]));
                            down1_Cr = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            bloc00 = matrice_Cr(&(mcu_bloc[1][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[1][1]));
                            bloc10 = matrice_Cr(&(mcu_bloc[1][2]));
                            down2_Cr = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                            vertical_Cr = horizontal_vertical_downsampling(down1_Cr, down2_Cr);
                            int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                            int16_t vecteur_Cr[64] = {0};
                            zigzag(dct_Cr, vecteur_Cr);
                            int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                            int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                            encode_DC(stream, htable_DC_Cb, coeff_Cr);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                            predicateur_Cr = quantified_Cr[0];
                            dct_free(dct_Cr);
                            free(quantified_Cr);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(down1_Cr);
                            matrix_free(down2_Cr);
                            matrix_free(vertical_Cr);
                        }
                        else if ((h3 == 1) && (v3 == 2))
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[s][0]));
                                bloc01 = matrice_Cr(&(mcu_bloc[s][1]));
                                bloc10 = matrice_Cr(&(mcu_bloc[s][2]));
                                vertical_Cr = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                                int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cb, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(bloc10);
                                matrix_free(vertical_Cr);
                            }
                        }
                        else if ((h2 == 3) && (v2 == 1))
                        {
                            for (uint8_t s = 0; s < 3; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[0][s]));
                                bloc01 = matrice_Cr(&(mcu_bloc[1][s]));
                                vertical_Cr = horizontal_vertical_downsampling(bloc00, bloc01);
                                int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cb, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(vertical_Cr);
                            }
                        }
                    }
                    else if ((h1 == 4) && (v1 == 2))
                    {
                        bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cr(&(mcu_bloc[0][1]));
                        bloc10 = matrice_Cr(&(mcu_bloc[0][2]));
                        bloc11 = matrice_Cr(&(mcu_bloc[0][3]));
                        down1_Cr = horizontal_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        bloc00 = matrice_Cr(&(mcu_bloc[1][0]));
                        bloc01 = matrice_Cr(&(mcu_bloc[1][1]));
                        bloc10 = matrice_Cr(&(mcu_bloc[1][2]));
                        bloc11 = matrice_Cr(&(mcu_bloc[1][3]));
                        down2_Cr = horizontal_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        vertical_Cr = horizontal_vertical_downsampling(down1_Cr, down2_Cr); 
                        int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                        int16_t vecteur_Cr[64] = {0};
                        zigzag(dct_Cr, vecteur_Cr);
                        int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                        int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                        encode_DC(stream, htable_DC_Cb, coeff_Cr);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                        predicateur_Cr = quantified_Cr[0];
                        dct_free(dct_Cr);
                        free(quantified_Cr);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        matrix_free(down1_Cr);
                        matrix_free(down2_Cr);
                        matrix_free(vertical_Cr);                        
                    }
                    else if ((h1 == 2) && (v1 == 4))
                    {
                        bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cr(&(mcu_bloc[1][0]));
                        bloc10 = matrice_Cr(&(mcu_bloc[2][0]));
                        bloc11 = matrice_Cr(&(mcu_bloc[3][0]));
                        down1_Cr = horizontal_vertical_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        bloc00 = matrice_Cr(&(mcu_bloc[0][1]));
                        bloc01 = matrice_Cr(&(mcu_bloc[1][1]));
                        bloc10 = matrice_Cr(&(mcu_bloc[2][1]));
                        bloc11 = matrice_Cr(&(mcu_bloc[3][1]));
                        down2_Cr = horizontal_vertical_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                        vertical_Cr = horizontal_downsampling(down1_Cr, down2_Cr); 
                        int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                        int16_t vecteur_Cr[64] = {0};
                        zigzag(dct_Cr, vecteur_Cr);
                        int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                        int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                        encode_DC(stream, htable_DC_Cb, coeff_Cr);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                        predicateur_Cr = quantified_Cr[0];
                        dct_free(dct_Cr);
                        free(quantified_Cr);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(bloc11);
                        matrix_free(down1_Cr);
                        matrix_free(down2_Cr);
                        matrix_free(vertical_Cr);      
                    }
                }
                else if (h1 > 1)
                {
                    if (h3 == h1)
                    {
                        for (uint8_t k = 0; k < h1; k++)
                        {
                            bloc00 = matrice_Cr(&(mcu_bloc[0][k]));
                            int16_t **dct_Cr = dct(bloc00, dct_used);
                            int16_t vecteur_Cr[64] = {0};
                            zigzag(dct_Cr, vecteur_Cr);
                            int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                            int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                            encode_DC(stream, htable_DC_Cr, coeff_Cr);
                            encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                            predicateur_Cr = quantified_Cr[0];
                            dct_free(dct_Cr);
                            free(quantified_Cr);
                            matrix_free(bloc00);
                        }
                    }
                    else if (h1 == 2)
                    {
                        bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cr(&(mcu_bloc[0][1]));
                        down1_Cr = horizontal_downsampling(bloc00, bloc01);
                        int16_t **dct_Cr = dct(down1_Cr, dct_used);
                        int16_t vecteur_Cr[64] = {0};
                        zigzag(dct_Cr, vecteur_Cr);
                        int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                        int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                        encode_DC(stream, htable_DC_Cr, coeff_Cr);
                        encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                        predicateur_Cr = quantified_Cr[0];
                        dct_free(dct_Cr);
                        free(quantified_Cr);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(down1_Cr);
                    }
                    else if (h1 == 3)
                    {
                        bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cr(&(mcu_bloc[0][1]));
                        bloc10 = matrice_Cr(&(mcu_bloc[0][2]));
                        down1_Cr = horizontal_downsampling_triple(bloc00, bloc01, bloc10);
                        int16_t **dct_Cr = dct(down1_Cr, dct_used);
                        int16_t vecteur_Cr[64] = {0};
                        zigzag(dct_Cr, vecteur_Cr);
                        int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                        int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                        encode_DC(stream, htable_DC_Cb, coeff_Cr);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                        predicateur_Cr = quantified_Cr[0];
                        dct_free(dct_Cr);
                        free(quantified_Cr);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(down1_Cr);
                    }
                    else if (h1 == 4)
                    {
                        if (h3 == 2)
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[0][2*s]));
                                bloc01 = matrice_Cr(&(mcu_bloc[0][2*s + 1]));
                                down1_Cr = horizontal_downsampling(bloc00, bloc01);
                                int16_t **dct_Cr = dct(down1_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cb, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(down1_Cr);
                            }
                        }
                        else
                        {
                            bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[0][1]));
                            bloc10 = matrice_Cr(&(mcu_bloc[0][2]));
                            bloc11 = matrice_Cr(&(mcu_bloc[0][3]));
                            down1_Cr = horizontal_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                            int16_t **dct_Cr = dct(down1_Cr, dct_used);
                            int16_t vecteur_Cr[64] = {0};
                            zigzag(dct_Cr, vecteur_Cr);
                            int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                            int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                            encode_DC(stream, htable_DC_Cb, coeff_Cr);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                            predicateur_Cr = quantified_Cr[0];
                            dct_free(dct_Cr);
                            free(quantified_Cr);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(bloc11);
                            matrix_free(down1_Cr);
                        }
                    }
                }
                else if (v1 > 1)
                {
                    if (v3 == v1)
                    {
                        for (uint8_t k = 0; k < v1; k++)
                        {
                            bloc00 = matrice_Cr(&(mcu_bloc[k][0]));
                            int16_t **dct_Cr = dct(bloc00, dct_used);
                            int16_t vecteur_Cr[64] = {0};
                            zigzag(dct_Cr, vecteur_Cr);
                            int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                            int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                            encode_DC(stream, htable_DC_Cr, coeff_Cr);
                            encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                            predicateur_Cr = quantified_Cr[0];
                            dct_free(dct_Cr);
                            free(quantified_Cr);
                            matrix_free(bloc00);
                        }
                    }
                    else if (v1 == 2)
                    {
                        bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                        bloc10 = matrice_Cr(&(mcu_bloc[1][0]));
                        vertical_Cr = horizontal_vertical_downsampling(bloc00, bloc10);
                        int16_t **dct_Cr = dct(vertical_Cr, dct_used);
                        int16_t vecteur_Cr[64] = {0};
                        zigzag(dct_Cr, vecteur_Cr);
                        int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                        int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                        encode_DC(stream, htable_DC_Cr, coeff_Cr);
                        encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                        predicateur_Cr = quantified_Cr[0];
                        dct_free(dct_Cr);
                        free(quantified_Cr);
                        matrix_free(bloc00);
                        matrix_free(bloc10);
                        matrix_free(vertical_Cr);
                    }
                    else if (v1 == 3)
                    {
                        bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                        bloc01 = matrice_Cr(&(mcu_bloc[1][0]));
                        bloc10 = matrice_Cr(&(mcu_bloc[2][0]));
                        down1_Cr = horizontal_vertical_downsampling_triple(bloc00, bloc01, bloc10);
                        int16_t **dct_Cr = dct(down1_Cr, dct_used);
                        int16_t vecteur_Cr[64] = {0};
                        zigzag(dct_Cr, vecteur_Cr);
                        int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                        int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                        encode_DC(stream, htable_DC_Cb, coeff_Cr);
                        encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                        predicateur_Cr = quantified_Cr[0];
                        dct_free(dct_Cr);
                        free(quantified_Cr);
                        matrix_free(bloc00);
                        matrix_free(bloc01);
                        matrix_free(bloc10);
                        matrix_free(down1_Cr);
                    }
                    else if (v1 == 4)
                    {
                        if (v3 == 2)
                        {
                            for (uint8_t s = 0; s < 2; s++)
                            {
                                bloc00 = matrice_Cr(&(mcu_bloc[2*s][0]));
                                bloc01 = matrice_Cr(&(mcu_bloc[2*s + 1][0]));
                                down1_Cr = horizontal_vertical_downsampling(bloc00, bloc01);
                                int16_t **dct_Cr = dct(down1_Cr, dct_used);
                                int16_t vecteur_Cr[64] = {0};
                                zigzag(dct_Cr, vecteur_Cr);
                                int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                                int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                                encode_DC(stream, htable_DC_Cb, coeff_Cr);
                                encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                                predicateur_Cr = quantified_Cr[0];
                                dct_free(dct_Cr);
                                free(quantified_Cr);
                                matrix_free(bloc00);
                                matrix_free(bloc01);
                                matrix_free(down1_Cr);
                            }
                        }
                        else
                        {
                            bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                            bloc01 = matrice_Cr(&(mcu_bloc[1][0]));
                            bloc10 = matrice_Cr(&(mcu_bloc[2][0]));
                            bloc11 = matrice_Cr(&(mcu_bloc[3][0]));
                            down1_Cr = horizontal_vertical_downsampling_4(bloc00, bloc01, bloc10, bloc11);
                            int16_t **dct_Cr = dct(down1_Cr, dct_used);
                            int16_t vecteur_Cr[64] = {0};
                            zigzag(dct_Cr, vecteur_Cr);
                            int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                            int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                            encode_DC(stream, htable_DC_Cb, coeff_Cr);
                            encode_vect_AC(stream, htable_AC_Cb, quantified_Cr);
                            predicateur_Cr = quantified_Cr[0];
                            dct_free(dct_Cr);
                            free(quantified_Cr);
                            matrix_free(bloc00);
                            matrix_free(bloc01);
                            matrix_free(bloc10);
                            matrix_free(bloc11);
                            matrix_free(down1_Cr);
                        }
                    }
                }
                else
                {
                    bloc00 = matrice_Cr(&(mcu_bloc[0][0]));
                    int16_t **dct_Cr = dct(bloc00, dct_used);
                    int16_t vecteur_Cr[64] = {0};
                    zigzag(dct_Cr, vecteur_Cr);
                    int16_t *quantified_Cr = quantization(vecteur_Cr, quantification_table_CbCr);
                    int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
                    encode_DC(stream, htable_DC_Cr, coeff_Cr);
                    encode_vect_AC(stream, htable_AC_Cr, quantified_Cr);
                    predicateur_Cr = quantified_Cr[0];
                    dct_free(dct_Cr);
                    free(quantified_Cr);
                    matrix_free(bloc00);
                }
            }
        }
    }       
        
    if (!gray_scale)
    {
        huffman_table_destroy(htable_DC_Cb);
        huffman_table_destroy(htable_DC_Cr);
        huffman_table_destroy(htable_AC_Cb);
        huffman_table_destroy(htable_AC_Cr);
    }
    jpeg_write_footer(jpeg);
    jpeg_destroy(jpeg);
    ppm_free(ppm);
    huffman_table_destroy(htable_DC_Y);
    huffman_table_destroy(htable_AC_Y);
    ycbcr_free(ycbcr_values, new_height);
    mcu_free(mcus, hauteur_blocs / v1, largeur_blocs / h1);
    for (uint16_t i = 0; i < hauteur_blocs; i++)
    {
        free(blocs[i]);
    }
    free(blocs);
    free(jpeg_filename);
    bitstream_destroy(stream);
    
    return EXIT_SUCCESS;
}
