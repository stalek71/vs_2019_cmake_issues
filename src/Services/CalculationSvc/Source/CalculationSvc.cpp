#include <DataCache/DataCache.h>
#include <Calc/Calculation.h>
#include <TimeSeriesStorage/Storage.h>
#include <iostream>
#include "CalculationSvc.h"

using namespace std;

int main()
{
	cout << "Hello from Calculation service! The value provided by the function is " << mesh::calc::SomeCalcFunction(200.) << endl;
	return 0;
}