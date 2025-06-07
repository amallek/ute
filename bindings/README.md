# UTE Bindings

This directory contains language-specific bindings for the Ultra Tiny Encoding (UTE) protocol, as well as cross-language interoperability tests.

## Structure

- `c/` — Native C binding for UTE
- `golang/` — Go binding for UTE

Each binding contains:
- Implementation source code
- A `test/` subdirectory for cross-language interoperability tests

## Cross-Language Interoperability Tests

The `test/` folder in each binding contains a program (`crosslang_test` or `test.go`) that demonstrates and verifies interoperability between the C and Go UTE bindings. These tests:

- **Serialize** a sample data structure to a file (e.g., `out.ute`) using the binding's implementation.
- **Deserialize** a file produced by the other language's binding, verifying that the data round-trips correctly.
- Use a shared schema (see `../../schemas/complex.yaml`) to ensure both bindings interpret the data identically.

### How to Use

1. **Build the test binaries** in both `c/test/` and `golang/test/` (see each folder's `Makefile`).
2. **Write**: Run the test in one language to serialize data:
   - C: `./crosslang_test write out-from-c.ute`
   - Go: `go run test.go write out-from-go.ute`
3. **Read**: Run the test in the other language to deserialize and print the data:
   - C: `./crosslang_test read out-from-go.ute`
   - Go: `go run test.go read out-from-c.ute`

You can swap the order to test both directions (C→Go and Go→C).

### Why This Matters

- Ensures that UTE is truly language-agnostic and interoperable.
- Catches subtle bugs in serialization, deserialization, or schema handling.
- Provides a clear, reproducible example for users integrating UTE into multi-language systems.

## See Also

- [README.md](../README.md) — Project overview and protocol details
- [README.md](./c/README.md) — C binding usage
- [README.md](./golang/README.md) — Go binding usage
