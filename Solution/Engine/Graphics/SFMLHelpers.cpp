#include "stdafx.h"
#include "Graphics/SFMLHelpers.h"
namespace Slush
{
	namespace SFMLHelpers
	{
		sf::Color GetColor(int aHexColor)
		{
			return{
			unsigned char((aHexColor >> 16) & 255),
			unsigned char((aHexColor >> 8) & 255),
			unsigned char((aHexColor >> 0) & 255),
			unsigned char((aHexColor >> 24) & 255)
			};
		}
	}
}