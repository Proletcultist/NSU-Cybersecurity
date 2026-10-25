#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <sodium.h>
#include <unistd.h>
#include "kuznechik.h"

#define IS_HEX(c) (isdigit(c) || ((c) >= 'a' && (c) <= 'f'))

typedef enum action {
    ENCRYPT,
    DECRYPT
} action_t;

int encrypt_decrypt_file(
    action_t action,
    FILE *input,
    FILE *output,
    off_t input_size,
    size_t key_size,
    const uint8_t key[key_size]
) {
    if (input_size < 0) {
        fprintf(stderr, "Unexpected error occured\n");
        return -1;
    }

    uint8_t round_keys[10][16];
    kuznechik_res_t kres = kuznechik_calculate_round_keys(
        key_size,
        sizeof(round_keys) / sizeof(round_keys[0]),
        sizeof(round_keys[0]),
        key,
        round_keys
    );
    if (KUZNECHIK_OK != kres) {
        return -1;
    }

    kuznechik_res_t (*func)(
        size_t block_size,
        size_t keys_amount,
        const uint8_t block[block_size],
        const uint8_t round_keys[keys_amount][block_size],
        uint8_t res[block_size]
    );
    switch (action) {
        case ENCRYPT:
            func = kuznechik_encrypt;
            break;
        case DECRYPT:
            func = kuznechik_decrypt;
            break;
    }

    uint8_t block[16];
    for (off_t i = 0; i < input_size / sizeof(block); i++) {
        if (fread(block, 1, sizeof(block), input) != sizeof(block)) {
            fprintf(stderr, "Error while reading input file\n");
            return -1;
        }

        kres = func(
            sizeof(block),
            sizeof(round_keys) / sizeof(round_keys[0]),
            block,
            round_keys,
            block
        );
        if (kres != KUZNECHIK_OK) {
            fprintf(stderr, "Unexpected error while encrypting/decrypting\n");
            return -1;
        }

        if (fwrite(block, 1, sizeof(block), output) != sizeof(block)) {
            perror("Error while writing output file");
            return -1;
        }
    }

    off_t tail_size = input_size % sizeof(block);
    if (0 != tail_size) {
        if (fread(block, 1, sizeof(block), input) != tail_size) {
            fprintf(stderr, "Error while reading input file\n");
            return -1;
        }
        memset(block + tail_size, 0, sizeof(block) - tail_size);

        kres = func(
            sizeof(block),
            sizeof(round_keys) / sizeof(round_keys[0]),
            block,
            round_keys,
            block
        );
        if (kres != KUZNECHIK_OK) {
            fprintf(stderr, "Unexpected error while encrypting/decrypting\n");
            return -1;
        }

        if (fwrite(block, 1, sizeof(block), output) != sizeof(block)) {
            perror("Error while writing output file");
            return -1;
        }
    }

    return 0;
}

int parse_key(
    const char *restrict str,
    size_t buff_size,
    uint8_t buff[restrict buff_size]
) {
    if (strlen(str) != buff_size * 2) {
        fprintf(stderr, "Key has invalid size, required: %zu bytes\n", buff_size);
        return -1;
    }

    for (size_t i = 0; i < buff_size; i++) {
        if (!IS_HEX(*str) || !IS_HEX(*(str + 1))) {
            fprintf(stderr, "Wrong key format, expected hexadecimal string with lowercase letters\n");
            return -1;
        }
        sscanf(str, "%2hhx", &buff[i]);

        str += 2;
    }

    return 0;
}

void print_hex(FILE *file, size_t size, const uint8_t buff[size]) {
    for (size_t i = 0; i < size; i++) {
        fprintf(file, "%02x", buff[i]);
    }
}

void print_help(char *name) {
    printf("Usage:\n");
    printf("\t%s [options] <path>\n", name);
    printf("Options:\n");
    printf("\t-o, --output <path>\tOutput ciphertext to file <path> (If not provided - output to stdout)\n");
    printf("\t-k, --key <hex>\tIf not provided, default key will be generated\n");
    printf("\t-e, --encrypt\t\tEncrypt file (default action)\n");
    printf("\t-d, --decrypt\t\tDecrypt file\n");
    printf("\t-h, --help\t\tOutput this help message\n");
}

int main(int argc, char **argv) {
    static struct option long_opts[] = {
        {
            .name = "output",
            .has_arg = 1,
            .flag = NULL,
            .val = 'o'
        },
        {
            .name = "help",
            .has_arg = 0,
            .flag = NULL,
            .val = 'h'
        },
        {
            .name = "key",
            .has_arg = 1,
            .flag = NULL,
            .val = 'k'
        },
        {
            .name = "encrypt",
            .has_arg = 1,
            .flag = NULL,
            .val = 'e'
        },
        {
            .name = "decrypt",
            .has_arg = 1,
            .flag = NULL,
            .val = 'd'
        },
        {0, 0, 0, 0}
    };

    char *output_path = NULL;
    char *key_string = NULL;
    char *input_path = NULL;
    action_t action = ENCRYPT;

    int opt;
    while ((opt = getopt_long(argc, argv, "o:k:edh", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_path = optarg;
                break;
            case 'k':
                key_string = optarg;               
                break;
            case 'e':
                action = ENCRYPT;
                break;
            case 'd':
                action = DECRYPT;
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            case '?':
            default:
                print_help(argv[0]);
                return -1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: Missing path to file to encrypt\n");
        print_help(argv[0]);
        return -1;
    }
    input_path = argv[optind];

    uint8_t key[32];
    if (NULL == key_string) {
        if (sodium_init() < 0) {
            fprintf(stderr, "Error: Failed to initiate PRNG\n");
            return -1;
        }
        randombytes_buf(key, sizeof(key));
    }
    else {
        if (parse_key(key_string, sizeof(key), key)) {
            return -1;
        }
    }
    fprintf(stderr, "Using key: ");
    print_hex(stderr, sizeof(key), key);
    fprintf(stderr, "\n");

    struct stat input_statbuf;
    if (stat(input_path, &input_statbuf)) {
        perror("Error: Failed to stat input file");
        return -1;
    }
    if (!S_ISREG(input_statbuf.st_mode)) {
        fprintf(stderr, "Error: Cannot read input from non-regular file\n");
        return -1;
    }

    if (NULL != output_path && access(output_path, F_OK) == 0) {
        struct stat output_statbuf;
        if (stat(output_path, &output_statbuf)) {
            perror("Error: Failed to stat output file");
            return -1;
        }
        if (input_statbuf.st_ino == output_statbuf.st_ino) {
            fprintf(stderr, "Cannot use the same file for input and output\n");
            return 01;
        }
    }

    FILE *input = fopen(input_path, "rb");
    if (NULL == input) {
        perror("Error: failed to open input file");
        return -1;
    }
    FILE *output = NULL == output_path ? stdout : fopen(output_path, "wb");
    if (NULL == output) {
        perror("Error: failed to open output file");
        fclose(input);
        return -1;
    }

    int res = encrypt_decrypt_file(
        action,
        input,
        output,
        input_statbuf.st_size,
        sizeof(key),
        key
    );

    fclose(input);
    fclose(output);

    return res;
}
