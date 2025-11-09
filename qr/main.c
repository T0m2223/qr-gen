#include <qr/qr.h>
#include <qr/matrix.h>
#include <stdio.h>
#include <string.h>

static void
print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <string> [error_correction]\n", program_name);
    fprintf(stderr, "  error_correction: L (7%%), M (15%%), Q (25%%), H (30%%). Default: L\n");
}

static qr_ec_level
parse_ec_level(const char *level_str)
{
    if (!level_str) return QR_EC_LEVEL_L;

    switch (level_str[0])
    {
    case 'L': case 'l': return QR_EC_LEVEL_L;
    case 'M': case 'm': return QR_EC_LEVEL_M;
    case 'Q': case 'q': return QR_EC_LEVEL_Q;
    case 'H': case 'h': return QR_EC_LEVEL_H;
    default: return QR_EC_LEVEL_L;
    }
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return 1;
    }

    const char *input = argv[1];
    qr_ec_level ec_level = (argc > 2) ? parse_ec_level(argv[2]) : QR_EC_LEVEL_L;

    unsigned version = qr_min_version(input, ec_level);
    if (version > QR_VERSION_COUNT)
    {
        fprintf(stderr, "Error: Input too large for QR code\n");
        return 1;
    }

    printf("QR Code Generation:\n");
    printf("  Input: %s\n", input);
    printf("  Error Correction: %s\n", (const char*[]){"L (7%)", "M (15%)", "Q (25%)", "H (30%)"}[ec_level]);
    printf("  Version: %u\n", version);
    printf("");

    qr_code *qr = qr_create(ec_level, QR_MODE_BYTE, version);
    qr_encode_bytes(qr, input, strlen(input));
    qr_matrix_print(qr);
    qr_destroy(qr);

    return 0;
}
