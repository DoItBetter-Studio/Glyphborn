import subprocess
from pathlib import Path

# Tool converter scripts
CONVERTER_SCRIPTS = [
    "convert_image.py",
    "convert_audio.py"
]

def build_assets():
    print("[🛠️] Building assets...")
    processed = 0
    
    for script_name in CONVERTER_SCRIPTS:
        script_path = Path(__file__).parent / script_name
        print (f"[▶] Running {script_name}")
        try:
            subprocess.run(["python", str(script_path)], check=True)
            processed += 1
        except subprocess.CalledProcessError as e:
            print(f"[❗] Failed in {script_name}: {e}")
    
    print(f"\n✅ Done. Ran {processed} converters.")

if __name__ == "__main__":
    build_assets()