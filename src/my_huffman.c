#include "my_huffman.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Type opaque représentant un arbre de Huffman. */
struct huff_table;

/*
    Construit un arbre de Huffman à  partir d'une table
    de symboles comme présenté en section 
    "Compression d'un bloc fréquentiel -> Le codage de Huffman" du sujet.
    nb_symb_per_lengths est un tableau contenant le nombre
    de symboles pour chaque longueur de 1 à  16,
    symbols est le tableau  des symboles ordonnés,
    et nb_symbols représente la taille du tableau symbols.
*/
struct huff_table *huffman_table_build(uint8_t *nb_symb_per_lengths, uint8_t *symbols, uint8_t nb_symbols)
{

    // Initialisation de la table de Huffman.
    struct huff_table *ht = calloc(1, sizeof(*ht));
    if (nb_symbols == 0) return NULL;

    // Construction de la table de Huffman.
    ht->nb_symb_per_lengths = nb_symb_per_lengths;
    ht->symbols = symbols;
    ht->nb_symbols = nb_symbols;

    // Construction de ht->length_per_codes contenant la taille de chaque code en binaire ce
    // représente sur combien de bits ce dernier est codé.
    ht->length_per_codes = calloc(nb_symbols, sizeof(ht->length_per_codes));
    uint8_t cur = 0;
    while(cur < nb_symbols){
        for(uint8_t i = 1; i <= 16; i++){
            uint8_t nb_len = nb_symb_per_lengths[i - 1];
            if(nb_len == 0) continue;
            for(uint8_t j = cur; j < cur + nb_len; j++){
                // nombre de bits sur lequel est écrit un code est égal à la longueur de son symbole
                ht->length_per_codes[j] = i; 
            }
            cur += nb_len; // avancer dans ht->length_per_codes
        }
    }

    // Construction des codes en binaire.
    ht->codes = calloc(nb_symbols, sizeof(ht->codes)); 
    uint32_t binary_code = 0;
    cur = 0;
    while(cur < nb_symbols){
        for(uint8_t i = 1; i <= 16; i++){
            uint8_t nb_len = nb_symb_per_lengths[i - 1];
            if(nb_len == 0) continue;
            for(uint8_t j = cur; j < cur + nb_len; j++){
                ht->codes[j] = binary_code;
                binary_code++; // Puisqu'on est dans la même profondeur, code_suivant = code_précèdent + 1
            }
            cur += nb_len;
            binary_code = binary_code  << (ht->length_per_codes[cur] - ht->length_per_codes[cur - 1]);
            // on utilise du décalage pour passer à la profondeur suivante dans l'arbre de Huffman. 
        }
    }
    
    // retourner la table de Huffman construite.
    return ht;
}

/*
    Retourne le chemin dans l'arbre ht permettant d'atteindre
    la feuille de valeur value. nb_bits est un paramà¨tre de sortie
    permettant de stocker la longueur du chemin retourné.
*/
uint32_t huffman_table_get_path(struct huff_table *ht, uint8_t value, uint8_t *nb_bits)
{
    // uint32_t binary_code = 0;
    uint8_t nb_symbols = ht->nb_symbols;
    for(uint8_t i = 0; i < nb_symbols; i++){
        if(ht->symbols[i] != value) continue;
        *nb_bits = ht->length_per_codes[i];
        return ht->codes[i];
    }
    // si value n'est pas dans les codes de ht, on retourne 0.
    return (uint32_t) 0;
}

/*
   Retourne le tableau des symboles associé à  l'arbre de
   Huffman passé en paramètre.
*/
uint8_t *huffman_table_get_symbols(struct huff_table *ht)
{
    return ht->symbols;
}

/*
    Retourne le tableau du nombre de symboles de chaque longueur
    associée à l'arbre de Huffman passé en paramètre.
*/
uint8_t *huffman_table_get_length_vector(struct huff_table *ht)
{
    return ht->nb_symb_per_lengths;
}


/*
    Détruit l'arbre de Huffman passé en paramètre et libére
    toute la mémoire qui lui est associée.
*/
void huffman_table_destroy(struct huff_table *ht)
{
    free(ht->length_per_codes);
    free(ht->codes);
    free(ht);
}
