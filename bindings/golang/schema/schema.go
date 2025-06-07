package schema

import (
	"fmt"
	"os"

	"github.com/amallek/ute/bindings/golang/types"
	"gopkg.in/yaml.v2"
)

// ParseSchemaField recursively parses a SchemaField and returns a ParsedField with resolved types and nested fields.
func ParseSchemaField(sf types.SchemaField) (types.ParsedField, error) {
	var ft types.FieldType
	switch sf.Type {
	case "null":
		ft = types.NullType
	case "bool":
		ft = types.BoolType
	case "int":
		ft = types.IntType
	case "string":
		ft = types.StringType
	case "list":
		ft = types.ListType
	case "struct":
		ft = types.StructType
	default:
		return types.ParsedField{}, fmt.Errorf("unknown type: %s", sf.Type)
	}
	pf := types.ParsedField{Name: sf.Name, Type: ft}
	if ft == types.ListType && sf.Elem != nil {
		elem, err := ParseSchemaField(*sf.Elem)
		if err != nil {
			return types.ParsedField{}, err
		}
		pf.Elem = &elem
	}
	if ft == types.StructType {
		for _, f := range sf.Fields {
			sub, err := ParseSchemaField(f)
			if err != nil {
				return types.ParsedField{}, err
			}
			pf.Fields = append(pf.Fields, sub)
		}
	}
	return pf, nil
}

// LoadSchema loads a schema file and returns all available versions.
func LoadSchema(filename string) ([]types.SchemaVersion, error) {
	data, err := os.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var multi types.MultiVersionSchema
	err = yaml.Unmarshal(data, &multi)
	if err == nil && len(multi.Versions) > 0 {
		return multi.Versions, nil
	}
	// fallback: try single-version schema (for backward compatibility)
	var single types.Schema
	err = yaml.Unmarshal(data, &single)
	if err != nil {
		return nil, err
	}
	return []types.SchemaVersion{{Version: 1, Fields: single.Fields}}, nil
}

// FindSchemaVersion returns the SchemaVersion for a given version number.
func FindSchemaVersion(versions []types.SchemaVersion, version int) (*types.SchemaVersion, error) {
	for _, v := range versions {
		if v.Version == version {
			return &v, nil
		}
	}
	return nil, fmt.Errorf("schema version %d not found", version)
}

// ParseSchemaFields parses a list of SchemaField into ParsedField.
func ParseSchemaFields(fields []types.SchemaField) ([]types.ParsedField, error) {
	var parsed []types.ParsedField
	for _, f := range fields {
		pf, err := ParseSchemaField(f)
		if err != nil {
			return nil, err
		}
		parsed = append(parsed, pf)
	}
	return parsed, nil
}
