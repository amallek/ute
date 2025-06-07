
#include "codex.h"
#include "schema.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Sentinel value returned when buffers are too small
#define ERR UTE_BUF_ERROR

// Macro to ensure there is enough space remaining in an output buffer
#define ENSURE_SPACE(wanted)               \
    do                                     \
    {                                      \
        if (out_size - written < (wanted)) \
            return ERR;                    \
    } while (0)

// Macro to ensure there is enough data left in an input buffer
#define ENSURE_RSPACE(wanted)          \
    do                                 \
    {                                  \
        if (in_size - read < (wanted)) \
            return ERR;                \
    } while (0)

// =========================================================
// Ultra Tiny Encoding (UTE) - Serialization/Deserialization
// =========================================================

// Internal helpers (static)
static size_t ute_encode_varint(uint64_t n, uint8_t *out);
static size_t ute_decode_varint(const uint8_t *in, size_t in_size, uint64_t *out);
static size_t ute_write_field(const struct ute_field *field, const void *value, uint8_t *out, size_t out_size);
static size_t ute_read_field(const struct ute_field *field, const uint8_t *in, size_t in_size, void *value);

// -------------------------
// Public API
// -------------------------

// Serialize data according to schema
size_t ute_serialize(const void *data, const void *schema, uint8_t *out_buf, size_t out_buf_size)
{
    // data: pointer to array of pointers (one per top-level field)
    // schema: pointer to ute_field array (top-level fields)
    const struct ute_field *fields = (const struct ute_field *)schema;
    size_t num_fields = 1; // for demo, only one top-level field ("devices")
    size_t written = 0;
    for (size_t i = 0; i < num_fields; ++i)
    {
        size_t sub = ute_write_field(&fields[i], ((const void *const *)data)[i], out_buf + written, out_buf_size - written);
        if (sub == ERR)
            return ERR;
        written += sub;
    }
    return written;
}

// Deserialize data according to schema
size_t ute_deserialize(const uint8_t *in_buf, size_t in_buf_size, const void *schema, void *out_data)
{
    // schema: pointer to ute_field array (top-level fields)
    const struct ute_field *fields = (const struct ute_field *)schema;
    size_t num_fields = 1; // for demo, only one top-level field ("devices")
    size_t read = 0;
    for (size_t i = 0; i < num_fields; ++i)
    {
        size_t sub = ute_read_field(&fields[i], in_buf + read, in_buf_size - read, ((void **)out_data)[i]);
        if (sub == ERR)
            return ERR;
        read += sub;
    }
    return read;
}

// -------------------------
// Internal helpers (static)
// -------------------------

// Encode varint (returns bytes written)
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

// Decode varint (returns bytes read)
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

// Write a field value to buffer (recursive for struct/list)
static size_t ute_write_field(const struct ute_field *field, const void *value, uint8_t *out, size_t out_size)
{
    size_t written = 0;
    if (!field || !out)
        return ERR;
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
            return ERR;
        }
        uint64_t v = *(const uint64_t *)value;
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_INT: value=%llu\n", (unsigned long long)v);
#endif
        ENSURE_SPACE(1);
        out[written++] = (2 << 5); // tInt
        uint8_t tmp[10];
        size_t var_len = ute_encode_varint(v, tmp);
        ENSURE_SPACE(var_len);
        memcpy(out + written, tmp, var_len);
        written += var_len;
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
            return ERR;
        }
        const char *s = (const char *)value;
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_STRING: value='%s'\n", s);
#endif
        ENSURE_SPACE(1);
        out[written++] = (3 << 5); // tBytes
        size_t len = strlen(s);
        uint8_t tmp[10];
        size_t var_len = ute_encode_varint(len, tmp);
        ENSURE_SPACE(var_len + len);
        memcpy(out + written, tmp, var_len);
        written += var_len;
        memcpy(out + written, s, len);
        written += len;
        break;
    }
    case UTE_TYPE_LIST:
    {
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_LIST: value ptr=%p\n", value);
#endif
        ENSURE_SPACE(1);
        out[written++] = (4 << 5); // tList
        size_t count = (size_t)(uintptr_t)(((const void **)value)[0]);
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_LIST: count=%zu\n", count);
#endif
        uint8_t tmp[10];
        size_t var_len = ute_encode_varint(count, tmp);
        ENSURE_SPACE(var_len);
        memcpy(out + written, tmp, var_len);
        written += var_len;
        const void **arr = &((const void **)value)[1];
        for (size_t i = 0; i < count; ++i)
        {
#ifdef UTE_DEBUG
            printf("    UTE_TYPE_LIST: arr[%zu]=%p\n", i, arr[i]);
#endif
            size_t sub = ute_write_field(field->elem, arr[i], out + written, out_size - written);
            if (sub == ERR)
                return ERR;
            written += sub;
        }
        break;
    }
    case UTE_TYPE_STRUCT:
    {
#ifdef UTE_DEBUG
        printf("  UTE_TYPE_STRUCT: struct_data=%p, num_fields=%zu\n", value, field->num_fields);
#endif
        ENSURE_SPACE(1);
        out[written++] = (5 << 5); // tStruct
        uint8_t tmp[10];
        size_t var_len = ute_encode_varint(field->num_fields, tmp);
        ENSURE_SPACE(var_len);
        memcpy(out + written, tmp, var_len);
        written += var_len;
        const void *struct_data = value;
        for (size_t i = 0; i < field->num_fields; ++i)
        {
#ifdef UTE_DEBUG
            printf("    UTE_TYPE_STRUCT: field %zu: name=%s offset=%zu\n", i, field->fields[i].name, field->fields[i].offset);
#endif
            size_t sub = ute_write_field(&field->fields[i], (const char *)struct_data + field->fields[i].offset, out + written, out_size - written);
            if (sub == ERR)
                return ERR;
            written += sub;
        }
        break;
    }
    default:
        break;
    }
    return written;
}

// Read a field value from buffer (recursive for struct/list)
static size_t ute_read_field(const struct ute_field *field, const uint8_t *in, size_t in_size, void *value)
{
    size_t read = 0;
    if (!field || !in)
        return ERR;
    ENSURE_RSPACE(1);
    uint8_t h = in[read++];
    switch (field->type)
    {
    case UTE_TYPE_INT:
    {
        if ((h >> 5) != 2)
            return ERR;
        uint64_t v = 0;
        size_t var_len = ute_decode_varint(in + read, in_size - read, &v);
        if (var_len == 0 || read + var_len > in_size)
            return ERR;
        read += var_len;
        *(uint64_t *)value = v;
        break;
    }
    case UTE_TYPE_STRING:
    {
        if ((h >> 5) != 3)
            return ERR;
        uint64_t len = 0;
        size_t var_len = ute_decode_varint(in + read, in_size - read, &len);
        if (var_len == 0 || read + var_len + len > in_size)
            return ERR;
        read += var_len;
        memcpy(value, in + read, len);
        ((char *)value)[len] = 0;
        read += len;
        break;
    }
    case UTE_TYPE_LIST:
    {
        if ((h >> 5) != 4)
            return ERR;
        uint64_t count = 0;
        size_t var_len = ute_decode_varint(in + read, in_size - read, &count);
        if (var_len == 0 || read + var_len > in_size)
            return ERR;
        read += var_len;
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
            size_t sub = ute_read_field(field->elem, in + read, in_size - read, arr[i]);
            if (sub == ERR)
                return ERR;
            read += sub;
        }
        break;
    }
    case UTE_TYPE_STRUCT:
    {
        if ((h >> 5) != 5)
            return ERR;
        uint64_t nfields = 0;
        size_t var_len = ute_decode_varint(in + read, in_size - read, &nfields);
        if (var_len == 0 || read + var_len > in_size)
            return ERR;
        read += var_len;
        for (size_t i = 0; i < nfields; ++i)
        {
            size_t sub = ute_read_field(&field->fields[i], in + read, in_size - read, (char *)value + i * sizeof(void *));
            if (sub == ERR)
                return ERR;
            read += sub;
        }
        break;
    }
    default:
        break;
    }
    return read;
}
