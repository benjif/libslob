#include <slob/slob.h>
#include <slob/dictionary.h>

int main(int argc, char **argv)
{
    // Term to collate and search for within SLOB file.
    const std::string searchterm = "black hole";

    SLOBReader s_reader;

    // Open file for parsing.
    s_reader.open_file("dictionaries/wordnet-3.1.slob");

    // Wrap parsed object with dictionary-like interface.
    SLOBDict dict(s_reader);

    // Search for "black hole". This returns a SLOBReference.
    // In order to obtain the actual item content, use
    // SLOBParser::item().
    auto matches = dict[searchterm];

    if (matches.size() > 0) {
        // If it is a direct match, display item content.
        if (matches[0].key == searchterm) {
            // Obtain item content.
            std::cout << matches[0].key << '\n';
            const std::string result = s_reader.item(matches[0].bin_index, matches[0].item_index);
            std::cout << result << "\n\n";
        }

        // Display other matched references.
        if (matches.size() > 1) {
            // Display other results
            std::cout << "All results: \n";
            for (auto &match : matches)
                std::cout << " - " << match.key << '\n';
            std::cout << "\n\n";
        }
    }

    return 0;
}
