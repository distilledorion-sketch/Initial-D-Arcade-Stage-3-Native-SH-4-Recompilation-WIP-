#!/usr/bin/env python3
"""Restore two exact course lookup files from a user-owned GDS-0033 dump.

This utility contains no game data or decryption key. It imports the DIMM ISO
reader from a user-supplied private tools directory, verifies the exact source
records, and writes a new private HOSTFS overlay without modifying its input.
"""

from __future__ import annotations

import argparse
import hashlib
import os
import shutil
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Asset:
    physical: str
    logical: str
    extent: int
    size: int
    digest: str


ASSETS = (
    Asset(
        "/ENV/K_DF1_PA.BIN",
        "env/k_df1_path_env.bin",
        0x000002DC,
        16_356,
        "5B57E3AF351D16F11F56414A200211C9CEE4C6E469E3D6FAC535670E990FE12F",
    ),
    Asset(
        "/PATH/K_DF_SDW.BIN",
        "path/k_df_sdw.bin",
        0x00000A56,
        16_356,
        "6A621D64A07A4643A9F87E00B089643AC305A57FEC35762DF126AB577B059D15",
    ),
)

V1022_BUNDLE = {
    "model/adv_newtitle/adv_newtitle_pol.bin.nz": (
        672,
        "B1BEF547C57B6039B4ADE58B81D77FED1A077F3C0439C95EE068B0BA384E9BCB",
    ),
    "model/adv_newtitle/adv_newtitle_pol.tbl": (
        8,
        "2B6D910BD88DC43D68E3283B89B4134E6423E67F9A459C8B85457254CB69AEF4",
    ),
    "model/adv_newtitle/adv_newtitle_tex.bin.nz": (
        557_056,
        "2E2BA7A6EDB13F708942A087E589C22E12FECE15031D4CFA44512A0F0908E9C4",
    ),
    "model/adv_newtitle/adv_newtitle_tex.tbl": (
        32,
        "726FDA272698A32F45FF68B887627BB1A732C2044F253968D574764311CC90AC",
    ),
}


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def verify(label: str, data: bytes, size: int, digest: str) -> None:
    actual = sha256(data)
    if len(data) != size or actual != digest:
        raise ValueError(
            f"{label} verification failed: size={len(data)} sha256={actual}; "
            f"expected size={size} sha256={digest}"
        )


def verify_base(base_drivea: Path) -> None:
    hostfs = base_drivea / "HOSTFS"
    if not hostfs.is_dir():
        raise FileNotFoundError(f"base HOSTFS is missing: {hostfs}")
    manifest = base_drivea / "IDAS3_DERIVED_ASSETS_v1022.txt"
    if not manifest.is_file():
        raise FileNotFoundError(f"v1022 manifest is missing: {manifest}")
    lines = manifest.read_text(encoding="utf-8").splitlines()
    if not lines or lines[0] != "IDAS3 v1022 private derived HOSTFS":
        raise ValueError("input does not have the expected v1022 manifest header")
    for logical, (size, digest) in V1022_BUNDLE.items():
        path = hostfs / Path(logical)
        if not path.is_file():
            raise FileNotFoundError(f"v1022 asset is missing: {path}")
        verify(f"v1022 {logical}", path.read_bytes(), size, digest)


def read_assets(tools: Path, bin_path: Path, pic_path: Path) -> dict[str, bytes]:
    tools = tools.resolve()
    extractor = tools / "extract_dimm_iso.py"
    if not extractor.is_file():
        raise FileNotFoundError(f"private extractor is missing: {extractor}")
    sys.path.insert(0, str(tools))
    from extract_dimm_iso import (  # type: ignore[import-not-found]
        DEFAULT_DIMM_ISO_BASE,
        DimmIso,
        EncryptedPayloadReader,
        find_dimm_iso_base,
        locate_encrypted_payload,
    )

    bin_path = bin_path.resolve()
    pic_path = pic_path.resolve()
    high_base, _target_name, target, pic_key = locate_encrypted_payload(
        bin_path, pic_path
    )
    outputs: dict[str, bytes] = {}
    with EncryptedPayloadReader(
        bin_path, high_base, target["extent"], target["size"], pic_key
    ) as reader:
        iso = DimmIso(
            reader, find_dimm_iso_base(reader, DEFAULT_DIMM_ISO_BASE)
        )
        for asset in ASSETS:
            entry, resolved = iso.resolve(asset.physical)
            if entry.is_dir:
                raise IsADirectoryError(str(resolved))
            if entry.extent != asset.extent:
                raise ValueError(
                    f"{asset.physical} extent is 0x{entry.extent:08X}; "
                    f"expected 0x{asset.extent:08X}"
                )
            payload = iso.read_file(entry)
            verify(asset.physical, payload, asset.size, asset.digest)
            outputs[asset.logical] = payload
    return outputs


def build(
    base_drivea: Path,
    output_drivea: Path | None,
    tools: Path,
    bin_path: Path,
    pic_path: Path,
) -> None:
    base_drivea = base_drivea.resolve()
    verify_base(base_drivea)
    outputs = read_assets(tools, bin_path, pic_path)

    if output_drivea is None:
        print("Verified exact course lookup assets from the user-owned dump:")
        for logical, payload in outputs.items():
            print(f"  {logical} size={len(payload)} sha256={sha256(payload)}")
        return

    output_drivea = output_drivea.resolve()
    if output_drivea == base_drivea:
        raise ValueError("output driveA must be distinct from the input tree")
    if output_drivea.exists():
        raise FileExistsError(f"refusing to replace existing output: {output_drivea}")
    output_drivea.parent.mkdir(parents=True, exist_ok=True)

    staging = Path(
        tempfile.mkdtemp(prefix=f".{output_drivea.name}.tmp-", dir=output_drivea.parent)
    )
    try:
        shutil.copytree(base_drivea / "HOSTFS", staging / "HOSTFS")
        for inherited in sorted(base_drivea.glob("IDAS3_DERIVED_ASSETS_v*.txt")):
            shutil.copy2(inherited, staging / inherited.name)
        for logical, payload in outputs.items():
            destination = staging / "HOSTFS" / Path(logical)
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(payload)

        manifest = [
            "IDAS3 v1023 private derived HOSTFS",
            "Exact course environment/path lookup recovery",
            "Source data remains in the user's own GDS-0033/PIC pair",
            f"base={base_drivea}",
            "",
        ]
        for asset in ASSETS:
            payload = outputs[asset.logical]
            manifest.append(
                f"{asset.physical} extent=0x{asset.extent:08X} "
                f"-> HOSTFS/{asset.logical} size={len(payload)} "
                f"sha256={sha256(payload)}"
            )
        (staging / "IDAS3_DERIVED_ASSETS_v1023.txt").write_text(
            "\n".join(manifest) + "\n", encoding="utf-8"
        )
        os.replace(staging, output_drivea)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise

    print(f"output driveA: {output_drivea}")
    for logical, payload in outputs.items():
        print(f"  {logical} size={len(payload)} sha256={sha256(payload)}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build or verify the private v1023 course lookup HOSTFS overlay"
    )
    parser.add_argument("--base-drivea", required=True, type=Path)
    parser.add_argument("--tools", required=True, type=Path)
    parser.add_argument("--bin", required=True, type=Path)
    parser.add_argument("--pic", required=True, type=Path)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--output-drivea", type=Path)
    mode.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()
    build(
        args.base_drivea,
        None if args.verify_only else args.output_drivea,
        args.tools,
        args.bin,
        args.pic,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
