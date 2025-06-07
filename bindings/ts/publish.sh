
#!/bin/zsh
# Build and publish the utep package to npmjs.com

set -e

echo "Building TypeScript project..."
npm run build

echo "Publishing to npmjs.com..."
npm publish --access public

echo "Done."
