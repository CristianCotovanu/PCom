#include "buffer.h"

buffer BufferInit() {
    buffer buffer;

    buffer.data = nullptr;
    buffer.size = 0;

    return buffer;
}

void BufferDestroy(buffer *buffer) {
    if (buffer->data != nullptr) {
        free(buffer->data);
        buffer->data = nullptr;
    }

    buffer->size = 0;
}

int BufferIsEmpty(buffer *buffer) {
    return buffer->data == nullptr;
}

void BufferAdd(buffer *buffer, const char *data, size_t dataSize) {
    if (buffer->data != nullptr) {
        buffer->data = (char *) realloc(buffer->data, (buffer->size + dataSize) * sizeof(char));
    } else {
        buffer->data = (char *) calloc(dataSize, sizeof(char));
    }

    memcpy(buffer->data + buffer->size, data, dataSize);

    buffer->size += dataSize;
}

int BufferFind(buffer *buffer, const char *data, size_t dataSize) {
    if (dataSize > buffer->size)
        return -1;

    size_t last_pos = buffer->size - dataSize + 1;

    for (size_t i = 0; i < last_pos; ++i) {
        size_t j;

        for (j = 0; j < dataSize; ++j) {
            if (buffer->data[i + j] != data[j]) {
                break;
            }
        }

        if (j == dataSize)
            return i;
    }

    return -1;
}

int BufferFindInsensitive(buffer *buffer, const char *data, size_t dataSize) {
    if (dataSize > buffer->size)
        return -1;

    size_t last_pos = buffer->size - dataSize + 1;

    for (size_t i = 0; i < last_pos; ++i) {
        size_t j;

        for (j = 0; j < dataSize; ++j) {
            if (tolower(buffer->data[i + j]) != tolower(data[j])) {
                break;
            }
        }

        if (j == dataSize)
            return i;
    }

    return -1;
}
