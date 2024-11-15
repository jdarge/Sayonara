// TODO : log all stdout messages into a file located in C:\Program Files\sayonara\log
// TODO : doesnt work when non ascii character present

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <windows.h>

// #define BUFFER_SIZE 65536  // 64 KB buffer size
#define MIN_BUFFER_SIZE 4096   // 4 KB
#define MAX_BUFFER_SIZE 1048576 // 1 MB

#define MAX_PATH_SIZE 256

int secure_delete(const char *filename);
int overwrite_file (const char *filename);
// int verify_overwrite (const char *filename);
long long get_file_size(const char *filename);
unsigned long long get_available_memory ();
int calculate_dynamic_buffer_size (long long file_size);

int main(int argc, char *argv[]) {

    // TODO!    : recursive delete when a directory is given
    
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return 1;
    }

    clock_t start_time = clock();

    for (int i = 1; i < argc; i++) 
    {

        char file_path[MAX_PATH];  // Buffer to store the file path
        DWORD result = GetFullPathName(argv[1], MAX_PATH, file_path, NULL);
    
        if (result == 0) {
            fprintf(stderr, "Error getting full path: %lu\n", GetLastError());
            continue;
        }

        // Print the size of the file.
        long long file_size = get_file_size(argv[i]);
        if (file_size == -1) {
            fprintf(stderr, "Unable to retrieve file size.");
            continue;
        }
        // fprintf(stderr, "[ %s ] file size: %lld bytes\n", file_path, file_size);

        // Attempt to securely delete selected file. 
        if (secure_delete(argv[i]) != 0) 
        {
            fprintf(stderr, "Failed to securely delete [ %s ].\n", file_path);
        }
    }

    clock_t end_time = clock();

    // Display elapsed time. 
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Time taken for secure delete: %.2f seconds\n", elapsed_time);

    return 0;
}

int secure_delete(const char *filename) {

    if (overwrite_file(filename) != 0) 
    {
        fprintf(stderr, "Failed to overwrite the file.\n");
        return -1;
    }

    /*
    if (verify_overwrite(filename) != 0) {
        fprintf(stderr, "Failed to verify file overwrite.\n");
        return -1;
    }
    */

    if (remove(filename) != 0) 
    {
        perror("Error deleting file");
        return -1;
    }

    printf("File securely deleted: %s\n", filename);

    return 0;
}

int overwrite_file (const char *filename) {

    HANDLE file;
    LARGE_INTEGER file_size;
    DWORD bytes_written;
    unsigned long long remaining_bytes;
    unsigned char *buffer;
    unsigned int buffer_size;

    file = CreateFileA (
        filename,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) 
    {
        perror("Error opening file");
        return -1;
    }

    if (!GetFileSizeEx(file, &file_size)) 
    {
        perror("Error getting file size");
        CloseHandle(file);
        return -1;
    }

    remaining_bytes = file_size.QuadPart;

    buffer_size = calculate_dynamic_buffer_size(file_size.QuadPart);
    buffer = (unsigned char *)malloc(buffer_size);

    if (!buffer) 
    {
        perror("Error allocating memory for buffer");
        CloseHandle(file);
        return -1;
    }

    memset(buffer, 0xFF, buffer_size);

    while (remaining_bytes > 0) 
    {
        DWORD chunk_size = (remaining_bytes < buffer_size) ? (DWORD)remaining_bytes : buffer_size;
        
        if (!WriteFile(file, buffer, chunk_size, &bytes_written, NULL) || bytes_written != chunk_size) 
        {
            perror("Error overwriting file");
            CloseHandle(file);
            return -1;
        }
        
        remaining_bytes -= chunk_size;
    }

    if (!FlushFileBuffers(file)) 
    {
        perror("Error flushing file buffers");
        CloseHandle(file);
        return -1;
    }

    CloseHandle(file);

    return 0;
}

long long get_file_size(const char *filename) {

    HANDLE file;
    LARGE_INTEGER file_size;

    file = CreateFile (
        filename, 
        GENERIC_READ, 
        0, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL
    );

    if (file == INVALID_HANDLE_VALUE)
    {
        perror("Failed to open file");
        return -1;
    }

    if (!GetFileSizeEx(file, &file_size)) 
    {
        perror("Failed to get file size");
        CloseHandle(file);
        return -1;
    }

    CloseHandle(file);

    return file_size.QuadPart;
}

unsigned long long get_available_memory () {

    MEMORYSTATUSEX memory_64;

    memory_64.dwLength = sizeof(memory_64);

    if (GlobalMemoryStatusEx(&memory_64))
        return memory_64.ullAvailPhys;

    return 0;
}

int calculate_dynamic_buffer_size (long long file_size) {

    unsigned long long available_memory = get_available_memory();
    int buffer_size = MIN_BUFFER_SIZE;

    if (available_memory == 0) 
    {
        perror("Error getting available memory");
        return MIN_BUFFER_SIZE;
    }

    if (file_size <= 10 * 1024 * 1024)
    {
        return MIN_BUFFER_SIZE;
    }

    buffer_size = (int)(available_memory / 100); 
    
    if (buffer_size > MAX_BUFFER_SIZE) 
    {
        buffer_size = MAX_BUFFER_SIZE;   
    }
    
    if (file_size < buffer_size && file_size > 0) 
    {
        buffer_size = (int)file_size;
    }

    return buffer_size;
}

/*
int verify_overwrite (const char *filename) {

    HANDLE file;
    LARGE_INTEGER file_size;
    DWORD bytes_read;
    long long remaining_bytes;
    unsigned char buffer[BUFFER_SIZE];
    unsigned char verify_buffer[BUFFER_SIZE];

    file = CreateFileA (
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) 
    {
        perror("Error opening file for verification");
        return -1;
    }
    
    if (!GetFileSizeEx(file, &file_size)) 
    {
        perror("Error getting file size for verification");
        CloseHandle(file);
        return -1;
    }
    
    memset(verify_buffer, 0xFF, BUFFER_SIZE);
    remaining_bytes = file_size.QuadPart;

    while (remaining_bytes > 0) 
    {

        DWORD chunk_size = (remaining_bytes < BUFFER_SIZE) ? (DWORD)remaining_bytes : BUFFER_SIZE;
        
        if (!ReadFile(file, buffer, chunk_size, &bytes_read, NULL) || bytes_read != chunk_size) 
        {
            perror("Error reading file for verification");
            CloseHandle(file);
            return -1;
        }

        if (memcmp(buffer, verify_buffer, chunk_size) != 0) 
        {
            fprintf(stderr, "Verification failed: File not fully overwritten.\n");
            CloseHandle(file);
            return -1;
        }

        remaining_bytes -= chunk_size;
    }

    CloseHandle(file);

    return 0;
}
*/