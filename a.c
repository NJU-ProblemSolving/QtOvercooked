#include <stdio.h>

int main() {
    int i = 0;
    char b[10000];
    while (1) {
        while (getchar() != '\0');
        printf("Frame %d\n", i++);
        printf("Move R\n");
        printf("Move U\n");
        fflush(stdout);
        usleep(10000);
    }
}
