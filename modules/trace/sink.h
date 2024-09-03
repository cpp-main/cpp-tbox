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
#ifndef TBOX_TRACE_SINK_H_20240525
#define TBOX_TRACE_SINK_H_20240525

#include <atomic>
#include <string>
#include <map>
#include <limits>
#include <mutex>
#include <set>
#include <vector>

#include <tbox/util/async_pipe.h>
#include <tbox/util/buffer.h>

namespace tbox {
namespace trace {

class Sink {

  public:
    static Sink& GetInstance();

    /**
     * 设置路径前缀
     *
     * 比如：/data/my_proc，模块会自动创建目录 /data/my_proc.20240525_123300.7723/ 目录
     *       其名称中 "20240525_123300" 为时间戳，"7723" 为进程号。
     *       目录结构:
     *       .
     *       |-- names.txt    # 函数名列表
     *       |-- modules.txt  # 模块名列表
     *       |-- threads.txt  # 线程名列表
     *       `-- records      # 记录文件目录，其下存在一个或多个记录文件
     *           `-- 20240530_041046.bin
     */
    bool setPathPrefix(const std::string &path_prefix);

    //! 设置是否开启实时落盘
    void setFileSyncEnable(bool is_enable);
    bool isFileSyncEnabled() const { return is_file_sync_enabled_; }

    //! 获取目录路径
    std::string getDirPath() const { return dir_path_; }
    //! 获取当前记录文件名
    std::string getCurrRecordFilename() const { return curr_record_filename_; }

    //! 设置记录文件的大小的上限
    void setRecordFileMaxSize(size_t max_size) { record_file_max_size_ = max_size; }
    size_t getRecordFileMaxSize() const { return record_file_max_size_; }

    bool enable();  //! 使能
    void disable(); //! 停止

    bool isEnabled() const { return is_enabled_; }

    //! 过滤策略
    enum class FilterStrategy {
        kPermit,  //! 允许
        kReject,  //! 拒绝
    };
    using ExemptSet = std::set<std::string>;
    //! 设置与获取过滤策略，默认允许或是拒绝
    void setFilterStrategy(FilterStrategy strategy) { filter_strategy_ = strategy; }
    FilterStrategy getFilterStrategy() const { return filter_strategy_; }
    //! 设置与获取豁免集合
    void setFilterExemptSet(const ExemptSet &exempt_set);
    ExemptSet getFilterExemptSet() const;

    /**
     * \brief 提交记录
     *
     * \param name          名称，通常指函数名或事件名，或函数名+文件名+行号
     * \param line          行号
     * \param end_ts        记录结束的时间点，单位: us
     * \param duration_us   记录持续时长，单位:us
     */
    void commitRecord(const char *name, const char *module, uint32_t line, uint64_t end_timepoint_us, uint64_t duration_us);

  protected:
    ~Sink();

    struct RecordHeader {
        long thread_id;
        uint64_t end_ts_us;
        uint64_t duration_us;
        uint32_t line;
        size_t name_size;
        size_t module_size;
    };

    using Index = uint64_t;

    void onBackendRecvData(const void *data, size_t size);
    void onBackendRecvRecord(const RecordHeader &record, const char *name, const char *module,
                             std::vector<uint8_t> &write_cache);

    bool checkAndWriteNames();
    bool checkAndWriteModules();
    bool checkAndWriteThreads();
    bool checkAndCreateRecordFile();

    bool isFilterPassed(const std::string &module) const;

    Index allocNameIndex(const std::string &name, uint32_t line);
    Index allocModuleIndex(const std::string &module);
    Index allocThreadIndex(long thread_id);

  private:
    std::string dir_path_;
    size_t record_file_max_size_ = std::numeric_limits<size_t>::max();
    std::string name_list_filename_;
    std::string module_list_filename_;
    std::string thread_list_filename_;
    bool is_file_sync_enabled_ = false;

    std::atomic_bool is_enabled_{false};

    util::AsyncPipe async_pipe_;

    //! 下面的成员变量，由后端线程读写
    util::Buffer buffer_;
    std::string curr_record_filename_;  //! 当前记录文件的全名
    int curr_record_fd_ = -1;       //! 当前记录文件描述符
    size_t total_write_size_ = 0;   //! 当前记录文件已写入数据量
    uint64_t last_timepoint_us_ = 0; //! 当前记录文件的上一条记录的时间戳(us)

    //! 名称编码
    std::map<std::string, Index> name_to_index_map_;
    uint32_t next_name_index_ = 0;
    //! 模块编码
    std::map<std::string, Index> module_to_index_map_;
    uint32_t next_module_index_ = 0;
    //! 线程号编码
    std::map<long, Index> thread_to_index_map_;
    int next_thread_index_ = 0;

    //! 过滤相关变量
    mutable std::mutex lock_;
    FilterStrategy filter_strategy_ = FilterStrategy::kPermit;  //! 默认允许
    ExemptSet filter_exempt_set_;
};

}
}

#endif //TBOX_TRACE_SINK_H_20240525
