#ifndef _DECOUPAGE_H_
#define _DECOUPAGE_H_

#include <stdint.h>
#include <stdbool.h>
#include "my_huffman.h"

struct rgb
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct ppm
{
    char magic_number[3];
    uint32_t largeur;
    uint32_t hauteur;
    unsigned int color_range;
    struct rgb **rgb_values;
    uint8_t v_remainder;
    uint8_t h_remainder;
};

struct YCbCr
{
    uint8_t Y;
    uint8_t Cb;
    uint8_t Cr;
};

struct bloc
{
    struct YCbCr **y_cb_cr;
};

 struct mcu
{
    struct bloc **blocs;
    uint8_t h;
    uint8_t v;
};

/*
    Indique si une image est en niveaux de gris ou en couleurs
*/
extern bool is_gray_scale(struct ppm *ppm);

/*
    Crée une structure ppm à partir d'un fichier ppm dont le nom est spécifié en paramètre
*/
extern struct ppm *ppm_create(const char *ppm_filename, uint8_t h, uint8_t v);

/* Convertit une couleur rgb en une couleur YCbCr */
extern struct YCbCr *RGB2YCbCr(struct rgb *rgb_color, bool gray_scale);

/*
    Convertit les valeurs RGB de rgb_values en YCbCr et les stocke dans une matrice
*/
extern struct YCbCr **convert(struct rgb **rgb, uint16_t hauteur, uint16_t largeur, bool grey_scale);

/*
    Découpe la matrice de couleurs YCbCr en blocs de taille 8x8
    et retourne une matrice contenant ces blocs
*/
extern struct bloc **decoupe_blocs(struct YCbCr **ycbcr, uint16_t hauteur, uint16_t largeur);

/*
    Retourne la matrice de luminance d'un bloc
*/
extern uint8_t **matrice_Y(struct bloc *bloc);

/*
    Retourne la matrice de chrominance bleue d'un bloc
*/
extern uint8_t **matrice_Cb(struct bloc *bloc);

/*
    Retourne la matrice de chrominance rouge d'un bloc
*/
extern uint8_t **matrice_Cr(struct bloc *bloc);

/*
    Retranche 128 de chaque valeur de la matrice
*/
extern int16_t **offset(uint8_t **matrice);

/*
    Fonction qui retourne l'indice d'une valeur de magnitude m
*/
extern uint16_t get_index(uint8_t magnitude, int16_t value);

/*
    Calcul la magnitude d'un coeffficient donné
*/

extern uint8_t calcul_magnitude(int16_t coeff);

/*
    Retourne le codage de la magnitude pour le coefficient DC
*/
extern void code_DC(struct bitstream *stream, struct huff_table *table_dc, int16_t coeff);

/*
    Retourne le code RLE pour un coefficient AC
*/
extern uint8_t coder_AC(int16_t coeff, uint8_t nb_zero);

extern void vecteur_AC(struct bitstream *stream, struct huff_table *ht, int16_t *vecteur_quantifie);

/*
    Prend en entrée une matrice de blocs et la regroupe en mcus de dimension h*v chacun
    Pour notre cas simple (photo invaders), on aura h = v = 1
*/

extern struct mcu **decoupe_mcus(struct bloc **blocs, uint16_t largeur, uint16_t hauteur, uint8_t h, uint8_t v);

/*
    Écrit un bloc dans un fichier stream
*/
extern void bloc_print_Y(struct bloc *to_print, FILE *stream);

extern void bloc_print_Cb(struct bloc *to_print, FILE *stream);

extern void bloc_print_Cr(struct bloc *to_print, FILE *stream);

/*
    Libère l'espace mémoire associé à struct ppm
*/
extern void ppm_free(struct ppm *ppm);

/*
    Libère l'espace mémoire associé à une matrice ycbcr
*/
extern void ycbcr_free(struct YCbCr **ycbcr, uint16_t hauteur);

/*
    Libère l'espace mémoire associé à une matrice uint8_t
*/
extern void matrix_free(uint8_t **matrice);

/*
    Libère l'espace mémoire associé à une matrice uint8_t
*/
extern void blocs_free(struct bloc **blocs, uint16_t hauteur, uint16_t largeur);

/*
    Libère l'espace mémoire associé à un matrice uint8_t
*/
extern void blocs_free(struct bloc **blocs, uint16_t hauteur, uint16_t largeur);

/*
    Libère l'espace mémoire associé à un matrice uint8_t
*/
extern void mcu_free(struct mcu **mcus, uint16_t hauteur, uint16_t largeur);

#endif /* DECOUPAGE_H */
