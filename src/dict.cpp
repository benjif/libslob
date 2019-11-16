#include <array>
#include <string>
#include <cstring>
#include <climits>
#include <iostream>
#include <algorithm>
#include "dict.h"
#include "iteration.h"

CollationKeyList::CollationKeyList(SLOBReader &sr)
    : m_slob_reader(sr)
{
    // TODO: handle all UErrorCode errors properly
    UErrorCode status = U_ZERO_ERROR;
    u_init(&status);
    if (U_FAILURE(status))
        throw std::runtime_error("ICU init failed");
    m_collator = Collator::createInstance(Locale(""), status);
    if (U_FAILURE(status))
        throw std::runtime_error("ICU error has occurred");
    m_collator->setStrength(Collator::PRIMARY);
    m_collator->setAttribute(UCOL_ALTERNATE_HANDLING, UCOL_SHIFTED, status);
    if (U_FAILURE(status))
        throw std::runtime_error("ICU error has occurred");
}

CollationKeyList::~CollationKeyList()
{
    delete m_collator;
    u_cleanup();
}

Collator *CollationKeyList::collator() const
{
    return m_collator;
}

void CollationKeyList::set_maxlength(int32_t len)
{
    m_maxlength = len;
}

ItemDict::ItemDict(SLOBReader &sr)
    : m_slob_reader(sr), m_key_list(sr)
{
}

static bool uint8cmp(uint8_t *a, uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
        if (a[i] != b[i])
            return false;
    return true;
}

std::vector<SLOBReference> ItemDict::operator[](const std::string &term)
{
    Collator *m_collator = m_key_list.collator();

    UChar u_content[term.length()+1];
    u_charsToUChars(term.c_str(), u_content, term.length()+1);
    UnicodeString u_string(u_content);

    auto maxlength = m_collator->getSortKey(u_string, nullptr, 0);
    uint8_t *sortkey = new uint8_t[maxlength];
    m_collator->getSortKey(u_string, sortkey, maxlength-1);

    std::vector<SLOBReference> matches;

    m_key_list.set_maxlength(maxlength);
    m_key_list.for_each_key([&](auto &other_key, auto &ref) {
        if (uint8cmp(sortkey, other_key, maxlength-1))
            matches.push_back(ref);
        return ITERATION::CONTINUE;
    });

    delete[] sortkey;
    return matches;
}
