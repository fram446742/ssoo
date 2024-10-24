#include <errno.h>
#include <fcntl.h>
#include <immintrin.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct
{
    char *mapped_file;
    size_t start;
    size_t end;
    size_t file_size;
    size_t position;
    pthread_mutex_t *mutex;
    double *results;
    int thread_id;
    int num_threads;
    double sum;
} ThreadData;

// Helper function to convert a string to a double
double fast_strtod(const char *str, char **endptr) {
    double result = 0.0, factor = 1.0;
    int decimal_found = 0;

    // Parse the base part of the number (before the 'e')
    while (*str && *str != '\n' && *str != 'e') {
        if (*str >= '0' && *str <= '9') {
            if (decimal_found) {
                factor *= 0.1;
                result += (*str - '0') * factor;
            } else {
                result = result * 10.0 + (*str - '0');
            }
        } else if (*str == '.') {
            decimal_found = 1;
        }
        str++;
    }

    // Handle 'e-' (skip 'e' and '-')
    if (*str == 'e') {
        int exponent = 0;
        str += 2; // Skip 'e' and '-'
        while (*str >= '0' && *str <= '9') {
            exponent = exponent * 10 + (*str - '0');
            str++;
        }
        result *= pow(10, -exponent);
    }

    if (endptr)
        *endptr = (char *)str;

    return result;
}

// METHOD 1
void *method1_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double local_sum = 0.0;
    char *ptr;

    while (1) {
        pthread_mutex_lock(data->mutex);
        if (data->position >= data->file_size) {
            pthread_mutex_unlock(data->mutex);
            break;
        }
        ptr = data->mapped_file + data->position;
        while (data->position < data->file_size && data->mapped_file[data->position] != '\n') {
            data->position++;
        }
        data->position++;
        pthread_mutex_unlock(data->mutex);

        if (ptr < data->mapped_file + data->position) {
            char *end;
            local_sum += fast_strtod(ptr, &end);
        }
    }

    double *result = malloc(sizeof(double));
    *result = local_sum;
    return result;
}

double method1(const char *filename, int num_threads) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }
    struct stat sb;
    if (fstat(fd, &sb) < 0) {
        perror("Error getting file stats");
        close(fd);
        exit(1);
    }
    size_t file_size = sb.st_size;
    char *mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    // Advise the kernel for sequential access
    madvise(mapped_file, file_size, MADV_SEQUENTIAL);

    // // random acces
    // madvise(mapped_file, file_size, MADV_RANDOM);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    ThreadData shared_data = {mapped_file, 0, 0, file_size, 0, &mutex, NULL, 0, num_threads, 0.0};

    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, method1_worker, &shared_data);

    double total_sum = 0.0;
    for (int i = 0; i < num_threads; i++) {
        double *local_sum;
        pthread_join(threads[i], (void **)&local_sum);
        total_sum += *local_sum;
        free(local_sum);
    }

    pthread_mutex_destroy(&mutex);
    munmap(mapped_file, file_size);
    close(fd);

    return total_sum;
}

// METHOD 2
void *method2_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double local_sum = 0.0;
    char *ptr = data->mapped_file + data->start;
    char *end = data->mapped_file + data->end;

    while (ptr < end) {
        char *num_end;
        local_sum += fast_strtod(ptr, &num_end);
        ptr = num_end;
        while (ptr < end && (*ptr == '\n' || *ptr == ' ' || *ptr == '\t'))
            ptr++;
    }

    data->sum = local_sum;
    return NULL;
}

double method2(const char *filename, int num_threads) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }
    struct stat sb;
    if (fstat(fd, &sb) < 0) {
        perror("Error getting file stats");
        close(fd);
        exit(1);
    }
    size_t file_size = sb.st_size;
    char *mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    // Advise the kernel for sequential access
    madvise(mapped_file, file_size, MADV_SEQUENTIAL);

    size_t line_count = 0;
    for (size_t i = 0; i < file_size; i++)
        if (mapped_file[i] == '\n')
            line_count++;

    size_t lines_per_thread = line_count / num_threads;
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    size_t current_position = 0, current_line = 0;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].mapped_file = mapped_file;
        thread_data[i].start = current_position;
        thread_data[i].sum = 0.0;

        size_t lines_in_chunk = (i == num_threads - 1) ? (line_count - current_line) : lines_per_thread;
        while (current_position < file_size && lines_in_chunk > 0)
            if (mapped_file[current_position++] == '\n')
                lines_in_chunk--;

        thread_data[i].end = current_position;
        pthread_create(&threads[i], NULL, method2_worker, &thread_data[i]);

        current_line += lines_per_thread;
    }

    double total_sum = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_sum += thread_data[i].sum;
    }

    munmap(mapped_file, file_size);
    close(fd);

    return total_sum;
}

// METHOD 3
void *method3_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double local_sum = 0.0;
    char *ptr = data->mapped_file;
    char *end = data->mapped_file + data->file_size;
    int thread_id = data->thread_id;
    int num_threads = data->num_threads;

    size_t line_num = 0;
    while (ptr < end) {
        char *num_end;
        if (line_num % num_threads == thread_id) {
            local_sum += fast_strtod(ptr, &num_end);
        } else {
            num_end = ptr;
            while (num_end < end && *num_end != '\n')
                num_end++; // Find the next newline
        }

        if (num_end)
            ptr = num_end + 1;
        else
            break;

        line_num++;
    }

    data->sum = local_sum;
    return NULL;
}

double method3(const char *filename, int num_threads) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }
    struct stat sb;
    if (fstat(fd, &sb) < 0) {
        perror("Error getting file stats");
        close(fd);
        exit(1);
    }
    size_t file_size = sb.st_size;
    char *mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    // Advise the kernel for sequential access
    madvise(mapped_file, file_size, MADV_RANDOM); // TEST

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].mapped_file = mapped_file;
        thread_data[i].file_size = file_size;
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].sum = 0.0;
        pthread_create(&threads[i], NULL, method3_worker, &thread_data[i]);
    }

    double total_sum = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_sum += thread_data[i].sum;
    }

    munmap(mapped_file, file_size);
    close(fd);

    return total_sum;
}

// METHOD 4
void *method4_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double local_sum = 0.0;
    char *ptr = data->mapped_file + data->start;
    char *end = data->mapped_file + data->end;

    while (ptr < end) {
        char *num_end;
        local_sum += fast_strtod(ptr, &num_end);
        ptr = num_end + 1;
    }

    data->sum = local_sum;
    return NULL;
}

double method4(const char *filename, int num_threads) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        exit(1);
    }
    struct stat sb;
    if (fstat(fd, &sb) < 0) {
        perror("Error getting file stats");
        close(fd);
        exit(1);
    }
    size_t file_size = sb.st_size;
    char *mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        exit(1);
    }

    // Advise the kernel for sequential access
    madvise(mapped_file, file_size, MADV_SEQUENTIAL);

    // Create thread structures
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    size_t chunk_size = file_size / num_threads;

    // Pre-calculate boundaries before thread creation
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].mapped_file = mapped_file;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? file_size : (i + 1) * chunk_size;

        if (i > 0) {
            char *new_start = memchr(mapped_file + thread_data[i].start, '\n', file_size - thread_data[i].start);
            if (new_start != NULL) {
                thread_data[i].start = (new_start - mapped_file) + 1;
            }
        }

        // Ensure thread ends at the end of a number
        if (i < num_threads - 1) {
            char *new_end = memchr(mapped_file + thread_data[i].end, '\n', file_size - thread_data[i].end);
            if (new_end != NULL) {
                thread_data[i].end = (new_end - mapped_file);
            }
        }

        pthread_create(&threads[i], NULL, method4_worker, &thread_data[i]);
    }

    // Aggregate results
    double total_sum = 0.0;

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);

        total_sum += thread_data[i].sum;
    }

    munmap(mapped_file, file_size);
    close(fd);

    return total_sum;
}

// Function to count the number of lines in a file
size_t count_lines(const char *filename) {
    // Open the file
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Could not open file %s: %s\n", filename, strerror(errno));
        return 0;
    }

    // Get the file size
    struct stat st;
    if (fstat(fd, &st) == -1) {
        fprintf(stderr, "Could not get file size: %s\n", strerror(errno));
        close(fd);
        return 0;
    }
    size_t filesize = st.st_size;

    // Memory-map the file
    char *file_data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        fprintf(stderr, "Memory mapping failed: %s\n", strerror(errno));
        close(fd);
        return 0;
    }

    close(fd);

    size_t number_count = 0;
    char *start = file_data;
    char *end = file_data + filesize;

    while (start < end) {
        // Skip any empty or whitespace-only lines
        char *line_start = start;
        while (start < end && *start != '\n')
            start++;

        if (start > line_start)
            number_count++;

        start++;
    }

    munmap(file_data, filesize);

    return number_count;
}

// Function pointer type for methods
typedef double (*SumMethod)(const char *, int);

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <filename> <num_threads> <method>\n", argv[0]);
        return 1;
    }

    // Get the number of threads from the command-line argument
    int num_threads = atoi(argv[2]);
    if (num_threads <= 0) {
        fprintf(stderr, "Invalid number of threads: %d\n", num_threads);
        return 1;
    }

    // Get the method from the command-line argument
    int method = atoi(argv[3]);
    if (method < 1 || method > 4) {
        fprintf(stderr, "Invalid method: %d\n", method);
        return 1;
    }

    // First count the number of lines in the file
    size_t total_num = count_lines(argv[1]);

    // Check if verbose is requested
    // int verbose = (argc == 5 && strcmp(argv[4], "-v") == 0);

    // Choose the appropriate method
    SumMethod selected_method;
    switch (method) {
    case 1:
        selected_method = method1;
        break;
    case 2:
        selected_method = method2;
        break;
    case 3:
        selected_method = method3;
        break;
    case 4:
        selected_method = method4;
        break;
    default:
        fprintf(stderr, "Invalid method: %d\n", method);
        return 1;
    }

    // Start timing
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Execute the selected method
    double total_sum = selected_method(argv[1], num_threads);
    double elapsed_time = 0.0;

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // if (verbose) {
    //     printf("Execution Time: %lf seconds\n", elapsed_time);
    //     // Check if the sum is correct
    //     double expected_sum = 4998823.098919;
    //     if (fabs(total_sum - expected_sum) < 1e-6) {
    //         printf("Sum is correct.\n");
    //     } else {
    //         printf("Sum is incorrect. Expected: %lf, Got: %lf\n", expected_sum, total_sum);
    //     }
    // }

    printf("El programa ejecuta %d threads y la suma total de %zu números es %lf "
           "en tiempo %lf (s)\n",
           num_threads, total_num, total_sum, elapsed_time);
    printf("Total Sum: %lf\n", total_sum);

    return 0;
}
