// UTE serialization/deserialization core logic for TypeScript
import { UteSchemaField, UteData } from './types';

// Type prefix constants (same as Go/C)
const T_NULL = 0b000 << 5;
const T_BOOL = 0b001 << 5;
const T_INT = 0b010 << 5;
const T_BYTES = 0b011 << 5;
const T_LIST = 0b100 << 5;
const T_STRUCT = 0b101 << 5;

// Encode a varint (unsigned)
function encodeVarint(n: number): Uint8Array {
    const out: number[] = [];
    let v = n >>> 0;
    while (v >= 0x80) {
        out.push((v | 0x80) & 0xff);
        v >>>= 7;
    }
    out.push(v);
    return Uint8Array.from(out);
}

// Decode a varint (returns [value, bytesRead])
function decodeVarint(buf: Uint8Array, offset: number): [number, number] {
    let result = 0, shift = 0, i = offset;
    while (i < buf.length) {
        const b = buf[i++];
        result |= (b & 0x7f) << shift;
        if (!(b & 0x80)) break;
        shift += 7;
    }
    return [result, i - offset];
}

// Serialize a value according to schema
export function serialize(data: any, schema: UteSchemaField[]): Uint8Array {
    const out: number[] = [];
    for (const field of schema) {
        const v = data[field.name];
        switch (field.type) {
            case 'int':
                out.push(T_INT);
                out.push(...encodeVarint(v));
                break;
            case 'string':
                out.push(T_BYTES);
                const strBytes = Buffer.from(v, 'utf8');
                out.push(...encodeVarint(strBytes.length));
                out.push(...strBytes);
                break;
            case 'list':
                out.push(T_LIST);
                out.push(...encodeVarint(v.length));
                for (const item of v) {
                    if (field.elem!.type === 'struct') {
                        out.push(...serialize(item, field.elem!.fields!));
                    } else {
                        out.push(...serialize({ '': item }, [field.elem!]));
                    }
                }
                break;
            case 'struct':
                out.push(T_STRUCT);
                out.push(...encodeVarint(field.fields!.length));
                out.push(...serialize(v, field.fields!));
                break;
            default:
                throw new Error('Unsupported type: ' + field.type);
        }
    }
    return Uint8Array.from(out);
}

// Deserialize a value according to schema
export function deserialize(buf: Uint8Array, schema: UteSchemaField[], offset = 0): [any, number] {
    const out: any = {};
    let i = offset;
    for (const field of schema) {
        const h = buf[i++];
        switch (field.type) {
            case 'int': {
                if ((h >> 5) !== 2) throw new Error('Expected int');
                const [v, n] = decodeVarint(buf, i);
                out[field.name] = v;
                i += n;
                break;
            }
            case 'string': {
                if ((h >> 5) !== 3) throw new Error('Expected string');
                const [len, n] = decodeVarint(buf, i);
                i += n;
                out[field.name] = Buffer.from(buf.slice(i, i + len)).toString('utf8');
                i += len;
                break;
            }
            case 'list': {
                if ((h >> 5) !== 4) throw new Error('Expected list');
                const [count, n] = decodeVarint(buf, i);
                i += n;
                const arr = [];
                for (let j = 0; j < count; ++j) {
                    if (field.elem!.type === 'struct') {
                        const [item, used] = deserialize(buf, field.elem!.fields!, i);
                        arr.push(item);
                        i += used;
                    } else {
                        const [item, used] = deserialize(buf, [field.elem!], i);
                        arr.push(item['']);
                        i += used;
                    }
                }
                out[field.name] = arr;
                break;
            }
            case 'struct': {
                if ((h >> 5) !== 5) throw new Error('Expected struct');
                const [nfields, n] = decodeVarint(buf, i);
                i += n;
                const [obj, used] = deserialize(buf, field.fields!, i);
                out[field.name] = obj;
                i += used;
                break;
            }
            default:
                throw new Error('Unsupported type: ' + field.type);
        }
    }
    return [out, i - offset];
}
