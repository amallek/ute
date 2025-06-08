
import fs from 'fs';
import yaml from 'yaml';
import { UteSchemaField, UteSchemaVersion, UteMultiVersionSchema } from './types';

/**
 * Recursively parse a SchemaField into a normalized structure (like Go's ParseSchemaField)
 */
export function parseSchemaField(sf: any): UteSchemaField {
    const out: UteSchemaField = {
        name: sf.name,
        type: sf.type,
    };
    if (sf.type === 'list' && sf.elem) {
        out.elem = parseSchemaField(sf.elem);
    }
    if (sf.type === 'struct' && Array.isArray(sf.fields)) {
        out.fields = sf.fields.map(parseSchemaField);
    }
    return out;
}

/**
 * Parse a list of SchemaFields (like Go's ParseSchemaFields)
 */
export function parseSchemaFields(fields: any[]): UteSchemaField[] {
    return fields.map(parseSchemaField);
}

/**
 * Load a UTE schema from a YAML string. Returns an array of schema versions (normalized).
 */
export function loadSchemaFromString(yamlString: string): UteSchemaVersion[] {
    const doc = yaml.parse(yamlString);
    if (doc.versions && Array.isArray(doc.versions)) {
        return doc.versions.map((v: any) => ({
            version: v.version,
            fields: parseSchemaFields(v.fields)
        }));
    }
    if (doc.fields && Array.isArray(doc.fields)) {
        // fallback: single-version schema
        return [{ version: 1, fields: parseSchemaFields(doc.fields) }];
    }
    throw new Error('Invalid schema string: missing versions or fields');
}

/**
 * Load a UTE schema from a YAML file. Returns an array of schema versions (normalized).
 */
export function loadSchemaFromFile(path: string): UteSchemaVersion[] {
    const text = fs.readFileSync(path, 'utf8');
    return loadSchemaFromString(text);
}
