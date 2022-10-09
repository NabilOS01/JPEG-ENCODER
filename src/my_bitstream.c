#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "my_bitstream.h"


/* Module utilisé par jpeg_writer */

/*
    Stocke dans le paramètre de sortie buffer l'octet se trouvant dans bitstream->buffer
*/
static void get_buffer(struct bitstream *stream, uint8_t *buffer)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        *buffer = (*buffer) << 1;
        *buffer = (*buffer) | stream->buffer[i];
    }
}

/*
Retourne un nouveau bitstream prêt à écrire dans le fichier filename.
*/
struct bitstream *bitstream_create(const char *filename)
{
    if (filename == NULL) return NULL;   //ajouter d'autres cas d'erreur 
    FILE *file;
    file = fopen(filename, "ab");       //file mode : écriture en mode binaire à la fin
    struct bitstream *stream = malloc(sizeof(struct bitstream));
    stream->jpeg_file = file;
    stream ->nb_bits_in_buffer = 0;
    stream->jpeg_filename = filename;
    stream->buffer = calloc(8, sizeof(unsigned char));
    return stream;
}

/*
    Force l'exécution des écritures en attente sur le bitstream, s'il en
    existe.
*/
void bitstream_flush(struct bitstream *stream)
{
    if (stream->nb_bits_in_buffer > 0)
    {
        uint8_t byte_to_write;
        get_buffer(stream, &byte_to_write);
        fwrite(&byte_to_write, 1, 1, stream->jpeg_file);
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        stream->buffer[i] = 0;
    }
    stream->nb_bits_in_buffer = 0;
}

/*
    Ecrit nb_bits bits dans le bitstream. La valeur portée par cet ensemble de
    bits est value. Le paramètre is_marker permet d'indiquer qu'on est en train
    d'écrire un marqueur de section dans l'entête JPEG ou non (voir section
    "Encodage dans le flux JPEG -> Byte stuffing" du sujet).
*/
void bitstream_write_bits(struct bitstream *stream, uint32_t value, uint8_t nb_bits, bool is_marker)
{
    if (is_marker)
    {
        bitstream_flush(stream);
    }
    uint32_t poid_fort = 1U << (nb_bits - 1);
    for (uint8_t i = 0; i < nb_bits; i++)
    {
        unsigned char bit = (value & poid_fort) ? 1 : 0;
        stream->buffer[stream->nb_bits_in_buffer] = bit;
        stream->nb_bits_in_buffer++;
        if (stream->nb_bits_in_buffer == 8)
        {
            uint8_t buffer = 0;
            get_buffer(stream, &buffer);
            bitstream_flush(stream);
            if (buffer == 0xff && !(is_marker))
            {
                fwrite("\x00", 1, 1, stream->jpeg_file);
            }
        }
        value <<= 1;
    }
}

/*
    Détruit le bitstream passé en paramètre, en libérant la mémoire qui lui est
    associée.
*/
void bitstream_destroy(struct bitstream *stream)
{
    fclose(stream->jpeg_file);
    free(stream->buffer);
    free(stream);
}
