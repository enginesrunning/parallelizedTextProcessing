#define _CRT_SECURE_NO_WARNINGS
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
    int line_count;
    int word_count;
    int char_count;
    int vowel_count;
} TextStats;

int is_vowel(char c) {
    c = tolower((unsigned char)c);  // cast here
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

TextStats process_text_chunk(const char* chunk, int chunk_size, int prev_ends_in_word) {
    TextStats stats = { 0, 0, 0, 0 };
    if (!chunk || chunk_size <= 0)
        return stats;

    int in_word = prev_ends_in_word ? 1 : 0;

    for (int i = 0; i < chunk_size; i++) {
        char c = chunk[i];

        stats.char_count++;

        if (is_vowel(c)) {
            stats.vowel_count++;
        }
        if (c == '\n') {
            stats.line_count++;
        }

        if (isalnum((unsigned char)c)) {  // cast here
            if (!in_word) {
                stats.word_count++;
                in_word = 1;
            }
        }
        else {
            in_word = 0;
        }
    }

    return stats;
}

int main(int argc, char* argv[]) {
    int rank, size;
    const char* filename = "C:/Users/Cristi/source/repos/mspmi/lab1/giantfile.txt";
    FILE* file = NULL;
    long file_size = 0;
    char* file_content = NULL;
    char* local_chunk = NULL;
    int* sendcounts = NULL;
    int* displs = NULL;
    TextStats local_stats = { 0 }, global_stats = { 0 };
    double start_time = 0.0, end_time = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 1) {
        if (rank == 0) {
            file = fopen(filename, "rb");
            if (!file) {
                printf("Error: Cannot open file '%s'\n", filename);
                MPI_Finalize();
                return 1;
            }
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (file_size <= 0) {
                printf("Error: File is empty\n");
                fclose(file);
                MPI_Finalize();
                return 1;
            }

            file_content = (char*)malloc(file_size);
            if (!file_content) {
                printf("Error: Cannot allocate memory\n");
                fclose(file);
                MPI_Finalize();
                return 1;
            }

            fread(file_content, 1, file_size, file);
            fclose(file);

            start_time = MPI_Wtime();

            local_stats = process_text_chunk(file_content, file_size, 0);

            end_time = MPI_Wtime();

            printf("\n=== FINAL RESULTS ===\n");
            printf("Total lines: %d\n", local_stats.line_count);
            printf("Total words: %d\n", local_stats.word_count);
            printf("Total characters: %d\n", local_stats.char_count);
            printf("Total vowels: %d\n", local_stats.vowel_count);
            printf("Processing time: %.4f seconds\n", end_time - start_time);
            printf("Processes used: %d\n", size);

            free(file_content);
        }
        MPI_Finalize();
        return 0;
    }

    if (rank == 0) {
        file = fopen(filename, "rb");
        if (!file) {
            printf("Error: Cannot open file '%s'\n", filename);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (file_size <= 0) {
            printf("Error: File is empty\n");
            fclose(file);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        file_content = (char*)malloc(file_size);
        if (!file_content) {
            printf("Error: Cannot allocate memory\n");
            fclose(file);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fread(file_content, 1, file_size, file);
        fclose(file);
    }

    MPI_Bcast(&file_size, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    sendcounts = (int*)malloc(size * sizeof(int));
    displs = (int*)malloc(size * sizeof(int));

    int base_chunk = file_size / size;
    int remainder = file_size % size;

    for (int i = 0; i < size; i++) {
        sendcounts[i] = base_chunk + (i < remainder ? 1 : 0);
        displs[i] = (i == 0) ? 0 : displs[i - 1] + sendcounts[i - 1];
    }

    local_chunk = (char*)malloc(sendcounts[rank]);
    if (!local_chunk) {
        printf("Process %d: Cannot allocate memory\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatterv(file_content, sendcounts, displs, MPI_CHAR,
        local_chunk, sendcounts[rank], MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        free(file_content);
    }

    int prev_ends_in_word = 0;

    if (rank == 0) {
        prev_ends_in_word = 0;
    }
    else {
        MPI_Recv(&prev_ends_in_word, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    start_time = MPI_Wtime();

    local_stats = process_text_chunk(local_chunk, sendcounts[rank], prev_ends_in_word);

    int current_ends_in_word = 0;
    if (sendcounts[rank] > 0) {
        char last_char = local_chunk[sendcounts[rank] - 1];
        current_ends_in_word = (isalnum((unsigned char)last_char)) ? 1 : 0;
    }

    if (rank < size - 1) {
        MPI_Send(&current_ends_in_word, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    end_time = MPI_Wtime();

    MPI_Reduce(&local_stats.line_count, &global_stats.line_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_stats.word_count, &global_stats.word_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_stats.char_count, &global_stats.char_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_stats.vowel_count, &global_stats.vowel_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("\n=== FINAL RESULTS ===\n");
        printf("Total lines: %d\n", global_stats.line_count);
        printf("Total words: %d\n", global_stats.word_count);
        printf("Total characters: %d\n", global_stats.char_count);
        printf("Total vowels: %d\n", global_stats.vowel_count);
        printf("Processing time: %.4f seconds\n", end_time - start_time);
        printf("Processes used: %d\n", size);
    }

    free(local_chunk);
    free(sendcounts);
    free(displs);
    MPI_Finalize();
    return 0;
}
