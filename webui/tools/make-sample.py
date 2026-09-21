#!/usr/bin/env python3
"""Rigenera js/sample.js a partire da un SVG, per il disegno di prova.

  python3 tools/make-sample.py "../gdc master.svg"

Il file generato serve solo all'anteprima e allo sviluppo: build.py lo esclude
dalla variante destinata all'ESP32, dove ogni kilobyte conta.
"""
import json, sys, pathlib

src = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '../gdc master.svg')
out = pathlib.Path(__file__).resolve().parent.parent / 'js' / 'sample.js'
svg = src.read_text(encoding='utf-8')
out.write_text(
    '/* Generato da tools/make-sample.py a partire da "%s".\n'
    "   Serve solo all'anteprima: la variante caricata sull'ESP32 lo esclude. */\n"
    'window.SAMPLE_SVG = %s;\n'
    "window.SAMPLE_NAME = '%s';\n" % (src.name, json.dumps(svg), src.name),
    encoding='utf-8')
print('%s  %d byte' % (out.name, out.stat().st_size))
