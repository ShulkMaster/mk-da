#!/usr/bin/env python3
"""Generate allowlisted CodeWarrior assembly macros from DTK retail assembly."""

from __future__ import annotations

import argparse
import json
import os
import re
import tempfile
from pathlib import Path


FUNCTION_RE = re.compile(r"^\.fn\s+([A-Za-z_][A-Za-z0-9_]*)\s*,")
END_FUNCTION_RE = re.compile(r"^\.endfn\s+([A-Za-z_][A-Za-z0-9_]*)\s*$")
LABEL_RE = re.compile(r"^\.sym\s+([A-Za-z_][A-Za-z0-9_]*)\s*,\s*global\s*$")
INSTRUCTION_RE = re.compile(
    r"^/\*\s*([0-9A-Fa-f]{8})\s+[0-9A-Fa-f]{8}\s+"
    r"((?:[0-9A-Fa-f]{2}\s+){3}[0-9A-Fa-f]{2})\s*\*/\s*(.+?)\s*$"
)
SYMBOL_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
EXTERNAL_BRANCH_RE = re.compile(r"^(?:b|bl)\s+[A-Za-z_][A-Za-z0-9_]*$")
BRANCH_TARGET_RE = re.compile(r"^b[a-z]*[+-]?\s+(?:cr[0-7],\s*)?(?P<target>\S+)$")
# Address halves are emitted as source so the compiler regenerates the relocation.
SYMBOL_HA_L_RE = re.compile(
    r"^(?:lis\s+r[0-9]+,\s*[A-Za-z_][A-Za-z0-9_]*@(?:ha|h)"
    r"|(?:addi|ori)\s+r[0-9]+,\s*r[0-9]+,\s*[A-Za-z_][A-Za-z0-9_]*@l)$"
)
SDA21_BASE_RE = re.compile(
    r"(?P<symbol>[A-Za-z_][A-Za-z0-9_]*)@sda21\((?P<base>r(?:0|13))\)"
)
SDA21_IMMEDIATE_RE = re.compile(r"(?P<symbol>[A-Za-z_][A-Za-z0-9_]*)@sda21\b")
SDA21_LI_RE = re.compile(
    r"^li\s+(?P<dest>r[0-9]+),\s*(?P<symbol>[A-Za-z_][A-Za-z0-9_]*)@sda21$"
)


def parse_int(value: object, field: str) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        return int(value, 0)
    raise ValueError(f"{field}: expected an integer or integer string")


def read_functions(path: Path) -> dict[str, tuple[int, tuple[tuple[int, str], ...]]]:
    functions: dict[str, tuple[int, tuple[tuple[int, str], ...]]] = {}
    name: str | None = None
    address: int | None = None
    instructions: list[tuple[int, str]] = []
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        start = FUNCTION_RE.match(line)
        if start:
            if name is not None:
                raise ValueError(f"{path}:{line_number}: nested .fn")
            name, address, instructions = start.group(1), None, []
            continue
        end = END_FUNCTION_RE.match(line)
        if end:
            if name is None or end.group(1) != name or address is None or not instructions:
                raise ValueError(f"{path}:{line_number}: invalid .endfn")
            if name in functions:
                raise ValueError(f"{path}:{line_number}: duplicate function {name}")
            functions[name] = (address, tuple(instructions))
            name = None
            continue
        instruction = INSTRUCTION_RE.match(line)
        if instruction and name is not None:
            current = int(instruction.group(1), 16)
            if address is None:
                address = current
            expected = address + 4 * len(instructions)
            if current != expected:
                raise ValueError(f"{path}:{line_number}: address 0x{current:X}, expected 0x{expected:X}")
            instructions.append(
                (int(instruction.group(2).replace(" ", ""), 16), instruction.group(3))
            )
    if name is not None:
        raise ValueError(f"{path}: unterminated function {name}")
    return functions


def read_labels(path: Path) -> dict[str, tuple[tuple[int, str], ...]]:
    """Global labels inside each function, keyed by instruction index."""
    labels: dict[str, list[tuple[int, str]]] = {}
    name: str | None = None
    count = 0
    for line in path.read_text(encoding="utf-8").splitlines():
        start = FUNCTION_RE.match(line)
        if start:
            name, count = start.group(1), 0
            labels[name] = []
        elif END_FUNCTION_RE.match(line):
            name = None
        elif name is not None:
            label = LABEL_RE.match(line)
            if label:
                labels[name].append((count, label.group(1)))
            elif INSTRUCTION_RE.match(line):
                count += 1
    return {key: tuple(value) for key, value in labels.items()}


def generate(manifest: Path, version: str, build_root: Path) -> tuple[Path, str]:
    data = json.loads(manifest.read_text(encoding="utf-8"))
    if data.get("version") != version:
        raise ValueError(f"{manifest}: manifest version does not match {version}")
    default_assembly = data.get("assembly")
    output = data.get("output")
    entries = data.get("functions")
    if not isinstance(default_assembly, str) or not isinstance(output, str):
        raise ValueError(f"{manifest}: assembly and output must be paths")
    if not isinstance(entries, list) or not entries:
        raise ValueError(f"{manifest}: functions must be a non-empty list")
    assemblies: dict[str, dict[str, tuple[int, tuple[tuple[int, str], ...]]]] = {}
    label_sets: dict[str, dict[str, tuple[tuple[int, str], ...]]] = {}
    lines = ["/* Generated from version-specific retail assembly. Do not edit. */", ""]
    seen: set[str] = set()
    for entry in entries:
        name = entry.get("name") if isinstance(entry, dict) else None
        if not isinstance(name, str) or not SYMBOL_RE.fullmatch(name) or name in seen:
            raise ValueError(f"{manifest}: invalid or duplicate function {name!r}")
        seen.add(name)
        source = entry.get("assembly", default_assembly)
        if not isinstance(source, str):
            raise ValueError(f"{name}.assembly: expected a path")
        assembly_path = build_root / version / "asm" / source
        if source not in assemblies:
            assemblies[source] = read_functions(assembly_path)
            label_sets[source] = read_labels(assembly_path)
        available = assemblies[source]
        # A "function" entry is an inline block inside that C function, not a whole function.
        function = entry.get("function", name)
        if not isinstance(function, str) or function not in available:
            raise ValueError(f"{assembly_path}: allowlisted function {function!r} not found")
        address, instructions = available[function]
        first = 0
        expected_address = parse_int(entry.get("address"), f"{name}.address")
        expected_size = parse_int(entry.get("size"), f"{name}.size")
        if function == name:
            if address != expected_address or len(instructions) * 4 != expected_size:
                raise ValueError(f"{name}: retail address or size does not match manifest")
            lines.extend([f"#define SEQ_{name}() \\", "    nofralloc; \\"])
        else:
            start = expected_address - address
            if (
                start <= 0
                or start % 4
                or expected_size <= 0
                or expected_size % 4
                or start + expected_size > len(instructions) * 4
            ):
                raise ValueError(f"{name}: block is not inside {function}")
            first = start // 4
            instructions = instructions[first : (start + expected_size) // 4]
            lines.append(f"#define SEQ_{name}() \\")
        # Exported labels inside the range become entry points at the same offset.
        entry_labels = {
            index - first: label
            for index, label in label_sets[source].get(function, ())
            if first <= index < first + len(instructions)
        }
        for index, (word, assembly) in enumerate(instructions):
            if index in entry_labels:
                lines.append(f"    entry {entry_labels[index]}; \\")
            suffix = " \\" if index + 1 < len(instructions) else ""
            if "@sda21" in assembly:
                address_load = SDA21_LI_RE.fullmatch(assembly)
                if address_load:
                    assembly = (
                        f"la {address_load.group('dest')}, "
                        f"{address_load.group('symbol')}(r13)"
                    )
                else:
                    assembly = SDA21_BASE_RE.sub(r"\g<symbol>(\g<base>)", assembly)
                    assembly = SDA21_IMMEDIATE_RE.sub(r"\g<symbol>", assembly)
                if "@sda21" in assembly:
                    raise ValueError(f"{name}: unsupported SDA21 syntax: {assembly}")
                lines.append(f"    {assembly};{suffix}")
            elif EXTERNAL_BRANCH_RE.fullmatch(assembly) or SYMBOL_HA_L_RE.fullmatch(assembly):
                lines.append(f"    {assembly};{suffix}")
            else:
                # Raw words carry retail-resolved fields; refuse any relocation.
                branch = BRANCH_TARGET_RE.fullmatch(assembly)
                if "@" in assembly or (
                    branch
                    and not branch.group("target").startswith(".L_")
                    and branch.group("target") != function
                    # Absolute branches (ba/bla) to a literal address carry no relocation.
                    and not re.fullmatch(r"b[a-z]*a[+-]?\s+(?:cr[0-7],\s*)?0x[0-9A-Fa-f]+", assembly)
                ):
                    raise ValueError(f"{name}: unsupported relocation: {assembly}")
                lines.append(f"    opword 0x{word:08X};{suffix}")
        lines.append("")
    return build_root / version / "include" / output, "\n".join(lines)


def atomic_write(path: Path, contents: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="ascii", newline="\n") as output:
            output.write(contents)
        os.replace(temporary, path)
    except BaseException:
        try:
            os.unlink(temporary)
        except FileNotFoundError:
            pass
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--build-root", type=Path, default=Path("build"))
    args = parser.parse_args()
    destination, contents = generate(args.manifest, args.version, args.build_root)
    atomic_write(destination, contents)
    print(destination)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
