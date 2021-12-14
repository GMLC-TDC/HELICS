#!/bin/sh
python -m sphinx -T -E -b html -d _build/doctrees -D language=en . _build/html
