"""Result-output helpers for Python priority-queue tests."""

import csv
import json
from pathlib import Path
from typing import Any, Dict, Iterable


def write_summary_json(path: Path, summary: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )


def write_push_trace_csv(path: Path, rows: Iterable[Dict[str, Any]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "run",
        "implementation",
        "source",
        "sequence",
        "node",
        "distance",
        "predecessor",
        "predicted_node_rank",
        "predicted_key_rank",
        "extracted",
        "stale",
        "extraction_rank",
    ]
    with path.open("w", encoding="utf-8", newline="") as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)
