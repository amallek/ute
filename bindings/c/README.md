# Ultra Tiny Encoding (UTE) C Binding

This directory contains a native C implementation of the UTE (Ultra Tiny Encoding) protocol, including schema handling, codex (serialization/deserialization), and tests.

## Structure

- `codex.c`, `codex.h` — Core serialization/deserialization logic
- `schema.c`, `schema.h` — Schema parsing and versioning logic (YAML or JSON-based)
- `ute.c` — Main example/test file for encoding/decoding


## Build

**Dependency:**

- You need the [libyaml](https://pyyaml.org/wiki/LibYAML) C library installed (e.g. `brew install libyaml` on macOS, `apt install libyaml-dev` on Debian/Ubuntu, or `dnf install libyaml-devel` on Fedora).

To build the main UTE C example and test program:

```sh
make        # builds the main ute example (./ute)
make debug  # builds with debug output enabled (UTE_DEBUG)
```




## Usage Example

Here's a minimal example of how to use the C-based codex and schema loader:

```c
#include "codex.h"
#include "schema.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void)
{
    // Load schema from YAML file
    struct ute_schema loaded_schema = {0};
    if (ParseSchema("../../schemas/complex.yaml", &loaded_schema) != 0) {
        fprintf(stderr, "Failed to load schema from YAML\n");
        return 1;
    }

    // Example data: list of devices
    struct {
        uint64_t id;
        char name[32];
    } devices[2] = {
        {1, "device1"},
        {2, "device2"}
    };

    // Prepare data for serialization: packed array [count, ptr, ptr, ...]
    size_t device_count = 2;
    void *devices_list[1 + 2];
    devices_list[0] = (void *)(uintptr_t)device_count;
    for (size_t i = 0; i < device_count; ++i)
        devices_list[1 + i] = &devices[i];
    void *top_data[1] = {devices_list};

    // Serialize
    uint8_t buf[256];
    size_t written = ute_serialize(top_data, loaded_schema.versions[0].fields, buf, sizeof(buf));
    printf("Serialized %zu bytes:\n", written);
    for (size_t i = 0; i < written; ++i)
        printf("%02x ", buf[i]);
    printf("\n");

    // Prepare output for deserialization
    struct {
        uint64_t id;
        char name[32];
    } out_devices[2];
    void *out_devices_list[1 + 2];
    out_devices_list[0] = 0; // count as value, will be set by ute_deserialize
    for (size_t i = 0; i < 2; ++i)
        out_devices_list[1 + i] = &out_devices[i];
    void *out_top_data[1] = {out_devices_list};

    // Deserialize
    size_t read = ute_deserialize(buf, written, loaded_schema.versions[0].fields, out_top_data);
    size_t out_count = (size_t)(uintptr_t)out_devices_list[0];
    printf("Deserialized %zu bytes, got %zu devices:\n", read, out_count);
    for (size_t i = 0; i < out_count; ++i)
        printf("  Device %zu: id=%llu, name=%s\n", i + 1, (unsigned long long)out_devices[i].id, out_devices[i].name);

    // Free schema memory
    FreeSchema(&loaded_schema);
    return 0;
}
```

See `ute.c` for a more complete, schema-driven example using dynamic YAML loading and serialization/deserialization.

### Notes
- The Makefile will auto-detect macOS or Linux and set the correct libyaml flags.
- To enable debug output, build with `make debug` or add `-DUTE_DEBUG` to your CFLAGS.
- All schema memory is freed with `FreeSchema()` after use.

## License & Distribution

See [LICENSE](../../LICENSE) for license details (MIT License).
See [CONTRIBUTING.md](../../CONTRIBUTING.md) for contribution and distribution guidelines.
