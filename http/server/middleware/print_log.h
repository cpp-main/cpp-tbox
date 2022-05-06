#include "../middleware.h"

namespace tbox {
namespace http {
namespace server {

class PrintLog : public Middleware {
  public:
    PrintLog(int level);

  public:
    virtual void handle(ContextSptr sp_ctx, const NextFunc &next) override;

  private:
    int level_ = 0;
};

}
}
}
