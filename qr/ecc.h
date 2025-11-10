#ifndef QR_ECC_H
#define QR_ECC_H

#include <qr/types.h>

typedef struct
{
    uint8_t *data;
    uint8_t *ecc;
    size_t data_length;
    size_t ecc_length;
    uint8_t *generator;
} qr_ec;

void qr_ec_encode(qr_code *qr);

#endif // QR_ECC_H
