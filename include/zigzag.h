#ifndef _ZIGZAG_H_
#define _ZIGZAG_H_

#include <stdlib.h>
#include <stdint.h>

/*
    Fonction qui prend en paramètre une matrice 8x8 et un vecteur de taille 64
    et remplie ce vecteur en effectuant le parcours en zigzag de la matrice 
*/
extern void zigzag(int16_t **matrice, int16_t vecteur[64]);

/*
    Fonction qui affiche les valeurs d'un vecteur zigzagué.
*/
extern void show_zigzag_vector(int16_t vect_zigzag[64]);

#endif /* _ZIGZAG_H_ */
