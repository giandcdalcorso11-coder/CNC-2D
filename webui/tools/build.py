#!/usr/bin/env python3
"""Compone l'interfaccia in un unico file HTML.

La memoria dell'ESP32 non ha una cartella comoda in cui tenere dodici file, e
ogni file separato e' una richiesta HTTP in piu' che il microcontrollore deve
servire mentre pilota i motori. Questo script incolla fogli di stile e script
dentro la pagina, nell'ordine in cui compaiono in index.html.

  python3 tools/build.py            -> dist/cnc2d-ui.html (documento completo)
  python3 tools/build.py --bare     -> dist/cnc2d-ui.bare.html (senza involucro)

La variante "bare" serve dove la pagina viene inserita dentro un involucro
gia' pronto, come nelle anteprime.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
bare = '--bare' in sys.argv

html = (ROOT / 'index.html').read_text(encoding='utf-8')

title = re.search(r'<title>(.*?)</title>', html, re.S).group(1).strip()
body = re.search(r'<body>(.*)</body>', html, re.S).group(1)

css_files = re.findall(r'<link rel="stylesheet" href="([^"]+)"', html)
js_files = re.findall(r'<script src="([^"]+)"></script>', html)

# Il disegno di esempio serve solo all'anteprima e allo sviluppo: sulla
# memoria dell'ESP32 sarebbero kilobyte spesi per un logo.
if not bare:
    js_files = [f for f in js_files if not f.endswith('sample.js')]

def read(rel):
    return (ROOT / rel).read_text(encoding='utf-8')

style = '\n'.join('/* ===== %s ===== */\n%s' % (f, read(f)) for f in css_files)
script = '\n'.join('/* ===== %s ===== */\n%s' % (f, read(f)) for f in js_files)

# Gli script erano caricati in fondo al body: li rimettiamo la', cosi' l'ordine
# di esecuzione e il momento in cui trovano il DOM restano quelli di sempre.
body = re.sub(r'<script src="[^"]+"></script>\s*', '', body)

parts = [
    '<title>%s</title>' % title,
    '<style>\n%s\n</style>' % style,
    body.strip(),
    '<script>\n%s\n</script>' % script,
]
out = '\n'.join(parts)

if not bare:
    out = ('<!DOCTYPE html>\n<html lang="it">\n<head>\n'
           '<meta charset="utf-8">\n'
           '<meta name="viewport" content="width=device-width, initial-scale=1">\n'
           + parts[0] + '\n' + parts[1] + '\n</head>\n<body>\n'
           + parts[2] + '\n' + parts[3] + '\n</body>\n</html>\n')

dist = ROOT / 'dist'
dist.mkdir(exist_ok=True)
name = 'cnc2d-ui.bare.html' if bare else 'cnc2d-ui.html'
(dist / name).write_text(out, encoding='utf-8')
print('%s  %d byte' % (name, len(out.encode('utf-8'))))
