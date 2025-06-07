// Usage:
//   go run test.go <read|write> <filename>
//
//   write:  Serialize Go data and write UTE binary to file (for C to read)
//   read:   Read UTE binary from file (produced by C) and deserialize in Go

package main

import (
	"bytes"
	"fmt"
	"os"

	"github.com/amallek/ute/bindings/golang/codex"
	"github.com/amallek/ute/bindings/golang/schema"
)

// main is the entry point for the cross-language UTE test. It parses arguments, loads the schema,
// and either serializes Go data to a file (write) or deserializes a file produced by C (read).
func main() {
	if len(os.Args) != 3 {
		fmt.Fprintf(os.Stderr, "Usage: %s <read|write> <filename>\n", os.Args[0])
		os.Exit(2)
	}
	mode := os.Args[1]
	filename := os.Args[2]

	schemaPath := "../../schemas/complex.yaml"
	allVersions, err := schema.LoadSchema(schemaPath)
	if err != nil {
		panic(err)
	}
	v1, err := schema.FindSchemaVersion(allVersions, 1)
	if err != nil {
		panic(err)
	}
	parsedFields, err := schema.ParseSchemaFields(v1.Fields)
	if err != nil {
		panic(err)
	}

	if mode == "write" {
		input := map[string]any{
			"devices": []any{
				map[string]any{"id": uint64(1), "name": "device1"},
				map[string]any{"id": uint64(2), "name": "device2"},
			},
		}
		binaryData, err := codex.Serialize(input, parsedFields)
		if err != nil {
			panic(err)
		}
		err = os.WriteFile(filename, binaryData, 0644)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Failed to write %s: %v\n", filename, err)
			os.Exit(4)
		}
		fmt.Printf("[crosslang] Wrote %d bytes to %s\n", len(binaryData), filename)
	} else if mode == "read" {
		bin2, err := os.ReadFile(filename)
		if err != nil {
			fmt.Fprintf(os.Stderr, "[crosslang] %s not found. Please run C test first.\n", filename)
			return
		}
		parsed, err := codex.Deserialize(bytes.NewReader(bin2), parsedFields)
		if err != nil {
			fmt.Fprintf(os.Stderr, "[crosslang] Deserialize error: %v\n", err)
			return
		}
		fmt.Printf("[crosslang] Read %d bytes from %s, parsed: %#v\n", len(bin2), filename, parsed)
	} else {
		fmt.Fprintf(os.Stderr, "Unknown mode: %s (expected 'read' or 'write')\n", mode)
		os.Exit(3)
	}
}
