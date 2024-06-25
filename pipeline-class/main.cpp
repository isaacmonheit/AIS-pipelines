#include "Pipeline.h"

int main(int argc, char *argv[])
{
    Pipeline pipi(Pipeline::Fork);
    Pipeline epo(Pipeline::Spoon);
    Pipeline kili(Pipeline::Knife);

    std::cout << "I am famished!" << std::endl;
    return 0;
}