#include "codex.h"
#include "schema.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Helper: encode varint (returns bytes written)
static size_t ute_encode_varint(uint64_t n, uint8_t *out)
{
    size_t i = 0;
    while (n >= 0x80)
    {
        out[i++] = (uint8_t)(n | 0x80);
        n >>= 7;
    }
    out[i++] = (uint8_t)n;
    return i;
}

// Helper: decode varint (returns bytes read)
static size_t ute_decode_varint(const uint8_t *in, size_t in_size, uint64_t *out)
{
    size_t i = 0;
    uint64_t result = 0;
    int shift = 0;
    while (i < in_size)
    {
        uint8_t b = in[i++];
        result |= (uint64_t)(b & 0x7F) << shift;
        if (!(b & 0x80))
            break;
        shift += 7;
    }
    *out = result;
    return i;
}

// Serialize a C struct (as a map) to UTE binary format

// Helper: write a field value to buffer (recursive for struct/list)
static size_t ute_write_field(const struct ute_field *field, const void *value, uint8_t *out, size_t out_size)
{
    size_t written = 0;
    if (!field || !out)
        return 0;
#ifdef UTE_DEBUG
    printf("ute_write_field: field=%s type=%d value=%p out=%p out_size=%zu\n",
           field->name ? field->name : "(anon)", field->type, value, out, out_size);
    fflush(stdout);
#endif
    switch (field->type)
    {
    case UTE_TYPE_INT:
    {
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_INT: value ptr=%p\n", value);
#endif
        if (!value)
        {
#ifdef UTE_DEBUG
            printf("  ERROR: value is NULL\n");
#endif
            return 0;
        }
        uint64_t v = *(const uint64_t *)value;
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_INT: value=%llu\n", (unsigned long long)v);
#endif
        if (out_size < 1)
            return 0;
        out[written++] = (2 << 5); // tInt
        written += ute_encode_varint(v, out + written);
        break;
    }
    case UTE_TYPE_STRING:
    {
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_STRING: value ptr=%p\n", value);
#endif
        if (!value)
        {
#ifdef UTE_DEBUG
            printf("  ERROR: value is NULL\n");
#endif
            return 0;
        }
        const char *s = (const char *)value;
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_STRING: value='%s'\n", s);
#endif
        if (out_size < 1)
            return 0;
        out[written++] = (3 << 5); // tBytes
        size_t len = strlen(s);
        written += ute_encode_varint(len, out + written);
        memcpy(out + written, s, len);
        written += len;
        break;
    }
    case UTE_TYPE_LIST:
    {
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_LIST: value ptr=%p\n", value);
#endif
        if (out_size < 1)
            return 0;
        out[written++] = (4 << 5); // tList
        size_t count = (size_t)(uintptr_t)(((const void **)value)[0]);
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_LIST: count=%zu\n", count);
#endif
        written += ute_encode_varint(count, out + written);
        const void **arr = &((const void **)value)[1];
        for (size_t i = 0; i < count; ++i)
        {
#ifdef UTE_DEBUG
            printf("    UTE_TYPE_LIST: arr[%zu]=%p\n", i, arr[i]);
#endif
            written += ute_write_field(field->elem, arr[i], out + written, out_size - written);
        }
        break;
    }
    case UTE_TYPE_STRUCT:
    {
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_STRUCT: struct_data=%p, num_fields=%zu\n", value, field->num_fields);
#endif
        if (out_size < 1)
            return 0;
        out[written++] = (5 << 5); // tStruct
        written += ute_encode_varint(field->num_fields, out + written);
        const void *struct_data = value;
        for (size_t i = 0; i < field->num_fields; ++i)
        {
#ifdef UTE_DEBUG
            printf("    UTE_TYPE_STRUCT: field %zu: name=%s offset=%zu\n", i, field->fields[i].name, field->fields[i].offset);
#endif
            written += ute_write_field(&field->fields[i], (const char *)struct_data + field->fields[i].offset, out + written, out_size - written);
        }
        break;
    }
    default:
        break;
    }
    return written;
}

size_t ute_serialize(const void *data, const void *schema, uint8_t *out_buf, size_t out_buf_size)
{
    // data: pointer to array of pointers (one per top-level field)
    // schema: pointer to ute_field array (top-level fields)
    const struct ute_field *fields = (const struct ute_field *)schema;
    size_t num_fields = 1; // for demo, only one top-level field ("devices")
    size_t written = 0;
    for (size_t i = 0; i < num_fields; ++i)
    {
        written += ute_write_field(&fields[i], ((const void *const *)data)[i], out_buf + written, out_buf_size - written);
    }
    return written;
}

// Deserialize UTE binary data to a C struct (as a map)

// Helper: read a field value from buffer (recursive for struct/list)
static size_t ute_read_field(const struct ute_field *field, const uint8_t *in, size_t in_size, void *value)
{
    size_t read = 0;
    if (!field || !in)
        return 0;
    uint8_t h = in[read++];
    switch (field->type)
    {
    case UTE_TYPE_INT:
    {
        if ((h >> 5) != 2)
            return 0;
        uint64_t v = 0;
        read += ute_decode_varint(in + read, in_size - read, &v);
        *(uint64_t *)value = v;
        break;
    }
    case UTE_TYPE_STRING:
    {
        if ((h >> 5) != 3)
            return 0;
        uint64_t len = 0;
        read += ute_decode_varint(in + read, in_size - read, &len);
        memcpy(value, in + read, len);
        ((char *)value)[len] = 0;
        read += len;
        break;
    }
    case UTE_TYPE_LIST:
    {
        if ((h >> 5) != 4)
            return 0;
        uint64_t count = 0;
        read += ute_decode_varint(in + read, in_size - read, &count);
        size_t *out_count = (size_t *)value;
        *out_count = (size_t)count;
        void **arr = (void **)((size_t *)value + 1);
        // IMPORTANT: arr[i] must point to user-allocated memory for each element.
        for (size_t i = 0; i < count; ++i)
        {
            if (!arr[i])
            {
                // Defensive: skip if pointer is NULL
                continue;
            }
            read += ute_read_field(field->elem, in + read, in_size - read, arr[i]);
        }
        break;
    }
    case UTE_TYPE_STRUCT:
    {
        if ((h >> 5) != 5)
            return 0;
        uint64_t nfields = 0;
        read += ute_decode_varint(in + read, in_size - read, &nfields);
        if (nfields != field->num_fields)
            return 0;
        for (size_t i = 0; i < nfields; ++i)
        {
            read += ute_read_field(&field->fields[i], in + read, in_size - read, (char *)value + field->fields[i].offset);
        }
        break;
    }
    default:
        break;
    }
    return read;
}

size_t ute_deserialize(const uint8_t *in_buf, size_t in_buf_size, const void *schema, void *out_data)
{
    // schema: pointer to ute_field array (top-level fields)
    const struct ute_field *fields = (const struct ute_field *)schema;
    size_t num_fields = 1; // for demo, only one top-level field ("devices")
    size_t read = 0;
    for (size_t i = 0; i < num_fields; ++i)
    {
        read += ute_read_field(&fields[i], in_buf + read, in_buf_size - read, ((void **)out_data)[i]);
    }
    return read;
}
