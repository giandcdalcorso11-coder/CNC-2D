#!/usr/bin/env python3
"""
Trascrive e rinomina automaticamente i voice over registrati col telefono.

Uso:
  python rinomina_voiceover.py <cartella>            elabora una volta i file nuovi
  python rinomina_voiceover.py <cartella> --watch    resta in ascolto e elabora ogni nuovo file

Richiede: pip install faster-whisper

I file originali non vengono toccati: vengono create copie rinominate
(più una trascrizione .txt) dentro <cartella>/rinominati.
"""
import argparse
import json
import re
import shutil
import time
from pathlib import Path

AUDIO_EXT = {".m4a", ".mp3", ".wav", ".aac", ".ogg", ".opus", ".flac", ".caf"}
LOG_NAME = "_log-trascrizioni.csv"
PROCESSED_NAME = "_elaborati.json"
MAX_WORDS = 10
MAX_LEN = 60


def slugify(text, max_words=MAX_WORDS, max_len=MAX_LEN):
    words = re.findall(r"[A-Za-zÀ-ÿ0-9]+", text.lower())[:max_words]
    slug = "-".join(words)
    return slug[:max_len].rstrip("-") or "senza-testo"


def load_model():
    from faster_whisper import WhisperModel
    return WhisperModel("small", device="cpu", compute_type="int8")


def transcribe(model, path):
    segments, _ = model.transcribe(str(path), language="it")
    return " ".join(seg.text.strip() for seg in segments).strip()


def process_file(model, src, out_dir, log_path):
    text = transcribe(model, src)
    slug = slugify(text)
    dest = out_dir / f"{slug}{src.suffix.lower()}"
    i = 2
    while dest.exists():
        dest = out_dir / f"{slug}-{i}{src.suffix.lower()}"
        i += 1
    shutil.copy2(src, dest)
    (out_dir / f"{dest.stem}.txt").write_text(text, encoding="utf-8")
    with open(log_path, "a", encoding="utf-8", newline="") as f:
        safe_text = text.replace('"', "'")
        f.write(f'"{src.name}";"{dest.name}";"{safe_text}"\n')
    print(f"{src.name}  ->  {dest.name}")
    return dest


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("cartella")
    ap.add_argument("--watch", action="store_true", help="resta in ascolto su nuovi file")
    ap.add_argument("--out", default=None, help="cartella di output (default: <cartella>/rinominati)")
    args = ap.parse_args()

    src_dir = Path(args.cartella)
    out_dir = Path(args.out) if args.out else src_dir / "rinominati"
    out_dir.mkdir(parents=True, exist_ok=True)
    processed_path = src_dir / PROCESSED_NAME
    log_path = out_dir / LOG_NAME
    if not log_path.exists():
        log_path.write_text('"file originale";"file rinominato";"trascrizione"\n', encoding="utf-8")
    processed = set(json.loads(processed_path.read_text())) if processed_path.exists() else set()

    print("Carico il modello di trascrizione (la prima volta scarica il modello, poi resta in cache)...")
    model = load_model()

    def scan_once():
        changed = False
        for f in sorted(src_dir.iterdir()):
            if f.is_file() and f.suffix.lower() in AUDIO_EXT and f.name not in processed:
                try:
                    process_file(model, f, out_dir, log_path)
                    processed.add(f.name)
                    changed = True
                except Exception as e:
                    print(f"Errore su {f.name}: {e}")
        if changed:
            processed_path.write_text(
                json.dumps(sorted(processed), ensure_ascii=False, indent=2), encoding="utf-8"
            )

    scan_once()

    if args.watch:
        print(f"In ascolto su {src_dir} — lascia questa finestra aperta, Ctrl+C per fermare.")
        try:
            while True:
                time.sleep(5)
                scan_once()
        except KeyboardInterrupt:
            print("Fermato.")


if __name__ == "__main__":
    main()
