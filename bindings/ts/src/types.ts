// UTE TypeScript types for schema and data

export type UteFieldType = 'null' | 'bool' | 'int' | 'string' | 'list' | 'struct';

export interface UteSchemaField {
    name: string;
    type: UteFieldType;
    elem?: UteSchemaField; // for lists
    fields?: UteSchemaField[]; // for structs
}

export interface UteSchemaVersion {
    version: number;
    fields: UteSchemaField[];
}

export interface UteMultiVersionSchema {
    versions: UteSchemaVersion[];
}

export type UteData = any; // For now, use 'any' for data, can be refined per schema
