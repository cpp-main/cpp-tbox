#include "keyword_scanner.h"
#include <algorithm>

namespace tbox::terminal {

bool KeywordScanner::addKeyword(const ByteVector &bytes, int id)
{
    if (id <= 0)
        return false;

    auto iter = std::find_if(keywords_.begin(), keywords_.end(),
        [&] (Keyword &item) {
            return item.first == id || item.second == bytes;
        }
    );

    if (iter != keywords_.end())
        return false;

    keywords_.push_back(std::make_pair(id, bytes));
    return true;

}

int KeywordScanner::feed(uint8_t v)
{
    if (cmp_pos == 0) {
        //! 从 keywords_ 中筛选出第一个字节匹配的 keyword
        std::for_each(keywords_.begin(), keywords_.end(),
            [this, v] (Keyword &item) {
                if (item.second[0] == v) {
                    candicate_keywords_.push_back(&item);
                }
            }
        );

    } else {
        //! 从 candicate_keywords_ 中删除不匹配的选项
        auto iter = std::remove_if(candicate_keywords_.begin(), candicate_keywords_.end(),
            [this, v] (Keyword *pitem) {
                return pitem->second[cmp_pos] != v;
            }
        ); 
        if (iter != candicate_keywords_.end())
            candicate_keywords_.erase(iter);
    }

    if (candicate_keywords_.size() == 1) {
        //! 如果仅剩一项，则表示
        int id = candicate_keywords_.at(0)->first;
        candicate_keywords_.clear();
        cmp_pos = 0;
        return id;
    } else if (candicate_keywords_.size() == 0) {
        return -1;
    } else {
        ++cmp_pos;
        return 0;
    }
}

}
