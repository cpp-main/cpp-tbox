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
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_HTTP_REQUEST_PARSER_H_20220502
#define TBOX_HTTP_REQUEST_PARSER_H_20220502

#include "../request.h"

namespace tbox {
namespace http {
namespace server {

//! 请求解析器
class RequestParser {
  public:
    //! 状态
    enum class State {
        kInit,              //!< 初始化，未开始
        kFinishedStartLine, //!< 完成了首行解析
        kFinishedHeads,     //!< 完成heads的解析
        kFinishedAll,       //!< 完成了整个Http的解析
        kFail,              //!< 出错
    };

    ~RequestParser();

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
     * \brief   取走Request对象
     * \return  Request*    请求对象
     * \note    只有state为kFinishedAll才会返回真实的对象，否则都是返回nullptr
     *          一旦Request对象被取走，RequestParser则不再管辖被取走对象的生命期
     *          交由用户自己管理
     */
    Request* getRequest();

    //! 交换
    void swap(RequestParser &other);

    //! 重置
    void reset();

  private:
    State state_ = State::kInit;
    Request *sp_request_ = nullptr;
    size_t content_length_ = 0;
};

}
}
}

#endif //TBOX_HTTP_REQUEST_PARSER_H_20220502
