#include <assert.h>
#include <qr/ecc.h>
#include <qr/info.h>
#include <qr/mask.h>
#include <qr/matrix.h>
#include <qr/patterns.h>
#include <qr/qr.h>
#include <stdlib.h>

static const size_t CODEWORD_COUNT[QR_VERSION_COUNT] =
{
      26,   44,   70,  100,  134,  172,  196,  242,  292,  346,
     404,  466,  532,  581,  655,  733,  815,  901,  991, 1085,
    1156, 1258, 1364, 1474, 1588, 1706, 1828, 1921, 2051, 2185,
    2323, 2465, 2611, 2761, 2876, 3034, 3196, 3362, 3532, 3706,
};

qr_code *
qr_create(qr_ec_level level, qr_encoding_mode mode, unsigned version)
{
    qr_code *qr = malloc(sizeof(qr_code));

    qr->level = level;
    qr->mode = mode;
    qr->version = version;
    qr->side_length = 21 + (qr->version * 4);
    qr->matrix = malloc((qr->side_length * qr->side_length) * sizeof(*qr->matrix));

    qr->codeword_count = CODEWORD_COUNT[qr->version];
    qr->codewords = malloc(qr->codeword_count * sizeof(word));

    return qr;
}

void
qr_destroy(qr_code *qr)
{
    free(qr->codewords);
    free(qr->matrix);
    free(qr);
}

void
qr_encode_bytes(qr_code *qr, const char *message, size_t n)
{
    // 1. enc
    // move this into enc
    assert("Message provided is too large");

    size_t i;
    for (i = 0; i < n; ++i)
        qr->codewords[i] = message[i];
    for (; i < qr->n_data_codewords; ++i)
        qr->data_codewords[i] = 0;

    // 2. ecc
    qr_ec_encode(qr);

    // 3. block
    qr_interleave_codewords(qr);

    // 4. matrix
    qr_place_codewords(qr);
    qr_finder_patterns_apply(qr);
    qr_separators_apply(qr);
    qr_timing_patterns_apply(qr);
    qr_alignment_patterns_apply(qr);

    // 5. masking
    qr_mask_apply(qr);

    // 6. info
    qr_format_info_apply(qr);
    qr_version_info_apply(qr);
}
