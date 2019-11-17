#ifndef _DICT_H
#define _DICT_H

#include <vector>
#include <string>
#include <unicode/coll.h>
#include <unicode/utf8.h>
#include <unicode/uclean.h>
#include <unicode/sortkey.h>
#include <unicode/errorcode.h>
#include "slob.h"

#define MAX_SORTKEY_LEN 256

class CollationKeyList {
public:
    CollationKeyList(SLOBReader &);
    ~CollationKeyList();

    // Set max collation key comparison length.
    // By default, this is 256 characters.
    void set_maxlength(int32_t);

    // Iterate over collation keys.
    template <typename C>
    void for_each_key(C);

    Collator *collator() const;

private:
    Collator *m_collator;
    SLOBReader &m_slob_reader;
    int32_t m_maxlength { MAX_SORTKEY_LEN };
};

template <typename C>
void CollationKeyList::for_each_key(C call)
{
    uint8_t *sortkey = new uint8_t[MAX_SORTKEY_LEN];
    m_slob_reader.for_each_reference([&](auto &ref) {
        UChar u_content[ref.key.length()+1];
        u_charsToUChars(ref.key.c_str(), u_content, ref.key.length()+1);
        UnicodeString u_string(u_content);
        m_collator->getSortKey(u_string, sortkey, m_maxlength-1);
        if (call(sortkey, ref))
            return ITERATION::BREAK;
        return ITERATION::CONTINUE;
    });
    delete[] sortkey;
}

class ItemDict {
public:
    ItemDict(SLOBReader &);

    // Search SLOB references for key.
    std::vector<SLOBReference> operator[](const std::string &term);

private:
    SLOBReader &m_slob_reader;
    CollationKeyList m_key_list;
};

#endif
