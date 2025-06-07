# Contributing to UTE

Thank you for considering contributing to the Ultra Tiny Encoding (UTE) project!

## How to Contribute

1. **Fork the repository** and create your branch from `main`.

2. **Write clear, concise code** and add documentation/comments to all exported functions and types (Go, C, or other languages).
3. **Add or update tests** for any new features or bug fixes. This includes cross-language tests in the `bindings/*/test/` folders to ensure interoperability between all supported bindings (Go, C, etc).
4. **Run all relevant tests and linters**:
   - For Go: `go test ./...` and `go vet ./...`
   - For C: Use the provided Makefile to build and run tests (see `bindings/c/README.md`)
   - For other languages: follow the conventions in their respective `README.md` files.
5. **Update documentation** (README, RFC, examples, binding-specific docs) if your change affects usage or behavior.
6. **Submit a pull request** with a clear description of your changes and why they are needed. Indicate which bindings are affected and whether cross-language tests pass.


## Code Style

- **Go:**
  - Follow standard Go formatting (`gofmt`).
  - Use idiomatic Go error handling (avoid `panic` in library code).
  - Keep functions small and focused.
  - Use descriptive variable and function names.
- **C:**
  - Use consistent indentation and brace style.
  - Free all dynamically allocated memory (no leaks).
  - Use clear, descriptive names for functions and variables.
  - Document all exported functions in headers.
- **Other languages:**
  - Follow the language's best practices and conventions.


## Reporting Issues

- Please use the GitHub issue tracker to report bugs or request features.
- Include as much detail as possible (steps to reproduce, expected/actual behavior, language/binding version, OS, etc.).


## Community

- Be respectful and constructive in all interactions.
- See `CODE_OF_CONDUCT.md` for our community guidelines (if present).


## License

By contributing, you agree that your contributions will be licensed under the [MIT License](./LICENSE).

## Protocol Reference

For protocol details and specification, see the [RFC](./RFC.md).
