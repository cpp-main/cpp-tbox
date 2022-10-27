#include "event_publisher_impl.h"

#include <algorithm>
#include "event_subscriber.h"

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
  {
    auto iter_end = subscriber_vec_.end();
    auto iter = std::remove(subscriber_vec_.begin(), iter_end, subscriber);
    subscriber_vec_.erase(iter, iter_end);
  }
  {
    if (!tmp_vec_.empty()) {
      auto iter_end = tmp_vec_.end();
      auto iter = std::remove(tmp_vec_.begin(), iter_end, subscriber);
      tmp_vec_.erase(iter, iter_end);
    }
  }
}

void EventPublisherImpl::publish(Event event) {
  auto tmp_vec_ = subscriber_vec_;
  while (!tmp_vec_.empty()) {
    auto top_subscriber = tmp_vec_.back();
    if (top_subscriber->onEvent(event))
      break;
    tmp_vec_.pop_back();
  }
}

}
}
