#include "stdafx.h"

#include "Navmesh.h"
#include "Graphics\Window.h"
#include "Core\Input.h"
#include "FW_Intersection.h"

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

	for (int y = 1; y < mySectorGrid.y; ++y)
	{
		for (int x = 1; x < mySectorGrid.x; ++x)
		{
			Vertex* bottomleft = GetVertex(x - 1, y);
			Vertex* bottomright = GetVertex(x, y);
			Vertex* topleft = GetVertex(x - 1, y - 1);
			Vertex* topRight = GetVertex(x, y - 1);

			CreateEdge(bottomleft, topleft);
			CreateEdge(topRight, bottomright);
			CreateEdge(topleft, topRight);
			CreateEdge(bottomright, bottomleft);
			CreateEdge(topleft, bottomright);
		}
	}

	for (int y = 1; y < mySectorGrid.y; ++y)
	{
		for (int x = 1; x < mySectorGrid.x; ++x)
		{
			Vertex* bottomleft = GetVertex(x - 1, y);
			Vertex* bottomright = GetVertex(x, y);
			Vertex* topleft = GetVertex(x - 1, y - 1);
			Vertex* topRight = GetVertex(x, y - 1);

			Edge* leftEdge = bottomleft->myEdges[0];
			Edge* rightEdge = bottomright->myEdges[0];
			Edge* topEdge = topleft->myEdges[1];
			Edge* bottomEdge = bottomleft->myEdges[1];
			Edge* middleEdge = topleft->myEdges[2];

			CreateTriangle(leftEdge, middleEdge, bottomEdge, bottomleft, topleft, bottomright);
			CreateTriangle(topEdge, rightEdge, middleEdge, topleft, topRight, bottomright);
		}
	}
}

Navmesh::~Navmesh()
{
	myTriangles.DeleteAll();
	myEdges.DeleteAll();
	myVertices.DeleteAll();
}

void Navmesh::Update()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	if (engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
	{
		myMouseAnchor = engine.GetInput().GetMousePositionf();
		myHasAnchor = true;
	}

	if (engine.GetInput().WasKeyReleased(Slush::Input::E))
	{
		int apa = 5;
		++apa;
	}
}

void Navmesh::Render()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Window& window = Slush::Engine::GetInstance().GetWindow();

	const Vector2f& mousePos = engine.GetInput().GetMousePositionf();
	FW_Intersection::LineSegment mouseSegment;
	mouseSegment.myStart = myMouseAnchor;
	mouseSegment.myEnd = mousePos;

	int lineColor = 0xFF00FFFF;
	int faceColor = 0xAA00AAFF;

	FW_GrowingArray<Vector2f> intersections;

	for (Triangle* triangle : myTriangles)
	{
		bool collision = false;
		Vector2f intersection;
		if (myHasAnchor)
		{
			FW_Intersection::LineSegment edge1{ triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos };
			FW_Intersection::LineSegment edge2{ triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos };
			FW_Intersection::LineSegment edge3{ triangle->myVertices[2]->myPos, triangle->myVertices[0]->myPos };

			if (FW_Intersection::LineSegmentVsLineSegment(mouseSegment, edge1, &intersection) || FW_Intersection::LineSegmentVsLineSegment(mouseSegment, edge2, &intersection) || FW_Intersection::LineSegmentVsLineSegment(mouseSegment, edge3, &intersection))
				collision = true;
		}
		else
		{
			collision = FW_Intersection::PointInsideTriangle(mousePos, triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos);
		}

		if (collision)
		{
			window.RenderTriangle(triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos, 0xFFFFFFFF);
			intersections.Add(intersection);
		}
		else
		{
			window.RenderTriangle(triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos, faceColor);
		}
		
	}

	for (Edge* edge : myEdges)
	{
		window.RenderLine(edge->myVertices[0]->myPos, edge->myVertices[1]->myPos, lineColor);
	}

	for (const Vector2f& intersect : intersections)
		window.RenderCircle(intersect, 3.f, 0xFFFF0000);

	if (myHasAnchor)
		window.RenderLine(mouseSegment.myStart, mouseSegment.myEnd, 0xFF000000);
}

Navmesh::Vertex* Navmesh::GetVertex(int x, int y) const
{
	int index = mySectorGrid.x * y + x;
	return myVertices[index];
}

void Navmesh::CreateEdge(Vertex* aV1, Vertex* aV2)
{
	Edge* edge = new Edge();
	edge->myVertices[0] = aV1;
	edge->myVertices[1] = aV2;
	aV1->myEdges.Add(edge);
	aV2->myEdges.Add(edge);
	myEdges.Add(edge);
}

void Navmesh::CreateTriangle(Edge* aE1, Edge* aE2, Edge* aE3, Vertex* aV1, Vertex* aV2, Vertex* aV3)
{
	Triangle* triangle = new Triangle();
	triangle->myEdges[0] = aE1;
	triangle->myEdges[1] = aE2;
	triangle->myEdges[2] = aE3;

	triangle->myVertices[0] = aV1;
	triangle->myVertices[1] = aV2;
	triangle->myVertices[2] = aV3;

	aE1->myTriangles.Add(triangle);
	aE2->myTriangles.Add(triangle);
	aE3->myTriangles.Add(triangle);
	myTriangles.Add(triangle);
}
