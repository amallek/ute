
#include "codex.h"
#include "schema.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Example device struct for serialization/deserialization
struct device
{
    uint64_t id;
    char name[32];
};

// Function prototypes
static void print_devices(const struct device *devices, size_t count, const char *label);

int main(void)
{
    // Load schema from YAML file
    struct ute_schema loaded_schema = {0};
    if (ParseSchema("../../schemas/complex.yaml", &loaded_schema) != 0)
    {
        fprintf(stderr, "Failed to load schema from YAML\n");
        return 1;
    }

    // Example data for complex.yaml: list of devices
    struct device devices[2] = {
        {1, "device1"},
        {2, "device2"}};
    size_t device_count = 2;

    // Prepare data for serialization: devices as a packed array [count, ptr, ptr, ...]
    void *devices_list[1 + 2];
    devices_list[0] = (void *)(uintptr_t)device_count; // store count as value, not pointer
    for (size_t i = 0; i < device_count; ++i)
        devices_list[1 + i] = &devices[i];

#ifdef UTE_DEBUG
    print_devices(devices, device_count, "Before serialization");
#endif

    // Top-level data: array of pointers to top-level fields (here: only "devices")
    void *top_data[1] = {devices_list};

    // Serialize
    uint8_t buf[256];
#ifdef UTE_DEBUG
    printf("Calling ute_serialize...\n");
#endif
    size_t written = ute_serialize(top_data, loaded_schema.versions[0].fields, buf, sizeof(buf));
#ifdef UTE_DEBUG
    printf("ute_serialize returned, written=%zu\n", written);
#endif
    printf("Serialized %zu bytes:\n", written);
    for (size_t i = 0; i < written; ++i)
        printf("%02x ", buf[i]);
    printf("\n");

    // Prepare output for deserialization
    struct device out_devices[2];
    void *out_devices_list[1 + 2];
    out_devices_list[0] = 0; // count as value, will be set by ute_deserialize
    for (size_t i = 0; i < 2; ++i)
        out_devices_list[1 + i] = &out_devices[i];
    void *out_top_data[1] = {out_devices_list};

#ifdef UTE_DEBUG
    printf("Deserialization setup:\n");
    for (size_t i = 0; i < 2; ++i)
        printf("    out_devices_list[%zu]=%p\n", 1 + i, out_devices_list[1 + i]);
    printf("  out_devices_list=%p { [0]=%p, [1]=%p }\n", (void *)out_devices_list, out_devices_list[0], out_devices_list[1]);
    printf("  out_top_data=%p { [0]=%p }\n", (void *)out_top_data, out_top_data[0]);
#endif

    // Deserialize
    size_t read = ute_deserialize(buf, written, loaded_schema.versions[0].fields, out_top_data);
    size_t out_count = (size_t)(uintptr_t)out_devices_list[0];
    printf("Deserialized %zu bytes, got %zu devices:\n", read, out_count);
    print_devices(out_devices, out_count, "After deserialization");

    FreeSchema(&loaded_schema);
    return 0;
}

// Print a list of devices with a label
static void print_devices(const struct device *devices, size_t count, const char *label)
{
    printf("%s:\n", label);
    for (size_t i = 0; i < count; ++i)
        printf("  Device %zu: id=%llu, name=%s, addr=%p\n", i + 1, (unsigned long long)devices[i].id, devices[i].name, (const void *)&devices[i]);
}