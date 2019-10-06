#include "slob.h"

int main(void)
{
    SLOBReader s_reader;

    s_reader.open_file("wordnet-3.1.slob");
    /*s_reader.for_each_reference([](auto &ref){
        std::cout << "Key: " << ref.key << std::endl <<
                     "Bin Index: " << ref.bin_index << std::endl <<
                     "Item Index: " << ref.item_index << std::endl <<
                     "Fragment: " << ref.fragment << std::endl;
    });*/
    s_reader.for_each_store_item([](auto &item){
    });

    return 0;
}
