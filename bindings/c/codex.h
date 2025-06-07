#ifndef UTE_CODEX_H
#define UTE_CODEX_H

#include <stddef.h>
#include <stdint.h>

// Returned when serialization or deserialization fails due to insufficient
// buffer space. Since size_t is unsigned, this uses the maximum value as an
// error sentinel.
#define UTE_BUF_ERROR ((size_t)-1)

#ifdef __cplusplus
extern "C"
{
#endif

    // Serialize a C struct (as a map) to UTE binary format
    size_t ute_serialize(const void *data, const void *schema, uint8_t *out_buf, size_t out_buf_size);

    // Deserialize UTE binary data to a C struct (as a map)
    size_t ute_deserialize(const uint8_t *in_buf, size_t in_buf_size, const void *schema, void *out_data);

#ifdef __cplusplus
}
#endif

#endif // UTE_CODEX_H
