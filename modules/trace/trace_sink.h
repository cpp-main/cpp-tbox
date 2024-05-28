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
#ifndef TBOX_TRACE_TRACE_SINK_H_20240525
#define TBOX_TRACE_TRACE_SINK_H_20240525

#include <atomic>
#include <string>
#include <map>
#include <limits>

#include <tbox/util/async_pipe.h>
#include <tbox/util/buffer.h>

namespace tbox {
namespace trace {

class TrackSink {

  public:
    static TrackSink& GetInstance();

    /**
     * 设置路径前缀
     *
     * 比如：/data/my_proc，模块会自动创建目录 /some/where/my_proc.<pid>/ 目录，并在该目录下
     *       创建对应的文件:
     *       - name_list.txt
     *       - thread_list.txt
     *       - record_20240525_123300.bin
     *       - record_20240525_151133.bin
     */
    void setPathPrefix(const std::string &path_prefix);

    //! 设置记录文件的大小的上限
    void setRecordFileMaxSize(size_t max_size) { record_file_max_size_ = max_size; }

    bool enable();  //! 使能
    void disable(); //! 停止

    /**
     * \brief 提交记录
     *
     * \param name          名称，通常指函数名或事件名，或函数名+文件名+行号
     * \param end_ts        记录结束的时间点，单位: us
     * \param duration_us   记录持续时长，单位:us
     */
    void commitRecord(const char *name, uint64_t end_timepoint_us, uint64_t duration_us);

  protected:
    TrackSink();
    ~TrackSink();

    struct RecordHeader {
        long thread_id;
        uint64_t end_ts_us;
        uint64_t duration_us;
        size_t name_size;
    };

    using Index = uint64_t;

    void onBackendRecvData(const void *data, size_t size);
    void onBackendRecvRecord(const RecordHeader &record, const char *name);

    bool checkAndCreateRecordFile();

    Index allocNameIndex(const std::string &name);
    Index allocThreadIndex(long thread_id);

  private:
    std::string path_;
    size_t record_file_max_size_ = std::numeric_limits<size_t>::max();
    std::string name_list_filename_;
    std::string thread_list_filename_;

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
    //! 线程号编码
    std::map<long, Index> thread_to_index_map_;
    int next_thread_index_ = 0;
};

}
}

#endif //TBOX_TRACE_TRACE_SINK_H_20240525
