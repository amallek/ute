// UTE schema loader and parser for TypeScript
import fs from 'fs';
import yaml from 'yaml';
import { UteSchemaField, UteSchemaVersion, UteMultiVersionSchema } from './types';

export function loadSchemaFromFile(path: string): UteSchemaVersion[] {
    const text = fs.readFileSync(path, 'utf8');
    const doc = yaml.parse(text);
    if (doc.versions && Array.isArray(doc.versions)) {
        return doc.versions as UteSchemaVersion[];
    }
    if (doc.fields && Array.isArray(doc.fields)) {
        // fallback: single-version schema
        return [{ version: 1, fields: doc.fields as UteSchemaField[] }];
    }
    throw new Error('Invalid schema file: missing versions or fields');
}
