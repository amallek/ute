# RFC: Ultra Tiny Encoding (UTE)

*Last updated: 2025-06-08*

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

- **null**: 1 byte (type prefix only)
- **bool**: 1 byte (type prefix in high bits, value in low bit)
- **int**: type prefix + unsigned varint (LEB128) encoding
- **string**: type prefix + varint length + UTF-8 bytes
- **list**: type prefix + varint length (number of elements) + encoded elements (each encoded recursively)
- **struct**: type prefix + varint field count + encoded fields (see below)

#### 4.1. Detailed Encoding

##### Type Prefix
The first 3 bits of the first byte always indicate the type. The remaining bits (and subsequent bytes) are used for value, length, or field count as appropriate.

##### Null
- 1 byte: 3-bit type prefix (000), remaining bits zeroed.

##### Bool
- 1 byte: 3-bit type prefix (001), 1 value bit (0=false, 1=true), remaining bits zeroed.

##### Int
- 1 byte: 3-bit type prefix (010), remaining bits start of unsigned varint (LEB128) encoding.
- The varint encodes a uint64 value, little-endian, 7 bits per byte, MSB=1 for continuation.

##### String
- 1 byte: 3-bit type prefix (011), remaining bits start of varint length.
- Varint: length of UTF-8 string in bytes.
- UTF-8 bytes follow.

##### List
- 1 byte: 3-bit type prefix (100), remaining bits start of varint length.
- Varint: number of elements.
- Each element is encoded recursively according to its type.

##### Struct
- 1 byte: 3-bit type prefix (101), remaining bits start of varint field count.
- Varint: number of fields present (not total fields in schema, but present in this instance).
- For each field:
  - Field index (varint): Index in schema's field list (0-based, as defined in schema YAML).
  - Field value: Encoded recursively according to field type.

#### 4.2. Field Order and Omission
- Fields may be omitted if not present in the data; omitted fields are not encoded.
- Fields may appear in any order, but implementations SHOULD preserve schema order for consistency.
- Unknown fields (not in schema) MUST NOT be encoded.

#### 4.3. Example Encoding (Struct)
Given schema:
```yaml
version: 1
fields:
  - name: id      # index 0
    type: int
  - name: name    # index 1
    type: string
  - name: active  # index 2
    type: bool
```
Data: `{id: 42, name: "device-123", active: true}`

Encoding steps:
1. Write struct type prefix (101) + varint field count (3)
2. For each field:
   - Write field index (varint)
   - Write value (using type-specific encoding)

Resulting bytes (hex, comments):
```
  a3                # struct (101), 3 fields
  00 52             # field 0 (id), int 42
  01 6b 64 65 76... # field 1 (name), string "device-123"
  02 21             # field 2 (active), bool true
```

#### 4.4. Deserialization

Deserialization is schema-driven:
- Read type prefix, dispatch to appropriate handler.
- For structs, use schema to map field indices to names/types.
- For lists, use schema to determine element type.
- For each field, decode recursively.
- Unknown or out-of-range field indices MUST be rejected.

#### 4.5. Error Handling
- If the type prefix does not match the schema, deserialization MUST fail.
- If a required field is missing, deserialization MAY fail or return a partial result, depending on implementation.
- If the varint or string length is invalid or exceeds buffer, deserialization MUST fail.

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
