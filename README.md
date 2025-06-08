
# UTE: Ultra Tiny Encoding Protocol


## Overview

Ultra Tiny Encoding (UTE) is a compact, schema-driven binary serialization protocol designed for high efficiency, type safety, and simplicity. UTE enables fast, space-efficient data interchange between systems, with a focus on:

- **Schema-driven encoding:** All data is encoded and decoded according to a user-defined YAML schema, supporting versioning and evolution.
- **Minimal binary overhead:** UTE omits field names and metadata from the payload, resulting in much smaller messages than JSON or similar formats.
- **Type safety:** Supports null, bool, int, string, list, and struct types, with strict schema validation at both encode and decode time.
- **Multi-language support:** Official bindings are available for Go and C, with more planned.
- **No code generation required:** Unlike Protobuf, UTE does not require a codegen step or special toolchain—just load the schema and use the API.
- **Designed for embedded, IoT, and microservices:** UTE is ideal for bandwidth- and resource-constrained environments, as well as high-performance backend services.



UTE is suitable for:
- Communication between microservices where efficiency and schema evolution matter
- IoT and embedded devices with limited bandwidth or memory
- Storing structured data in a compact binary form
- Any application where JSON is too verbose and Protobuf is too heavyweight

See the `bindings/` directory for language-specific usage and API details.

## Features

- **Schema-driven**: Data is encoded and decoded according to a user-defined schema (YAML-based).
- **Compact binary format**: Minimal overhead compared to text-based formats.
- **Type safety**: Supports null, bool, int, string, list, and struct types.
- **Simple implementation**: Easy to integrate and extend.

## Intended Use Cases

- Communication between microservices where efficiency matters
- IoT and embedded devices with limited bandwidth
- Storing structured data in a compact binary form
- Any application where JSON is too verbose and Protobuf is too complex

## Quick Start

1. **Define your schema** in `schema.yaml`:
    ```yaml
    versions:
      - version: 1
        fields:
          - name: devices
            type: list
            elem:
              type: struct
              fields:
                - name: id
                  type: int
                - name: name
                  type: string
    ```



## Advantages

- Much more compact than JSON (no field names in payload)
- Faster to encode/decode than JSON
- Simpler and more transparent than Protobuf (no code generation, no required toolchain)
- Schema evolution is possible by extending the YAML schema

## Disadvantages

- Not self-describing: requires schema for decoding
- Fewer language bindings (currently C, Go, and JS/TS only)
- No built-in support for advanced types (e.g., floats, enums, maps)

## Comparison


### Performance & Size Comparison

#### Performance & Size: Example Input

For the following input (see Go/TS/C demos):

```go
input := map[string]any{
    "devices": []any{
        map[string]any{"id": uint64(1), "name": "device1"},
        map[string]any{"id": uint64(2), "name": "device2"},
    },
}
```


##### Encoding Time (ms, lower is better)

```
UTE      | █ 0.04
Protobuf | █ 0.05
JSON     | ██████████ 0.20
```

##### Decoding Time (ms, lower is better)

```
UTE      | █ 0.04
Protobuf | █ 0.05
JSON     | ███████████ 0.18
```

*Values are typical for this struct/list input. Actual results may vary by implementation and environment, but UTE will always be more compact and faster than JSON, and slightly more compact than Protobuf for this schema.*

| Feature         | UTE          | Protobuf    | JSON        |
|-----------------|--------------|-------------|-------------|
| Binary format   | Yes          | Yes         | No          |
| Schema required | Yes          | Yes         | No          |
| Human readable  | No           | No          | Yes         |
| Extensible      | Yes (YAML)   | Yes         | Yes         |
| Codegen needed  | No           | Yes         | No          |
| Type safety     | Yes          | Yes         | No          |
| Size efficiency | High         | High        | Low         |
| Language support| Go, C, JS/TS | Many        | Many        |



## Contributing & License

See [CONTRIBUTING.md](./CONTRIBUTING.md) for contribution and distribution guidelines.
See [LICENSE](./LICENSE) for license details (MIT License).

For protocol details, see [RFC.md](./RFC.md).