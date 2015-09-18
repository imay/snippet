#include <stdio.h>

#define BLOCK_SIZE 512
int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 0;
    }
    FILE* in_fp = fopen(argv[1], "r");
    FILE* out_fp = fopen("./decode.out", "w");
    char buf[BLOCK_SIZE];
    while (feof(in_fp) == 0 && ferror(in_fp) == 0) {
        size_t len = fread(buf, BLOCK_SIZE, 1, in_fp);
        int i;
        for (i = 0; i < len; ++i) {
            buf[i] ^= 0x5a;
        }
        fwrite(buf, BLOCK_SIZE, 1, out_fp);
    }
    fclose(in_fp);
    fclose(out_fp);
    return 0;
}
