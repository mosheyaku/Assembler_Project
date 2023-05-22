#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builder.h"
#include "helper.h"

#define LIMIT_FILE_PATH (256)

/* function that does one run of the assembler */
static int one_run(const char* prefix) {

    FILE* file_as;
    FILE* file_am;
    FILE* file_ent;
    FILE* file_ext;
    FILE* file_ob;
    builder_t* builder;
    char file_path[LIMIT_FILE_PATH];

    file_ext = NULL;
    file_ent = NULL;

    strcpy(file_path, prefix);
    strcat(file_path, ".as");
    file_as = fopen(file_path, "r");

    if  (file_as == NULL) {

        fprintf(
            stderr,
            "error: fopen '%s'!\n",
            file_path
        );

        return 0;

    }

    strcpy(file_path, prefix);
    strcat(file_path, ".am");
    file_am = fopen(file_path, "w");

    if  (file_am == NULL) {

        fprintf(
            stderr,
            "error: fopen '%s'!\n",
            file_path
        );

        return 0;

    }

    builder = builder_create(
        file_as,
        file_am,
        NULL
    );

    builder_first_pass(builder);

    if (builder->errors_found > 0) {

        fprintf(
            stderr,
            "found %d errors, cannot assemble file.\n",
            (int) builder->errors_found
        );

        return 0;

    }

    strcpy(file_path, prefix);
    strcat(file_path, ".ob");
    file_ob = fopen(file_path, "w");

    if  (file_ob == NULL) {

        fprintf(
            stderr,
            "error: fopen '%s'!\n",
            file_path
        );

        return 0;

    }

    builder->file_ob = file_ob;

    if (builder->length_entry > 0) {

        strcpy(file_path, prefix);
        strcat(file_path, ".ent");
        file_ent = fopen(file_path, "w");

        if  (file_ent == NULL) {

            fprintf(
                stderr,
                "error: fopen '%s'!\n",
                file_path
            );

            return 0;

        }

    }

    if (builder->length_extern > 0) {

        strcpy(file_path, prefix);
        strcat(file_path, ".ext");
        file_ext = fopen(file_path, "w");

        if  (file_ext == NULL) {

            fprintf(
                stderr,
                "error: fopen '%s'!\n",
                file_path
            );

            return 0;

        }

    }

    builder_second_pass(builder, file_ent, file_ext);

    if (file_ext != NULL) {

        fclose(file_ext);

    }

    if (file_ent != NULL) {

        fclose(file_ent);

    }

    builder_destroy(builder);

    fclose(file_ob);
    fclose(file_am);
    fclose(file_as);

    return 0;

}

/* main function */
int main(int argc, char* argv[]) {

    int index;

    if (argc < 2) {

        fprintf(
            stderr,
            "Use: %s <file-prefixes>\n",
            argv[0]
        );

        return 0;

    }

    for (index = 1; index < argc; index++) {

        one_run(argv[index]);

    }

    return 0;

}