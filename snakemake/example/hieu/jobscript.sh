#!/bin/sh
export GIT_PYTHON_REFRESH=quiet
export PATH=$PATH:/lustre/home/dc007/mespla/miniconda3/bin/:/lustre/home/dc007/mespla/miniconda3
source activate bitextor
export LD_LIBRARY_PATH=/lustre/sw/gcc/6.2.0/lib64
export LANG=en_US.UTF-8
# properties = {properties}
{exec_job} 
# 1> /lustre/home/dc007/hhoang/workspace/temp/$$.tmp
