#include "slob.h"
#include "dict.h"
#include "iteration.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <search term>\n";
        return 0;
    }

    std::string searchterm = argv[1];

    SLOBReader s_reader;
    s_reader.open_file("wordnet-3.1.slob");

    ItemDict dict(s_reader);
    auto matches = dict[searchterm];

    std::cout << matches[0].key << '\n';
    std::string result = s_reader.item(matches[0].bin_index, matches[0].item_index);

    std::cout << result << "\n\n";

    /*if (!matches.empty())
        if (matches[0].key.compare(searchterm) == 0)
            std::cout << matches[0].key << '\n';
        else
            for (auto match : matches)
                std::cout << match.key << '\n';
    else
        std::cout << "No results" << '\n';*/

    return 0;
}
