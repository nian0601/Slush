#pragma once

namespace Slush
{
	class RectSprite;
}

class Tilemap
{
public:
	Tilemap();
	~Tilemap();

	void Render();

private:
	int myTileSize = 48;
	FW_StaticArray<FW_StaticArray<int, 16>, 10> myTiles;
	FW_StaticArray<Slush::RectSprite*, 16> myTileSprites;
};