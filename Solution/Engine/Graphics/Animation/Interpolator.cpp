#include "stdafx.h"
#include "Interpolator.h"

namespace Slush
{
	float Interpolator::GetValue(float aProgress) const
	{
		if (myType == Linear)
			return FW_Lerp(myStartValue, myEndValue, aProgress);
		else if (myType == Constant)
			return myStartValue;

		return 0.f;
	}

	void Interpolator::MakeLinear(float aStart, float aEnd)
	{
		myType = Linear;
		myStartValue = aStart;
		myEndValue = aEnd;
	}

	void Interpolator::MakeConstant(float aValue)
	{
		myType = Constant;
		myStartValue = aValue;
		myEndValue = aValue;
	}

	void Interpolator::OnParse(AssetParser::Handle aHandle)
	{
		AssetParser::Handle interpolatorHandle = aHandle.ParseChildElement("interpolator");
		interpolatorHandle.ParseFloatField("startvalue", myStartValue);
		interpolatorHandle.ParseFloatField("endvalue", myEndValue);

		int typeAsInt = myType;
		interpolatorHandle.ParseIntField("type", typeAsInt);
		myType = static_cast<Type>(typeAsInt);
	}
}