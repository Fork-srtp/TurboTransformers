#!/bin/bash


set -e
# onnxrt does not work well for albert
FRAMEWORKS=("turbo-transformers" "torch")
# pip install onnxruntime-gpu before benchmarking onnxrt
# FRAMEWORKS=("onnxruntime")
SEQ_LEN=(10 20 40 60 80 100 200 300 400 500)
BATCH_SIZE=(1 20)

N=150
MODELS=("bert" "albert")
for model in ${MODELS[*]}
do
for batch_size in ${BATCH_SIZE[*]}
do
  for seq_len in ${SEQ_LEN[*]}
  do
    for framework in ${FRAMEWORKS[*]}
    do
      python benchmark.py ${model} --seq_len=${seq_len} --batch_size=${batch_size}\
          -n ${N} --framework=${framework} --use_gpu
    done
  done
done
done

USE_NVPROF="NO"
if [ $USE_NVPROF == "YES" ]; then
N=150
for batch_size in ${BATCH_SIZE[*]}
do
  for seq_len in ${SEQ_LEN[*]}
  do
    for framework in ${FRAMEWORKS[*]}
    do
       nvprof -f -o profile_dir/bert_${framework}_${batch_size}_${seq_len}.nvvp python gpu_benchmark.py ${MODEL} --seq_len=${seq_len} --batch_size=${batch_size}\
          -n ${N} --framework=${framework}
    done
  done
done
fi
