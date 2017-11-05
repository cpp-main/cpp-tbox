#ifndef TBOX_EVENT_ITEM_H_20170627
#define TBOX_EVENT_ITEM_H_20170627

namespace tbox {
namespace event {

class Event {
  public:
    enum class Mode {
        kPersist,
        kOneshot
    };

    virtual bool isEnabled() const = 0;
    virtual bool enable() = 0;
    virtual bool disable() = 0;

  public:
    virtual ~Event() { }
};

}
}

#endif //TBOX_EVENT_ITEM_H_20170627
