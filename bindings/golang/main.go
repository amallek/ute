package main

import (
	"bytes"
	"fmt"

	"github.com/amallek/ute/bindings/golang/codex"
	"github.com/amallek/ute/bindings/golang/schema"
)

// main demonstrates loading a schema, serializing a map, and deserializing it back using the codex package.
//
// Loads the schema, serializes a sample device list, prints the binary, then deserializes and prints the result.
func main() {
	schemaPath := "../../schemas/complex.yaml"
	allVersions, err := schema.LoadSchema(schemaPath)
	if err != nil {
		panic(err)
	}
	v1, err := schema.FindSchemaVersion(allVersions, 1) // use desired version
	if err != nil {
		panic(err)
	}
	parsedFields, err := schema.ParseSchemaFields(v1.Fields)
	if err != nil {
		panic(err)
	}

	input := map[string]any{
		"devices": []any{
			map[string]any{
				"id":   uint64(1),
				"name": "device1",
			},
			map[string]any{
				"id":   uint64(2),
				"name": "device2",
			},
		},
	}
	binaryData, err := codex.Serialize(input, parsedFields)
	if err != nil {
		panic(err)
	}
	fmt.Printf("Serialized: %x\n", binaryData)

	parsed, err := codex.Deserialize(bytes.NewReader(binaryData), parsedFields)
	if err != nil {
		panic(err)
	}
	fmt.Printf("Deserialized: %#v\n", parsed)
}
