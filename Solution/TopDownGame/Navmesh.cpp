#include "stdafx.h"

#include "Navmesh.h"
#include "Graphics\Window.h"

Navmesh::Navmesh()
{
	Vector2f offset = { 20.f, 20. };
	for (int y = 0; y < mySectorGrid.y; ++y)
	{
		for (int x = 0; x < mySectorGrid.x; ++x)
		{
			Vertex* vert = new Vertex();
			vert->myPos.x = static_cast<float>(x * mySectorGridSize.x);
			vert->myPos.y = static_cast<float>(y * mySectorGridSize.y);
			vert->myPos += offset;
			myVertices.Add(vert);
		}
	}
}

void Navmesh::Render()
{
	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();

	int lineColor = 0xFF00FFFF;
	int faceColor = 0xAA00AAFF;

	for (int y = 1; y < mySectorGrid.y; ++y)
	{
		for (int x = 1; x < mySectorGrid.x; ++x)
		{
			Vertex* left = GetVertex(x - 1, y);
			Vertex* right = GetVertex(x, y);
			Vertex* topleft = GetVertex(x - 1, y - 1);
			Vertex* topRight = GetVertex(x, y - 1);

			window.RenderTriangle(topleft->myPos, right->myPos, left->myPos, faceColor);
			window.RenderTriangle(topRight->myPos, right->myPos, topleft->myPos, faceColor);

			window.RenderLine(left->myPos, right->myPos, lineColor);
			window.RenderLine(topleft->myPos, right->myPos, lineColor);
			window.RenderLine(topleft->myPos, left->myPos, lineColor);
		}
	}

	for (int x = 1; x < mySectorGrid.x; ++x)
	{
		Vertex* left = GetVertex(x - 1, 0);
		Vertex* right = GetVertex(x, 0);
		window.RenderLine(left->myPos, right->myPos, lineColor);
	}

	for (int y = 1; y < mySectorGrid.y; ++y)
	{
		Vertex* top = GetVertex(mySectorGrid.x - 1, y - 1);
		Vertex* bottom = GetVertex(mySectorGrid.x - 1, y);
		window.RenderLine(top->myPos, bottom->myPos, lineColor);
	}
}

Navmesh::Vertex* Navmesh::GetVertex(int x, int y) const
{
	int index = mySectorGrid.x * y + x;
	return myVertices[index];
}
