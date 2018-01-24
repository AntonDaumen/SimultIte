#!/bin/bash
#@ class            = clgpu
#@ job_name         = SimultIte
#@ node_usage       = not_shared
#@ wall_clock_limit = 00:05:00
#@ output           = $(job_name).$(jobid).log
#@ error            = $(job_name).$(jobid).err
#@ job_type         = serial
#@ environment      = COPY_ALL
#@ queue

module load gnu-env/5.4.0
module load cuda

./build/SimultIte -i test.mtx -n 5
