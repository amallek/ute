# RFC: Ultra Tiny Encoding (UTE)

*Last updated: 2025-06-07*

### 1. Introduction

UTE is a binary serialization protocol for structured data, designed to be simple, efficient, and schema-driven. It is suitable for applications where JSON is too verbose and Protobuf is too heavyweight.

### 2. Data Types

- **null**: Represents a null value
- **bool**: Boolean (true/false)
- **int**: Unsigned integer (uint64)
- **string**: UTF-8 encoded string
- **list**: List of elements of a single type
- **struct**: Object with named fields

### 3. Type Prefixes

Each value is prefixed with a 3-bit type code:

| Type   | Prefix (bits) |
|--------|---------------|
| null   | 000           |
| bool   | 001           |
| int    | 010           |
| string | 011           |
| list   | 100           |
| struct | 101           |

### 4. Encoding Rules

- **null**: 1 byte (type prefix)
- **bool**: 1 byte (type prefix + value bit)
- **int**: type prefix + varint encoding
- **string**: type prefix + varint length + bytes
- **list**: type prefix + varint length + encoded elements
- **struct**: type prefix + varint field count + encoded fields

### 5. Schema


Schemas are defined in YAML and specify the schema version, field names, types, and nesting. The `version` field is required and must be an integer. Example:

```yaml
version: 1
fields:
  - name: id
    type: int
  - name: name
    type: string
  - name: active
    type: bool
```

The `version` field allows for explicit schema versioning. Implementations MUST check the schema version and MAY reject data or schemas with unsupported versions. This enables forward and backward compatibility as schemas evolve.

### 6. Extensibility

- New types can be added by extending the type prefix table.
- Fields can be added to structs for forward compatibility.

### 7. Security Considerations

- UTE is not self-describing; always validate input against a trusted schema.
- No built-in encryption or authentication.

### 8. Example

Given the versioned schema above, the following Go code serializes and deserializes a record:

```go
schema, _ := schema.LoadSchema("schema.yaml") // schema.yaml contains a 'version' field
input := map[string]any{"id": uint64(42), "name": "device-123", "active": true}
binaryData, _ := codex.Serialize(input, schema)
parsed, _ := codex.Deserialize(bytes.NewReader(binaryData), schema)
```
