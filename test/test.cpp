#include <slob/slob.h>
#include <slob/dictionary.h>

int main(int argc, char **argv)
{
    const std::string searchterm = "black hole";

    SLOBReader s_reader;
    s_reader.open_file("dictionaries/wordnet-3.1.slob");

    ItemDict dict(s_reader);
    auto matches = dict[searchterm];

    if (matches.size() > 0) {
        if (matches[0].key == searchterm) {
            std::cout << matches[0].key << '\n';
            const std::string result = s_reader.item(matches[0].bin_index, matches[0].item_index);
            std::cout << result << "\n\n";
        }

        if (matches.size() > 1) {
            std::cout << "All results: \n";
            for (auto &match : matches)
                std::cout << " - " << match.key << '\n';
            std::cout << "\n\n";
        }
    }

    return 0;
}
