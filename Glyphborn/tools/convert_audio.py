import wave
import os
import struct
from pathlib import Path

ASSET_DIR = Path(__file__).resolve().parent.parent / "assets" / "audio"
OUTPUT_DIR = Path(__file__).resolve().parent.parent / "resources" / "audio"

def sanitize_name(path: Path) -> str:
    return "_".join(path.with_suffix("").parts).replace("-", "_")

def needs_rebuilt(src: Path, dst: Path) -> bool:
    return not dst.exists() or src.stat().st_mtime > dst.stat().st_mtime

def write_header(output_path: Path, name: str, pcm_bytes: bytes):
    guard = name.upper() + "_H"
    
    with open(output_path, "w") as f:
        f.write(f"#ifndef {guard}\n#define {guard}\n\n")
        f.write(f"#define {name.upper()}_LEN {len(pcm_bytes)}\n\n")
        
        f.write(f"static const unsigned char {name}[] = {{\n")
        for i in range(0, len(pcm_bytes), 12):
            chunk = pcm_bytes[i:i+12]
            f.write("   " + ", ".join(str(b) for b in chunk) + ",\n")
        f.write("};\n\n")
        
        f.write(f"#endif // !{guard}\n")

def convert_audio(file_path: Path, out_path: Path):
    with wave.open(str(file_path), "rb") as wf:
        nchannels = wf.getnchannels()
        framerate = wf.getframerate()
        frames = wf.readframes(wf.getnframes())
        sampwidth = wf.getsampwidth()
        
        if nchannels != 1:
            raise ValueError(f"{file_path.name} is not mono.")
        if framerate != 44100:
            raise ValueError(f"{file_path.name} must be 44.1kHz.")
        if sampwidth == 2:
            samples = struct.unpack("<" + "h" * (len(frames) // 2), frames)
            pcm_bytes = [((s + 32768) >> 8) - 128 for s in samples]
        elif sampwidth == 1:
            samples = struct.unpack("<" + "B" * len(frames), frames)
            pcm_bytes = [s - 128 for s in samples]
        else:
            raise ValueError(f"{file_path.name}: Unsupported sample width {sampwidth}")

    write_header(out_path, sanitize_name(file_path.relative_to(ASSET_DIR)), pcm_bytes)
    print(f"[✔️] {file_path.name} -> {out_path.name} ({len(pcm_bytes)} samples)")

def convert_all_audio():
    if not ASSET_DIR.exists():
        print("[❗] No assets/audio/ folder found.")
        return
    
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    for root, _, files in os.walk(ASSET_DIR):
        for file in files:
            if not file.lower().endswith(".wav"):
                continue
            
            full_path = Path(root) / file
            rel_path = full_path.relative_to(ASSET_DIR)
            out_name = sanitize_name(rel_path) + ".h"
            out_path = OUTPUT_DIR / out_name
            
            try:
                if needs_rebuilt(full_path, out_path):
                    convert_audio(full_path, out_path)
                else:
                    print(f"[=] Skipping {rel_path} (up-to-date)")
            except Exception as e:
                print(f"[❗] Error processing {rel_path}: {e}")

if __name__ == "__main__":
    convert_all_audio()