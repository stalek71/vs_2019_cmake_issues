#pragma once
#include <chrono>

namespace mesh::common
{
	class TimePoint: public std::chrono::time_point<std::chrono::system_clock>
	{
	private:
		using base = std::chrono::time_point<std::chrono::system_clock>;

	public:
		TimePoint();
	};
}