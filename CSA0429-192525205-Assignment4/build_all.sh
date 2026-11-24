#!/bin/bash
# Compile every C program of Assignment 4 with the required flags.
set -e
cd "$(dirname "$0")"
FLAGS="-std=c11 -Wall -Wextra"
for f in Task*/*.c; do
  case "$f" in */stream_sim.c|*/disk_sched.c) L=-lm;; *) L=;; esac
  gcc $FLAGS -O1 -o "${f%.c}" "$f" $L
  echo "compiled $f"
done
