"""
build_hd_icon_pack.py — Package HD item-icon PNGs into 2ship .o2r texture-pack mods.

Produces one .o2r per art style (toon / 3d) that OVERRIDES the base 32x32 NEI icons
baked into 2ship.o2r. Each override is a LUS "Texture" resource (binary, version 1)
stored at the exact base path `textures/icon_item_custom/gItemIcon<Name>Tex`.

How HD scaling works (verified against libultraship):
  - The display list still declares the original 32x32 tile.
  - The Texture resource carries the real (large) dimensions + HByteScale / VPixelScale.
  - interpreter.cpp reads those scales (tex->HByteScale/VPixelScale) and multiplies the
    tile load size, so a 1024x1024 image renders in the 32x32 slot.
  - scale = realDim / baseDim  (uniform 1024/32 = 32.0 for every icon here).

Binary layout replicated (see ResourceLoader.cpp + TextureFactory.cpp):
  OTR header (fixed 64 bytes):
    u8  ByteOrder = 0 (little)
    u8  IsCustom  = 1
    u8  pad[2]
    u32 Type      = 0x4F544558 ('OTEX')
    u32 Version   = 1
    u64 Id        = 0xDEADBEEFDEADBEEF
    (zero pad to 64)
  Texture body (version 1):
    u32 TextureType   = 1 (RGBA32bpp)
    u32 Width
    u32 Height
    u32 Flags         = 1  # TEX_FLAG_LOAD_AS_RAW — required for HByteScale to apply
    f32 HByteScale
    f32 VPixelScale
    u32 ImageDataSize  = Width*Height*4
    u8  ImageData[...] = raw RGBA32, row-major top-to-bottom

Usage:
    python build_hd_icon_pack.py
"""

import os
import struct
import zipfile
import sys

try:
    from PIL import Image
except ImportError:
    print("ERROR: needs Pillow (pip install Pillow)")
    sys.exit(1)

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "..", ".."))
BASE_O2R = os.path.join(REPO, "mm", "2ship.o2r")
OUT_DIR = os.path.join(REPO, "mm", "mods", "hd_icon_packs")

# Source art folders (1024x1024 RGBA) -> output pack name.
STYLES = [
    ("toon", r"C:\Users\LENOVO\Downloads\hd 2d"),
    ("3d", r"C:\Users\LENOVO\Downloads\hd 3d"),
]

# Normalized source-filename token -> gItemIcon<Name> (the base resource path stem).
# Normalization: lowercase, drop extension, remove 'hd' / '2d' / '3d' tokens and every
# non-alphanumeric char. Confirmed with user: rocs=Feather, caps=Cape.
NAME_MAP = {
    "ballchain": "BallAndChain",
    "beetle": "Beetle",
    "caps": "RocsCape",
    "firerod": "FireRod",
    "gustjar": "GustJar",
    "icerod": "IceRod",
    "minishcap": "MinishCap",
    "mitt": "MogmaMitts",
    "mitts": "MogmaMitts",
    "rocs": "RocsFeather",
    "shovel": "Shovel",
    "somaria": "CaneOfSomaria",
    "spinner": "Spinner",
    "switchhook": "SwitchHook",
    "timegate": "TimeGate",
    "whip": "Whip",
    "dekuleaf": "DekuLeaf",
}

OTR_HEADER_SIZE = 64
TYPE_OTEX = 0x4F544558
RES_VERSION = 1
RES_ID = 0xDEADBEEFDEADBEEF
TEXTURE_TYPE_RGBA32 = 1


def normalize(fname):
    stem = os.path.splitext(fname)[0].lower()
    for tok in ("hd", "3d", "2d"):
        stem = stem.replace(tok, "")
    return "".join(c for c in stem if c.isalnum())


def base_dims(base_zip):
    """Return {IconName: (w,h)} read from the base 2ship.o2r texture bodies."""
    dims = {}
    for name in set(NAME_MAP.values()):
        path = "textures/icon_item_custom/gItemIcon%sTex" % name
        try:
            d = base_zip.read(path)
        except KeyError:
            print("  WARN: base path missing, cannot verify dims: %s" % path)
            continue
        _typ, w, h, _sz = struct.unpack("<IIII", d[0x40:0x50])
        dims[name] = (w, h)
    return dims


def make_texture_resource(rgba_bytes, w, h, h_scale, v_scale):
    hdr = bytearray(OTR_HEADER_SIZE)
    hdr[0] = 0          # ByteOrder = little
    hdr[1] = 1          # IsCustom
    struct.pack_into("<I", hdr, 4, TYPE_OTEX)
    struct.pack_into("<I", hdr, 8, RES_VERSION)
    struct.pack_into("<Q", hdr, 12, RES_ID)

    body = struct.pack(
        "<IIIIffI",
        TEXTURE_TYPE_RGBA32,
        w, h,
        1,                 # Flags = TEX_FLAG_LOAD_AS_RAW (1<<0). REQUIRED — the interpreter only
                           # applies HByteScale/VPixelScale on the LOAD_AS_RAW path (ImportTextureRaw).
                           # With Flags=0 the HD data is read as a base-size tile -> blank. MM_Reloaded
                           # sets Flags=1 too; the earlier "Flags=0" was wrong.
        h_scale, v_scale,
        len(rgba_bytes),
    )
    return bytes(hdr) + body + rgba_bytes


def build_style(style, src_dir, dims):
    if not os.path.isdir(src_dir):
        print("SKIP style '%s': folder not found: %s" % (style, src_dir))
        return
    out_path = os.path.join(OUT_DIR, "nei_icons_hd_%s.o2r" % style)
    os.makedirs(OUT_DIR, exist_ok=True)

    packed, skipped = [], []
    with zipfile.ZipFile(out_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for fname in sorted(os.listdir(src_dir)):
            if not fname.lower().endswith(".png"):
                continue
            key = normalize(fname)
            icon = NAME_MAP.get(key)
            if icon is None:
                skipped.append("%s (norm='%s' -> no mapping)" % (fname, key))
                continue

            im = Image.open(os.path.join(src_dir, fname)).convert("RGBA")
            w, h = im.width, im.height
            bw, bh = dims.get(icon, (32, 32))
            # Aspect check: HD art must match base aspect or the icon skews.
            if bw and bh and abs((w / h) - (bw / bh)) > 0.001:
                skipped.append("%s -> %s ASPECT MISMATCH hd=%dx%d base=%dx%d"
                               % (fname, icon, w, h, bw, bh))
                continue
            h_scale = float(w) / float(bw)
            v_scale = float(h) / float(bh)

            res = make_texture_resource(im.tobytes(), w, h, h_scale, v_scale)
            arc = "textures/icon_item_custom/gItemIcon%sTex" % icon
            zf.writestr(arc, res)
            packed.append("%-24s -> %-16s %dx%d scale=%.1f" % (fname, icon, w, h, h_scale))

    print("\n=== %s -> %s ===" % (style, out_path))
    for p in packed:
        print("  OK   " + p)
    for s in skipped:
        print("  SKIP " + s)
    print("  packed %d icon(s), skipped %d" % (len(packed), len(skipped)))


def main():
    if not os.path.isfile(BASE_O2R):
        print("ERROR: base archive not found: %s" % BASE_O2R)
        sys.exit(1)
    with zipfile.ZipFile(BASE_O2R) as bz:
        dims = base_dims(bz)
    for style, src in STYLES:
        build_style(style, src, dims)
    print("\nDone. Drop ONE pack into the game's 'mods/' folder (not both at once).")


if __name__ == "__main__":
    main()
