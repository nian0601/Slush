#include "stdafx.h"

#include "ActionGameGlobals.h"
#include "Tilemap.h"

#include <Core\Assets\AssetStorage.h>
#include <Graphics\RectSprite.h>
#include <Graphics\Texture.h>

Tilemap::Tilemap()
{
	//	  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
	//0 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	//1 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	//2 { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	//3 { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	//4 { 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
	//5 { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0 },
	//6 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0 },
	//7 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0 },
	//8 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	//9 { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

	for (FW_StaticArray<int, 16>& row : myTiles)
		row.Fill(0);

	myTiles[2][2] = 1;
	myTiles[3][2] = 1;
	myTiles[4][2] = 1;
	myTiles[3][3] = 1;
	myTiles[4][3] = 1;

	myTiles[4][6] = 1;
	myTiles[3][7] = 1;
	myTiles[4][7] = 1;
	myTiles[5][7] = 1;
	myTiles[4][8] = 1;

	myTiles[5][10] = 1;
	myTiles[6][10] = 1;
	myTiles[7][10] = 1;
	myTiles[5][11] = 1;
	myTiles[7][11] = 1;
	myTiles[5][12] = 1;
	myTiles[6][12] = 1;
	myTiles[7][12] = 1;

	

	const Slush::AssetStorage<Slush::Texture>& textures = ActionGameGlobals::GetInstance().GetTextureStorage();
	const Slush::Texture* groundTexture = textures.GetAsset("RA_Ground_Tiles");

	myTileSprites.Fill(nullptr);
	myTileSprites[0] = new Slush::RectSprite();
	myTileSprites[0]->SetSize((float)myTileSize, (float)myTileSize);
	myTileSprites[0]->SetTexture(*groundTexture);
	myTileSprites[0]->SetTextureRect(2 * myTileSize, 5 * myTileSize, myTileSize, myTileSize);

	myTileSprites[1] = new Slush::RectSprite();
	myTileSprites[1]->SetSize((float)myTileSize, (float)myTileSize);
	myTileSprites[1]->SetTexture(*groundTexture);
	myTileSprites[1]->SetTextureRect(2 * myTileSize, 13 * myTileSize, myTileSize, myTileSize);
}

Tilemap::~Tilemap()
{
	myTileSprites.DeleteAll();
}

void Tilemap::Render()
{
	int yCount = myTiles.Count();
	int xCount = myTiles[0].Count();
	for (int y = 0; y < yCount; ++y)
	{
		for (int x = 0; x < xCount; ++x)
		{
			int tileType = myTiles[y][x];
			Slush::RectSprite* tileSprite = myTileSprites[tileType];
			if (!tileSprite)
			{
				SLUSH_ERROR("Missing sprite for TileType %i", tileType);
				continue;
			}

			tileSprite->Render(x * myTileSize, y * myTileSize);

		}
	}
}
