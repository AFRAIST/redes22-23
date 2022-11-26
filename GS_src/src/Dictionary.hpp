#include "content.hpp"

namespace rcwg {
class Dictionary {
  private:
    constexpr static inline char delim[2] = {' ', '\n'};

    constexpr static inline char minimal_string[] = {'a', ' ', 'a', '\n'};
    static_assert(minimal_string[1] == delim[0]);
    static_assert(minimal_string[3] == delim[1]);

    constexpr static inline size_t minimal_size =
        sizeof(minimal_string) / sizeof(char);

    class DictionaryEntry {
      private:
        const char *word;
        const char *word_class;

      public:
        constexpr ALWAYS_INLINE DictionaryEntry() = default;

        constexpr ALWAYS_INLINE DictionaryEntry(const char *word,
                                                const char *word_class)
            : word{word}, word_class{word_class} {}

        constexpr ALWAYS_INLINE const char *GetWord() { return this->word; }

        constexpr ALWAYS_INLINE void SetWord(const char *word) {
            this->word = word;
        }

        constexpr ALWAYS_INLINE void SetWordClass(const char *word_class) {
            this->word_class = word_class;
        }
    };

    std::unique_ptr<DictionaryEntry[]> entries;

  public:
    Dictionary(char *data, const size_t size);
};
} // namespace rcwg
