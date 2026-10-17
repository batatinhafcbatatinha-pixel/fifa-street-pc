#!/bin/sh
# quick.sh <seconds> <extra env ;-list>: short boot, then frame brightness + key log lines
W=/c/Users/dudua/Documents/Codex/2026-10-09/install-rea-and-connect-it-to/work
cd "$W/street1"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File ../test-fifa-boot.ps1 -Seconds "$1" -CaptureFrames \
  -ExtraEnv "RECOMP_HLE_PRIMARY=1;RECOMP_HLE_NOGPU=1;RECOMP_AUDIO_NATIVE=1;$2" > /dev/null 2>&1
d=$(ls -td runs/*/ | head -1)
echo "run $d env [$2]"
python -c "
from PIL import Image, ImageStat;import glob
for f in sorted(glob.glob('${d}frame-hle*.bmp'))[:8]:
    im=Image.open(f).convert('RGB'); print(f.split('/')[-1], [round(x) for x in ImageStat.Stat(im).mean])"
grep -a -E "no code|kernel ordinal|compile failed|disabled|HLE-API\] native|HLE-D3D11\] swap" "${d}boot.stderr.log" | tail -4
