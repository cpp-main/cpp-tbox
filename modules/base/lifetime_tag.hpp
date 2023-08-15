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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_LIFETIME_TAG_H_20221215
#define TBOX_LIFETIME_TAG_H_20221215

namespace tbox {

/**
 * 生命期标签
 *
 * 在有些情况下，我们需要将一个对象的指针传给其它对象，使得其它对象可以随时对该指针所
 * 指向的对象进行访问。然而这存在一个风险：如果这个对象被提前析构了，而访问者并不知晓
 * 如果继续访问那将会出现不可预计的错误。
 * 为了解决这个问题，需要有一个标签，标识被标记的对象生命期是否有效。
 * 这就是本模块的设计初宗。
 *
 * LifetimeTag 为生命期监控对象。它可以监控其宿主的生命期是否存在。
 *
 * 示例如下：
 * struct HostObject {
 *   int value = 0;     //!< 其它成员
 *   LifetimeTag tag;   //!< 生命期标签
 * };
 *
 * HostObject *o = new HostObject;
 * LifetimeTag::Watcher w = o->tag;
 *
 * if (w)   //! 条件成立
 *   cout << "value:" << o->value << endl;
 * delete o;    //! 析构对象
 *
 * 实现原理：
 * 该模块借鉴了智能指针的引用计数原理。它定义了两个类：
 * - LifetimeTag，生命期标签
 * - LifetimeTag::Watcher，生命期标签观察器
 * 它们内部都有一个指针d_指向引用计数对象LifetimeTag::Detail
 * 当LifetimeTag被实例化的时候，就会new一个Detail对象。
 * 当LifetimeTag被析构的时候，会检查d_所指向的Detail对象中的watch_counter是否为0
 * 如果==0，则表示没有Watcher对该对象的生命期进行关注，即可释放Detail对象，否则不释放对
 * 象，仅驻对Detail中的alive标记为false，表示标签已不存在。
 * 当Watcher被实例化的时候，就会将d_所指向的Detail对象中的watch_counter自增1
 * 当Watcher被析构的时候，则与实例化过程相反，并且如果发现watch_counter减为了0，则顺带一
 * 同析构d_所指向的Detail对象。
 * 当使用者通过Watcher对LifetimeTag的生命期进行观察时，实质就是检查d_所指向Detail对象的
 * alive是否为true，该值就表示了LitetimeTag的生命期。
 *
 * 注意：
 * 目前该模块未做加锁保护，还不支持多线程。
 */
class LifetimeTag {
 private:
  struct Detail {
    bool  alive = true;
    int   watcher_counter = 0;
  };
  Detail *d_;

 public:
  class Watcher {
   public:
    ~Watcher() {
      if (d_ != nullptr) {
        --d_->watcher_counter;
        if (d_->watcher_counter == 0 && !d_->alive)
          delete d_;
      }
    }

    inline Watcher() { }
    inline Watcher(const LifetimeTag &tag) : d_(tag.d_) { ++d_->watcher_counter; }
    inline Watcher(const Watcher &other) : d_(other.d_) { ++d_->watcher_counter; }
    inline Watcher(Watcher &&other) : d_(other.d_) { other.d_ = nullptr; }

    Watcher& operator = (const LifetimeTag &tag) {
      reset();
      d_ = tag.d_;
      ++d_->watcher_counter;
      return *this;
    }

    Watcher& operator = (const Watcher &other) {
      if (this != &other) {
        reset();
        d_ = other.d_;
        ++d_->watcher_counter;
      }
      return *this;
    }

    Watcher& operator = (Watcher &&other) {
      if (this != &other) {
        reset();
        swap(other);
      }
      return *this;
    }

    inline void swap(Watcher &other) { std::swap(d_, other.d_); }
    inline void reset() { Watcher tmp; swap(tmp); }

    inline bool isNull() const { return d_ != nullptr; }
    inline bool isAlive() const { return (d_ != nullptr && d_->alive); }
    inline operator bool () const { return isAlive(); }

   private:
    Detail *d_ = nullptr;
  };

 public:
  ~LifetimeTag() {
    if (d_->watcher_counter == 0)
      delete d_;
    else
      d_->alive = false;
  }

  inline LifetimeTag() : d_(new Detail) { }
  inline LifetimeTag(const LifetimeTag &) : d_(new Detail) { }
  inline LifetimeTag(LifetimeTag &&) : d_(new Detail) { }

  inline LifetimeTag& operator = (const LifetimeTag &) { return *this; }
  inline LifetimeTag& operator = (LifetimeTag &&) { return *this; }

  inline Watcher get() const { return *this; }
};

}

#endif //TBOX_LIFETIME_TAG_H_20221215
