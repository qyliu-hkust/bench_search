#! /usr/bin/env bash

echo "Generate normal and lognormal data..."
python ./gen_normal.py

echo "Generate uniform data..."
python ./gen_uniform.py --sparse --uint64 --many
