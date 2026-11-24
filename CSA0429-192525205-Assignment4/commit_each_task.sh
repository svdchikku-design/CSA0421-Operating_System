#!/bin/bash
# Run inside your cloned repository AFTER you have re-run each task yourself.
# One commit per task. Better: commit each task on the day you finish it.
set -e
cd "$(dirname "$0")"
[ -d .git ] || git init
for t in Task1 Task2 Task3 Task4 Task5 Task6; do
  git add "$t"; git commit -m "$t: code, outputs and notes" || true
done
git add README.md build_all.sh commit_each_task.sh .gitignore figures Report_CO4.docx 2>/dev/null || true
git commit -m "Add README, figures, report and helper scripts" || true
