// crosslang_test.c: C <-> Go UTE interoperability test
#include "../codex.h"
#include "../schema.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <read|write> <filename>\n", argv[0]);
        return 2;
    }
    const char *mode = argv[1];
    const char *filename = argv[2];
    struct ute_schema loaded_schema = {0};
    if (ParseSchema("../../../schemas/complex.yaml", &loaded_schema) != 0)
    {
        fprintf(stderr, "Failed to load schema from YAML\n");
        return 1;
    }

    if (strcmp(mode, "write") == 0)
    {
        // --- C -> Go: Serialize in C, output to file ---
        struct
        {
            uint64_t id;
            char name[32];
        } devices[2] = {
            {1, "device1"}, {2, "device2"}};
        size_t device_count = 2;
        void *devices_list[1 + 2];
        devices_list[0] = (void *)(uintptr_t)device_count;
        for (size_t i = 0; i < device_count; ++i)
            devices_list[1 + i] = &devices[i];
        void *top_data[1] = {devices_list};
        uint8_t buf[256];
        size_t written = ute_serialize(top_data, loaded_schema.versions[0].fields, buf, sizeof(buf));
        if (written == UTE_BUF_ERROR)
        {
            fprintf(stderr, "Serialization failed due to buffer size\n");
            FreeSchema(&loaded_schema);
            return 3;
        }
        FILE *fout = fopen(filename, "wb");
        if (!fout)
        {
            fprintf(stderr, "Failed to open %s for writing\n", filename);
            FreeSchema(&loaded_schema);
            return 3;
        }
        fwrite(buf, 1, written, fout);
        fclose(fout);
        printf("[crosslang] Wrote %zu bytes to %s\n", written, filename);
    }
    else if (strcmp(mode, "read") == 0)
    {
        // --- Go -> C: Read file produced by Go, decode in C ---
        FILE *fin = fopen(filename, "rb");
        if (!fin)
        {
            fprintf(stderr, "[crosslang] %s not found.\n", filename);
        }
        else
        {
            uint8_t buf2[256];
            size_t n = fread(buf2, 1, sizeof(buf2), fin);
            fclose(fin);
            struct
            {
                uint64_t id;
                char name[32];
            } out_devices[2];
            void *out_devices_list[1 + 2];
            out_devices_list[0] = 0;
            for (size_t i = 0; i < 2; ++i)
                out_devices_list[1 + i] = &out_devices[i];
            void *out_top_data[1] = {out_devices_list};
            size_t read = ute_deserialize(buf2, n, loaded_schema.versions[0].fields, out_top_data);
            if (read == UTE_BUF_ERROR)
            {
                fprintf(stderr, "Deserialization failed due to buffer size\n");
                FreeSchema(&loaded_schema);
                return 4;
            }
            size_t out_count = (size_t)(uintptr_t)out_devices_list[0];
            printf("[crosslang] Read %zu bytes from %s, got %zu devices:\n", read, filename, out_count);
            for (size_t i = 0; i < out_count; ++i)
                printf("  Device %zu: id=%llu, name=%s\n", i + 1, (unsigned long long)out_devices[i].id, out_devices[i].name);
        }
    }
    else
    {
        fprintf(stderr, "Unknown mode: %s (expected 'read' or 'write')\n", mode);
        FreeSchema(&loaded_schema);
        return 4;
    }

    FreeSchema(&loaded_schema);
    return 0;
}
