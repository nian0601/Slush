#include "stdafx.h"

#include "Navmesh.h"
#include "Graphics\Window.h"
#include "Core\Input.h"
#include "FW_Intersection.h"

Navmesh::Navmesh()
{
	Vector2f offset = { 20.f, 20. };
	Vertex* topLeft = CreateVertex(offset);

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
		myCutPositions.Add(engine.GetInput().GetMousePositionf());
	}
	else if (engine.GetInput().WasMouseReleased(Slush::Input::RIGHTMB))
	{
		PerformCut();
		myCutPositions.RemoveAll();
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
	
	int lineColor = 0xFF00FFFF;
	int faceColor = 0xAA00AAFF;

	FW_GrowingArray<Vector2f> intersections;

	for (Triangle* triangle : myTriangles)
	{
		bool collision = false;
		Vector2f intersection;
		if (!myCutPositions.IsEmpty())
		{
			FW_Intersection::LineSegment edge1{ triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos };
			FW_Intersection::LineSegment edge2{ triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos };
			FW_Intersection::LineSegment edge3{ triangle->myVertices[2]->myPos, triangle->myVertices[0]->myPos };

			for (int i = 0; i < myCutPositions.Count(); ++i)
			{
				FW_Intersection::LineSegment cutSegment;
				cutSegment.myStart = myCutPositions[i];
				
				if (i == myCutPositions.Count() - 1)
					cutSegment.myEnd = mousePos;
				else
					cutSegment.myEnd = myCutPositions[i+1];

				if (FW_Intersection::LineSegmentVsLineSegment(cutSegment, edge1, &intersection))
				{
					intersections.Add(intersection);
					collision = true;
				}

				if (FW_Intersection::LineSegmentVsLineSegment(cutSegment, edge2, &intersection))
				{
					intersections.Add(intersection);
					collision = true;
				}

				if (FW_Intersection::LineSegmentVsLineSegment(cutSegment, edge3, &intersection))
				{
					intersections.Add(intersection);
					collision = true;
				}
			}
			
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

	for (int i = 0; i < myCutPositions.Count() - 1; ++i)
	{
		window.RenderLine(myCutPositions[i], myCutPositions[i+1], 0xFF000000);
	}

	if (!myCutPositions.IsEmpty())
		window.RenderLine(myCutPositions.GetLast(), mousePos, 0xFF000000);
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
		Vector2f vertPos = aTopLeftVertex->myPos;
		vertPos.y += mySectorGridSize.y;

		bottomLeft = CreateVertex(vertPos);
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
		Vector2f vertPos = aTopLeftVertex->myPos;
		vertPos.x += mySectorGridSize.x;

		topRight = CreateVertex(vertPos);
		top = CreateEdge(aTopLeftVertex, topRight);
	}

	Vector2f vertPos = aTopLeftVertex->myPos;
	vertPos.x += mySectorGridSize.x;
	vertPos.y += mySectorGridSize.y;

	Vertex* bottomRight = CreateVertex(vertPos);

	Edge* right = CreateEdge(topRight, bottomRight);
	Edge* bottom = CreateEdge(bottomLeft, bottomRight);
	Edge* middle = CreateEdge(bottomLeft, topRight);

	CreateTriangle(left, top, middle);
	CreateTriangle(right, bottom, middle);

	aTopLeftVertex = topRight;
	aLeftEdge = right;
	aTopEdge = bottom;
}

Navmesh::Vertex* Navmesh::CreateVertex(const Vector2f aPosition)
{
	Vertex* vertex = new Vertex();
	vertex->myPos = aPosition;
	myVertices.Add(vertex);
	return vertex;
}

void Navmesh::DeleteVertexIfNeeded(Vertex* aVertex)
{
	if (!aVertex->myEdges.IsEmpty())
		return;

	myVertices.DeleteCyclic(aVertex);
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

void Navmesh::DeleteEdgeIfNeeded(Edge* aEdge)
{
	if (aEdge->myTriangles[0] != nullptr || aEdge->myTriangles[1] != nullptr)
		return;

	aEdge->myVertices[0]->myEdges.RemoveCyclic(aEdge);
	aEdge->myVertices[1]->myEdges.RemoveCyclic(aEdge);

	DeleteVertexIfNeeded(aEdge->myVertices[0]);
	DeleteVertexIfNeeded(aEdge->myVertices[1]);

	myEdges.DeleteCyclic(aEdge);
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

	aE1->AddTriangle(triangle);
	aE2->AddTriangle(triangle);
	aE3->AddTriangle(triangle);
	myTriangles.Add(triangle);
	return triangle;
}

void Navmesh::DeleteTriangle(Triangle* aTriangle)
{
	aTriangle->myEdges[0]->RemoveTriangle(aTriangle);
	DeleteEdgeIfNeeded(aTriangle->myEdges[0]);

	aTriangle->myEdges[1]->RemoveTriangle(aTriangle);
	DeleteEdgeIfNeeded(aTriangle->myEdges[1]);

	aTriangle->myEdges[2]->RemoveTriangle(aTriangle);
	DeleteEdgeIfNeeded(aTriangle->myEdges[2]);

	myTriangles.DeleteCyclic(aTriangle);
}

void Navmesh::PerformCut()
{
	if (myCutPositions.Count() < 3)
		return;

	myCutPositions.Add(myCutPositions[0]);
	for (int i = 0; i < myCutPositions.Count() - 1; ++i)
	{
		Cut(myCutPositions[i], myCutPositions[i + 1]);
	}

	for (int i = 0; i < myTriangles.Count(); ++i)
	{
		if (IsInsideCutArea(myTriangles[i]))
		{
			DeleteTriangle(myTriangles[i]);
			--i;
		}
	}
}

void Navmesh::Cut(const Vector2f& aV1, const Vector2f& aV2)
{
	// There is not guarantee that there will be a Vertex created exactly at aV1 and aV2, so the
	// further away from an edge those postions are within a triangle, the less accurate the cut becomes
	// in that triangle.
	// Ideally a Vertex should always be created on those specific positions, but that also makes the
	// cutting more complex.

	FW_Intersection::LineSegment cuttingLine;
	cuttingLine.myStart = aV1;
	cuttingLine.myEnd = aV2;

	FW_GrowingArray<CutEdge> cuttingEdges;
	CollectCutEdges(cuttingLine, cuttingEdges);

	FW_GrowingArray<Triangle*> oldTriangles;
	for (CutEdge& cutEdge : cuttingEdges)
	{
		Vertex* cutVertex = CreateVertex(cutEdge.myCutPosition);

		Edge* newEdge1 = CreateEdge(cutEdge.myEdge->myVertices[0], cutVertex);
		Edge* newEdge2 = CreateEdge(cutVertex, cutEdge.myEdge->myVertices[1]);
		CutTriangle(cutEdge.myEdge->myTriangles[0], cutEdge.myEdge, cutVertex, newEdge1, newEdge2);
		CutTriangle(cutEdge.myEdge->myTriangles[1], cutEdge.myEdge, cutVertex, newEdge1, newEdge2);

		oldTriangles.Add(cutEdge.myEdge->myTriangles[0]);
		oldTriangles.Add(cutEdge.myEdge->myTriangles[1]);
	}

	for (Triangle* oldTriangle : oldTriangles)
	{
		if (oldTriangle)
			DeleteTriangle(oldTriangle);
	}
}

void Navmesh::CollectCutEdges(const FW_Intersection::LineSegment& aCuttingLine, FW_GrowingArray<CutEdge>& outCutEdges) const
{
	FW_Intersection::LineSegment edgeSegment;
	Vector2f intersectionPoint;
	for (Edge* edge : myEdges)
	{
		edgeSegment.myStart = edge->myVertices[0]->myPos;
		edgeSegment.myEnd = edge->myVertices[1]->myPos;
		if (FW_Intersection::LineSegmentVsLineSegment(aCuttingLine, edgeSegment, &intersectionPoint))
		{
			CutEdge& cutEdge = outCutEdges.Add();
			cutEdge.myEdge = edge;
			cutEdge.myCutPosition = intersectionPoint;
		}
	}
}

void Navmesh::CutTriangle(Triangle* aTriangle, Edge* aCutEdge, Vertex* aCutVertex, Edge* aNewEdge1, Edge* aNewEdge2)
{
	if (!aTriangle)
		return;

	Vertex* oppositeVertex = aTriangle->GetOtherVertex(aCutEdge);

	Edge* centerEdge = CreateEdge(aCutVertex, oppositeVertex);

	Edge* oldEdge1 = aTriangle->GetOtherEdge(aCutEdge, aCutEdge->myVertices[0]);
	oldEdge1->RemoveTriangle(aTriangle);
	CreateTriangle(aNewEdge1, oldEdge1, centerEdge);

	Edge* oldEdge2 = aTriangle->GetOtherEdge(aCutEdge, aCutEdge->myVertices[1]);
	oldEdge2->RemoveTriangle(aTriangle);
	CreateTriangle(centerEdge, oldEdge2, aNewEdge2);
}

bool Navmesh::IsInsideCutArea(Triangle* aTriangle)
{
	Vector2f center = aTriangle->GetCenterPosition();

	for (int i = 0; i < myCutPositions.Count() - 1; ++i)
	{
		FW_Intersection::LineSegment cutSegment;
		cutSegment.myStart = myCutPositions[i];
		cutSegment.myEnd = myCutPositions[i + 1];

		FW_Intersection::Line cutLine;
		cutLine.FromSegment(cutSegment);

		Vector2f cutCenter = (cutSegment.myStart + cutSegment.myEnd) / 2.f;
		
		Vector2f triangleToCutCenter = cutCenter - center;
		Normalize(triangleToCutCenter);

		if (Dot(cutLine.myNormal, triangleToCutCenter) > 0.f)
			return false;
	}

	return true;
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
	for (int i = 0; i < 3; ++i)
	{
		Edge* edge = myEdges[i];

		if (edge->myVertices[0] != aVertex && edge->myVertices[1] != aVertex)
			return edge;
	}

	FW_ASSERT_ALWAYS("No edge without vertex");
	return nullptr;
}

Navmesh::Edge* Navmesh::Triangle::GetOtherEdge(Edge* aEdge, Vertex* aVertex) const
{
	for (int i = 0; i < 3; ++i)
	{
		Edge* otherEdge = myEdges[i];
		if (otherEdge == aEdge)
			continue;

		if (otherEdge->myVertices[0] == aVertex || otherEdge->myVertices[1] == aVertex)
			return otherEdge;
	}

	FW_ASSERT_ALWAYS("No other edge");
	return nullptr;
}

Navmesh::Vertex* Navmesh::Triangle::GetOtherVertex(Edge* aEdge) const
{
	for (int i = 0; i < 3; ++i)
	{
		Vertex* vert = myVertices[i];
		if (vert != aEdge->myVertices[0] && vert != aEdge->myVertices[1])
			return vert;
	}

	FW_ASSERT_ALWAYS("Couldnt find OtherVertex");
	return nullptr;
}

Vector2f Navmesh::Triangle::GetCenterPosition() const
{
	Vector2f pos;
	pos += myVertices[0]->myPos;
	pos += myVertices[1]->myPos;
	pos += myVertices[2]->myPos;
	pos /= 3.f;
	return pos;
}

void Navmesh::Edge::AddTriangle(Triangle* aTriangle)
{
	if (myTriangles[0] == nullptr)
		myTriangles[0] = aTriangle;
	else if (myTriangles[1] == nullptr)
		myTriangles[1] = aTriangle;
	else
		FW_ASSERT_ALWAYS("Tried to add more than two triangles to an edge");
}

bool Navmesh::Edge::RemoveTriangle(Triangle* aTriangle)
{
	if (myTriangles[0] == aTriangle)
		myTriangles[0] = nullptr;
	else if (myTriangles[1] == aTriangle)
		myTriangles[1] = nullptr;

	return myTriangles[0] == nullptr && myTriangles[1] == nullptr;
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
