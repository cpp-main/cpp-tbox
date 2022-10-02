#include "event_publisher_impl.h"
#include <algorithm>

namespace tbox {
namespace action {

void EventPublisherImpl::subscribe(EventSubscriber *subscriber) {
  auto iter_end = subscriber_vec_.end();
  auto iter = std::remove(subscriber_vec_.begin(), iter_end, subscriber);
  if (iter == iter_end) {
    subscriber_vec_.push_back(subscriber);
  }
}

void EventPublisherImpl::unsubscribe(EventSubscriber *subscriber) {
  auto iter_end = subscriber_vec_.end();
  auto iter = std::remove(subscriber_vec_.begin(), iter_end, subscriber);
  subscriber_vec_.erase(iter, iter_end);
}

void EventPublisherImpl::onEvent(int event_id, void *event_data) {
  auto todo_vec = subscriber_vec_;
  while (!todo_vec.empty()) {
    auto top_subscriber = todo_vec.back();
    if (top_subscriber->onEvent(event_id, event_data))
      break;
    todo_vec.pop_back();
  }
}

}
}
