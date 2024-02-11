#pragma once

#include "Graphics/BaseSprite.h"

namespace Slush
{
	class RectSprite : public BaseSprite
	{
	public:
		RectSprite();

		void SetSize(float aWidth, float aHeight);
		void SetWidth(float aWidth);
		void SetHeight(float aHeight);
		const Vector2f& GetSize() const { return mySize; }
		
	private:
		Vector2f mySize;
	};
}
