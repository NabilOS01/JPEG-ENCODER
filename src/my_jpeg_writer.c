#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "my_jpeg_writer.h"
#include "my_bitstream.h"
#include "htables.h"
#include "my_huffman.h"
#include "qtables.h"

/* Alloue et retourne une nouvelle structure jpeg. */
struct jpeg *jpeg_create(void)
{
    struct jpeg *jpg = calloc(1, sizeof(struct jpeg));
    jpg->hauteur = 0;
    jpg->largeur = 0;
    jpg->nb_components = 0;
    for (uint8_t i = 0; i < NB_COLOR_COMPONENTS; i++)
    {
        for (uint8_t j = 0; j < NB_DIRECTIONS; j++)
        {
            jpg->sampling_factor[i][j] = 0;
        }
    }
    return jpg;
}

/*
    Détruit une structure jpeg. 
    Toute la mémoire qui lui est associée est libérée.
*/
void jpeg_destroy(struct jpeg *jpg)
{
    free(jpg);
}

/* Ecriture de la section DHT dans l'entête. */
void jpeg_write_dht_section(FILE *file, struct huff_table *htable, const char *HT_ID)
{
    fwrite("\xff\xc4", 2, 1, file); //marqueur DHT

    uint32_t dht_len = (uint32_t)(19 + htable->nb_symbols);  // longueur de frame
    //On doit écrire l'octet de poids fort puis l'octet de poids faible 
    const unsigned char len_poids_fort = (dht_len & 0xffff) >> 8;
    const unsigned char len_poinds_faible = dht_len & 0xff;

    fwrite(&len_poids_fort, 1, 1, file);

    fwrite(&len_poinds_faible, 1, 1, file);

    fwrite(HT_ID, 1, 1, file);

    uint8_t *nb_symb_per_lengths = htable->nb_symb_per_lengths;
    fwrite(nb_symb_per_lengths, sizeof(nb_symb_per_lengths[0]), 16, file);

    uint8_t *symbols = htable->symbols;
    fwrite(symbols, sizeof(symbols[0]), htable->nb_symbols, file);
}

/*
    Ecrit tout l'en-tête JPEG dans le fichier de sortie à partir des
    informations contenues dans la structure jpeg passée en paramètre. 
    En sortie, le bitstream est positionné juste après l'écriture de 
    l'en-tête SOS, à  l'emplacement du premier octet de données brutes à écrire.
*/
void jpeg_write_header(struct jpeg *jpg)
{
    FILE *file;
    file = fopen(jpg->name, "wb");
    if (file==NULL){
        printf("File does not exist :( ");
        return;
    }
    
    fwrite("\xff\xd8",2,1,file); //SOI
    fwrite("\xff\xe0", 2, 1, file); //APP0
    fwrite("\x00\x10",2 ,1, file); //length of the frame, dans ce cas 16 bytes
    fwrite("\x4a\x46\x49\x46\x00\x01\x01", 7, 1, file); //JFIF0 + version JFIF(1.1)
    fwrite("\x00\x00\x00\x00\x00\x00\x00", 7, 1, file); //Données spéficiques au JFIF, non traitées

    /* Section Define Quantization Table*/

    //DQT pour Y
    
    fwrite("\xff\xdb", 2, 1, file);  //Marqueur DQT
    fwrite("\x00\x43", 2, 1, file);  //Longueur de frame 
    uint8_t *tableY = quantification_table_Y;
    fwrite("\x00",1, 1, file);  //Précision = 0, indice = 0
    fwrite(tableY, sizeof(tableY[0]), 64, file);
    
    if (jpg->nb_components == 3){
        //DQT pour Cb et Cr
        fwrite("\xff\xdb", 2, 1, file); 
        fwrite("\x00\x43", 2, 1, file); 
        uint8_t *tableCbCr = jpeg_get_quantization_table(jpg,1);
        fwrite("\x01",1, 1, file); //Précision = 0, indice = 1
        fwrite(tableCbCr, sizeof(tableCbCr[0]), 64, file); 
    }

    fwrite("\xff\xc0", 2, 1, file); //SOFO
    if (jpg->nb_components == 3){ 
        fwrite("\x00\x11\x08", 3, 1, file);
    }

    else{
        fwrite("\x00\x0b\x08", 3, 1, file);
    }
    
    const unsigned char hauteur_poids_fort = (jpg->hauteur & 0xffff) >> 8;
    const unsigned char hauteur_poids_faible = jpg->hauteur & 0xff;
    fwrite(&hauteur_poids_fort, 1, 1, file);
    fwrite(&hauteur_poids_faible, 1, 1, file);
    const unsigned char largeur_poids_fort = (jpg->largeur & 0xffff) >> 8;
    const unsigned char largeur_poids_faible = jpg->largeur & 0xff;
    fwrite(&largeur_poids_fort, 1, 1, file);
    fwrite(&largeur_poids_faible, 1, 1, file);

    fwrite(&(jpg->nb_components), sizeof(jpg->nb_components),1, file);
    fwrite("\x01", 1 ,1, file); //identifiant de composante pour Y
    uint8_t sample_Y_H = jpeg_get_sampling_factor(jpg, Y, H);
    uint8_t sample_Y_V = jpeg_get_sampling_factor(jpg, Y, V);
    uint8_t sample_factor_y = (uint8_t)((sample_Y_H << 4) | sample_Y_V);
     
    fwrite(&(sample_factor_y), 1, 1, file);
    fwrite("\x00", 1, 1, file); //indice de qtable associée

    if (jpg->nb_components == 3){
        fwrite("\x02", 1 ,1, file); //identifiant de composante pour Cb
        uint8_t sample_Cb_H = jpeg_get_sampling_factor(jpg, Cb, H);
        uint8_t sample_Cb_V = jpeg_get_sampling_factor(jpg, Cb, V);
        uint8_t sample_factor_cb = (uint8_t)((sample_Cb_H << 4) | sample_Cb_V);
        fwrite(&(sample_factor_cb), 1, 1, file);
        fwrite("\x01", 1, 1, file); //indice de qtable associée

        fwrite("\x03", 1 ,1, file); //identifiant de composante pour Cr
        uint8_t sample_Cr_H = jpeg_get_sampling_factor(jpg, Cr, H);
        uint8_t sample_Cr_V = jpeg_get_sampling_factor(jpg, Cr, V);
        uint8_t sample_factor_cr = (uint8_t)((sample_Cr_H << 4) | sample_Cr_V);
        fwrite(&(sample_factor_cr), 1, 1, file);
        fwrite("\x01", 1, 1, file); //indice de qtable associée
    }
    
    /* Section Define Huffman Table*/

    jpeg_write_dht_section(file, jpg->huff_table[0][0], "\x00");
    jpeg_write_dht_section(file, jpg->huff_table[0][1], "\x10");
    if (jpg->nb_components == 3){
        jpeg_write_dht_section(file, jpg->huff_table[1][0], "\x01");
        jpeg_write_dht_section(file, jpg->huff_table[1][1], "\x11");
    }
 

    /*Section SOS*/
    
    fwrite("\xff\xda", 2, 1, file); //Marqeur SOS

    if (jpg->nb_components == 1){
        fwrite("\x00\x08", 2, 1, file); //Longueur de la section=2N+6
        fwrite("\x01", 1, 1, file);     //nb composants
    }
    else{
        fwrite("\x00\x0c", 2, 1, file); //Longueur de la section
        fwrite("\x03", 1, 1, file);
    }

    fwrite("\x01", 1, 1, file); // identifiant pour Y
    fwrite("\x00", 1, 1, file); //id table de huff pour DC/id table de huff pour AC

    if (jpg->nb_components == 3){
        fwrite("\x02", 1, 1, file); // identifiant pour Cb
        fwrite("\x11", 1, 1, file); //id table de huff pour DC/id table de huff pour AC
        fwrite("\x03", 1, 1, file); // identifiant pour Cr
        fwrite("\x11", 1, 1, file); //id table de huff pour DC/id table de huff pour AC
    }

    fwrite("\x00", 1, 1, file); //premier indice de la selection spectrale
    fwrite("\x3f", 1, 1, file); //dernier indice de la selection spectrale
    fwrite("\x00", 1, 1, file); //approximation successive
    fclose(file);
    
}


/* Ecrit le footer JPEG (marqueur EOI) dans le fichier de sortie. */

void jpeg_write_footer(struct jpeg *jpg)
{
    struct bitstream *stream = jpg->bitstream;
    bitstream_write_bits(stream,255, 8,true);
    bitstream_write_bits(stream,217, 8,false);
}


/*
    Retourne le bitstream associé au fichier de sortie enregistré 
    dans la structure jpeg.
*/

struct bitstream *jpeg_get_bitstream(struct jpeg *jpg)
{
    struct bitstream *stream = bitstream_create(jpg->name);
    jpg->bitstream = stream;
    return jpg->bitstream;
}


/****************************************************/
/* Gestion des paramètres de l'encodeur via le jpeg */
/****************************************************/

/* Ecrit le nom de fichier PPM ppm_filename dans la structure jpeg. */
void jpeg_set_ppm_filename(struct jpeg *jpg, const char *ppm_filename)
{
    jpg->ppm_name = (char *) ppm_filename;
}

/* Retourne le nom de fichier PPM lu dans la structure jpeg. */
char *jpeg_get_ppm_filename(struct jpeg *jpg)
{
    return jpg->ppm_name;
}

/* Ecrit le nom du fichier de sortie jpeg_filename dans la structure jpeg. */
void jpeg_set_jpeg_filename(struct jpeg *jpg, const char *jpeg_filename)
{
    jpg->name = (char *) jpeg_filename;
}

/* Retourne le nom du fichier de sortie lu depuis la structure jpeg. */
char *jpeg_get_jpeg_filename(struct jpeg *jpg)
{
    return jpg->name;
}

/*
    Ecrit la hauteur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
void jpeg_set_image_height(struct jpeg *jpg, uint32_t image_height)
{
    jpg->hauteur = image_height;
}

/*
    Retourne la hauteur de l'image traitée, en nombre de pixels,
    lue dans la structure jpeg.
*/
uint32_t jpeg_get_image_height(struct jpeg *jpg)
{
    return jpg->hauteur;
}

/*
    Ecrit la largeur de l'image traitée, en nombre de pixels,
    dans la structure jpeg.
*/
void jpeg_set_image_width(struct jpeg *jpg, uint32_t image_width)
{
    jpg->largeur = image_width;
}

/*
    Retourne la largeur de l'image traitée, en nombre de pixels,
    lue dans la structure jpeg.
*/
uint32_t jpeg_get_image_width(struct jpeg *jpg)
{
    return jpg->largeur;
}


/*
    Ecrit le nombre de composantes de couleur de l'image traitée
    dans la structure jpeg.
*/
void jpeg_set_nb_components(struct jpeg *jpg, uint8_t nb_components)
{
    jpg->nb_components = nb_components;
}

/*
    Retourne le nombre de composantes de couleur de l'image traitée 
    lu dans la structure jpeg.
*/
uint8_t jpeg_get_nb_components(struct jpeg *jpg)
{
    return jpg->nb_components;
}

/*
    Ecrit dans la structure jpeg le facteur d'échantillonnage sampling_factor
    à utiliser pour la composante de couleur cc et la direction dir.
*/
void jpeg_set_sampling_factor(struct jpeg *jpg, enum color_component cc, enum direction dir, uint8_t sampling_factor)
{
    switch(dir) {
        case H:
            switch(cc) {
                case Y:
                    jpg->sampling_factor[0][0] = sampling_factor;
                    break;
                case Cb:
                    jpg->sampling_factor[1][0] = sampling_factor;
                    break;
                case Cr:
                    jpg->sampling_factor[2][0] = sampling_factor;
                    break;
                default:
                    return; 
            }
        case V:
            switch(cc) {
                case Y:
                    jpg->sampling_factor[0][1] = sampling_factor;
                    break;
                case Cb:
                    jpg->sampling_factor[1][1] = sampling_factor;
                    break;
                case Cr:
                    jpg->sampling_factor[2][1] = sampling_factor;
                    break;
                default:
                    return; 
            }
        default:
            return;
    }
}

/*
    Retourne le facteur d'échantillonnage utilisé pour la composante 
    de couleur cc et la direction dir, lu dans la structure jpeg.
*/
uint8_t jpeg_get_sampling_factor(struct jpeg *jpg, enum color_component cc, enum direction dir)
{
    switch(dir) {
        case H:
            switch(cc) {
                case Y:
                    return jpg->sampling_factor[0][0];
                case Cb:
                    return jpg->sampling_factor[1][0];
                case Cr:
                    return jpg->sampling_factor[2][0];
                default:
                    return (uint8_t) 0; 
            }
        case V:
            switch(cc) {
                case Y:
                    return jpg->sampling_factor[0][1];
                case Cb:
                    return jpg->sampling_factor[1][1];
                case Cr:
                    return jpg->sampling_factor[2][1];
                default:
                    return (uint8_t) 0; 
            }
        default:
            return (uint8_t) 0;
    }
}


/*
    Ecrit dans la structure jpeg la table de Huffman huff_table à utiliser
    pour encoder les données de la composante fréquentielle acdc, pour la
    composante de couleur cc.
*/

void jpeg_set_huffman_table(struct jpeg *jpg, enum sample_type acdc, enum color_component cc, struct huff_table *htable)
{
    switch(acdc) {
        case DC:
            switch(cc) {
                case Y:
                    jpg->huff_table[0][0] = htable;
                    break;
                case Cb:
                case Cr:
                    jpg->huff_table[1][0] = htable;
                    break;
                default:
                    return; 
            }
        case AC:
            switch(cc) {
                case Y:
                    jpg->huff_table[0][1] = htable;
                    break;
                case Cb:
                case Cr:
                    jpg->huff_table[1][1] = htable;
                    break;
                default:
                    return; 
            }
        default:
            return;
    }
}

/*
    Retourne un pointeur vers la table de Huffman utilisée pour encoder
    les données de la composante fréquentielle acdc pour la composante 
    de couleur cc, lue dans la structure jpeg.
*/
struct huff_table *jpeg_get_huffman_table(struct jpeg *jpg, enum sample_type acdc, enum color_component cc)
{
    switch(acdc) {
        case DC:
            switch(cc) {
                case Y:
                    return jpg->huff_table[0][0];
                case Cb:
                case Cr:
                    return jpg->huff_table[1][0];
                default:
                    return NULL; 
            }
        case AC:
            switch(cc) {
                case Y:
                    return jpg->huff_table[0][1];
                case Cb:
                case Cr:
                    return jpg->huff_table[1][1];
                default:
                    return NULL; 
            }
        default:
            return NULL;
    }
}

/*
    Ecrit dans la structure jpeg la table de quantification à utiliser
    pour compresser les coefficients de la composante de couleur cc.
*/
void jpeg_set_quantization_table(struct jpeg *jpg, enum color_component cc, uint8_t *qtable)
{
    switch(cc) {
        case Y:
            jpg->qtable[0] = qtable;
            break;
        case Cb:
            jpg->qtable[1] = qtable;
            break;
        case Cr:
            jpg->qtable[1] = qtable;
            break;
        default:
            return; 
    }
}

/*
    Retourne un pointeur vers la table de quantification associée à la 
    composante de couleur cc, lue dans a structure jpeg.
*/
uint8_t *jpeg_get_quantization_table(struct jpeg *jpg, enum color_component cc)
{
    switch(cc) {
        case Y:
            return jpg->qtable[0];
        case Cb:
        case Cr:
            return jpg->qtable[1];
        default:
            return NULL;
    }
}
