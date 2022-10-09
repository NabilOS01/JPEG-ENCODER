// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <inttypes.h>
// #include <getopt.h>
// #include <ctype.h>
// #include "decoupage.h"
// #include "zigzag.h"
// #include "qtables.h"
// #include "htables.h"
// #include "my_jpeg_writer.h"
// #include "my_bitstream.h"
// #include "my_huffman.h"
// #include "downsampling.h"
// #include "ppm2jpeg.h"

// /*
//     transforme le nom d'un fichier ppm ou pgm en celui d'un fichier portant
//     le mÃªme nom, mais avec l'extension .jpeg au lieu de .ppm ou .pgm
// */
// char *change_extentension(char *name, char *new_name)
// {
//     uint8_t length = strlen(name);
//     for (uint8_t i = 0; i <= length - 4; i++)
//     {
//         new_name[i] = name[i];
//     }
//     new_name[length - 3] = *"j";
//     new_name[length - 2] = *"p";
//     new_name[length - 1] = *"g";
//     new_name[length] = *"\0";
//     return new_name;
// }

// /*
//     Vérifie la syntaxe de l'argument sample et enregistre dans une structure arg
// */
// int verify_sample(struct arg *sample, char *arg)
// {
//     if (strlen(arg) != 11)
//     {
//         printf("Wrong sampling factors, try --help for more.\n");
//         return 0;
//     }
//     else
//     {
//         for (uint8_t i = 0; i < 6; i++)
//         {
//             if (!isdigit(arg[2*i]) || atoi(&arg[2*i]) > 4 || atoi(&arg[2*i]) < 1)
//             {
//                 printf("Sampling factors have to be in the form hxv where h and v are integers between 1 and 4.\n");
//                 return 0;
//             }
//         }
//         if (arg[1] != 'x' || arg[5] != 'x' || arg[9] != 'x')
//         {
//             printf("Sampling factors have to be in the form hxv.\n");
//             return 0;
//         }
//         if (arg[3] != ',' || arg[7] != ',')
//         {
//             printf("Wrong format ! Expected : hxv,h2xv2,h3xv3.\n");
//             return 0;
//         }
//     }
//     sample->h1 = atoi(&arg[0]);
//     sample->v1 = atoi(&arg[2]);
//     sample->h2 = atoi(&arg[4]);
//     sample->v2 = atoi(&arg[6]);
//     sample->h3 = atoi(&arg[8]);
//     sample->v3 = atoi(&arg[10]);
//     return 1;
// }

// /*
//     Affiche le menu d'aide
// */
// void print_help(void)
// {
//     printf("Usage : ./ppm2jpeg [options] image_to_convert.{ppm, pgm}\n");
//     printf("options are the following : \n");
//     printf("--sample=h1xv1,h2xv2,h3xv3 : Sets horizontal and vertical sampling factors for\n");
//     printf("                             Y, Cb and Cr.\n");
//     printf("                             Example : --sample=2x2,1x1,1x1\n");
//     printf("--outfile=filename : Sets the name of the output file to filename\n");
//     printf("--help : Prints this help.\n");
// }


// void coeff_AC(struct bitstream *stream, struct huff_table *ht, int16_t coeff, int8_t nb_zero, uint8_t index){
        

        
//         if(coeff == 0){
//             nb_zero++;
//         }
//         else{
            
//             if(nb_zero < 16){
                
//                 uint8_t RLE_code = coder_AC(coeff, nb_zero);
//                 nb_zero = 0;

//                 // écrire dans bitstream
//                 uint8_t nb_bits = 0;
//                 uint16_t indice = get_index(calcul_magnitude(coeff), coeff);
//                 uint8_t magnitude = calcul_magnitude(indice);
//                 uint32_t huff_path_RLE = huffman_table_get_path(ht, RLE_code, &nb_bits);
//                 bitstream_write_bits(stream, huff_path_RLE, nb_bits, false);
//                 printf("value = %d \n", coeff);
//                 printf(" VALEUR RLE %d \n", RLE_code);
//                 printf("bitsream -> huff_path_RLE = %d sur %d bits   ", huff_path_RLE, nb_bits);
//                 // uint8_t nb_bits2 = 1;
//                 bitstream_write_bits(stream, indice, calcul_magnitude(coeff), false);
//                 printf("bitsream -> indice = %d  sur %d bits \n", indice, calcul_magnitude(coeff));
// }
//             else{
                
//                 while(nb_zero >= 16){
                    
//                     nb_zero -= 16;
//                     uint8_t RLE_code = 240;
//                     uint8_t nb_bits = 0;
//                     uint32_t huff_path_RLE = huffman_table_get_path(ht, RLE_code, &nb_bits);
//                     bitstream_write_bits(stream, huff_path_RLE, nb_bits, false);
//                     printf("bitstream -> value = %d, nb_bits = %d \n", huff_path_RLE, nb_bits);
//                 }
//                 uint8_t RLE_code = coder_AC(coeff, nb_zero);
//                 nb_zero = 0;

//                 // écrire dans bitstream
//                 uint8_t nb_bits = 0;
//                 uint16_t indice = get_index(calcul_magnitude(coeff), coeff);
//                 uint8_t magnitude = calcul_magnitude(indice);
//                 uint32_t huff_path_RLE = huffman_table_get_path(ht, RLE_code, &nb_bits);
//                 bitstream_write_bits(stream, huff_path_RLE, nb_bits, false);
//                 printf("value = %d \n", coeff);
//                 printf("bitsream -> huff_path_RLE = %d sur %d bits   ", huff_path_RLE, nb_bits);
//                 // uint8_t nb_bits2 = 1;
//                 bitstream_write_bits(stream, indice, calcul_magnitude(coeff), false);
//                 printf("bitsream -> indice = %d  sur %d bits \n", indice, calcul_magnitude(coeff));
//         }
//     }
//     if (index == 63){
//         printf("Here \n");
//         if(nb_zero != 0){
//         uint8_t RLE_code = 0;
//         uint8_t nb_bits = 0;
//         uint32_t huff_path_RLE = huffman_table_get_path(ht, RLE_code, &nb_bits);
//         bitstream_write_bits(stream, huff_path_RLE, nb_bits, false);
//         printf("bitstream -> value = %d, nb_bits = %d\n", huff_path_RLE, nb_bits);
//     }

//     }
// }

// int main(int argc, char *argv[]) {
//     int option_char, option_index;
//     // char *ppm_filename = argv[1];
//     char *ppm_filename;
//     struct arg *sample = malloc(sizeof(struct arg));
//     sample->h1 = 0;
//     sample->h2 = 0;
//     sample->h3 = 0;
//     sample->v1 = 0;
//     sample->v2 = 0;
//     sample->v3 = 0;
//     sample->outfile = NULL;
//     int status;
//     static struct option long_options[] = {
//         {"sample", required_argument, 0, 's'},
//         {"outfile", required_argument, 0, 'o'},
//         {"help", no_argument, 0, 'h'},
//         {NULL, 0, 0, 0}
//     };
//     while ((option_char = getopt_long(argc, argv, "s:o:h", long_options, &option_index)) != -1)
//     {
//         switch (option_char) {
//             case (int)'?':
//                 printf("Error : Try --help for more information\n");
//                 exit(1);
//                 break;
//             case (int)'s':
//                 status = verify_sample(sample, optarg);
//                 if (status == 0)
//                 {
//                     return EXIT_FAILURE;
//                 }
//                 break;
//             case (int)'o':
//                 sample->outfile = optarg;
//                 printf("%s\n", sample->outfile);
//                 break;
//             case (int)'h':
//                 print_help();
//                 exit(0);
//                 break;
//             default:
//                 return 1;
//         }
//     }
//     uint8_t h1 = sample->h1 > 0 ? sample->h1 :1;
//     uint8_t v1 = sample->v1 > 0 ? sample->v1 :1;
//     uint8_t h2 = sample->h2 > 0 ? sample->h2 :1;
//     uint8_t v2 = sample->v2 > 0 ? sample->v2 :1;
//     uint8_t h3 = sample->h3 > 0 ? sample->h3 :1;
//     uint8_t v3 = sample->v3 > 0 ? sample->v3 :1;
//     if (h1 > 0 && h2 > 0 && h3 > 0 && v1 > 0 && v2 > 0 && v3 > 0)
//     {
//         if (h1 * v1 + h2 * v2 + h3 * v3 > 10)
//         {
//             printf("La somme des hxv doit être <= 10.\n");
//             return EXIT_FAILURE;
//         }
//         if (h1 % h2 != 0 || h1 % h3 != 0 || v1 % v2 != 0 || v1 % v3 != 0)
//         {
//             printf("Les facteurs d'échantillonage des chrominances doivent diviser ceux de la luminance.\n");
//             return EXIT_FAILURE;
//         }
//     }
//     if (optind < argc)
//     {
//         ppm_filename = argv[optind];
//         optind++;
//     }
//     struct ppm *ppm = ppm_create(ppm_filename, h1, v1);
//     uint16_t hauteur = ppm->hauteur;
//     uint16_t largeur = ppm->largeur;
//     uint16_t new_height = hauteur + ppm->v_remainder;
//     uint16_t new_width = largeur + ppm->h_remainder;
    
//     //Test
//     printf("Magic number : %s\n", ppm->magic_number);
//     printf("Largeur : %u, Hauteur : %u\n", largeur, hauteur);
//     printf("Color_range : %u\n", ppm->color_range);

    
//     struct jpeg *jpeg = jpeg_create();
//     jpeg_set_ppm_filename(jpeg, ppm_filename);
//     /*char *jpeg_filename = "./images/invader.jpg";*/
    
//     char *jpeg_filename;
//     if (sample->outfile == NULL)
//     {
//         jpeg_filename = malloc((strlen(ppm_filename)+1)*sizeof(char));
//         jpeg_filename = change_extentension(ppm_filename, jpeg_filename);
//     }
//     else
//     {
//         jpeg_filename = malloc((strlen(sample->outfile)+1)*sizeof(char));
//         strcpy(jpeg_filename, sample->outfile);
//     }
//     jpeg_set_jpeg_filename(jpeg, jpeg_filename);
//     jpeg_set_image_width(jpeg, largeur);
//     jpeg_set_image_height(jpeg, hauteur);
//     jpeg_set_nb_components(jpeg, (is_gray_scale(ppm) ? 1 :3));

//     //Test
//     char *ppm_name = jpeg_get_ppm_filename(jpeg);
//     char *jpeg_name = jpeg_get_jpeg_filename(jpeg);
//     printf("Nom du fichier ppm : %s \n", ppm_name);
//     printf("Nom du fichier jpeg : %s \n", jpeg_name);
    
    
//     jpeg_set_sampling_factor(jpeg,0,0,h1);
//     jpeg_set_sampling_factor(jpeg,1,0,h2); // FACTEUR A REVOIR
//     jpeg_set_sampling_factor(jpeg,2,0,h3);

//     jpeg_set_sampling_factor(jpeg,0,1,v1);
//     jpeg_set_sampling_factor(jpeg,1,1,v2); // FACTEUR A REVOIR
//     jpeg_set_sampling_factor(jpeg,2,1,v3);

    
//     //Table de Huffman pour l'Ã©chantillon DC et la composante Y
//     struct huff_table *htable_DC_Y =  huffman_table_build(htables_nb_symb_per_lengths[0][0], htables_symbols[0][0], htables_nb_symbols[0][0]);
//     jpeg_set_huffman_table(jpeg,0,0,htable_DC_Y);

//     //Table de Huffman pour l'Ã©chantillon DC et la composante Cb
//     struct huff_table *htable_DC_Cb =  huffman_table_build(htables_nb_symb_per_lengths[0][1], htables_symbols[0][1], htables_nb_symbols[0][1]);
//     jpeg_set_huffman_table(jpeg,0,1,htable_DC_Cb);

//     //Table de Huffman pour l'Ã©chantillon DC et la composante Cr
//     struct huff_table *htable_DC_Cr =  huffman_table_build(htables_nb_symb_per_lengths[0][2], htables_symbols[0][2], htables_nb_symbols[0][2]);
//     jpeg_set_huffman_table(jpeg,0,2,htable_DC_Cr);

//     //Table de Huffman pour l'Ã©chantillon AC et la composante Y
//     struct huff_table *htable_AC_Y =  huffman_table_build(htables_nb_symb_per_lengths[1][0], htables_symbols[1][0], htables_nb_symbols[1][0]);
//     jpeg_set_huffman_table(jpeg,1,0,htable_AC_Y);

//     //Table de Huffman pour l'Ã©chantillon AC et la composante Cb
//     struct huff_table *htable_AC_Cb =  huffman_table_build(htables_nb_symb_per_lengths[1][1], htables_symbols[1][1], htables_nb_symbols[1][1]);
//     jpeg_set_huffman_table(jpeg,1,1,htable_AC_Cb);

//     //Table de Huffman pour l'Ã©chantillon AC et la composante Cr
//     struct huff_table *htable_AC_Cr =  huffman_table_build(htables_nb_symb_per_lengths[1][2], htables_symbols[1][2], htables_nb_symbols[1][2]);
//     jpeg_set_huffman_table(jpeg,1,2,htable_AC_Cr);

    
//     // Tables de quantifications pour les composantes Y, Cb, Cr
//     jpeg_set_quantization_table(jpeg,0,quantification_table_Y);
//     jpeg_set_quantization_table(jpeg,1,quantification_table_CbCr);
//     jpeg_set_quantization_table(jpeg,2,quantification_table_CbCr);

//     jpeg_write_header(jpeg);

//     struct bitstream *stream = jpeg_get_bitstream(jpeg);





//     //Récupération et traitement des données de l'image ppm
//     bool gray_scale = is_gray_scale(ppm);
//     struct YCbCr **ycbcr_values = convert(ppm->rgb_values, new_height, new_width, gray_scale);
//     struct bloc **blocs = decoupe_blocs(ycbcr_values, new_height, new_width);
//     uint16_t hauteur_blocs = new_height / 8;
//     uint16_t largeur_blocs = new_width / 8;
//     uint32_t nb_bloc = hauteur*largeur/64;
//     int16_t **tab_quantified_Y = malloc(nb_bloc * sizeof(int16_t *));
//     int16_t **tab_quantified_Cb = malloc(nb_bloc * sizeof(int16_t *));
//     int16_t **tab_quantified_Cr = malloc(nb_bloc * sizeof(int16_t *));
//     int8_t *nb_zero_Y = calloc(nb_bloc , sizeof(int8_t));
//     int8_t *nb_zero_Cb = calloc(nb_bloc , sizeof(int8_t));
//     int8_t *nb_zero_Cr = calloc(nb_bloc , sizeof(int8_t));


 
//     uint32_t k =0;

//     for (uint16_t i = 0; i < hauteur_blocs ; i++)
//     {
//         for (uint16_t j = 0; j < largeur_blocs ; j++)
//         {

//             uint8_t **bloc_Y = matrice_Y(&(blocs[i][j]));
//             int16_t **dct_Y = dct(bloc_Y);
//             int16_t vecteur_Y[64] = {0};
//             zigzag(dct_Y, vecteur_Y);
//             int16_t *quantified_Y = vect_quantifie(vecteur_Y, quantification_table_Y);
//             tab_quantified_Y[k] = quantified_Y;
//             dct_free(dct_Y);
//             matrix_free(bloc_Y);
//             //free(quantified_Y);

//             uint8_t **bloc_Cb = matrice_Cb(&(blocs[i][j]));
//             int16_t **dct_Cb = dct(bloc_Cb);
//             int16_t vecteur_Cb[64] = {0};
//             zigzag(dct_Cb, vecteur_Cb);
//             int16_t *quantified_Cb = vect_quantifie(vecteur_Cb, quantification_table_CbCr);
//             tab_quantified_Cb[k] = quantified_Cb;
//             dct_free(dct_Cb);
//             matrix_free(bloc_Cb);
//             //free(quantified_Cb);

//             uint8_t **bloc_Cr = matrice_Cr(&(blocs[i][j]));
//             int16_t **dct_Cr = dct(bloc_Cr);
//             int16_t vecteur_Cr[64] = {0};
//             zigzag(dct_Cr, vecteur_Cr);
//             int16_t *quantified_Cr = vect_quantifie(vecteur_Cr, quantification_table_CbCr);
//             tab_quantified_Cr[k] = quantified_Cr;
//             dct_free(dct_Cr);
//             matrix_free(bloc_Cr);
//             //free(quantified_Cr);

//             k++;
//         }
//     }

//     // Affichage des coeff DC

//     FILE *file;
//     file = stream->jpeg_file;


    
//     fwrite("\xff\xda", 2, 1, file); //Marqeur SOS

//     if (jpeg->nb_components == 1){
//         fwrite("\x00\x08", 2, 1, file); //Longueur de la section=2N+6
//         fwrite("\x01", 1, 1, file); //nb composants

//     }
//     else{
//         fwrite("\x00\x0c", 2, 1, file); //Longueur de la section
//         fwrite("\x03", 1, 1, file);
//     }

//     fwrite("\x01", 1, 1, file); // identifiant pour Y
//     fwrite("\x00", 1, 1, file); //id table de huff pour DC/id table de huff pour AC

//     if (jpeg->nb_components == 3){
//     fwrite("\x02", 1, 1, file); // 
//     fwrite("\x11", 1, 1, file); //id table de huff pour DC/id table de huff pour AC
//     fwrite("\x03", 1, 1, file); // 
//     fwrite("\x11", 1, 1, file); //id table de huff pour DC/id table de huff pour AC
//     }

//     fwrite("\x00", 1, 1, file); //premier indice de la selection spectrale
//     fwrite("\x00", 1, 1, file); //dernier indice de la selection spectrale
//     fwrite("\x00", 1, 1, file); //approximation successive 
   
    

//     int16_t predicateur_Y = 0;
//     int16_t predicateur_Cb = 0;
//     int16_t predicateur_Cr = 0;
//     for (uint32_t i =0;i < nb_bloc; i++){
//         printf("Bloc numéro %d \n", i);
//         int16_t *quantified_Y = tab_quantified_Y[i];
//         int16_t coeff_Y = quantified_Y[0] - predicateur_Y;
//         code_DC(stream, htable_DC_Y, coeff_Y);
//         predicateur_Y = quantified_Y[0];

//         int16_t *quantified_Cb = tab_quantified_Cb[i];
//         int16_t coeff_Cb = quantified_Cb[0] - predicateur_Cb;
//         code_DC(stream, htable_DC_Cb, coeff_Cb);
//         predicateur_Cb = quantified_Cb[0];

//         int16_t *quantified_Cr = tab_quantified_Cr[i];
//         int16_t coeff_Cr = quantified_Cr[0] - predicateur_Cr;
//         code_DC(stream, htable_DC_Cr, coeff_Cr);
//         predicateur_Cr = quantified_Cr[0];

//     }

//         // Scan des 9 premiers coeff AC de Y

//     jpeg_write_dht_section(file, jpeg->huff_table[0][1], "\x10");
   
//     fwrite("\xff\xda", 2, 1, file); //Marqeur SOS

//     fwrite("\x00\x08", 2, 1, file); //Longueur de la section=2N+6
//     fwrite("\x01", 1, 1, file); //nb composants


//     fwrite("\x01", 1, 1, file); // identifiant pour Y
//     fwrite("\x00", 1, 1, file); //id table de huff pour DC/id table de huff pour AC

//     fwrite("\x01", 1, 1, file); //premier indice de la selection spectrale
//     fwrite("\x05", 1, 1, file); //dernier indice de la selection spectrale
//     fwrite("\x00", 1, 1, file); //approximation successive

//     for (uint32_t i = 0 ; i< nb_bloc; i++){
//         int16_t *quantified_Y = tab_quantified_Y[i];
//         for (uint8_t j = 1; j< 64; j++){
//             coeff_AC(stream, htable_AC_Y, quantified_Y[j],nb_zero_Y[i], i);
//         }
//     }


//     if (jpeg->nb_components == 3){
//     // Scan de tous les coeff AC de Cr
//     jpeg_write_dht_section(file, jpeg->huff_table[1][1], "\x11");
//     fwrite("\xff\xda", 2, 1, file); //Marqeur SOS

//     fwrite("\x00\x08", 2, 1, file); //Longueur de la section=2N+6
//     fwrite("\x01", 1, 1, file); //nb composants


//     fwrite("\x03", 1, 1, file); // identifiant pour Y
//     fwrite("\x11", 1, 1, file); //id table de huff pour DC/id table de huff pour AC

//     fwrite("\x01", 1, 1, file); //premier indice de la selection spectrale
//     fwrite("\x3f", 1, 1, file); //dernier indice de la selection spectrale
//     fwrite("\x00", 1, 1, file); //approximation successive

//     for (uint32_t i = 0; i< nb_bloc; i++){
//         int16_t *quantified_Cr = tab_quantified_Cr[i];
//         for (uint8_t j = 1; j< 64; j++){
//             coeff_AC(stream, htable_AC_Cr, quantified_Cr[j],nb_zero_Cr[i], i);
//         }
//     }


//         // Scan de tous les coeff AC de Cb
//     jpeg_write_dht_section(file, jpeg->huff_table[1][1], "\x11");
//     fwrite("\xff\xda", 2, 1, file); //Marqeur SOS

//     fwrite("\x00\x08", 2, 1, file); //Longueur de la section=2N+6
//     fwrite("\x01", 1, 1, file); //nb composants


//     fwrite("\x02", 1, 1, file); // identifiant pour Y
//     fwrite("\x11", 1, 1, file); //id table de huff pour DC/id table de huff pour AC

//     fwrite("\x01", 1, 1, file); //premier indice de la selection spectrale
//     fwrite("\x3f", 1, 1, file); //dernier indice de la selection spectrale
//     fwrite("\x00", 1, 1, file); //approximation successive

//     for (uint32_t i = 0; i< nb_bloc; i++){
//         int16_t *quantified_Cb = tab_quantified_Cb[i];
//         for (uint8_t j = 1 ; j< 64; j++){
//             coeff_AC(stream, htable_AC_Cb, quantified_Cb[j],nb_zero_Cb[i], i);
//         }
//     }
//     }



//     // Scan des du reste des 54 coefficients AC de Y
//     jpeg_write_dht_section(file, jpeg->huff_table[0][1], "\x10");
//     fwrite("\xff\xda", 2, 1, file); //Marqeur SOS

//     fwrite("\x00\x08", 2, 1, file); //Longueur de la section=2N+6
//     fwrite("\x01", 1, 1, file); //nb composants


//     fwrite("\x01", 1, 1, file); // identifiant pour Y
//     fwrite("\x00", 1, 1, file); //id table de huff pour DC/id table de huff pour AC

//     fwrite("\x06", 1, 1, file); //premier indice de la selection spectrale
//     fwrite("\x3f", 1, 1, file); //dernier indice de la selection spectrale
//     fwrite("\x00", 1, 1, file); //approximation successive

//     for (uint32_t i = 0 ; i< nb_bloc; i++){
//         int16_t *quantified_Y = tab_quantified_Y[i];
//         for (uint8_t j = 10; j< 64; j++){
//             coeff_AC(stream, htable_AC_Y, quantified_Y[j],nb_zero_Y[i], i);
//         }
//     }


//     if (!gray_scale)
//     {
//         huffman_table_destroy(htable_DC_Cb);
//         huffman_table_destroy(htable_DC_Cr);
//         huffman_table_destroy(htable_AC_Cb);
//         huffman_table_destroy(htable_AC_Cr);
//     }
//     for (uint32_t i =0; i< nb_bloc; i++){

//         free(tab_quantified_Cb[i]);
//         free(tab_quantified_Y[i]);
//         free(tab_quantified_Cr[i]);
//     }
//     free(tab_quantified_Cb);
//     free(tab_quantified_Cr);
//     free(tab_quantified_Y);
//     free(nb_zero_Y);
//     free(nb_zero_Cb);
//     free(nb_zero_Cr);
//     jpeg_write_footer(jpeg);
//     jpeg_destroy(jpeg);
//     ppm_free(ppm);
//     huffman_table_destroy(htable_DC_Y);
//     huffman_table_destroy(htable_AC_Y);
//     ycbcr_free(ycbcr_values, new_height);
//     free(blocs);
//     free(jpeg_filename);
    
//     return EXIT_SUCCESS;

// }
