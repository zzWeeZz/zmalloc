#include <iostream>
#include <Core/Core.h>
int main(int, char**)
{
	std::cout << "Hello World!" << std::endl;
	Core::Core core;
	core.PrintMessage();
	return 0;
}