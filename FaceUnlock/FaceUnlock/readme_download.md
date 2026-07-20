---
license: apache-2.0
library_name: onnx
tags:
  - face-anti-spoofing
  - liveness-detection
  - minifasnet
  - onnx
language:
  - en
pipeline_tag: image-classification
---

# MiniFASNet-V2 (ONNX)

ONNX export of the **MiniFASNet-V2 (2.7_80x80)** face anti-spoofing model from
[minivision-ai/Silent-Face-Anti-Spoofing](https://github.com/minivision-ai/Silent-Face-Anti-Spoofing).

## What this is

A single-image face anti-spoofing classifier — given an 80×80 BGR face crop,
returns a 3-class softmax `[live, print-attack, replay-attack]`. Used to gate
identification flows against printed-photo / screen-replay attacks.

This repo redistributes the upstream weights converted to ONNX opset 11 so they
can be loaded with `onnxruntime` (no PyTorch dependency at inference time). The
model weights themselves are bit-equivalent to the upstream `.pth` — only the
serialization format changed.

## Provenance

- **Upstream weights:** [`2.7_80x80_MiniFASNetV2.pth`](https://github.com/minivision-ai/Silent-Face-Anti-Spoofing/raw/master/resources/anti_spoof_models/2.7_80x80_MiniFASNetV2.pth) — SHA-256 `a5eb02e1843f19b5386b953cc4c9f011c3f985d0ee2bb9819eea9a142099bec0`.
- **Upstream architecture:** `MiniFASNetV2(embedding_size=128, conv6_kernel=(5,5), drop_p=0.2, num_classes=3, img_channel=3)` per `minivision-ai/Silent-Face-Anti-Spoofing/src/model_lib/MiniFASNet.py`.
- **Conversion:** torch 2.2.2 → ONNX opset 11, `torch.onnx.export` with dynamic batch axis. Conversion script: see `convert_minifasnet_to_onnx.py` reproduced from [garciafido/agilface](https://github.com/garciafido/agilface)'s `scripts/` directory.
- **ONNX SHA-256:** `d7b3cd9ba8a7ceb13baa8c4720902e27ca3112eff52f926c08804af6b6eecc7b`
- **Size:** 1,744,116 bytes.

## Preprocessing

Input shape: `(1, 3, 80, 80)`, float32, **BGR**, range `[0.0, 1.0]` (i.e. `pixel / 255`).

The reference pipeline:

1. Detect a face (e.g. with OpenCV's YuNet) and obtain the bounding box.
2. Crop the image with a **2.7× scale margin** around the bbox center (matches the
   upstream filename prefix `2.7_80x80`).
3. Resize the crop to 80×80, BGR, no alignment warp.
4. Normalize: `pixel / 255` → `[0, 1]`.
5. Reshape HWC → NCHW, run inference.
6. Apply softmax over the 3-class output. Liveness score = `1 - (p[print] + p[replay])`.

A reference implementation lives at
[`packages/infrastructure-py/src/agilface_infrastructure/ml/liveness_detector.py`](https://github.com/garciafido/agilface)
in the AgilFace repo.

## License

Apache 2.0 — same license as the upstream model. The original `LICENSE` file is
included in this repo (`LICENSE`). Attribution: minivision-ai (Beijing
Mininglamp Vision Technology Co., Ltd.). This repo does not claim authorship or
training of the model — only the format conversion.

## Citation

If you use this model, cite the upstream work:

```
@misc{minivisionai2020silentantispoofing,
  title  = {Silent-Face-Anti-Spoofing},
  author = {Mininglamp Vision Technology Co., Ltd.},
  year   = {2020},
  howpublished = {\\url{https://github.com/minivision-ai/Silent-Face-Anti-Spoofing}},
}
```
