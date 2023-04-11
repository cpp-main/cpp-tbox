#ifndef TBOX_EVENT_ITEM_H_20170627
#define TBOX_EVENT_ITEM_H_20170627

#include <string>

namespace tbox {
namespace event {

class Loop;

class Event {
  public:
    Event(const std::string &what) : what_(what) { }

    enum class Mode {
        kPersist,
        kOneshot
    };

    virtual bool isEnabled() const = 0;
    virtual bool enable() = 0;
    virtual bool disable() = 0;

    virtual Loop* getLoop() const = 0;

    std::string what() const { return what_; }

  public:
    virtual ~Event() { }

  protected:
    std::string what_;
};

}
}

#endif //TBOX_EVENT_ITEM_H_20170627
