#include "stdafx.h"

#include "Navmesh.h"
#include "Graphics\Window.h"
#include "Core\Input.h"
#include "FW_Intersection.h"

Navmesh::Navmesh()
{
	Vector2f offset = { 20.f, 20. };
	Vertex* topLeft = new Vertex();
	topLeft->myPos = offset;

	Edge* outRight = nullptr;
	FW_GrowingArray<Edge*> bottomEdges;
	for (int i = 0; i < mySectorGrid.x; ++i)
		bottomEdges.Add(nullptr);

	for (int y = 0; y < mySectorGrid.y; ++y)
	{
		for (int x = 0; x < mySectorGrid.x; ++x)
		{
			CreateQuad(topLeft, outRight, bottomEdges[x]);
		}

		topLeft = bottomEdges[0]->myVertices[0];
		outRight = nullptr;
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
	else if (engine.GetInput().WasMouseReleased(Slush::Input::RIGHTMB))
	{
		if (myHasAnchor)
		{
			myHasAnchor = false;
			Cut(myMouseAnchor, engine.GetInput().GetMousePositionf());
		}
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

	FW_Intersection::Ray mouseRay;
	mouseRay.myStart = myMouseAnchor;
	mouseRay.myDirection = GetNormalized(mousePos - myMouseAnchor);

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

#if 0
			if (FW_Intersection::LineSegmentVsLineSegment(mouseSegment, edge1, &intersection))
			{
				intersections.Add(intersection);
				collision = true;
			}

			if (FW_Intersection::LineSegmentVsLineSegment(mouseSegment, edge2, &intersection))
			{
				intersections.Add(intersection);
				collision = true;
			}

			if (FW_Intersection::LineSegmentVsLineSegment(mouseSegment, edge3, &intersection))
			{
				intersections.Add(intersection);
				collision = true;
			}
#else
			if (FW_Intersection::RayVsLineSegment(mouseRay, edge1, &intersection))
			{
				intersections.Add(intersection);
				collision = true;
			}

			if (FW_Intersection::RayVsLineSegment(mouseRay, edge2, &intersection))
			{
				intersections.Add(intersection);
				collision = true;
			}

			if (FW_Intersection::RayVsLineSegment(mouseRay, edge3, &intersection))
			{
				intersections.Add(intersection);
				collision = true;
			}
#endif
		}
		else
		{
			collision = triangle->PointInside(mousePos);
		}

		if (collision)
		{
			window.RenderTriangle(triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos, 0xFFFFFFFF);
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

Navmesh::Edge* Navmesh::GetEdgeWithVertex(Vertex* aV, Edge* aE1, Edge* aE2) const
{
	if (aE1->myVertices[0] == aV || aE1->myVertices[1] == aV)
		return aE1;

	if (aE2->myVertices[0] == aV || aE2->myVertices[1] == aV)
		return aE2;

	FW_ASSERT_ALWAYS("No Edge with that Vertex");
	return nullptr;
}

Navmesh::Edge* Navmesh::GetEdgeWithoutVertex(Vertex* aV, Edge* aE1, Edge* aE2) const
{
	if (aE1->myVertices[0] != aV && aE1->myVertices[1] != aV)
		return aE1;

	if (aE2->myVertices[0] != aV && aE2->myVertices[1] != aV)
		return aE2;

	FW_ASSERT_ALWAYS("No Edge without that Vertex");
	return nullptr;
}

void Navmesh::CreateQuad(Vertex*& aTopLeftVertex, Edge*& aLeftEdge, Edge*& aTopEdge)
{
	Edge* left = nullptr;
	Vertex* bottomLeft = nullptr;
	if (aLeftEdge)
	{
		left = aLeftEdge;
		bottomLeft = left->myVertices[1];
	}
	else if (!left)
	{
		bottomLeft = new Vertex();
		bottomLeft->myPos = aTopLeftVertex->myPos;
		bottomLeft->myPos.y += mySectorGridSize.y;
		myVertices.Add(bottomLeft);

		left = CreateEdge(aTopLeftVertex, bottomLeft);
	}

	Edge* top = nullptr;
	Vertex* topRight = nullptr;
	if (aTopEdge)
	{
		top = aTopEdge;
		topRight = top->myVertices[1];
	}
	else if (!aTopEdge)
	{
		topRight = new Vertex();
		topRight->myPos = aTopLeftVertex->myPos;
		topRight->myPos.x += mySectorGridSize.x;
		myVertices.Add(topRight);

		top = CreateEdge(aTopLeftVertex, topRight);
	}

	Vertex* bottomRight = new Vertex();
	bottomRight->myPos = aTopLeftVertex->myPos;
	bottomRight->myPos.x += mySectorGridSize.x;
	bottomRight->myPos.y += mySectorGridSize.y;
	myVertices.Add(bottomRight);

	Edge* right = CreateEdge(topRight, bottomRight);
	Edge* bottom = CreateEdge(bottomLeft, bottomRight);
	Edge* middle = CreateEdge(bottomLeft, topRight);

	CreateTriangle(left, top, middle);
	CreateTriangle(right, bottom, middle);

	aTopLeftVertex = topRight;
	aLeftEdge = right;
	aTopEdge = bottom;
}

Navmesh::Edge* Navmesh::CreateEdge(Vertex* aV1, Vertex* aV2)
{
	Edge* edge = new Edge();
	edge->myVertices[0] = aV1;
	edge->myVertices[1] = aV2;
	aV1->myEdges.Add(edge);
	aV2->myEdges.Add(edge);
	myEdges.Add(edge);
	return edge;
}

Navmesh::Triangle* Navmesh::CreateTriangle(Edge* aE1, Edge* aE2, Edge* aE3)
{
	Triangle* triangle = new Triangle();
	triangle->myEdges[0] = aE1;
	triangle->myEdges[1] = aE2;
	triangle->myEdges[2] = aE3;

	triangle->myVertices[0] = aE1->myVertices[0];
	triangle->myVertices[1] = aE1->myVertices[1];
	triangle->myVertices[2] = aE2->GetSharedVertex(aE3);

	aE1->myTriangles.Add(triangle);
	aE2->myTriangles.Add(triangle);
	aE3->myTriangles.Add(triangle);
	myTriangles.Add(triangle);
	return triangle;
}

void Navmesh::Cut(const Vector2f& aV1, const Vector2f& aV2)
{
	Triangle* startTriangle = nullptr;
	Triangle* endTriangle = nullptr;

	for (Triangle* triangle : myTriangles)
	{
		if (triangle->PointInside(aV1))
		{
			FW_ASSERT(startTriangle == nullptr);
			startTriangle = triangle;
			if (endTriangle)
				break;
		}
		if (triangle->PointInside(aV2))
		{
			FW_ASSERT(endTriangle == nullptr);
			endTriangle = triangle;
			if (startTriangle)
				break;
		}
	}

	if (startTriangle && startTriangle == endTriangle)
	{
		SLUSH_INFO("Cut completely contained");

		Vertex* v1 = new Vertex();
		v1->myPos = aV1;

		Vertex* v2 = new Vertex();
		v2->myPos = aV2;

		Edge* cutEdge = CreateEdge(v1, v2);

		FW_Intersection::Ray ray;
		ray.myStart = aV1;
		ray.myDirection = GetNormalized(aV1 - aV2);

		Edge* intersectEdge1 = startTriangle->FindIntersectingEdge(ray);
		Edge* newEdge1 = CreateEdge(intersectEdge1->myVertices[0], v1);
		Edge* newEdge2 = CreateEdge(intersectEdge1->myVertices[1], v1);
		CreateTriangle(intersectEdge1, newEdge1, newEdge2);


		ray.myStart = aV2;
		ray.myDirection = GetNormalized(aV2 - aV1);
		Edge* intersectEdge2 = startTriangle->FindIntersectingEdge(ray);
		Edge* newEdge3 = CreateEdge(intersectEdge2->myVertices[0], v2);
		Edge* newEdge4 = CreateEdge(intersectEdge2->myVertices[1], v2);
		CreateTriangle(intersectEdge2, newEdge3, newEdge4);

		Vertex* sharedVertex = intersectEdge1->GetSharedVertex(intersectEdge2);
		Edge* e1 = GetEdgeWithVertex(sharedVertex, newEdge1, newEdge2);
		Edge* e2 = GetEdgeWithVertex(sharedVertex, newEdge3, newEdge4);
		CreateTriangle(e1, e2, cutEdge);

		Edge* opposite = startTriangle->GetEdgeWithoutVertex(sharedVertex);
		Edge* newOpposite1 = GetEdgeWithoutVertex(sharedVertex, newEdge1, newEdge2);
		Edge* newOpposite2 = GetEdgeWithoutVertex(sharedVertex, newEdge3, newEdge4);
		Edge* extraOpposite = CreateEdge(v1, newOpposite1->GetOtherUniqueVertex(v1, opposite));

		CreateTriangle(opposite, newOpposite1, extraOpposite);
		CreateTriangle(cutEdge, newOpposite2, extraOpposite);

		myTriangles.RemoveCyclic(startTriangle);
		FW_SAFE_DELETE(startTriangle);
	}
	else
	{
		SLUSH_INFO("Annoying cut");
	}
}

Navmesh::Triangle::~Triangle()
{
	for (int i = 0; i < 3; ++i)
	{
		myEdges[i]->RemoveTriangle(this);
	}
}

bool Navmesh::Triangle::PointInside(const Vector2f& aPoint) const
{
	return FW_Intersection::PointInsideTriangle(aPoint, myVertices[0]->myPos, myVertices[1]->myPos, myVertices[2]->myPos);
}

Navmesh::Edge* Navmesh::Triangle::FindIntersectingEdge(const FW_Intersection::Ray& aRay) const
{
	FW_Intersection::LineSegment edgeSegment;
	for (int i = 0; i < 3; ++i)
	{
		edgeSegment.myStart = myEdges[i]->myVertices[0]->myPos;
		edgeSegment.myEnd = myEdges[i]->myVertices[1]->myPos;
		if (FW_Intersection::RayVsLineSegment(aRay, edgeSegment))
			return myEdges[i];
	}

	FW_ASSERT_ALWAYS("Assuming that ray always hits for now, since this is only used for a ray starting inside the triangle");
	return nullptr;
}

Navmesh::Edge* Navmesh::Triangle::GetEdgeWithoutVertex(Vertex* aVertex) const
{
	if (myEdges[0]->myVertices[0] != aVertex && myEdges[0]->myVertices[1] != aVertex)
		return myEdges[0];

	if (myEdges[1]->myVertices[0] != aVertex && myEdges[1]->myVertices[1] != aVertex)
		return myEdges[1];

	if (myEdges[2]->myVertices[0] != aVertex && myEdges[2]->myVertices[1] != aVertex)
		return myEdges[2];

	FW_ASSERT_ALWAYS("No edge without vertex");
	return nullptr;
}

void Navmesh::Edge::RemoveTriangle(Triangle* aTriangle)
{
	myTriangles.RemoveCyclic(aTriangle);
}

Navmesh::Vertex* Navmesh::Edge::GetSharedVertex(Edge* aEdge) const
{
	if (myVertices[0] == aEdge->myVertices[0] || myVertices[0] == aEdge->myVertices[1])
		return myVertices[0];

	if (myVertices[1] == aEdge->myVertices[0] || myVertices[1] == aEdge->myVertices[1])
		return myVertices[1];

	FW_ASSERT_ALWAYS("No shared vertex found");
	return nullptr;
}

Navmesh::Vertex* Navmesh::Edge::GetOtherUniqueVertex(Vertex* aVertex, Edge* aEdge) const
{
	if (myVertices[0] != aEdge->myVertices[0] && myVertices[0] != aEdge->myVertices[1])
	{
		if (myVertices[0] != aVertex)
			return myVertices[0];
	}

	if (myVertices[1] != aEdge->myVertices[0] && myVertices[1] != aEdge->myVertices[1])
	{
		if (myVertices[1] != aVertex)
			return myVertices[1];
	}

	if (aEdge->myVertices[0] != myVertices[0] && aEdge->myVertices[0] != myVertices[1])
	{
		if (aEdge->myVertices[0] != aVertex)
			return aEdge->myVertices[0];
	}

	if (aEdge->myVertices[1] != myVertices[0] && aEdge->myVertices[1] != myVertices[1])
	{
		if (aEdge->myVertices[1] != aVertex)
			return aEdge->myVertices[1];
	}

	FW_ASSERT_ALWAYS("No unique vertex found");
	return nullptr;
}
