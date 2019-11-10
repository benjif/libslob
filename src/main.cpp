#include "slob.h"

int main(void)
{
    SLOBReader s_reader;

    s_reader.open_file("wordnet-3.1.slob");

    /*s_reader.for_each_reference([](auto &ref) {
        std::cout << "Key: " << ref.key << '\n' <<
                     "Bin Index: " << ref.bin_index << '\n' <<
                     "Item Index: " << ref.item_index << '\n';

        return CONTINUE;
    });*/

    /*s_reader.for_each_store_item([&s_reader](auto &item) {
        for (auto &id : item.content_type_ids) {
            std::cout << "Content Type: " << s_reader.content_type(id) << '\n';
        }

        return CONTINUE;
    });*/

    //SLOBStoreItem item = s_reader.store_item(2);
    
    s_reader.for_each_item([](auto &item) {
        //std::cout << item << "\n\n";
        return CONTINUE;
    });

    return 0;
}
