/**
 * 异步数据流
 *
 * 在日志输出到文件的应用场景，如果每写一条日志都保存到文件，而写文件是非常耗时的
 * 阻塞性操作。如果不想阻塞，那写文件的操作必须要交给特定的线程去处理。
 * 本类就用于满足该场景的，使用方法：
 *
 * AsyncDataStream ads;
 * AsyncDataStream::Config cfg;
 * cfg.sync_cb = \
 *     [](const void *data_ptr, size_t data_size) {
 *         //! 由子线程调用，保存数据到文件的操作
 *     };
 * ads.initialize(cfg);
 * ads 对象会创建子线程，负责调用cfg.sync_cb所指回调函数。
 *
 * 当有数据要保存的时候，只需要：
 * ads.asyncWrite(...);
 *
 * 在以下两种情况下 ads 会回调函数：
 * 1）缓冲写满；2）距上次同步数据超过cfg.sync_interval秒数
 *
 * 当对象被销毁或cleanup()时，会停止后台的线程，并将所有缓冲的数据同步调用预设置的回调
 */
#ifndef TBOX_ASYNC_DATA_STREAM_H_20211219
#define TBOX_ASYNC_DATA_STREAM_H_20211219

#include <functional>

namespace tbox {
namespace util {

class AsyncDataStream {
  public:
    AsyncDataStream();
    ~AsyncDataStream();

  public:
    using SyncCallback = std::function<void(const void *, size_t)>;
    struct Config {
        size_t buff_size = 1024;    //!< 缓冲大小，默认1KB
        int min_buff_num = 2;       //!< 最小缓冲数，默认2
        int max_buff_num = 0;       //!< 最大缓冲数，默认0，表示不限制
        int sync_interval = 3;      //!< 同步间隔，默认3秒

        SyncCallback sync_cb;       //!< 同步回调
    };

    bool initialize(const Config &cfg); //! 初始化
    void cleanup(); //! 清理
    void asyncWrite(const void *data_ptr, size_t data_size); //! 异步写入

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}
}

#endif //TBOX_ASYNC_DATA_STREAM_H_20211219
