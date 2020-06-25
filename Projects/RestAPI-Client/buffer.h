#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

typedef struct {
    char *data;
    size_t size;
} buffer;

// initializes a buffer
buffer BufferInit();

// destroys a buffer
void BufferDestroy(buffer *buffer);

// adds data of size data_size to a buffer
void BufferAdd(buffer *buffer, const char *data, size_t dataSize);

// checks if a buffer is empty
int BufferIsEmpty(buffer *buffer);

// finds data of size dataSize in a buffer and returns its position
int BufferFind(buffer *buffer, const char *data, size_t dataSize);

// finds data of size dataSize in a buffer in a
// case-insensitive fashion and returns its position
int BufferFindInsensitive(buffer *buffer, const char *data, size_t dataSize);