#pragma once
#include "calculation_export.h"

namespace mesh::common
{
	class TSPoint
	{
	public:
		TSPoint();

	private:
		double _time;
		double _value;
	};
}