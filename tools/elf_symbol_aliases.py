#!/usr/bin/env python3
"""Generate zero-size retail symbol aliases without changing allocated bytes.

DTK can emit a boundary label twice and before the function at that address.
The legacy MW linker loops on that symbol order. Keep a single alias after the
real symbols, remapping relocations and CodeWarrior's per-symbol metadata.
"""

import argparse
import struct
from pathlib import Path


def normalize(source: Path, output: Path, aliases: dict) -> None:
    blob = source.read_bytes()
    header = list(struct.unpack_from(">16sHHIIIIIHHHHHH", blob))
    if (
        header[0][:6] != b"\x7fELF\x01\x02"
        or header[1:3] != [1, 20]
        or header[10] != 0
        or header[11] != 40
    ):
        raise ValueError("Expected a big-endian ELF32 PowerPC relocatable object")
    sections = [
        list(struct.unpack_from(">IIIIIIIIII", blob, header[6] + i * 40))
        for i in range(header[12])
    ]
    payloads = [blob[s[4] : s[4] + s[5]] if s[1] != 8 else b"" for s in sections]
    tables = [i for i, s in enumerate(sections) if s[1] == 2]
    if len(tables) != 1:
        raise ValueError("Expected one ELF symbol table")
    table = tables[0]
    if sections[table][9] != 16:
        raise ValueError("Unexpected symbol entry size")
    strings = sections[table][6]
    original = [
        list(struct.unpack_from(">IIIBBH", payloads[table], i))
        for i in range(0, len(payloads[table]), 16)
    ]

    def name(symbol):
        return payloads[strings][symbol[0] :].split(b"\0", 1)[0].decode()

    symbols = []
    old_to_new = {}
    seen_aliases = {}
    for i, symbol in enumerate(original):
        symbol_name = name(symbol)
        if symbol_name in aliases and symbol_name in seen_aliases:
            index = seen_aliases[symbol_name]
            if symbol[1:] != symbols[index][1:]:
                raise ValueError(f"Conflicting alias definitions: {symbol_name}")
            old_to_new[i] = index
        else:
            old_to_new[i] = len(symbols)
            if symbol_name in aliases:
                seen_aliases[symbol_name] = len(symbols)
            symbols.append(symbol)

    for alias, target in aliases.items():
        targets = [s for s in symbols if name(s) == target]
        if len(targets) != 1:
            raise ValueError(f"Expected one defined target: {target}")
        symbol = targets[0]
        if symbol[3] != 0x12 or not symbol[2] or not 0 < symbol[5] < len(sections):
            raise ValueError(
                f"Alias target must be a defined global function: {target}"
            )
        existing = [s for s in symbols if name(s) == alias]
        expected = [symbol[1], 0, 0x10, 0, symbol[5]]
        if existing and existing[0][5] == 0:
            # A reference from the same object becomes the alias definition.
            existing[0][1:] = expected
        elif existing:
            if existing[0][1:] != expected:
                raise ValueError(f"Alias has a different address or binding: {alias}")
        else:
            offset = len(payloads[strings])
            payloads[strings] += alias.encode() + b"\0"
            symbols.append([offset, *expected])

    order = [i for i, s in enumerate(symbols) if name(s) not in aliases]
    order += [i for i, s in enumerate(symbols) if name(s) in aliases]
    inverse = {old: new for new, old in enumerate(order)}
    symbols = [symbols[i] for i in order]
    old_to_new = {old: inverse[new] for old, new in old_to_new.items()}

    for i, section in enumerate(sections):
        if payloads[i].startswith(b"CodeWarrior"):
            comment = payloads[i]
            if len(comment) != 44 + len(original) * 8:
                raise ValueError("Unexpected CodeWarrior symbol metadata length")
            records = [None] * len(symbols)
            for old, new in old_to_new.items():
                if records[new] is None:
                    records[new] = comment[44 + old * 8 : 52 + old * 8]
            payloads[i] = comment[:44] + b"".join(
                record if record is not None else bytes(8) for record in records
            )
        if section[1] in (4, 9) and section[6] == table:
            relocations = bytearray(payloads[i])
            for offset in range(0, len(relocations), section[9]):
                info = struct.unpack_from(">I", relocations, offset + 4)[0]
                info = (old_to_new[info >> 8] << 8) | (info & 255)
                struct.pack_into(">I", relocations, offset + 4, info)
            payloads[i] = bytes(relocations)
    payloads[table] = b"".join(struct.pack(">IIIBBH", *s) for s in symbols)

    result = bytearray(blob[: header[8]])
    for i, section in enumerate(sections):
        if i == 0:
            continue
        result.extend(bytes((-len(result)) % max(section[8], 1)))
        section[4] = len(result)
        if section[1] != 8:
            section[5] = len(payloads[i])
            result.extend(payloads[i])
    result.extend(bytes((-len(result)) % 4))
    header[6] = len(result)
    result.extend(b"".join(struct.pack(">IIIIIIIIII", *s) for s in sections))
    struct.pack_into(">16sHHIIIIIHHHHHH", result, 0, *header)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(result)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--alias", action="append", required=True)
    args = parser.parse_args()
    aliases = dict(item.split("=", 1) for item in args.alias)
    normalize(args.source, args.output, aliases)


if __name__ == "__main__":
    main()
