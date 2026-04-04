#!/usr/bin/env python3

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONFIG_PATH = ROOT / "subtrees.json"


def run(cmd):
    print(f"> {' '.join(cmd)}")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        sys.exit(result.returncode)


def run_capture(cmd):
    return subprocess.run(cmd, capture_output=True, text=True)


def ensure_git_repo():
    result = run_capture(["git", "rev-parse", "--is-inside-work-tree"])
    if result.returncode != 0 or "true" not in result.stdout:
        print("Error: not inside a git repository")
        sys.exit(1)


def load_config():
    if not CONFIG_PATH.exists():
        print("Error: subtrees.json not found at repo root")
        sys.exit(1)

    with open(CONFIG_PATH, "r", encoding="utf-8") as f:
        data = json.load(f)

    if "subtrees" not in data:
        print("Error: invalid config file (missing 'subtrees')")
        sys.exit(1)

    return data["subtrees"]


def has_changes_in_prefix(prefix: str) -> bool:
    result = run_capture(["git", "status", "--porcelain", "--", prefix])
    return bool(result.stdout.strip())


def subtree_add(st):
    if has_changes_in_prefix(st["prefix"]):
        print(
            f"Error: subtree '{st['name']}' has local changes. Commit or stash them first."
        )
        sys.exit(1)

    run(
        [
            "git",
            "subtree",
            "add",
            "--prefix",
            st["prefix"],
            st["url"],
            st["branch"],
            "--squash",
        ]
    )


def subtree_pull(st):
    if has_changes_in_prefix(st["prefix"]):
        print(
            f"Error: subtree '{st['name']}' has local changes. Commit or stash them first."
        )
        sys.exit(1)

    run(
        [
            "git",
            "subtree",
            "pull",
            "--prefix",
            st["prefix"],
            st["url"],
            st["branch"],
            "--squash",
        ]
    )


def subtree_push(st):
    if has_changes_in_prefix(st["prefix"]):
        print(
            f"Error: subtree '{st['name']}' has local changes. Commit or stash them first."
        )
        sys.exit(1)

    run(["git", "subtree", "push", "--prefix", st["prefix"], st["url"], st["branch"]])


def list_subtrees(subtrees):
    print("Configured subtrees:")
    for st in subtrees:
        print(f"- {st['name']} ({st['prefix']}) → {st['url']} [{st['branch']}]")
    print("")


def run_for_all(subtrees, action):
    for st in subtrees:
        print(f"\n-> {action.__name__.replace('subtree_', '')} {st['name']}...")
        action(st)


def main():
    ensure_git_repo()

    if len(sys.argv) < 2:
        print("Usage: python scripts/subtree.py [list|add|pull|push]")
        sys.exit(1)

    cmd = sys.argv[1]
    subtrees = load_config()

    if cmd == "list":
        list_subtrees(subtrees)
        return

    if cmd == "add":
        run_for_all(subtrees, subtree_add)
    elif cmd == "pull":
        run_for_all(subtrees, subtree_pull)
    elif cmd == "push":
        run_for_all(subtrees, subtree_push)
    else:
        print("Unknown command")
        sys.exit(1)


if __name__ == "__main__":
    main()
