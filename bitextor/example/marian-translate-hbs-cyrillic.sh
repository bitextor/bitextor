#!/usr/bin/env bash
cyrtranslit -l sr | /data1/lpla/marian-dev/build/marian-decoder -c /data1/students/shen.student.tiny11/config.intgemm8bit.alphas.yml --quiet --cpu-threads 1
