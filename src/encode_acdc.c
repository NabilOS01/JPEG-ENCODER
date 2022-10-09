#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "my_jpeg_writer.h"
#include "my_bitstream.h"
#include "htables.h"
#include "my_huffman.h"


/*
    Fonction qui retourne l'indice d'une valeur de magnitude m
*/
static uint16_t get_index(uint8_t magnitude, int16_t value)
{
    if (value == 0)
    {
        return 0;
    }
    if(pow(2, magnitude - 1) == abs(value))
    {
        if (value > 0)
        {
            return value;
        }
        return -value - 1;
    }
    if (value > 0)
    {
        return value;
    }
    uint16_t diff = pow(2, magnitude) + value;
    return diff - 1;
}

/*
    Fonction qui retourne la magnitude d'une coefficient coeff.
*/
static uint8_t get_magnitude(int16_t coeff)
{
    if (coeff == 0)
    {
        // log2(0) n'est pas défini
        return 0; 
    }
    uint8_t resultat = (uint8_t) log2(abs(coeff)) + 1;
    return resultat;
}

/*
    Calcule le coefficient DC assossié au coefficient "coeff"
    et l'écrit dans le bitstream "stream".
*/
void encode_DC(struct bitstream *stream, struct huff_table *table_dc, int16_t coeff)
{
    //coeff = difference entre deux blocs
    uint8_t magnitude = get_magnitude(coeff);
    uint16_t indice = get_index(magnitude, coeff);
    uint8_t nb_bits= 0;
    uint32_t code_magnitude = huffman_table_get_path(table_dc, magnitude, &nb_bits);
    bitstream_write_bits(stream, code_magnitude, nb_bits, false);
    bitstream_write_bits(stream, indice, magnitude, false);
}

/*
    Calcule le code AC associé à in coefficient coeff non nul
    en prenant en compte le nombre de zéros avant ce dernier.
*/
static uint8_t encode_AC(int16_t coeff, uint8_t nb_zero)
{
    uint8_t magnitude = get_magnitude(coeff);
    uint8_t symbole = (uint8_t) ((nb_zero << 4) | (magnitude & 0x0f));
    return symbole;
}

/*
    Écrit le code AC, ,RLE_CODE, du coefficient coeff retourné par la fonction
    encode_AC dans le bitstream stream.
*/
static void write_AC(struct bitstream *stream, struct huff_table *ht, int16_t coeff, uint8_t RLE_code)
{
    uint8_t nb_bits = 0;
    uint16_t indice = get_index(get_magnitude(coeff), coeff);
    uint32_t huff_path_RLE = huffman_table_get_path(ht, RLE_code, &nb_bits);
    bitstream_write_bits(stream, huff_path_RLE, nb_bits, false);
    bitstream_write_bits(stream, indice, get_magnitude(coeff), false);
}

/*
    Code tous les coefficients non nuls dans le vecteur quantifiée, vecteue_quantifie,
    et les écrits dans le bitsteam stream.
*/
void encode_vect_AC(struct bitstream *stream, struct huff_table *ht, int16_t *vecteur_quantifie)
{
    uint8_t nb_zero = 0;
    for(uint8_t i = 1; i < 64; i++){
        int16_t coeff = vecteur_quantifie[i];
        if(coeff == 0){
            nb_zero++;
        }
        else{
            if(nb_zero < 16){
                uint8_t RLE_code = encode_AC(coeff, nb_zero);
                nb_zero = 0;

                // écrire dans bitstream
                write_AC(stream, ht, coeff, RLE_code);
        
            }else{
                while(nb_zero >= 16){
                    nb_zero -= 16;
                    uint8_t RLE_code = 240;
                    uint8_t nb_bits = 0;
                    uint32_t huff_path_RLE = huffman_table_get_path(ht, RLE_code, &nb_bits);
                    bitstream_write_bits(stream, huff_path_RLE, nb_bits, false);
                }
                uint8_t RLE_code = encode_AC(coeff, nb_zero);
                nb_zero = 0;

                // écrire dans bitstream
                write_AC(stream, ht, coeff, RLE_code);
        }
    }
}
    // s'il ya des zéros à la fin.
    if(nb_zero != 0){
        uint8_t RLE_code = 0;
        uint8_t nb_bits = 0;
        uint32_t huff_path_RLE = huffman_table_get_path(ht, RLE_code, &nb_bits);
        bitstream_write_bits(stream, huff_path_RLE, nb_bits, false);
    }
}
