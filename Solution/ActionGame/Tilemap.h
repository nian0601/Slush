#pragma once

namespace Slush
{
	class RectSprite;
	class Texture;
}

class Tilemap
{
public:
	Tilemap();
	~Tilemap();

	void Render();

	void ToggleSubTypes() { myDisableSubTypes = !myDisableSubTypes; }

private:
	struct Layer;

	enum TileType
	{
		NONE,
		GRASS,
		DIRT,
	};

	void RandomMutation(Layer& aLayer, int aNewTileType, float aChance);
	void CelluarAutomataMutation(Layer& aLayer, int aOldTileType, int aNewTileType, int aNeighbourType, int aNeighbourMinCount, int aNeighbourMaxCount, float aChance);
	int GetNeighbourCount(Layer& aLayer, int aNeighbourType, int aX, int aY);

	struct Layer
	{
		Layer();
		~Layer();

		void SetupSprites(const Slush::Texture* aTexture, int aStartTextureTileX, int aStartTextureTileY);
		void FillWith(int aTileType);
		void CalculateSubTypes();

		int GetTileValue(int x, int y) const;
		int GetTileIndex(int x, int y) const;

		int GetCornerBitValue(int x, int y) const;

		FW_GrowingArray<int> myTiles; //0 or 1 based on if this Layer has a tile in that spot
		FW_GrowingArray<int> myTilesSubType; //0-16 based on neighbouring tiles
		FW_StaticArray<Slush::RectSprite*, 16> myTileSprites;

		int myTileSize = 48;
		int myXCount = 32;
		int myYCount = 16;
	};
	FW_GrowingArray<Layer> myLayers;
	bool myDisableSubTypes = false;
	
};