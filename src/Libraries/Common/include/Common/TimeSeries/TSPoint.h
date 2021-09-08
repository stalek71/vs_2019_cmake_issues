#pragma once

namespace mesh::common
{
	class TSPoint
	{
	public:
		TSPoint();

	private:
		TimePoint _time;
		double _value;
	};
}