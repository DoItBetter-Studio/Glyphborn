import os
from pathlib import Path
from PIL import Image

ASSET_DIR = Path(__file__).resolve().parent.parent / "assets" / "images"
OUTPUT_DIR = Path(__file__).resolve().parent.parent / "resources" / "images"

# GBA Limits
MAX_COLORS_4BPP = 16
MAX_COLORS_8BPP = 255

def sanitize_name(path: Path) -> str:
    return "_".join(path.with_suffix("").parts).replace("-", "_")

def needs_rebuild(src: Path, dst: Path) -> bool:
    return not dst.exists() or src.stat().st_mtime > dst.stat().st_mtime

def detect_palette(img: Image.Image):
    if img.mode != 'P':
        raise ValueError(f"Image mode must be 'P' (indexed). Got '{img.mode}'.")
    
    pixels = set(img.getdata())
    num_colors = len(pixels)
    
    if num_colors <= 2:
        bitdepth = 1
    elif num_colors <= 4:
        bitdepth = 2
    elif num_colors <= MAX_COLORS_4BPP:
        bitdepth = 4
    elif num_colors <= MAX_COLORS_8BPP:
        bitdepth = 8
    else:
        raise ValueError(f"Pallete has {num_colors} colors, exceeding 8bpp max ({MAX_COLORS_8BPP}).")
    
    return num_colors, bitdepth

def pack_pixels(pixels, bitdepth):
    """Pack pixel indices into bytes according to bitdepth."""
    packed = []
    pixels_per_byte = 8 // bitdepth
    mask = (1 << bitdepth) - 1
    
    for i in range(0, len(pixels), pixels_per_byte):
        byte = 0
        for j in range(pixels_per_byte):
            if i + j < len(pixels):
                pix = pixels[i + j] & mask
                shift = (pixels_per_byte - 1 - j) * bitdepth
                byte |= pix << shift
        packed.append(byte)
    return packed

def palette_to_rgb_array(palette, num_colors):
    """Extract RGB palette entries as uin8_t array [R, G, B, ...]."""
    rgb = []
    for i in range(num_colors):
        r = palette[i*3]
        g = palette[i*3+1]
        b = palette[i*3+2]
        rgb.extend([r,g,b])
    return rgb

def write_header(output_path: Path, name: str, width: int, height: int, bitdepth: int, pixel_data: list, palette_data: list):
    guard = name.upper() + "_H"
    with open(output_path, "w") as f:
        f.write(f"#ifndef {guard}\n#define {guard}\n\n")
        f.write(f"#define {name.upper()}_WIDTH {width}\n")
        f.write(f"#define {name.upper()}_HEIGHT {height}\n")
        f.write(f"#define {name.upper()}_BITDEPTH {bitdepth}\n")
        
        # Palette
        if palette_data:
            f.write(f"const unsigned char {name}_palette[] = {{\n")
            for i in range(0, len(palette_data), 12):
                chunk = palette_data[i:i+12]
                f.write("   "+", ".join(f"0x{c:02X}" for c in chunk) + ",\n")
            f.write("};\n\n")
        
        # Pixel data
        f.write(f"const unsigned char {name}[] = {{\n")
        for i in range(0, len(pixel_data), 12):
            chunk = pixel_data[i:i+12]
            f.write("   "+", ".join(f"0x{b:02X}" for b in chunk) + ",\n")
        f.write("};\n\n")
        
        f.write(f"#endif // !{guard}\n")

def convert_image(file_path: Path, out_path: Path):
    img = Image.open(file_path)
    num_colors, bitdepth = detect_palette(img)
    pixels = list(img.getdata())
    palette = img.getpalette()
    packed_pixels = pack_pixels(pixels, bitdepth)
    palette_rgb = palette_to_rgb_array(palette, num_colors)
    
    name = sanitize_name(file_path.relative_to(ASSET_DIR))
    write_header(out_path, name, img.width, img.height, bitdepth, packed_pixels, palette_rgb)
    print(f"[✔️] {file_path.relative_to(ASSET_DIR)} ({num_colors} colors, {bitdepth}bpp) -> {out_path.name}")

def convert_all_images():
    if not ASSET_DIR.exists():
        print("[❗] No assets/images/ folder found.")
        return
    
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    for root, _, files in os.walk(ASSET_DIR):
        for file in files:
            if not file.lower().endswith(".png"):
                continue
            
            full_path = Path(root) / file
            rel_path = full_path.relative_to(ASSET_DIR)
            out_name = sanitize_name(rel_path) + ".h"
            out_path = OUTPUT_DIR / out_name
            
            try:
                if needs_rebuild(full_path, out_path):
                    convert_image(full_path, out_path)
                else:
                    print(f"[=] Skipping {rel_path} (up-to-date)")
            except Exception as e:
                print(f"[❗] Error processing {rel_path}: {e}")

if __name__ == "__main__":
    convert_all_images()