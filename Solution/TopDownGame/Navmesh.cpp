#include "stdafx.h"

#include "Navmesh.h"
#include "Graphics\Window.h"
#include "Graphics\Renderer.h"
#include "Core\Input.h"
#include "FW_Intersection.h"

Navmesh::Navmesh()
{
}

Navmesh::~Navmesh()
{
	myTriangles.DeleteAll();
	myEdges.DeleteAll();
	myVertices.DeleteAll();
}

void Navmesh::GenerateDefaultGrid()
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

void Navmesh::Save(Slush::AssetParser::Handle aRootHandle) const
{
	// Edges are never stored directly - an edge is always exactly "the edge shared by two
	// triangles" (or unshared, on the mesh boundary) and gets reconstructed from the triangle
	// list on Load(), via FindOrCreateEdge().
	Slush::AssetParser::Handle verticesHandle = aRootHandle.ParseChildElement("vertices");
	for (Vertex* vertex : myVertices)
	{
		Slush::AssetParser::Handle vertexHandle = verticesHandle.ParseChildElement("vertex");
		vertexHandle.ParseVec2fField("pos", vertex->myPos);
	}

	Slush::AssetParser::Handle trianglesHandle = aRootHandle.ParseChildElement("triangles");
	for (Triangle* triangle : myTriangles)
	{
		Slush::AssetParser::Handle triangleHandle = trianglesHandle.ParseChildElement("triangle");
		int indices[3];
		for (int i = 0; i < 3; ++i)
			indices[i] = myVertices.Find(triangle->myVertices[i]);

		triangleHandle.ParseIntField("v0", indices[0]);
		triangleHandle.ParseIntField("v1", indices[1]);
		triangleHandle.ParseIntField("v2", indices[2]);
	}
}

void Navmesh::Load(Slush::AssetParser::Handle aRootHandle)
{
	myTriangles.DeleteAll();
	myEdges.DeleteAll();
	myVertices.DeleteAll();

	Slush::AssetParser::Handle verticesHandle = aRootHandle.ParseChildElement("vertices");
	int numVertices = verticesHandle.GetNumChildElements();
	FW_GrowingArray<Vertex*> loadedVertices;
	loadedVertices.Reserve(numVertices);
	for (int i = 0; i < numVertices; ++i)
	{
		Slush::AssetParser::Handle vertexHandle = verticesHandle.GetChildElementAtIndex(i);
		Vector2f pos;
		vertexHandle.ParseVec2fField("pos", pos);
		loadedVertices[i] = CreateVertex(pos);
	}

	Slush::AssetParser::Handle trianglesHandle = aRootHandle.ParseChildElement("triangles");
	int numTriangles = trianglesHandle.GetNumChildElements();
	for (int i = 0; i < numTriangles; ++i)
	{
		Slush::AssetParser::Handle triangleHandle = trianglesHandle.GetChildElementAtIndex(i);
		int indices[3] = {};
		triangleHandle.ParseIntField("v0", indices[0]);
		triangleHandle.ParseIntField("v1", indices[1]);
		triangleHandle.ParseIntField("v2", indices[2]);

		Vertex* v0 = loadedVertices[indices[0]];
		Vertex* v1 = loadedVertices[indices[1]];
		Vertex* v2 = loadedVertices[indices[2]];

		Edge* e0 = FindOrCreateEdge(v0, v1);
		Edge* e1 = FindOrCreateEdge(v1, v2);
		Edge* e2 = FindOrCreateEdge(v2, v0);

		CreateTriangle(e0, e1, e2);
	}
}

void Navmesh::Update()
{
	Slush::Engine& engine = Slush::Engine::GetInstance();

	if (myBoxCutModeActive)
	{
		if (engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		{
			if (!myBoxCutHasStartCorner)
			{
				myBoxCutStartCorner = engine.GetInput().GetMousePositionf();
				myBoxCutHasStartCorner = true;
			}
			else
			{
				CommitBoxCut(myBoxCutStartCorner, engine.GetInput().GetMousePositionf());
				myBoxCutHasStartCorner = false;
			}
		}
	}
	else if (myManualCutModeActive)
	{
		if (engine.GetInput().WasMouseReleased(Slush::Input::LEFTMB))
		{
			myCutPositions.Add(engine.GetInput().GetMousePositionf());
		}
		else if (engine.GetInput().WasMouseReleased(Slush::Input::RIGHTMB))
		{
			PerformCut();
			myCutPositions.RemoveAll();
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
	Slush::Renderer& renderer = Slush::Engine::GetInstance().GetWindow().GetRenderer();

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
			renderer.RenderTriangle(triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos, 0xFFFFFFFF);
		}
		else
		{
			renderer.RenderTriangle(triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos, faceColor);
		}

	}

	for (Edge* edge : myEdges)
	{
		renderer.RenderLine(edge->myVertices[0]->myPos, edge->myVertices[1]->myPos, lineColor);
	}

	for (const Vector2f& intersect : intersections)
		renderer.RenderCircle(intersect, 3.f, 0xFFFF0000);

	for (int i = 0; i < myCutPositions.Count() - 1; ++i)
	{
		renderer.RenderLine(myCutPositions[i], myCutPositions[i+1], 0xFF000000);
	}

	if (!myCutPositions.IsEmpty())
		renderer.RenderLine(myCutPositions.GetLast(), mousePos, 0xFF000000);

	if (myBoxCutModeActive && myBoxCutHasStartCorner)
	{
		const Vector2f topLeft{ FW_Min(myBoxCutStartCorner.x, mousePos.x), FW_Min(myBoxCutStartCorner.y, mousePos.y) };
		const Vector2f bottomRight{ FW_Max(myBoxCutStartCorner.x, mousePos.x), FW_Max(myBoxCutStartCorner.y, mousePos.y) };
		const Vector2f topRight{ bottomRight.x, topLeft.y };
		const Vector2f bottomLeft{ topLeft.x, bottomRight.y };

		const int boxPreviewColor = 0xFF00FF00;
		renderer.RenderLine(topLeft, topRight, boxPreviewColor);
		renderer.RenderLine(topRight, bottomRight, boxPreviewColor);
		renderer.RenderLine(bottomRight, bottomLeft, boxPreviewColor);
		renderer.RenderLine(bottomLeft, topLeft, boxPreviewColor);
	}
}

Navmesh::Triangle* Navmesh::FindTriangleContaining(const Vector2f& aPosition) const
{
	for (Triangle* triangle : myTriangles)
	{
		if (triangle->PointInside(aPosition))
			return triangle;
	}

	return nullptr;
}

bool Navmesh::FindTrianglePath(const Vector2f& aStart, Triangle* aStartTriangle, Triangle* aGoalTriangle, PathCorridor& outCorridor) const
{
	FW_GrowingArray<AStarNode> nodes;

	AStarNode& startNode = nodes.Add();
	startNode.myTriangle = aStartTriangle;
	startNode.myGCost = 0.f;
	startNode.myFCost = Length(aStartTriangle->GetCenterPosition() - aGoalTriangle->GetCenterPosition());

	while (true)
	{
		int bestIndex = -1;
		float bestFCost = 0.f;
		for (int i = 0; i < nodes.Count(); ++i)
		{
			if (nodes[i].myIsClosed)
				continue;

			if (bestIndex == -1 || nodes[i].myFCost < bestFCost)
			{
				bestIndex = i;
				bestFCost = nodes[i].myFCost;
			}
		}

		if (bestIndex == -1)
			return false;

		nodes[bestIndex].myIsClosed = true;

		if (nodes[bestIndex].myTriangle == aGoalTriangle)
		{
			FW_GrowingArray<Edge*> reversedEdges;
			int index = bestIndex;
			while (nodes[index].myParentIndex != -1)
			{
				reversedEdges.Add(nodes[index].myEdgeFromParent);
				index = nodes[index].myParentIndex;
			}

			Vertex* prevLeft = nullptr;
			Vertex* prevRight = nullptr;
			for (int i = reversedEdges.Count() - 1; i >= 0; --i)
			{
				Edge* edge = reversedEdges[i];
				Vertex* v0 = edge->myVertices[0];
				Vertex* v1 = edge->myVertices[1];

				Vertex* left = nullptr;
				Vertex* right = nullptr;

				if (prevLeft == nullptr && prevRight == nullptr)
				{
					// Orient the first portal so it satisfies the funnel algorithm's own
					// apex/left/right invariant with the initial apex (aStart): the sweep
					// from left to right around aStart must be non-negative (CCW-or-degenerate).
					if (Cross(v0->myPos - aStart, v1->myPos - aStart) >= 0.f)
					{
						left = v0;
						right = v1;
					}
					else
					{
						left = v1;
						right = v0;
					}
				}
				else if (v0 == prevLeft)
				{
					left = v0;
					right = v1;
				}
				else if (v0 == prevRight)
				{
					right = v0;
					left = v1;
				}
				else if (v1 == prevLeft)
				{
					left = v1;
					right = v0;
				}
				else if (v1 == prevRight)
				{
					right = v1;
					left = v0;
				}
				else
				{
					FW_ASSERT_ALWAYS("Expected consecutive portal edges to share a vertex");
					left = v0;
					right = v1;
				}

				Portal& portal = outCorridor.myPortals.Add();
				portal.myLeft = left->myPos;
				portal.myRight = right->myPos;

				prevLeft = left;
				prevRight = right;
			}

			return true;
		}

		Triangle* currentTriangle = nodes[bestIndex].myTriangle;
		Vector2f currentCenter = currentTriangle->GetCenterPosition();
		float currentGCost = nodes[bestIndex].myGCost;

		for (int e = 0; e < 3; ++e)
		{
			Edge* edge = currentTriangle->myEdges[e];

			for (int t = 0; t < 2; ++t)
			{
				Triangle* neighbor = edge->myTriangles[t];
				if (neighbor == nullptr || neighbor == currentTriangle)
					continue;

				float tentativeGCost = currentGCost + Length(neighbor->GetCenterPosition() - currentCenter);

				int existingIndex = -1;
				for (int i = 0; i < nodes.Count(); ++i)
				{
					if (nodes[i].myTriangle == neighbor)
					{
						existingIndex = i;
						break;
					}
				}

				if (existingIndex != -1)
				{
					if (tentativeGCost >= nodes[existingIndex].myGCost)
						continue;

					nodes[existingIndex].myGCost = tentativeGCost;
					nodes[existingIndex].myFCost = tentativeGCost + Length(neighbor->GetCenterPosition() - aGoalTriangle->GetCenterPosition());
					nodes[existingIndex].myParentIndex = bestIndex;
					nodes[existingIndex].myEdgeFromParent = edge;
					nodes[existingIndex].myIsClosed = false;
				}
				else
				{
					AStarNode& newNode = nodes.Add();
					newNode.myTriangle = neighbor;
					newNode.myGCost = tentativeGCost;
					newNode.myFCost = tentativeGCost + Length(neighbor->GetCenterPosition() - aGoalTriangle->GetCenterPosition());
					newNode.myParentIndex = bestIndex;
					newNode.myEdgeFromParent = edge;
				}
			}
		}
	}
}

bool Navmesh::BuildEdgeCenterPath(const Vector2f& aStart, const Vector2f& aGoal, const PathCorridor& aCorridor, FW_GrowingArray<Vector2f>& outWaypoints) const
{
	outWaypoints.RemoveAll();
	outWaypoints.Add(aStart);

	for (const Portal& portal : aCorridor.myPortals)
		outWaypoints.Add((portal.myLeft + portal.myRight) / 2.f);

	outWaypoints.Add(aGoal);
	return true;
}

namespace
{
	float TriArea2(const Vector2f& aA, const Vector2f& aB, const Vector2f& aC)
	{
		return Cross(aB - aA, aC - aA);
	}
}

bool Navmesh::StringPull(const Vector2f& aStart, const Vector2f& aGoal, const PathCorridor& aCorridor, FW_GrowingArray<Vector2f>& outWaypoints) const
{
	outWaypoints.RemoveAll();

	if (aCorridor.myPortals.IsEmpty())
	{
		outWaypoints.Add(aStart);
		outWaypoints.Add(aGoal);
		return true;
	}

	// "Simple Stupid Funnel Algorithm": portals[0] and portals[Count()-1] are degenerate
	// (both sides equal to aStart/aGoal) so the loop below can treat every step uniformly.
	FW_GrowingArray<Vector2f> portalLefts;
	FW_GrowingArray<Vector2f> portalRights;
	portalLefts.Add(aStart);
	portalRights.Add(aStart);
	for (const Portal& portal : aCorridor.myPortals)
	{
		portalLefts.Add(portal.myLeft);
		portalRights.Add(portal.myRight);
	}
	portalLefts.Add(aGoal);
	portalRights.Add(aGoal);

	outWaypoints.Add(aStart);

	Vector2f apex = aStart;
	Vector2f left = portalLefts[0];
	Vector2f right = portalRights[0];
	int apexIndex = 0;
	int leftIndex = 0;
	int rightIndex = 0;

	for (int i = 1; i < portalLefts.Count(); ++i)
	{
		const Vector2f& newLeft = portalLefts[i];
		const Vector2f& newRight = portalRights[i];

		if (TriArea2(apex, right, newRight) <= 0.f)
		{
			if (apex == right || TriArea2(apex, left, newRight) > 0.f)
			{
				right = newRight;
				rightIndex = i;
			}
			else
			{
				outWaypoints.Add(left);

				apex = left;
				apexIndex = leftIndex;
				left = apex;
				right = apex;
				leftIndex = apexIndex;
				rightIndex = apexIndex;

				i = apexIndex;
				continue;
			}
		}

		if (TriArea2(apex, left, newLeft) >= 0.f)
		{
			if (apex == left || TriArea2(apex, right, newLeft) < 0.f)
			{
				left = newLeft;
				leftIndex = i;
			}
			else
			{
				outWaypoints.Add(right);

				apex = right;
				apexIndex = rightIndex;
				left = apex;
				right = apex;
				leftIndex = apexIndex;
				rightIndex = apexIndex;

				i = apexIndex;
				continue;
			}
		}
	}

	outWaypoints.Add(aGoal);
	return true;
}

bool Navmesh::FindPath(const Vector2f& aStart, const Vector2f& aGoal, FW_GrowingArray<Vector2f>& outWaypoints, PathCorridor& outCorridor) const
{
	Triangle* startTriangle = FindTriangleContaining(aStart);
	Triangle* goalTriangle = FindTriangleContaining(aGoal);
	if (startTriangle == nullptr || goalTriangle == nullptr)
		return false;

	PathCorridor foundCorridor;
	if (startTriangle != goalTriangle)
	{
		if (!FindTrianglePath(aStart, startTriangle, goalTriangle, foundCorridor))
			return false;
	}

	// Build into a local corridor and only hand it to the caller once the search has
	// actually succeeded, so a reused outCorridor is replaced rather than appended to
	// (guaranteeing the zero-portal same-triangle case) and stays untouched on failure.
	outCorridor = foundCorridor;
	return StringPull(aStart, aGoal, outCorridor, outWaypoints);
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

Navmesh::Edge* Navmesh::FindOrCreateEdge(Vertex* aV1, Vertex* aV2)
{
	for (Edge* edge : aV1->myEdges)
	{
		if ((edge->myVertices[0] == aV1 && edge->myVertices[1] == aV2) ||
			(edge->myVertices[0] == aV2 && edge->myVertices[1] == aV1))
			return edge;
	}

	return CreateEdge(aV1, aV2);
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

	CutPolygon(myCutPositions);
}

void Navmesh::CutHole(const FW_GrowingArray<Vector2f>& aPolygon)
{
	if (aPolygon.Count() < 3)
		return;

	CutPolygon(aPolygon);
}

void Navmesh::SetBoxCutModeActive(bool anIsActive)
{
	myBoxCutModeActive = anIsActive;
	myBoxCutHasStartCorner = false;
}

void Navmesh::SetManualCutModeActive(bool anIsActive)
{
	myManualCutModeActive = anIsActive;

	if (!anIsActive)
		myCutPositions.RemoveAll();
}

int Navmesh::GetTriangleCount() const
{
	return myTriangles.Count();
}

int Navmesh::GetVertexCount() const
{
	return myVertices.Count();
}

bool Navmesh::HasVertexNear(const Vector2f& aPosition, float anEpsilon) const
{
	return FindNearbyVertex(aPosition, anEpsilon) != nullptr;
}

void Navmesh::CommitBoxCut(const Vector2f& aStartCorner, const Vector2f& anEndCorner)
{
	const Vector2f topLeft{ FW_Min(aStartCorner.x, anEndCorner.x), FW_Min(aStartCorner.y, anEndCorner.y) };
	const Vector2f bottomRight{ FW_Max(aStartCorner.x, anEndCorner.x), FW_Max(aStartCorner.y, anEndCorner.y) };

	FW_GrowingArray<Vector2f> boxCorners;
	boxCorners.Add(topLeft);
	boxCorners.Add(Vector2f{ bottomRight.x, topLeft.y });
	boxCorners.Add(bottomRight);
	boxCorners.Add(Vector2f{ topLeft.x, bottomRight.y });

	CutHole(boxCorners);
}

void Navmesh::CutPolygon(const FW_GrowingArray<Vector2f>& aPolygonPoints)
{
	EnsureCutterVerticesExist(aPolygonPoints);

	FW_GrowingArray<Vector2f> closedPolygon = aPolygonPoints;
	closedPolygon.Add(closedPolygon[0]);

	for (int i = 0; i < closedPolygon.Count() - 1; ++i)
	{
		Cut(closedPolygon[i], closedPolygon[i + 1]);
	}

	for (int i = 0; i < myTriangles.Count(); ++i)
	{
		if (IsInsideCutArea(closedPolygon, myTriangles[i]))
		{
			DeleteTriangle(myTriangles[i]);
			--i;
		}
	}
}

namespace
{
	const float CutterVertexEpsilon = 1.f;
}

void Navmesh::EnsureCutterVerticesExist(const FW_GrowingArray<Vector2f>& aPolygonPoints)
{
	// A cutter shape's own corner points are never guaranteed to land on an existing mesh edge, so
	// without this step Cut()'s edge-crossing logic below only ever creates vertices where a cut
	// segment crosses an *existing* edge - the shape's own corners can fall anywhere inside a
	// triangle and drift from the clicked/authored positions. Snapping within CutterVertexEpsilon
	// instead of always inserting exactly avoids scattering near-duplicate vertices/edges a hair's
	// width apart from whatever's already there (e.g. re-cutting close to a previous cut's own
	// corners, or a corner that's already effectively on a shared edge/vertex by construction).
	// The remaining imprecision is bounded by the epsilon rather than by triangle size.
	for (const Vector2f& point : aPolygonPoints)
	{
		if (FindNearbyVertex(point, CutterVertexEpsilon) != nullptr)
			continue;

		if (Edge* nearbyEdge = FindNearbyEdge(point, CutterVertexEpsilon))
		{
			SplitEdgeAtPosition(nearbyEdge, point);
			continue;
		}

		if (Triangle* triangle = FindTriangleContaining(point))
			InsertVertexInTriangle(triangle, point);
	}
}

void Navmesh::Cut(const Vector2f& aV1, const Vector2f& aV2)
{
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
	// LineSegmentVsLineSegment's general (non-parallel) case is endpoint-inclusive, so a cutting
	// segment that starts/ends exactly on an existing vertex (as EnsureCutterVerticesExist now
	// guarantees) reports a spurious "crossing" against every edge incident to that vertex, right
	// at the segment's own endpoint. That's not an interior crossing needing a new vertex there -
	// the vertex already exists - so skip any hit that lands on the cutting segment's own start/end.
	const float DegenerateEpsilonSq = 0.0001f;

	FW_Intersection::LineSegment edgeSegment;
	Vector2f intersectionPoint;
	for (Edge* edge : myEdges)
	{
		edgeSegment.myStart = edge->myVertices[0]->myPos;
		edgeSegment.myEnd = edge->myVertices[1]->myPos;
		if (FW_Intersection::LineSegmentVsLineSegment(aCuttingLine, edgeSegment, &intersectionPoint))
		{
			if (Length2(intersectionPoint - aCuttingLine.myStart) <= DegenerateEpsilonSq)
				continue;
			if (Length2(intersectionPoint - aCuttingLine.myEnd) <= DegenerateEpsilonSq)
				continue;

			CutEdge& cutEdge = outCutEdges.Add();
			cutEdge.myEdge = edge;
			cutEdge.myCutPosition = intersectionPoint;
		}
	}
}

Navmesh::Vertex* Navmesh::InsertVertexInTriangle(Triangle* aTriangle, const Vector2f& aPosition)
{
	Vertex* newVertex = CreateVertex(aPosition);

	Vertex* v0 = aTriangle->myVertices[0];
	Vertex* v1 = aTriangle->myVertices[1];
	Vertex* v2 = aTriangle->myVertices[2];

	Edge* edge01 = aTriangle->GetEdgeWithoutVertex(v2);
	Edge* edge12 = aTriangle->GetEdgeWithoutVertex(v0);
	Edge* edge20 = aTriangle->GetEdgeWithoutVertex(v1);

	Edge* spoke0 = CreateEdge(newVertex, v0);
	Edge* spoke1 = CreateEdge(newVertex, v1);
	Edge* spoke2 = CreateEdge(newVertex, v2);

	myTriangles.DeleteCyclic(aTriangle);

	CreateTriangle(edge01, spoke0, spoke1);
	CreateTriangle(edge12, spoke1, spoke2);
	CreateTriangle(edge20, spoke2, spoke0);

	return newVertex;
}

Navmesh::Vertex* Navmesh::SplitEdgeAtPosition(Edge* aEdge, const Vector2f& aPosition)
{
	Vertex* cutVertex = CreateVertex(aPosition);

	Edge* newEdge1 = CreateEdge(aEdge->myVertices[0], cutVertex);
	Edge* newEdge2 = CreateEdge(cutVertex, aEdge->myVertices[1]);

	Triangle* triangle0 = aEdge->myTriangles[0];
	Triangle* triangle1 = aEdge->myTriangles[1];

	CutTriangle(triangle0, aEdge, cutVertex, newEdge1, newEdge2);
	CutTriangle(triangle1, aEdge, cutVertex, newEdge1, newEdge2);

	if (triangle0)
		DeleteTriangle(triangle0);
	if (triangle1)
		DeleteTriangle(triangle1);

	return cutVertex;
}

Navmesh::Vertex* Navmesh::FindNearbyVertex(const Vector2f& aPosition, float anEpsilon) const
{
	for (Vertex* vertex : myVertices)
	{
		if (Length2(vertex->myPos - aPosition) <= anEpsilon * anEpsilon)
			return vertex;
	}

	return nullptr;
}

namespace
{
	float DistancePointToSegmentSq(const Vector2f& aPoint, const Vector2f& aSegmentStart, const Vector2f& aSegmentEnd)
	{
		Vector2f segmentDir = aSegmentEnd - aSegmentStart;
		float segmentLength2 = Length2(segmentDir);
		if (segmentLength2 == 0.f)
			return Length2(aPoint - aSegmentStart);

		float t = FW_Clamp(Dot(aPoint - aSegmentStart, segmentDir) / segmentLength2, 0.f, 1.f);
		Vector2f closestPoint = aSegmentStart + segmentDir * t;
		return Length2(aPoint - closestPoint);
	}
}

Navmesh::Edge* Navmesh::FindNearbyEdge(const Vector2f& aPosition, float anEpsilon) const
{
	for (Edge* edge : myEdges)
	{
		if (DistancePointToSegmentSq(aPosition, edge->myVertices[0]->myPos, edge->myVertices[1]->myPos) <= anEpsilon * anEpsilon)
			return edge;
	}

	return nullptr;
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

bool Navmesh::IsInsideCutArea(const FW_GrowingArray<Vector2f>& aCutPositions, Triangle* aTriangle)
{
	Vector2f center = aTriangle->GetCenterPosition();

	for (int i = 0; i < aCutPositions.Count() - 1; ++i)
	{
		FW_Intersection::LineSegment cutSegment;
		cutSegment.myStart = aCutPositions[i];
		cutSegment.myEnd = aCutPositions[i + 1];

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
