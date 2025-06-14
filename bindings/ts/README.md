
# utep

Ultra Tiny Encoding Protocol (UTE) — JavaScript/TypeScript binding

Schema-driven, compact binary serialization for Node.js and TypeScript. Compatible with Go and C UTE bindings.

## Features

- Schema-driven encoding/decoding (YAML schema, versioned)
- Compact binary format (no field names in payload)
- TypeScript types for schema and data
- No code generation required

## Installation

```sh
npm i utep
```


## Usage

```ts
import UTEP from 'utep';

// Load schema (YAML file, versioned)
const schema = UTEP.loadSchemaFromFile('path/to/schema.yaml')[0].fields;
// const schema = UTEP.loadSchemaFromString('#schema#')[0].fields;

// Data to encode
const data = { devices: [ { id: 1, name: 'device1' } ] };

// Serialize to UTE binary
const encoded = UTEP.serialize(data, schema);

// Deserialize from UTE binary
const [decoded] = UTEP.deserialize(encoded, schema);
console.log(decoded);
```

## Example Schema

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

See [`test/test.ts`](./test/test.ts) for a more complete example.

## API

- `loadSchemaFromFile(path: string): UteSchemaVersion[]` — Load and parse a YAML schema file
- `serialize(data: any, schema: UteSchemaField[]): Uint8Array` — Serialize data to UTE binary
- `deserialize(buf: Uint8Array, schema: UteSchemaField[], offset = 0): [any, number]` — Deserialize UTE binary to JS object

TypeScript types for schema and data are included.