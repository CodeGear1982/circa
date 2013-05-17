// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "control_flow.h"
#include "kernel.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

struct BlobData {
    int refCount;
    int length;
    char data[0];
};

void incref(BlobData* data)
{
    data->refCount++;
}

void decref(BlobData* data)
{
    ca_assert(data->refCount > 0);
    data->refCount--;
    if (data->refCount == 0) {
        free(data);
    }
}

BlobData* blob_create(int length)
{
    BlobData* result = (BlobData*) malloc(sizeof(BlobData) + length);
    result->refCount = 1;
    result->length = length;
    return result;
}

BlobData* blob_duplicate(BlobData* original)
{
    BlobData* dupe = blob_create(original->length);
    memcpy(dupe->data, original->data, original->length);
    return dupe;
}

void blob_touch(BlobData** blob)
{
    ca_assert((*blob)->refCount > 0);
    if ((*blob)->refCount == 1)
        return;

    BlobData* dup = blob_duplicate(*blob);
    decref(*blob);
    *blob = dup;
}

void blob_resize(BlobData** existing, int length)
{
    if (*existing == NULL) {
        *existing = blob_create(length);
        return;
    }

    if ((*existing)->refCount == 1) {
        *existing = (BlobData*) realloc(*existing, sizeof(BlobData) + length);
        (*existing)->length = length;
        return;
    }

    BlobData* resized = blob_create(length);
    memcpy(resized->data, (*existing)->data, std::min(length, (*existing)->length));
    decref(*existing);
    *existing = resized;
}

int blob_size(BlobData* data)
{
    return data->length;
}

int blob_size(caValue* blob)
{
    ca_assert(is_blob(blob));
    return blob_size((BlobData*) blob->value_data.ptr);
}

void blob_resize(caValue* blob, int size)
{
    ca_assert(is_blob(blob));
    blob_resize((BlobData**) &blob->value_data.ptr, size);
}

void blob_append_char(caValue* blob, char c)
{
    int size = blob_size(blob);
    blob_resize(blob, size + 1);
    as_blob(blob)[size] = c;
}

void blob_append_int(caValue* blob, unsigned int val)
{
    int size = blob_size(blob);
    blob_resize(blob, size + 4);

    unsigned int* position = (unsigned int*) &as_blob(blob)[size];

    // Convert from little-endian to big-endian. TODO, this should not convert when
    // compiled on big-endian arch.
    *position = ((val & 0xff000000) >> 24) + ((val & 0x00ff0000) >> 8)
        + ((val & 0x0000ff00) << 8) + ((val & 0x000000ff) << 24);
}

char blob_read_char(char* data, int* pos)
{
    char c = data[*pos];
    *pos += 1;
    return c;
}

unsigned int blob_read_int(char* data, int* pos)
{
    int val = *((unsigned int*) &data[*pos]);
    *pos += 4;
    return ((val & 0xff000000) >> 24) + ((val & 0x00ff0000) >> 8)
        + ((val & 0x0000ff00) << 8) + ((val & 0x000000ff) << 24);
}

static char to_hex_digit(int i)
{
    if (i >= 0 && i < 10)
        return '0' + i;
    return 'a' + (i - 10);
}

void blob_to_hex_string(caValue* blob, caValue* str)
{
    set_string(str, "");

    for (int i=0; i < blob_size(blob); i++) {
        char c = as_blob(blob)[i];

        string_append_char(str, to_hex_digit(c / 16));
        string_append_char(str, to_hex_digit(c % 16));
    }
}

bool is_blob(caValue* value)
{
    return value->value_type == TYPES.blob;
}

char* as_blob(caValue* value)
{
    ca_assert(is_blob(value));
    return ((BlobData*) value->value_data.ptr)->data;
}

void set_blob(caValue* value, int length)
{
    change_type(value, TYPES.blob);
    value->value_data.ptr = blob_create(length);
}

std::string blob_toString(caValue* value)
{
    Value asHex;
    blob_to_hex_string(value, &asHex);
    return as_cstring(&asHex);
}

void blob_setup_type(Type* type)
{
    reset_type(type);
    set_string(&type->name, "Blob");
    type->storageType = sym_StorageTypeString;
    type->toString = blob_toString;
}

} // namespace circa