#include <qr/matrix.h>
#include <qr/patterns.h>
#include <stdio.h>

qr_module_state
qr_module_get(const qr_code *qr, size_t i, size_t j)
{
    return qr->data[i * qr->side_length + j] ? QR_MODULE_DARK : QR_MODULE_LIGHT;
}

void
qr_module_set(qr_code *qr, size_t i, size_t j, qr_module_state value)
{
    qr->data[i * qr->side_length + j] = value;
}

void
qr_matrix_print(const qr_code *qr)
{
    size_t i, j;

    for (i = 0; i < qr->side_length; ++i)
    {
        for (j = 0; j < qr->side_length; ++j)
            printf("%s", qr_module_get(qr, i, j) ? "  " : "\x1b[7m  \x1b[27m");

        printf("\n");
    }
}

int
qr_module_is_reserved(const qr_code *qr, size_t i, size_t j)
{
    // finder pattern (7) + separator (1)
    int in_finder_upper_left = i < 8 && j < 8;
    int in_finder_upper_right = i < 8 && j >= qr->side_length - 8;
    int in_finder_lower_left = i >= qr->side_length - 8 && j < 8;
    int in_finder = in_finder_upper_left || in_finder_upper_right || in_finder_lower_left;

    int in_timing = i == 6 || j == 6;
    int in_alignment = qr_is_in_alignment_patterns(qr->version, i, j);

    int in_version_lower_left = i < 6 && j >= qr->side_length - 11;
    int in_version_upper_right = i >= qr->side_length - 11 && j < 6;
    int in_version = qr->version >= 7 && (in_version_lower_left || in_version_upper_right);

    int in_format_upper_left = i < 9 && j < 9;
    int in_format_upper_right = i < 9 && j >= qr->side_length - 8;
    int in_format_lower_left = i >= qr->side_length - 8 && j < 9;
    int in_format = in_format_upper_left || in_format_upper_right || in_format_lower_left;

    return in_finder || in_timing || in_alignment || in_version || in_format;
}

// Place a single bit in the matrix, skipping reserved areas
static void place_bit(qr_matrix *matrix, size_t *x, size_t *y, int *up, uint8_t bit)
{
    size_t size = matrix->size;

    while (1)
    {
        // Skip if this is a reserved module
        if (!is_reserved_module(matrix, *x, *y))
        {
            set_module(matrix, *x, *y, bit);

            // Move to the next position after placing the bit
            if (*up)
            {
                if (*x == 0 || *y == size - 1)
                {
                    (*y)--;
                    *up = 0;
                    if (*y == 6) (*y)--;  // Skip timing pattern
                } else {
                    (*x)--;
                    (*y)++;
                }
            } else {
                if (*x == size - 1 || *y == 0)
                {
                    (*y)--;
                    *up = 1;
                    if (*y == 6) (*y)--;  // Skip timing pattern
                } else {
                    (*x)++;
                    (*y)--;
                }
            }
            break;
        }

        // Move to the next position if current is reserved
        if (*up)
        {
            if (*x == 0 || *y == size - 1)
            {
                (*y)--;
                *up = 0;
                if (*y == 6) (*y)--;  // Skip timing pattern
            } else {
                (*x)--;
                (*y)++;
            }
        } else {
            if (*x == size - 1 || *y == 0)
            {
                (*y)--;
                *up = 1;
                if (*y == 6) (*y)--;  // Skip timing pattern
            } else {
                (*x)++;
                (*y)--;
            }
        }
    }
}

// Place codewords in the QR code matrix
void qr_place_codewords(qr_matrix *matrix, const uint8_t *data, size_t data_len, const uint8_t *ecc, size_t ecc_len)
{
    if (!matrix || !data || !ecc) return;

    size_t x = matrix->size - 1;
    size_t y = matrix->size - 1;
    int up = 1;  // Direction: 1 = up-right, 0 = down-left

    // Place data codewords
    for (size_t i = 0; i < data_len; i++)
    {
        uint8_t byte = data[i];
        for (int j = 7; j >= 0; j--)
        {
            place_bit(matrix, &x, &y, &up, (byte >> j) & 1);
        }
    }

    // Place ECC codewords
    for (size_t i = 0; i < ecc_len; i++)
    {
        uint8_t byte = ecc[i];
        for (int j = 7; j >= 0; j--)
        {
            place_bit(matrix, &x, &y, &up, (byte >> j) & 1);
        }
    }
}

