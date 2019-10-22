#include "slob.h"

int main(void)
{
    SLOBReader s_reader;

    s_reader.open_file("wordnet-3.1.slob");

    /*s_reader.for_each_reference([](auto &ref) {
        std::cout << "Key: " << ref.key << '\n' <<
                     "Bin Index: " << ref.bin_index << '\n' <<
                     "Item Index: " << ref.item_index << '\n';
    });*/

    s_reader.for_each_store_item([&s_reader](auto &item) {
        /*for (auto &id : item.content_type_ids) {
            std::cout << "Type id: " << int(id) << '\n';
        }*/
    });

    return 0;
}
