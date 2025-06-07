#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include "schema.h"
// Recursively free ute_field (and children)
// Must include schema.h before this definition so struct ute_field is visible
static void free_field(struct ute_field *field)
{
    if (!field)
        return;
    if (field->name)
        free((void *)field->name);
    if (field->type == UTE_TYPE_LIST && field->elem)
    {
        free_field((struct ute_field *)field->elem);
        free((void *)field->elem);
    }
    if (field->type == UTE_TYPE_STRUCT && field->fields)
    {
        for (size_t i = 0; i < field->num_fields; ++i)
            free_field((struct ute_field *)&field->fields[i]);
        free((void *)field->fields);
    }
}

void FreeSchema(struct ute_schema *schema)
{
    if (!schema || !schema->versions)
        return;
    for (size_t i = 0; i < schema->num_versions; ++i)
    {
        struct ute_schema_version *ver = (struct ute_schema_version *)&schema->versions[i];
        if (ver->fields)
        {
            for (size_t j = 0; j < ver->num_fields; ++j)
                free_field((struct ute_field *)&ver->fields[j]);
            free((void *)ver->fields);
        }
    }
    free((void *)schema->versions);
    schema->versions = NULL;
    schema->num_versions = 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>
#include "schema.h"

// Forward declaration for helper
static yaml_node_t *get_mapping_value(yaml_document_t *doc, yaml_node_t *map, const char *key);

int ParseSchema(const char *filename, struct ute_schema *out_schema)
{

    if (!filename || !out_schema)
    {
#ifdef UTE_DEBUG
        fprintf(stderr, "DEBUG: filename or out_schema is NULL\n");
#endif
        return -1;
    }
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
#ifdef UTE_DEBUG
        fprintf(stderr, "DEBUG: Could not open file: %s\n", filename);
#endif
        return -2;
    }
    yaml_parser_t parser;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, f);

    yaml_document_t doc;
    if (!yaml_parser_load(&parser, &doc))
    {
#ifdef UTE_DEBUG
        fprintf(stderr, "DEBUG: yaml_parser_load failed\n");
#endif
        fclose(f);
        yaml_parser_delete(&parser);
        return -3;
    }

    yaml_node_t *root = yaml_document_get_root_node(&doc);
    if (!root || root->type != YAML_MAPPING_NODE)
    {
#ifdef UTE_DEBUG
        fprintf(stderr, "DEBUG: root node missing or not a mapping\n");
#endif
        yaml_document_delete(&doc);
        yaml_parser_delete(&parser);
        fclose(f);
        return -4;
    }

    // Try to find "versions" (multi-version schema)
    yaml_node_t *versions_node = get_mapping_value(&doc, root, "versions");
    if (versions_node && versions_node->type == YAML_SEQUENCE_NODE)
    {
        size_t n = versions_node->data.sequence.items.top - versions_node->data.sequence.items.start;
        struct ute_schema_version *versions = calloc(n, sizeof(struct ute_schema_version));
        for (size_t i = 0; i < n; ++i)
        {
            yaml_node_t *ver_map = yaml_document_get_node(&doc, versions_node->data.sequence.items.start[i]);
            yaml_node_t *ver_num = get_mapping_value(&doc, ver_map, "version");
            yaml_node_t *fields_node = get_mapping_value(&doc, ver_map, "fields");
            if (!ver_num || !fields_node || fields_node->type != YAML_SEQUENCE_NODE)
            {
#ifdef UTE_DEBUG
                fprintf(stderr, "DEBUG: version or fields missing or fields not a sequence (multi-version)\n");
#endif
                yaml_document_delete(&doc);
                yaml_parser_delete(&parser);
                fclose(f);
                free(versions);
                return -5;
            }
            int version = atoi((char *)ver_num->data.scalar.value);
            size_t nf = fields_node->data.sequence.items.top - fields_node->data.sequence.items.start;
            struct ute_field *fields = calloc(nf, sizeof(struct ute_field));
            for (size_t j = 0; j < nf; ++j)
            {
                yaml_node_t *fnode = yaml_document_get_node(&doc, fields_node->data.sequence.items.start[j]);
                if (ParseSchemaField(&doc, fnode, &fields[j]) != 0)
                {
#ifdef UTE_DEBUG
                    fprintf(stderr, "DEBUG: ParseSchemaField failed for field %zu in version %d\n", j, version);
#endif
                    yaml_document_delete(&doc);
                    yaml_parser_delete(&parser);
                    fclose(f);
                    free(fields);
                    free(versions);
                    return -6;
                }
            }
            versions[i].version = version;
            versions[i].fields = fields;
            versions[i].num_fields = nf;
        }
        out_schema->versions = versions;
        out_schema->num_versions = n;
    }
    else
    {
        // Fallback: single-version schema (fields at root)
        yaml_node_t *fields_node = get_mapping_value(&doc, root, "fields");
        if (!fields_node || fields_node->type != YAML_SEQUENCE_NODE)
        {
#ifdef UTE_DEBUG
            fprintf(stderr, "DEBUG: fields missing or not a sequence (single-version)\n");
#endif
            yaml_document_delete(&doc);
            yaml_parser_delete(&parser);
            fclose(f);
            return -7;
        }
        size_t nf = fields_node->data.sequence.items.top - fields_node->data.sequence.items.start;
        struct ute_field *fields = calloc(nf, sizeof(struct ute_field));
        for (size_t j = 0; j < nf; ++j)
        {
            yaml_node_t *fnode = yaml_document_get_node(&doc, fields_node->data.sequence.items.start[j]);
            if (ParseSchemaField(&doc, fnode, &fields[j]) != 0)
            {
#ifdef UTE_DEBUG
                fprintf(stderr, "DEBUG: ParseSchemaField failed for field %zu (single-version)\n", j);
#endif
                yaml_document_delete(&doc);
                yaml_parser_delete(&parser);
                fclose(f);
                free(fields);
                return -8;
            }
        }
        struct ute_schema_version *versions = calloc(1, sizeof(struct ute_schema_version));
        versions[0].version = 1;
        versions[0].fields = fields;
        versions[0].num_fields = nf;
        out_schema->versions = versions;
        out_schema->num_versions = 1;
    }

    yaml_document_delete(&doc);
    yaml_parser_delete(&parser);
    fclose(f);
    return 0;
}

// Forward declarations for helpers
static yaml_node_t *get_mapping_value(yaml_document_t *doc, yaml_node_t *map, const char *key);
int ParseSchemaField(yaml_document_t *doc, yaml_node_t *node, struct ute_field *out_field);

// Helper: duplicate string
static char *ute_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *out = malloc(len);
    memcpy(out, s, len);
    return out;
}

// Helper: get value for a key in a YAML mapping node
static yaml_node_t *get_mapping_value(yaml_document_t *doc, yaml_node_t *map, const char *key)
{
    for (yaml_node_pair_t *pair = map->data.mapping.pairs.start;
         pair < map->data.mapping.pairs.top; ++pair)
    {
        yaml_node_t *k = yaml_document_get_node(doc, pair->key);
        yaml_node_t *v = yaml_document_get_node(doc, pair->value);
        if (k->type == YAML_SCALAR_NODE && strcmp((char *)k->data.scalar.value, key) == 0)
        {
            return v;
        }
    }
    return NULL;
}

int ParseSchemaField(yaml_document_t *doc, yaml_node_t *node, struct ute_field *out_field)
{

    if (!doc || !node || !out_field)
    {
#ifdef UTE_DEBUG
        fprintf(stderr, "DEBUG: ParseSchemaField: null doc/node/out_field\n");
#endif
        return -1;
    }
    if (node->type != YAML_MAPPING_NODE)
    {
#ifdef UTE_DEBUG
        fprintf(stderr, "DEBUG: ParseSchemaField: node is not a mapping\n");
#endif
        return -1;
    }

    // Get "name" (optional for anonymous/elem fields) and "type" (required)
    yaml_node_t *name_node = get_mapping_value(doc, node, "name");
    yaml_node_t *type_node = get_mapping_value(doc, node, "type");
    if (!type_node)
    {
#ifdef UTE_DEBUG
        fprintf(stderr, "DEBUG: ParseSchemaField: missing 'type'\n");
#endif
        return -1;
    }
    if (name_node)
        out_field->name = ute_strdup((char *)name_node->data.scalar.value);
    else
        out_field->name = NULL;

    // Map type string to int
    const char *type_str = (char *)type_node->data.scalar.value;
    if (strcmp(type_str, "null") == 0)
        out_field->type = UTE_TYPE_NULL;
    else if (strcmp(type_str, "bool") == 0)
        out_field->type = UTE_TYPE_BOOL;
    else if (strcmp(type_str, "int") == 0)
        out_field->type = UTE_TYPE_INT;
    else if (strcmp(type_str, "string") == 0)
        out_field->type = UTE_TYPE_STRING;
    else if (strcmp(type_str, "list") == 0)
        out_field->type = UTE_TYPE_LIST;
    else if (strcmp(type_str, "struct") == 0)
        out_field->type = UTE_TYPE_STRUCT;
    else
        return -1;

    out_field->elem = NULL;
    out_field->fields = NULL;
    out_field->num_fields = 0;

    // Recursively parse "elem" for lists
    if (out_field->type == UTE_TYPE_LIST)
    {
        yaml_node_t *elem_node = get_mapping_value(doc, node, "elem");
        if (!elem_node)
            return -1;
        struct ute_field *elem = malloc(sizeof(struct ute_field));
        if (ParseSchemaField(doc, elem_node, elem) != 0)
        {
            free(elem);
            return -1;
        }
        out_field->elem = elem;
    }

    // Recursively parse "fields" for structs
    if (out_field->type == UTE_TYPE_STRUCT)
    {
        yaml_node_t *fields_node = get_mapping_value(doc, node, "fields");
        if (!fields_node || fields_node->type != YAML_SEQUENCE_NODE)
            return -1;
        size_t n = fields_node->data.sequence.items.top - fields_node->data.sequence.items.start;
        struct ute_field *fields = calloc(n, sizeof(struct ute_field));
        out_field->num_fields = n;
        out_field->fields = fields;
        for (size_t i = 0; i < n; ++i)
        {
            yaml_node_t *f = yaml_document_get_node(doc, fields_node->data.sequence.items.start[i]);
            if (ParseSchemaField(doc, f, &fields[i]) != 0)
                return -1;
        }
        // Set offsets for fields in the struct (hardcoded for device struct: id:uint64_t, name[32])
        size_t running_offset = 0;
        for (size_t i = 0; i < n; ++i)
        {
            if (fields[i].type == UTE_TYPE_INT)
            {
                fields[i].offset = running_offset;
                running_offset += sizeof(uint64_t);
            }
            else if (fields[i].type == UTE_TYPE_STRING)
            {
                fields[i].offset = running_offset;
                running_offset += 32; // fixed size for name[32]
            }
            // Add more types as needed
        }
    }
    return 0;
}
