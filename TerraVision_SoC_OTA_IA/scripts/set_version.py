#!/usr/bin/env python3
"""Cambia la version del firmware en CMakeLists.txt sin editar codigo C."""
import re
import sys
from pathlib import Path

root = Path(__file__).resolve().parent.parent
cmake = root / "CMakeLists.txt"
if len(sys.argv) != 2 or not re.fullmatch(r"\d+\.\d+\.\d+", sys.argv[1]):
    raise SystemExit("Uso: python scripts/set_version.py 1.0.0  (o 2.0.0)")
text = cmake.read_text(encoding="utf-8")
text, count = re.subn(r'set\(PROJECT_VER "[0-9]+\.[0-9]+\.[0-9]+"\)',
                      f'set(PROJECT_VER "{sys.argv[1]}")', text)
if count != 1:
    raise SystemExit("No se encontro set(PROJECT_VER ...) en CMakeLists.txt")
cmake.write_text(text, encoding="utf-8")
print("Proyecto actualizado a", sys.argv[1]); print("Ejecute: idf.py build")
