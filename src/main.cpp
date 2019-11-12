#include "slob.h"
#include "dict.h"
#include "iteration.h"

int main(void)
{
    const std::string searchterm = "black hole";

    SLOBReader s_reader;
    s_reader.open_file("wordnet-3.1.slob");

    ItemDict dict(s_reader);
    auto matches = dict[searchterm];

    if (!matches.empty())
        if (matches[0].key.compare(searchterm) == 0)
            std::cout << matches[0].key << '\n';
        else
            for (auto match : matches)
                std::cout << match.key << '\n';
    else
        std::cout << "No results" << '\n';

    return 0;
}
