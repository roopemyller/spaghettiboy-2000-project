from __future__ import annotations

import argparse
import re
from pathlib import Path


TONE_RE = re.compile(r"\btone\s*\(\s*[^,]+,\s*([^,]+?)\s*,\s*([^)]+?)\s*\)")
DELAY_RE = re.compile(r"\bdelay\s*\(\s*([^)]+?)\s*\)")


def parse_tokens(source: str) -> list[object]:
    out: list[object] = []

    for line in source.splitlines():
        # strip simple line comments
        line = line.split("//", 1)[0]

        m_delay = DELAY_RE.search(line)
        if m_delay:
            delay_arg = m_delay.group(1)
            if "longDelayTime" in delay_arg:
                out.append("delay")

        m_tone = TONE_RE.search(line)
        if m_tone:
            raw_freq = m_tone.group(1).strip()
            raw_dur = m_tone.group(2).strip()
            try:
                freq: object = int(raw_freq)
            except ValueError:
                freq = raw_freq

            if "longDur" in raw_dur:
                out.append(f"L{freq}")
            else:
                out.append(freq)

    return out


def write_python_list(path: Path, tokens: list[object]) -> None:
    path.write_text(
        "tones = " + repr(tokens) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--input",
        default=str(Path(__file__).with_name("pitches.h")),
        help="Path to pitches.h",
    )
    parser.add_argument(
        "--output",
        default=str(Path(__file__).with_name("tones_list.py")),
        help="Output python file containing list",
    )
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)

    tokens = parse_tokens(input_path.read_text(encoding="utf-8", errors="replace"))
    write_python_list(output_path, tokens)

    print(f"Wrote {len(tokens)} tokens to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

