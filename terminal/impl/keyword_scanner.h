#ifndef TBOX_TERMINAL_KEYWORD_SCANNER_H_20220203
#define TBOX_TERMINAL_KEYWORD_SCANNER_H_20220203

#include <cstdint>
#include <vector>

namespace tbox::terminal {

class KeywordScanner {
  public:
    using ByteVector = std::vector<uint8_t>;

    bool addKeyword(const ByteVector &bytes, int id);

    /**
     * \return  0   Not Sure
     * \return -1   Miss
     * \return >0   Match
     */
    int feed(uint8_t v);

  private:
    using Keyword = std::pair<int, ByteVector>;

    size_t cmp_pos = 0;
    std::vector<Keyword>  keywords_;
    std::vector<Keyword*> candicate_keywords_;
};

}

#endif //TBOX_TERMINAL_KEYWORD_SCANNER_H_20220203
