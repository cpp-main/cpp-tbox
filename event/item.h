#ifndef TBOX_EVENT_ITEM_H_20170627
#define TBOX_EVENT_ITEM_H_20170627

namespace tbox {
namespace event {

class Item {
  public:
    enum class Mode {
        kPersist,
        kOneshot
    };

    virtual bool isEnabled() const = 0;
    virtual bool enable() = 0;
    virtual bool disable() = 0;

  public:
    virtual ~Item() { }
};

}
}

#endif //TBOX_EVENT_ITEM_H_20170627
