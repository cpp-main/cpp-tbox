/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <iostream>
#include <limits>

#include <tbox/util/fs.h>
#include <tbox/util/string.h>
#include <tbox/util/buffer.h>
#include <tbox/util/scalable_integer.h>

#include "writer.h"

using namespace std;
using namespace tbox;

//! 统计
struct Stat {
    size_t    times = 0;            //! 次数
    uint64_t  dur_acc_us = 0;       //! 累积时长
    uint64_t  dur_min_us = std::numeric_limits<uint64_t>::max(); //! 最小时长
    uint64_t  dur_max_us = 0;       //! 最大时长
    uint64_t  dur_max_ts_us = 0;    //! 最大时长的时间点

    uint64_t  dur_avg_us = 0;       //! 平均时长
    uint64_t  dur_warn_line_us = 0; //! 警告水位线

    uint64_t  dur_warn_count = 0;   //! 超过警告水位线次数
};

using StringVec = std::vector<std::string>;
using StatVec = std::vector<Stat>;

//! start_time_us, duration_us, name_index, module_index, thread_index
using RecordHandleFunc = std::function<void(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t)>;

void PrintUsage(const char *proc_name)
{
    std::cout
      << "Usage: " << proc_name << " <dir_path> [output_filename]" << std::endl
      << "Exp  : " << proc_name << " /some/where/my_proc.20240531_032237.114" << std::endl
      << "       " << proc_name << " /some/where/my_proc.20240531_032237.114 output.json" << std::endl;
}

bool PickRecord(util::Buffer &buffer, uint64_t &end_diff_us, uint64_t &duration_us,
                uint64_t &thread_index, uint64_t &name_index, uint64_t &module_index)
{
    uint8_t *buffer_begin = buffer.readableBegin();
    size_t   buffer_size  = buffer.readableSize();

    size_t  parse_size = 0;
    size_t  data_size = 0;

    parse_size = util::ParseScalableInteger((buffer_begin + data_size), (buffer_size - data_size), end_diff_us);
    if (parse_size == 0)
        return false;
    data_size += parse_size;

    parse_size = util::ParseScalableInteger((buffer_begin + data_size), (buffer_size - data_size), duration_us);
    if (parse_size == 0)
        return false;
    data_size += parse_size;

    parse_size = util::ParseScalableInteger((buffer_begin + data_size), (buffer_size - data_size), thread_index);
    if (parse_size == 0)
        return false;
    data_size += parse_size;

    parse_size = util::ParseScalableInteger((buffer_begin + data_size), (buffer_size - data_size), name_index);
    if (parse_size == 0)
        return false;
    data_size += parse_size;

    parse_size = util::ParseScalableInteger((buffer_begin + data_size), (buffer_size - data_size), module_index);
    if (parse_size == 0)
        return false;
    data_size += parse_size;

    buffer.hasRead(data_size);
    return true;
}

void ReadRecordFile(const std::string &filename, const RecordHandleFunc &func)
{
    std::ifstream ifs(filename, std::ifstream::binary);
    if (!ifs) {
        std::cerr << "read '" << filename << "' fail!" << std::endl;
        return;
    }

    uint64_t last_end_ts_us = 0;
    util::Buffer buffer;

    while (true) {
        char tmp[1024];
        auto rsize = ifs.readsome(tmp, sizeof(tmp));
        if (rsize == 0)
            break;

        buffer.append(tmp, rsize);

        while (buffer.readableSize() >= 4) {
            uint64_t end_diff_us, duration_us, thread_index, name_index, module_index;
            if (!PickRecord(buffer, end_diff_us, duration_us, thread_index, name_index, module_index))
                break;

            uint64_t end_ts_us = last_end_ts_us + end_diff_us;
            uint64_t start_ts_us = end_ts_us - duration_us;
            last_end_ts_us = end_ts_us;

            func(start_ts_us, duration_us, name_index, module_index, thread_index);
        }
    }
}

void ReadAllRecordFiles(const std::string &records_dir, const StringVec &record_file_vec, const RecordHandleFunc &func)
{
    for (auto record_file : record_file_vec)
        ReadRecordFile(records_dir + '/' + record_file, func);
}

void DumpStatToFile(const StringVec &name_vec, const StatVec &stat_vec, const std::string &stat_filename)
{
    std::ofstream ofs(stat_filename);
    if (!ofs) {
        std::cout << "Error: open stat file '" << stat_filename << "' fail!" << std::endl;
        return;
    }

    auto size = name_vec.size();
    for (size_t i = 0; i < size; ++i) {
        auto &name = name_vec.at(i);
        auto &stat = stat_vec.at(i);

        ofs << std::string(name.size(), '=') << std::endl
            << name << std::endl
            << std::string(name.size(), '-') << std::endl
            << "times            : " << stat.times << std::endl
            << "dur_min_us       : " << stat.dur_min_us << " us" << std::endl
            << "dur_avg_us       : " << stat.dur_avg_us << " us" << std::endl
            << "dur_max_us       : " << stat.dur_max_us << " us" << std::endl
            << "dur_max_at_us    : " << stat.dur_max_ts_us << " us" << std::endl
            << "dur_warn_line_us : " << stat.dur_warn_line_us << " us" << std::endl
            << "dur_warn_count   : " << stat.dur_warn_count << std::endl
            << std::endl;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    std::string dir_path = argv[1];
    if (!util::fs::IsDirectoryExist(dir_path)) {
        std::cout << "Error: dir_path '" << dir_path << "' not exist!" << std::endl;
        return 0;
    }

    std::string output_filename = dir_path + "/output.json";
    if (argc > 2)
      output_filename = argv[2];

    trace::Writer writer;
    if (!writer.open(output_filename)) {
        std::cout << "Error: output_filename '" << output_filename << "' can't be create!" << std::endl;
        return 0;
    }

    //! 从 threads.txt, names.txt, modules.txt 文件中导入数据
    std::string names_filename = dir_path + "/names.txt";
    std::string modules_filename = dir_path + "/modules.txt";
    std::string threads_filename = dir_path + "/threads.txt";

    StringVec name_vec, module_vec, thread_vec;
    if (!util::fs::ReadAllLinesFromTextFile(names_filename, name_vec))
        std::cerr << "Warn: load names.txt fail!" << std::endl;
    if (!util::fs::ReadAllLinesFromTextFile(modules_filename, module_vec))
        std::cerr << "Warn: load modules.txt fail!" << std::endl;
    if (!util::fs::ReadAllLinesFromTextFile(threads_filename, thread_vec))
        std::cerr << "Warn: load threads.txt fail!" << std::endl;

    std::string records_dir = dir_path + "/records";
    StringVec record_file_name_vec;
    if (!util::fs::ListDirectory(records_dir, record_file_name_vec)) {
      std::cerr << "Err: list '" << records_dir << "' fail!" << std::endl;
      return 0;
    }

    std::vector<Stat> stat_vec(name_vec.size());

    writer.writeHeader();

    //! 第一次遍历记录文件
    ReadAllRecordFiles(records_dir, record_file_name_vec,
        [&] (uint64_t start_ts_us, uint64_t duration_us, uint64_t name_index, uint64_t module_index, uint64_t thread_index) {
            std::string name = "unknown-name", thread = "unknown-thread", module = "unknown-module";

            if (name_index < name_vec.size())
                name = name_vec[name_index];

            if (thread_index < thread_vec.size())
                thread = thread_vec[thread_index];

            if (module_index < module_vec.size())
                module = module_vec[module_index];

            writer.writeRecorder(name, module, thread, start_ts_us, duration_us);

            auto &stat = stat_vec.at(name_index);
            ++stat.times;
            stat.dur_acc_us += duration_us;
            if (stat.dur_max_us < duration_us) {
                stat.dur_max_us = duration_us;
                stat.dur_max_ts_us = start_ts_us;
            }

            if (stat.dur_min_us > duration_us) {
                stat.dur_min_us = duration_us;
            }
        }
    );

    //! 处理统计数据
    for (auto &stat : stat_vec) {
        stat.dur_avg_us = stat.dur_acc_us / stat.times;
        stat.dur_warn_line_us = (stat.dur_avg_us + stat.dur_max_us) / 2;
    }

    //! 第二次遍历记录文件，标出超出警告线的
    ReadAllRecordFiles(records_dir, record_file_name_vec,
        [&] (uint64_t start_ts_us, uint64_t duration_us, uint64_t name_index, uint64_t module_index, uint64_t) {
            auto &stat = stat_vec.at(name_index);
            if (duration_us < stat.dur_warn_line_us)
                return;

            std::string name = "unknown-name", module = "unknown-module";

            if (name_index < name_vec.size())
                name = name_vec[name_index];

            if (module_index < module_vec.size())
                module = module_vec[module_index];

            ++stat.dur_warn_count;
            writer.writeRecorder(name, module, "WARN", start_ts_us, duration_us);
        }
    );

    //! 标记出最大时间点
    auto size = name_vec.size();
    for (size_t i = 0; i < size; ++i) {
        auto &name = name_vec.at(i);
        auto &stat = stat_vec.at(i);
        writer.writeRecorder(name, "", "MAX", stat.dur_max_ts_us, stat.dur_max_us);
    }

    writer.writeFooter();

    //! 输出统计到 stat.txt
    std::string stat_filename = dir_path + "/stat.txt";
    DumpStatToFile(name_vec, stat_vec, stat_filename);
    return 0;
}

