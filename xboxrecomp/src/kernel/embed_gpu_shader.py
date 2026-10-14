"""Regenerate the checked-in C string include after editing the HLSL file."""
import json
from pathlib import Path

root = Path(__file__).resolve().parent
source = (root / "nv2a_gpu_combiner.hlsl").read_text(encoding="utf-8")
result = "\n".join(json.dumps(line + "\n") for line in source.splitlines()) + "\n"
(root / "nv2a_gpu_combiner.inc").write_text(result, encoding="utf-8")
