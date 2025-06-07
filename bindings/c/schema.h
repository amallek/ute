

#ifndef UTE_SCHEMA_H
#define UTE_SCHEMA_H

#include <stddef.h>
#include <stdint.h>

// Forward declare YAML types for header
typedef struct yaml_document_s yaml_document_t;
typedef struct yaml_node_s yaml_node_t;

// UTE field types
#define UTE_TYPE_NULL 0
#define UTE_TYPE_BOOL 1
#define UTE_TYPE_INT 2
#define UTE_TYPE_STRING 3
#define UTE_TYPE_LIST 4
#define UTE_TYPE_STRUCT 5

// Field definition
struct ute_field
{
    const char *name;
    int type;
    const struct ute_field *elem;   // for lists
    const struct ute_field *fields; // for structs
    size_t num_fields;
    size_t buf_size; // capacity for string values (0 if unspecified)
    size_t offset; // offset within struct (for struct fields)
};

// Schema version definition
struct ute_schema_version
{
    int version;
    const struct ute_field *fields;
    size_t num_fields;
};

// Schema definition (multi-version)
struct ute_schema
{
    const struct ute_schema_version *versions;
    size_t num_versions;
};

#ifdef __cplusplus
extern "C"
{
#endif

    // Recursively parse a YAML node into a ute_field (returns 0 on success, -1 on error)
    int ParseSchemaField(yaml_document_t *doc, yaml_node_t *node, struct ute_field *out_field);
    // Parse a YAML schema file and build a ute_schema (multi-version aware)
    int ParseSchema(const char *filename, struct ute_schema *out_schema);
    // Free all memory allocated for a ute_schema (recursively)
    void FreeSchema(struct ute_schema *schema);

#ifdef __cplusplus
}
#endif

#endif // UTE_SCHEMA_H
