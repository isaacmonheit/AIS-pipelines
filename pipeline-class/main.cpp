#include "Pipeline.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    Pipeline* fork = new Pipeline(Pipeline::ForkedSource);
    Pipeline* filesaver = new Pipeline(Pipeline::FileSaver);
    Pipeline* viewer = new Pipeline(Pipeline::Viewer);

    sleep(5);
    std::cout << "I am famished!" << std::endl;

    std::cerr << "deleting fork\n";
    delete fork;
    std::cerr << "deleting filesaver\n";
    delete filesaver;
    std::cerr << "deleting viewer\n";
    delete viewer;

    return 0;
}
