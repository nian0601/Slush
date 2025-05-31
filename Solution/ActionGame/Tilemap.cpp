#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "Tilemap.h"

#include <Core\Assets\AssetStorage.h>
#include <Graphics\RectSprite.h>
#include <Graphics\Texture.h>

Tilemap::Tilemap()
{
	const Slush::AssetStorage<Slush::Texture>& textures = ActionGameGlobals::GetInstance().GetTextureStorage();
	const Slush::Texture* groundTexture = textures.GetAsset("RA_Ground_Tiles");

	Layer& grassLayer = myLayers.Add();
	grassLayer.SetupSprites(groundTexture, 1, 1);
	grassLayer.FillWith(GRASS);
	
	Layer& dirtLayer = myLayers.Add();
	dirtLayer.SetupSprites(groundTexture, 1, 9);
	dirtLayer.FillWith(NONE);
	
	RandomMutation(dirtLayer, DIRT, 0.1f);
	CelluarAutomataMutation(dirtLayer, NONE, DIRT, DIRT, 1, 3, 0.1f);
	CelluarAutomataMutation(dirtLayer, NONE, DIRT, DIRT, 1, 3, 0.1f);
	CelluarAutomataMutation(dirtLayer, NONE, DIRT, DIRT, 1, 3, 0.1f);
	CelluarAutomataMutation(dirtLayer, NONE, DIRT, DIRT, 2, 7, 1.f);

	for (Layer& layer : myLayers)
		layer.CalculateSubTypes();
}

Tilemap::~Tilemap()
{
}

void Tilemap::Render()
{
	for (const Layer& layer : myLayers)
	{
		for (int y = 0; y < layer.myYCount - 1; ++y)
		{
			for (int x = 0; x < layer.myXCount - 1; ++x)
			{
				int tileIndex = layer.GetTileIndex(x, y);
				int subType = layer.myTilesSubType[tileIndex];
				if (myDisableSubTypes)
				{
					if (layer.myTiles[tileIndex] == NONE)
						continue;

					subType = 15;
				}

				Slush::RectSprite* tileSprite = layer.myTileSprites[subType];
				if (!tileSprite)
				{
					SLUSH_ERROR("Missing sprite for SubType %i", subType);
					continue;
				}

				float offset = 1.f;
				if (myDisableSubTypes)
					offset = 0.5f;

				tileSprite->Render(x * layer.myTileSize + layer.myTileSize * offset, y * layer.myTileSize + layer.myTileSize * offset);
			}
		}
	}
}

void Tilemap::RandomMutation(Layer& aLayer, int aNewTileType, float aChance)
{
	for (int& tile : aLayer.myTiles)
	{
		if (FW_RandFloat() > aChance)
			continue;

		tile = aNewTileType;
	}
}

void Tilemap::CelluarAutomataMutation(Layer& aLayer, int aOldTileType, int aNewTileType, int aNeighbourType, int aNeighbourMinCount, int aNeighbourMaxCount, float aChance)
{
	FW_GrowingArray<int> newTiles = aLayer.myTiles;

	for (int y = 0; y < aLayer.myYCount; ++y)
	{
		for (int x = 0; x < aLayer.myXCount; ++x)
		{
			int tileIndex = aLayer.GetTileIndex(x, y);
			if (newTiles[tileIndex] != aOldTileType)
				continue;

			int neighbourCount = GetNeighbourCount(aLayer, aNeighbourType, x, y);
			if (neighbourCount < aNeighbourMinCount || aNeighbourMinCount > aNeighbourMaxCount)
				continue;

			if (FW_RandFloat() > aChance)
				continue;

			newTiles[tileIndex] = aNewTileType;
		}
	}

	aLayer.myTiles = newTiles;
}

int Tilemap::GetNeighbourCount(Layer& aLayer, int aNeighbourType, int aX, int aY)
{
	int count = 0;

	for (int y = aY - 1; y < aY + 2; y++)
	{
		if (y < 0 || y >= aLayer.myYCount)
			continue;

		for (int x = aX - 1; x < aX + 2; x++)
		{
			if (x < 0 || x >= aLayer.myXCount)
				continue;

			if (x == aX && y == aY)
				continue;

			int tileIndex = aLayer.GetTileIndex(x, y);
			if (aLayer.myTiles[tileIndex] == aNeighbourType)
				count++;
		}
	}

	return count;
}

//////////////////////////////////////////////////////////////////////////

Tilemap::Layer::Layer()
{
	myTileSprites.Fill(nullptr);
}

Tilemap::Layer::~Layer()
{
	myTileSprites.DeleteAll();
}

void Tilemap::Layer::SetupSprites(const Slush::Texture* aTexture, int aStartTextureTileX, int aStartTextureTileY)
{
	for (int i = 0; i < 16; ++i)
	{
		myTileSprites[i] = new Slush::RectSprite();
		myTileSprites[i]->SetSize((float)myTileSize, (float)myTileSize);
		myTileSprites[i]->SetTexture(*aTexture);
	}
	
	const Vector2i textureOffsets[16] = 
	{
		{1,1}, // 0
		{0, 5}, // 1
		{0, 3}, // 2
		{0, 4}, // 3
		{2, 3}, // 4
		{0, 6}, // 5
		{1, 3}, // 6
		{2, 2}, // 7
		{2, 5}, // 8
		{1, 5}, // 9
		{2, 6}, // 10
		{2, 0}, // 11
		{0, 1}, // 12
		{0, 0}, // 13
		{0, 2}, // 14
		{1, 4}, // 15
	};

	for (int i = 0; i < 16; ++i)
	{
		int x = (aStartTextureTileX + textureOffsets[i].x) * myTileSize;
		int y = (aStartTextureTileY + textureOffsets[i].y) * myTileSize;
		myTileSprites[i]->SetTextureRect(x, y, myTileSize, myTileSize);
	}
}

void Tilemap::Layer::FillWith(int aTileType)
{
	myTiles.Reserve(myXCount * myYCount);
	myTilesSubType.Reserve(myXCount * myYCount);
	for (int i = 0; i < myXCount * myYCount; ++i)
	{
		myTiles[i] = aTileType;
		myTilesSubType[i] = 0;
	}
}

void Tilemap::Layer::CalculateSubTypes()
{
	for (int y = 0; y < myYCount - 1; ++y)
	{
		for (int x = 0; x < myXCount - 1; ++x)
		{
			myTilesSubType[GetTileIndex(x, y)] = GetCornerBitValue(x, y);
		}
	}
}

int Tilemap::Layer::GetTileValue(int x, int y) const
{
	// Should we handle reading outside of the map here? or on the callsite?
	// Not sure.. handling it here for now..
	return myTiles[GetTileIndex(x, y)];
}

int Tilemap::Layer::GetTileIndex(int x, int y) const
{
	return y * myXCount + x;
}

int Tilemap::Layer::GetCornerBitValue(int x, int y) const
{
	int value = 0;

	Vector2i tileOffsets[4] =
	{
		{1, 0},
		{1, 1},
		{0, 1},
		{0, 0}
	};

	int tileValues[4] = { 1, 2, 4, 8 };

	for (int i = 0; i < 4; ++i)
	{
		int cornerX = x + tileOffsets[i].x;
		int cornerY = y + tileOffsets[i].y;

		if (cornerX < 0 || cornerX >= myXCount)
		{
			value += tileValues[i];
			continue;
		}

		if (cornerY < 0 || cornerY >= myYCount)
		{
			value += tileValues[i];
			continue;
		}

		if (GetTileValue(cornerX, cornerY) != 0)
			value += tileValues[i];
	}

	return value;
}
