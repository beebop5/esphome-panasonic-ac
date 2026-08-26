#!/usr/bin/env python3
"""Verify every visible silkscreen text is on the board, clear of pads, and not overlapping."""
import sys, pcbnew

BRD = sys.argv[1] if len(sys.argv) > 1 else "panasonic-p11-revG.kicad_pcb"
S = 1e6
b = pcbnew.LoadBoard(BRD)

def on_board(x, y):
    if not (20 <= x <= 68.5 and 23 <= y <= 41): return False
    if x < 21.5 and y < 24.5 and (21.5 - x) + (24.5 - y) > 1.5: return False
    if x > 67.0 and y < 24.5 and (x - 67.0) + (24.5 - y) > 1.5: return False
    if x < 21.5 and y > 39.5 and (21.5 - x) + (y - 39.5) > 1.5: return False
    if x > 67.0 and y > 39.5 and (x - 67.0) + (y - 39.5) > 1.5: return False
    if x > 65.5 and 29 < y < 35: return False          # notch
    return True

texts = []
for d in b.GetDrawings():
    if d.GetClass() == "PCB_TEXT" and b.GetLayerName(d.GetLayer()) in ("F.SilkS","B.SilkS","F.Silkscreen","B.Silkscreen"):
        texts.append((d.GetText(), d.GetPosition(), d.GetTextHeight(), b.GetLayerName(d.GetLayer()), abs(d.GetTextAngle().AsDegrees())%180))
for f in b.GetFootprints():
    for d in list(f.GraphicalItems()) + [f.Reference(), f.Value()]:
        if d.GetClass() in ("PCB_TEXT", "PCB_FIELD") and b.GetLayerName(d.GetLayer()) in ("F.SilkS","B.SilkS","F.Silkscreen","B.Silkscreen"):
            if hasattr(d, "IsVisible") and not d.IsVisible(): continue
            texts.append((d.GetText(), d.GetPosition(), d.GetTextHeight(), b.GetLayerName(d.GetLayer()), abs(d.GetTextAngle().AsDegrees())%180))

boxes = []
for t, p, h, lay, ang in texts:
    cx, cy, sz = p.x / S, p.y / S, h / S
    w = len(t) * sz * 0.85
    if 45 < ang < 135: w, sz = sz, w      # rotated ~90 deg
    boxes.append((t, lay, cx - w / 2, cy - sz / 2, cx + w / 2, cy + sz / 2))

pads = []
for f in b.GetFootprints():
    for p in f.Pads():
        c = p.GetPosition(); s = p.GetSize()
        pads.append((f.GetReference() + "." + p.GetNumber(),
                     c.x / S - s.x / S / 2, c.y / S - s.y / S / 2,
                     c.x / S + s.x / S / 2, c.y / S + s.y / S / 2))

def ov(a, b_):
    return not (a[4] <= b_[1] or b_[3] <= a[2] or a[5] <= b_[2] or b_[4] <= a[3])

print("silk text items:", len(boxes))
bad = 0
for t, lay, x1, y1, x2, y2 in boxes:
    for cx, cy in ((x1, y1), (x2, y1), (x1, y2), (x2, y2)):
        if not on_board(cx, cy):
            print("  OFF-BOARD  %-8s %-8s (%.2f,%.2f)-(%.2f,%.2f)" % (t, lay, x1, y1, x2, y2)); bad += 1; break

for i in range(len(boxes)):
    for j in range(i + 1, len(boxes)):
        a, c = boxes[i], boxes[j]
        if a[1] != c[1]: continue
        if not (a[4]<=c[2] or c[4]<=a[2] or a[5]<=c[3] or c[5]<=a[3]):
            print("  TEXT-OVERLAP  '%s' <-> '%s' on %s" % (a[0], c[0], a[1])); bad += 1

for t, lay, x1, y1, x2, y2 in boxes:
    # through-hole pads exist on BOTH faces - check front and back silk alike
    for ref, px1, py1, px2, py2 in pads:
        if not (x2 <= px1 or px2 <= x1 or y2 <= py1 or py2 <= y1):
            print("  TEXT-ON-PAD   '%s' (%s) over %s" % (t, lay, ref)); bad += 1

print("PROBLEMS:", bad)
