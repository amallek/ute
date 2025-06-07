# UTE Go Binding

This directory contains the Go implementation and CLI for the Ultra Tiny Encoding (UTE) protocol.

## Structure

- `main.go` — Example CLI for encoding/decoding data using UTE schemas
- `codex/` — Core serialization/deserialization logic
- `schema/` — Schema parsing and versioning logic
- `types/` — Type definitions for UTE schemas
- `Makefile` — Build and clean targets for the CLI
- `go.mod`, `go.sum` — Go module files

## Quick Start

1. **Build the CLI:**

   ```sh
   make
   ```

2. **Run the CLI:**

   ```sh
   ./ute
   ```


3. **Use in your Go project:**

   Import the relevant packages:
   ```go
   import (
       "github.com/amallek/ute/bindings/golang/codex"
       "github.com/amallek/ute/bindings/golang/schema"
   )
   ```

   Example: Load a schema, serialize, and deserialize a record:

   ```go
   // Load the schema from YAML
   sch, err := schema.LoadSchema("schema.yaml")
   if err != nil {
       panic(err)
   }
   v1, err := schema.FindSchemaVersion(sch, 1)
   if err != nil {
       panic(err)
   }

   // Example data to encode
   input := map[string]any{
       "devices": []any{
           map[string]any{"id": uint64(1), "name": "device1"},
           map[string]any{"id": uint64(2), "name": "device2"},
       },
   }

   // Serialize
   binary, err := codex.Serialize(input, v1.Fields)
   if err != nil {
       panic(err)
   }
   fmt.Printf("Serialized: %x\n", binary)

   // Deserialize
   parsed, err := codex.Deserialize(bytes.NewReader(binary), v1.Fields)
   if err != nil {
       panic(err)
   }
   fmt.Printf("Deserialized: %+v\n", parsed)
   ```


## Development

- Run `make clean` to remove the built binary.
- Run `go test ./...` to test all packages (add tests in the future!).
- Edit `schemas/` for example schema files.

## Contributing & License

See [CONTRIBUTING.md](../../CONTRIBUTING.md) for contribution and distribution guidelines.
See [LICENSE](../../LICENSE) for license details (GPL3).
