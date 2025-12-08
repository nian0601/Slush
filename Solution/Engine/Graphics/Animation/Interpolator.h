#pragma once

namespace Slush
{
	struct Interpolator
	{
		float GetValue(float aProgress) const;

		void MakeLinear(float aStart, float aEnd);
		void MakeConstant(float aValue);

		void OnParse(AssetParser::Handle aHandle);

		enum Type
		{
			None,
			Linear,
			Constant,
		};
		int myType = None;
		float myStartValue = 0.f;
		float myEndValue = 0.f;
	};
}
