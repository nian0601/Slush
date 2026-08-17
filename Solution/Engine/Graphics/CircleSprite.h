#pragma once

#include "Graphics/BaseSprite.h"

namespace Slush
{
	class CircleSprite : public BaseSprite
	{
	public:
		CircleSprite();
		CircleSprite(float aRadius);

		void SetRadius(float aRadius);
		float GetRadius() const { return myRadius; }

		void Render() override;
		void Render(float x, float y) override;

	private:
		float myRadius;
	};
}
