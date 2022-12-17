#ifndef TBOX_LIFETIME_TAG_H_20221215
#define TBOX_LIFETIME_TAG_H_20221215

namespace tbox {

/**
 * 生命期标签
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
