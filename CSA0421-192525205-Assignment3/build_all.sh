#!/bin/bash
# Compile every C program of Assignment 3 with the required flags.
set -e
cd "$(dirname "$0")"
FLAGS="-std=c11 -Wall -Wextra -pthread"
# Q1: libraries first, then the three versions of the program
( cd Q1
  gcc $FLAGS -fPIC -shared -o libenc.so encrypt_v1.c && cp libenc.so libenc_v1.so
  gcc $FLAGS -fPIC -shared -o libenc_v2.so encrypt_v2.c
  gcc $FLAGS -c encrypt_v1.c -o encrypt_v1.o && ar rcs libenc.a encrypt_v1.o && rm encrypt_v1.o
  gcc $FLAGS -o bank_static bank.c -static -L. -l:libenc.a
  gcc $FLAGS -o bank_dynamic bank.c -L. -lenc -Wl,-rpath,'$ORIGIN'
  gcc $FLAGS -o bank_dlopen bank_dlopen.c -ldl
  echo "compiled Q1 (libraries + bank_static, bank_dynamic, bank_dlopen)" )
for f in Q[2-7]/*.c; do
  gcc $FLAGS -O1 -o "${f%.c}" "$f"
  echo "compiled $f"
done
