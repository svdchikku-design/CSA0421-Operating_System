#!/bin/bash
# Run this inside your cloned repository AFTER you have re-run each task yourself.
# It makes one commit per task so that the commit history shows progressive work.
# Better: run the commit line for a task right after you finish that task, on the day you do it.
set -e
cd "$(dirname "$0")"
[ -d .git ] || git init
for t in Q1 Q2 Q3 Q4 Q5 Q6 Q7; do
  git add "$t"
  git commit -m "$t: code, outputs and notes" || true
  # (pause here in real use: do the next task, then commit it)
done
git add README.md build_all.sh commit_each_task.sh .gitignore 2>/dev/null || true
git commit -m "Add README and helper scripts" || true
