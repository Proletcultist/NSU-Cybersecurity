#include <stdio.h>
#include "sha3.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong amount of arguments. Expected filepath as an only argument, got %d arguments\n", argc);
        return -1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Failed to open provided file");
        return -1;
    }

    char out[256 / 8];
    int res = sha3_256_hashfile(file, out, sizeof(out));
    fclose(file);

    if (res != SHA3_OK) {
        // TODO: Handle error
        fprintf(stderr, "error\n");
        return -1;
    }

    for (size_t i = 0; i < sizeof(out); i++) {
        printf("%02X", (unsigned char) out[i]);
    }
    printf("\n");

    return 0;
}
