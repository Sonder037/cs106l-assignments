#include "spellcheck.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

template<typename Iterator, typename UnaryPred>
std::vector<Iterator> find_all(Iterator begin, Iterator end, UnaryPred pred);

Corpus tokenize(std::string &source) {
    Corpus ret{};
    auto spaces = find_all(source.begin(), source.end(), [](char ch) -> bool {
        return ch == ' ' || ch == '\n' || ch == '\t';
    });
    std::transform(
        spaces.begin(),
        spaces.end() - 1,
        spaces.begin() + 1,
        std::inserter(ret, ret.end()),
        [&source](auto it1, auto it2) -> Token {
            return Token(source, it1, it2);
        }
    );
    std::erase_if(ret, [](const Token &token) -> bool {
        return token.content.empty();
    });
    return ret;
}

std::set<Misspelling> spellcheck(const Corpus &source, const Dictionary &dictionary) {
    std::set<Misspelling> ret{};
    auto view = source
                | std::ranges::views::filter([&dictionary](const Token &token) -> bool {
                    return !dictionary.count(token.content);
                })
                | std::ranges::views::transform([&dictionary](const Token &token) -> Misspelling {
                    Misspelling ret;
                    ret.token = token;
                    for (const auto &word: dictionary) {
                        if (levenshtein(word, token.content) == 1) {
                            ret.suggestions.insert(word);
                        }
                    }
                    return ret;
                })
                | std::ranges::views::filter([](const Misspelling &misspelling) -> bool {
                    return !misspelling.suggestions.empty();
                });
    std::ranges::copy(view, std::inserter(ret, ret.end()));
    return ret;
};

#include "utils.cpp"
