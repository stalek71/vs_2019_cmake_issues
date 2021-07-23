// Mesh.cpp : Defines the entry point for the application.
//

#include <Calc/Calculation.h>
#include "Storage.h"

using namespace std;

int main()
{
	cout << "Hello Mesh! The value provided by the function is " << mesh::calc::SomeCalcFunction(200.) << endl;
	return 0;
}
