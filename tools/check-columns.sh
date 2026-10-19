#!/bin/sh
# Fail if a line in a tracked file is longer than 80 columns, counting a tab
# as up to 8 columns, as an editor shows it. Markdown is not checked here:
# markdownlint does that (see .markdownlint-cli2.jsonc), because it knows
# which lines are table rows, which cannot be wrapped.
status=0
for f in $(git ls-files ':!:*.md'); do
    expand "$f" | awk -v f="$f" '
        length > 80 {
            printf "%s:%d: %d columns (max 80)\n", f, FNR, length
            bad = 1
        }
        END { exit bad }' || status=1
done
exit $status
