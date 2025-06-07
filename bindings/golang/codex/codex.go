// Package codex provides serialization and deserialization utilities for UTE data structures.
package codex

import (
	"bytes"
	"fmt"
	"io"

	"github.com/amallek/ute/bindings/golang/types"
)

// encodeVarint writes a uint64 as a variable-length integer to the given buffer.
//
// Used internally for UTE serialization.
func encodeVarint(buf *bytes.Buffer, n uint64) {
	for n >= 0x80 {
		buf.WriteByte(byte(n) | 0x80)
		n >>= 7
	}
	buf.WriteByte(byte(n))
}

// decodeVarint reads a variable-length integer from the given bytes.Reader and returns it as uint64.
//
// Used internally for UTE deserialization.
func decodeVarint(r *bytes.Reader) (uint64, error) {
	var result uint64
	var shift uint
	for {
		b, err := r.ReadByte()
		if err != nil {
			return 0, err
		}
		result |= uint64(b&0x7F) << shift
		if b&0x80 == 0 {
			break
		}
		shift += 7
	}
	return result, nil
}

// Serialize encodes a map[string]any according to the provided schema and returns the serialized bytes.
//
// Takes a data map and a parsed schema, and returns a UTE-encoded byte slice or an error.
func Serialize(data map[string]any, schema []types.ParsedField) ([]byte, error) {
	buf := new(bytes.Buffer)
	for _, field := range schema {
		val := data[field.Name]
		switch field.Type {
		case types.NullType:
			buf.WriteByte(types.TNull)
		case types.BoolType:
			b := byte(types.TBool)
			if val.(bool) {
				b |= 0x10
			}
			buf.WriteByte(b)
		case types.IntType:
			buf.WriteByte(types.TInt)
			encodeVarint(buf, val.(uint64))
		case types.StringType:
			s := val.(string)
			buf.WriteByte(types.TBytes)
			encodeVarint(buf, uint64(len(s)))
			buf.WriteString(s)
		case types.ListType:
			list := val.([]any)
			buf.WriteByte(types.TList)
			encodeVarint(buf, uint64(len(list)))
			for _, item := range list {
				serialized, err := Serialize(map[string]any{"": item}, []types.ParsedField{*field.Elem})
				if err != nil {
					return nil, err
				}
				buf.Write(serialized)
			}
		case types.StructType:
			child := val.(map[string]any)
			buf.WriteByte(types.TStruct)
			encodeVarint(buf, uint64(len(field.Fields)))
			nested, err := Serialize(child, field.Fields)
			if err != nil {
				return nil, err
			}
			buf.Write(nested)
		}
	}
	return buf.Bytes(), nil
}

// Deserialize decodes bytes from the given reader according to the provided schema and returns a map[string]any.
//
// Takes a bytes.Reader and a parsed schema, and returns a map of field names to values or an error.
func Deserialize(r *bytes.Reader, schema []types.ParsedField) (map[string]any, error) {
	out := make(map[string]any)
	for _, field := range schema {
		h, err := r.ReadByte()
		if err != nil {
			return nil, err
		}
		typ := h >> 5
		switch field.Type {
		case types.NullType:
			if typ != 0 {
				return nil, fmt.Errorf("expected null")
			}
			out[field.Name] = nil
		case types.BoolType:
			if typ != 1 {
				return nil, fmt.Errorf("expected bool")
			}
			out[field.Name] = (h & 0x10) != 0
		case types.IntType:
			if typ != 2 {
				return nil, fmt.Errorf("expected int")
			}
			val, err := decodeVarint(r)
			if err != nil {
				return nil, err
			}
			out[field.Name] = val
		case types.StringType:
			if typ != 3 {
				return nil, fmt.Errorf("expected string")
			}
			slen, err := decodeVarint(r)
			if err != nil {
				return nil, err
			}
			buf := make([]byte, slen)
			_, err = io.ReadFull(r, buf)
			if err != nil {
				return nil, err
			}
			out[field.Name] = string(buf)
		case types.ListType:
			if typ != 4 {
				return nil, fmt.Errorf("expected list")
			}
			count, err := decodeVarint(r)
			if err != nil {
				return nil, err
			}
			list := make([]any, 0, count)
			for i := 0; i < int(count); i++ {
				itemMap, err := Deserialize(r, []types.ParsedField{*field.Elem})
				if err != nil {
					return nil, err
				}
				list = append(list, itemMap[""])
			}
			out[field.Name] = list
		case types.StructType:
			if typ != 5 {
				return nil, fmt.Errorf("expected struct")
			}
			_, err := decodeVarint(r)
			if err != nil {
				return nil, err
			}
			child, err := Deserialize(r, field.Fields)
			if err != nil {
				return nil, err
			}
			out[field.Name] = child
		default:
			return nil, fmt.Errorf("unknown field type")
		}
	}
	return out, nil
}
