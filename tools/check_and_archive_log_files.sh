#!/bin/bash

#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#  //  E A S Y  /  \/ \
# ++ ----------.  \/\  .
#  \\     \     \ /\  /
#   \\     \     \   /
#    \\     \     \ /
#     -============'
#
# Copyright (c) 2025 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

# 打包日志文件脚本

if [ $# -lt 2 ]; then
  echo "Usage: $0 <log_path> <log_prefix> [log_file_num] [zip_file_num]"
  echo "Exp  : $0 /var/log/ sample"
  echo "Exp  : $0 /var/log/ sample 5 10"
  exit
fi

log_path=$1
log_prefix=$2

log_file_num=5
zip_file_num=10

[ $# -ge 3 ] && log_file_num=$3
[ $# -ge 4 ] && zip_file_num=$4

if [ ! -d $log_path ]; then
  echo "Error: path '$log_path' not exist"
  exit
fi

cd $log_path

log_files=(`ls -t ${log_prefix}.*.log*`)
echo ${log_files[*]}

if [ ${#log_files[*]} -gt ${log_file_num} ]; then
  unset log_files[0]

  zip_filename=${log_prefix}.log.1.zip
  if [ -f ${zip_filename} ]; then
    zip_files=(`ls ${log_prefix}.log.*.zip`)
    index=${#zip_files[*]}
    while [ ${index} -ge 1 ]; do
      if [ ${index} -ge ${zip_file_num} ]; then
        rm ${log_prefix}.log.${index}.zip
      else
        mv ${log_prefix}.log.${index}.zip ${log_prefix}.log.$((index+1)).zip 
      fi
      index=$((index-1))
    done
  fi
  zip ${zip_filename} ${log_files[*]}
  rm ${log_files[*]}
  echo "Info: done"
else
  echo "Info: skip"
fi
