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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_HTTP_CLIENT_RESPOND_PARSER_H_20260614
#define TBOX_HTTP_CLIENT_RESPOND_PARSER_H_20260614

#include "../respond.h"

namespace tbox {
namespace http {
namespace client {

//! 响应解析器
class RespondParser {
  public:
    //! 状态
    enum class State {
        kInit,              //!< 初始化，未开始
        kFinishedStartLine, //!< 完成了首行解析
        kFinishedHeads,     //!< 完成 heads 的解析
        kFinishedAll,       //!< 完成了整个 HTTP 响应的解析
        kFail,              //!< 解析出错
    };

    ~RespondParser();

    /**
     * \brief   解析
     * \param   data_ptr    数据地址
     * \param   data_size   数据大小
     * \return  size_t      已处理数据大小
     */
    size_t parse(const void *data_ptr, size_t data_size);

    //! 获取状态
    State state() const { return state_; }

    /**
     * \brief   取走 Respond 对象
     * \return  Respond*    响应对象
     * \note    只有 state 为 kFinishedAll 才会返回真实的对象，否则都是返回 nullptr
     *          一旦 Respond 对象被取走，RespondParser 则不再管辖被取走对象的生命期
     *          交由用户自己管理
     */
    Respond* getRespond();

    //! 重置
    void reset();

  private:
    State state_ = State::kInit;
    Respond *sp_respond_ = nullptr;
    size_t content_length_ = 0;
};

}
}
}

#endif //TBOX_HTTP_CLIENT_RESPOND_PARSER_H_20260614
