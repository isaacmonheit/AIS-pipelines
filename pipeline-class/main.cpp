#include "Pipeline.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    Pipeline* pipi = new Pipeline(Pipeline::Fork);
    Pipeline* epo = new Pipeline(Pipeline::Spoon);
    Pipeline* kili = new Pipeline(Pipeline::Knife);

    sleep(5);
    std::cout << "I am famished!" << std::endl;

    std::cerr << "Deletionary fairy...\n";

    std::cerr << "kili\n";
    delete kili;
    std::cerr << "epo\n";
    delete epo;
    std::cerr << "pipi\n";
    delete pipi;

    return 0;
}
