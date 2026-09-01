#!/usr/bin/env bash
# Install this repository as a Claude Code skill by symlinking it into the
# skills directory. The five SKILL.md files below then surface as /systemc,
# /sysc-lt, /sysc-at, /sysc-ca and /sysc-verify after a restart.
#
#   ./install.sh              user-wide, into ~/.claude/skills/systemc
#   ./install.sh --project    project-scoped, into ./.claude/skills/systemc
#   ./install.sh --uninstall  remove the link (either location)

set -euo pipefail

src=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)
root="$HOME/.claude/skills"
uninstall=0

for arg in "$@"; do
    case "$arg" in
        --project)   root=".claude/skills" ;;
        --uninstall) uninstall=1 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done
dest="$root/systemc"

if (( uninstall )); then
    if [[ -L $dest ]]; then
        rm "$dest"
        echo "removed $dest"
    else
        echo "nothing to remove at $dest" >&2
    fi
    exit 0
fi

[[ -f $src/SKILL.md ]] || { echo "$src is not the SKILL-SYSC repository" >&2; exit 1; }

if [[ -e $dest || -L $dest ]]; then
    if [[ -L $dest && $(readlink -f "$dest") == "$src" ]]; then
        echo "already installed: $dest -> $src"
        exit 0
    fi
    echo "$dest already exists and is not a link to $src. Remove it first." >&2
    exit 1
fi

mkdir -p "$root"
ln -s "$src" "$dest"
echo "installed: $dest -> $src"

# The names come from each SKILL.md's frontmatter, not from the directory.
echo
echo "Restart Claude Code, then check that these are offered:"
grep -h '^name:' "$src/SKILL.md" "$src"/sysc-*/SKILL.md | sed 's/^name: /  \//'
