#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv) {
    FILE *input = fopen(argv[1], "r");
    uint8_t head = 0xFF;
    uint8_t last = 0xFF;
    int repetitions = 0;
    int total = 0;
    int used = 0;

    while (fread(&head, 1, 1, input)) {

        if (head == '\n') {
            continue;
        }

        if (head == last && repetitions < 30) {
            repetitions++;
            total++;
        } else {
            if (last != 0xFF) {
                if (last == '\\') {
                    if (repetitions == 0) {
                        printf("'\\\\',");
                        used++;
                    } else {
                        printf("%d, '\\\\',", (repetitions + 1));
                        used += 2;
                    }
                } else {
                    /*  know your data! */
                    if (repetitions == 0) {
                        printf(" '%c',", last);
                        used++;
                    } else {
                        printf(" %d, '%c',", (repetitions + 1), last);
                        used += 2;
                    }
                }
            }
            last = head;
            repetitions = 0;
            total++;
        }
    }

    if (total <= (32 * 32)) {
        if (repetitions != 0) {
            printf("%d, '%c', ", (repetitions + 1), last);
            used += 2;
        } else {
            printf("'%c', ", last);
            used += 1;
        }
    }

    fclose(input);

    //not proud of this
    fprintf(stderr, "%d,\n", used);

    return 0;
}
