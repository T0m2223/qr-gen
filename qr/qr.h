#ifndef QR_QR_H
#define QR_QR_H

#include <qr/types.h>

qr_code *qr_create(qr_ec_level level, qr_encoding_mode mode, unsigned version);
void qr_encode_message(qr_code *qr, const char *message);
void qr_destroy(qr_code *qr);

#endif // QR_QR_H
