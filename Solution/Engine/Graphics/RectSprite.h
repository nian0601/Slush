#pragma once

#include "Graphics/BaseSprite.h"

namespace Slush
{
	class RectSprite : public BaseSprite
	{
	public:
		enum class Origin
		{
			CENTER,
			TOP_LEFT,
		};

		RectSprite();

		void SetOrigin(Origin anOrigin);
		void SetSize(float aWidth, float aHeight);
		void SetWidth(float aWidth);
		void SetHeight(float aHeight);
		const Vector2f& GetSize() const { return mySize; }
		
	private:
		Vector2f mySize;
		Origin myOrigin = Origin::CENTER;
	};
}
