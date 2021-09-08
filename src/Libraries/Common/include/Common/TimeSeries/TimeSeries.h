#pragma once
#include <Common/TimeSeries/TSPoint.h>

namespace mesh::common
{
	class TimeSeries
	{
	public:
		TimeSeries();

        // Iterators to access time series points (TSPoint instances) - never ever use vectors to store all the points
        // TODO: Unfinished yet
	};
}