#include "Dictionary.hpp"

namespace rcwg {
Dictionary::Dictionary(char *data, const size_t size) {
    /* Index data. */
    u32 counter = 0;
    const size_t alloc_size = size / this->minimal_size;
    this->entries.reset(new DictionaryEntry[alloc_size]);

    char cc = this->delim[0];
    size_t amt = 0;
    auto IsWord = [&]() ALWAYS_INLINE_LAMBDA -> bool {
        return cc == this->delim[0];
    };
    /*auto IsClass = [&]() ALWAYS_INLINE_LAMBDA -> bool { return cc ==
     * this->delim[1]; };*/

    for (size_t i = 0; i != size; ++i) {
        /* Keep checking word validity. */
        R_EXIT_IF(BRANCH_UNLIKELY((
                      IsWord() && (data[i] == this->delim[1] || counter > 30))),
                  "Invalid word size!");

        /* Terminator. */
        if (data[i] == cc) {
            if (IsWord()) {
                /* Assert size again. */
                R_EXIT_IF(BRANCH_UNLIKELY(counter < 3), "Invalid word size!");

                cc = this->delim[1];
                this->entries.get()[amt].SetWord(
                    std::addressof(data[i - counter]));
            } else /*if (IsClass())*/ {
                cc = this->delim[0];
                this->entries.get()[amt++].SetWordClass(
                    std::addressof(data[i - counter]));
            }

            counter = 0;
            data[i] = '\0';
        }

        /* Increment. */
        ++counter;
    }

    R_EXIT_IF(BRANCH_UNLIKELY(cc != this->delim[0] || data[size] != cc),
              "Invalid dictionary last line!");
}
} // namespace rcwg
