#!/bin/bash
# Run inside your cloned repository AFTER you have re-run each task yourself; one commit per task.
set -e
cd "$(dirname "$0")"
[ -d .git ] || git init
for t in B1 B2 B3 B4; do git add "$t"; git commit -m "$t: files, outputs and notes" || true; done
git add README.md commit_each_task.sh .gitignore figures Report_CO5.docx 2>/dev/null || true
git commit -m "Add README, figures and report" || true
