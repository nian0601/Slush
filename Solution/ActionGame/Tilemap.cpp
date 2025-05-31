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
	grassLayer.SetupGroundSprites(groundTexture, 1, 1);
	grassLayer.FillWith(GRASS);
	
	Layer& dirtLayer = myLayers.Add();
	dirtLayer.SetupGroundSprites(groundTexture, 1, 9);
	dirtLayer.FillWith(NONE);
	
	RandomMutation(dirtLayer, DIRT, 0.1f); // Initial seeds
	CelluarAutomataMutation(dirtLayer, NONE, DIRT, DIRT, 1, 3, 0.1f); // Grow initial seeds
	CelluarAutomataMutation(dirtLayer, NONE, DIRT, DIRT, 2, 3, 0.25f); // Continue to grow pairs
	CelluarAutomataMutation(dirtLayer, NONE, DIRT, DIRT, 4, 7, 1.f); // Fill in gaps in big groups
	CelluarAutomataMutation(dirtLayer, DIRT, NONE, NONE, 4, 4, 1.f); // Remove any dirt with no dirt-neighbours

	grassLayer.CalculateSubTypes();
	dirtLayer.CalculateSubTypes();

	const Slush::Texture* cryptTexture = textures.GetAsset("RA_Crypt");
	Layer& wallLayer = myLayers.Add();
	wallLayer.FillWith(0);
	wallLayer.AddSprite(cryptTexture, 1, 3, 8); // TopLeft
	wallLayer.AddSprite(cryptTexture, 2, 4, 8); // Top
	wallLayer.AddSprite(cryptTexture, 3, 4, 9); // Top2
	wallLayer.AddSprite(cryptTexture, 4, 5, 8); // TopRight
	wallLayer.AddSprite(cryptTexture, 5, 3, 9); // Left
	wallLayer.AddSprite(cryptTexture, 6, 5, 9); // Right
	wallLayer.AddSprite(cryptTexture, 7, 3, 11); // BottomLeft
	wallLayer.AddSprite(cryptTexture, 8, 4, 11); // Bottom
	wallLayer.AddSprite(cryptTexture, 9, 5, 11); // BottomRight

	for (int x = 0; x < wallLayer.myXCount; ++x)
	{
		wallLayer.myVisualTiles[wallLayer.GetTileIndex(x, 0)] = 2;
		wallLayer.myVisualTiles[wallLayer.GetTileIndex(x, 1)] = 3;
		wallLayer.myVisualTiles[wallLayer.GetTileIndex(x, wallLayer.myYCount-1)] = 8;
	}

	for (int y = 0; y < wallLayer.myYCount; ++y)
	{
		wallLayer.myVisualTiles[wallLayer.GetTileIndex(0, y)] = 5;
		wallLayer.myVisualTiles[wallLayer.GetTileIndex(wallLayer.myXCount -1, y)] = 6;
	}

	wallLayer.myVisualTiles[wallLayer.GetTileIndex(0, 0)] = 1;
	wallLayer.myVisualTiles[wallLayer.GetTileIndex(wallLayer.myXCount-1, 0)] = 4;
	wallLayer.myVisualTiles[wallLayer.GetTileIndex(0, wallLayer.myYCount-1)] = 7;
	wallLayer.myVisualTiles[wallLayer.GetTileIndex(wallLayer.myXCount - 1, wallLayer.myYCount - 1)] = 9;
}

Tilemap::~Tilemap()
{
}

void Tilemap::Render()
{
	for (const Layer& layer : myLayers)
	{
		for (int y = 0; y < layer.myYCount; ++y)
		{
			for (int x = 0; x < layer.myXCount; ++x)
			{
				int tileIndex = layer.GetTileIndex(x, y);
				int subType = layer.myVisualTiles[tileIndex];
				if (myDisableSubTypes)
				{
					if (layer.myTiles[tileIndex] == NONE)
						continue;

					subType = 15;
				}

				if (subType == 0)
					continue;

				Slush::RectSprite* tileSprite = layer.myTileSprites[subType];
				if (!tileSprite)
				{
					SLUSH_ERROR("Missing sprite for SubType %i", subType);
					continue;
				}

				float offset = 0.f;
				if (myDisableSubTypes)
					offset = -0.5f;

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

void Tilemap::Layer::AddSprite(const Slush::Texture* aTexture, int aTileID, int aTextureTileX, int aTextureTileY)
{
	myTileSprites[aTileID] = new Slush::RectSprite();
	myTileSprites[aTileID]->SetTexture(*aTexture);
	myTileSprites[aTileID]->SetSize((float)myTileSize, (float)myTileSize);
	myTileSprites[aTileID]->SetTextureRect(aTextureTileX * myTileSize, aTextureTileY * myTileSize, myTileSize, myTileSize);
	myTileSprites[aTileID]->SetOrigin(Slush::RectSprite::Origin::TOP_LEFT);
}

void Tilemap::Layer::SetupGroundSprites(const Slush::Texture* aTexture, int aStartTextureTileX, int aStartTextureTileY)
{
	for (int i = 0; i < 16; ++i)
	{
		myTileSprites[i] = new Slush::RectSprite();
		myTileSprites[i]->SetSize((float)myTileSize, (float)myTileSize);
		myTileSprites[i]->SetTexture(*aTexture);
		myTileSprites[i]->SetOrigin(Slush::RectSprite::Origin::TOP_LEFT);
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
	myVisualTiles.Reserve(myXCount * myYCount);
	for (int i = 0; i < myXCount * myYCount; ++i)
	{
		myTiles[i] = aTileType;
		myVisualTiles[i] = 0;
	}
}

void Tilemap::Layer::CalculateSubTypes()
{
	for (int y = 0; y < myYCount; ++y)
	{
		for (int x = 0; x < myXCount; ++x)
		{
			myVisualTiles[GetTileIndex(x, y)] = GetCornerBitValue(x, y);
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
