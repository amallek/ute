package types

// FieldType represents the type of a field in a schema or parsed structure.
type FieldType int

// Supported field types for schema and parsed fields.
const (
	NullType   FieldType = iota // Null value
	BoolType                    // Boolean value
	IntType                     // Integer value
	StringType                  // String value
	ListType                    // List value
	StructType                  // Struct/object value
)

// Type prefix constants for UTE serialization format.
const (
	TNull   = 0b000 << 5 // Null value
	TBool   = 0b001 << 5 // Boolean value
	TInt    = 0b010 << 5 // Integer value
	TBytes  = 0b011 << 5 // String/bytes value
	TList   = 0b100 << 5 // List value
	TStruct = 0b101 << 5 // Struct/object value
)

// SchemaField represents a field as defined in a YAML schema file.
type SchemaField struct {
	Name   string        `yaml:"name"`             // Field name
	Type   string        `yaml:"type"`             // Field type as string
	Elem   *SchemaField  `yaml:"elem,omitempty"`   // Element type for lists
	Fields []SchemaField `yaml:"fields,omitempty"` // Nested fields for structs
}

// ParsedField represents a field with resolved types and nested structure after parsing.
type ParsedField struct {
	Name   string        // Field name
	Type   FieldType     // Field type
	Elem   *ParsedField  // Element type for lists
	Fields []ParsedField // Nested fields for structs
}

// Schema represents the root of a YAML schema file (single-version fallback).
type Schema struct {
	Fields []SchemaField `yaml:"fields"`
}

// SchemaVersion represents a single version of a schema (for multi-version support).
type SchemaVersion struct {
	Version int           `yaml:"version"`
	Fields  []SchemaField `yaml:"fields"`
}

// MultiVersionSchema allows multiple schema versions in one YAML file.
type MultiVersionSchema struct {
	Versions []SchemaVersion `yaml:"versions"`
}
