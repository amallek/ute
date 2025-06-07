// Minimal test for UTE TypeScript binding
import { loadSchemaFromFile } from '../src/schema';
import { serialize, deserialize } from '../src/codex';
import * as path from 'path';

const schemaPath = path.join(__dirname, '../../schemas/complex.yaml');
const versions = loadSchemaFromFile(schemaPath);
const schema = versions[0].fields;

const data = {
    devices: [
        { id: 1, name: 'device1' },
        { id: 2, name: 'device2' },
    ],
};

const encoded = serialize(data, schema);
console.log('Serialized:', Buffer.from(encoded).toString('hex'));

const [decoded] = deserialize(encoded, schema);
console.log('Deserialized:', JSON.stringify(decoded, null, 2));
