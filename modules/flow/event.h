#ifndef TBOX_FLOW_EVENT_H_20221022
#define TBOX_FLOW_EVENT_H_20221022

namespace tbox {
namespace flow {

struct Event {
  using ID = int;

  ID id = 0;
  const void *extra = nullptr;

  Event() { }

  template <typename ET>
    Event(ET e) : id(static_cast<ID>(e)) { }

  template <typename ET, typename DT>
    Event(ET e, DT *p) : id(static_cast<ID>(e)), extra(p) { }
};

}
}

#endif //TBOX_FLOW_EVENT_H_20221022
