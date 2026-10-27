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
	Clear();
}

void Navmesh::CopyFrom(const Navmesh& aOther)
{
	if (this == &aOther)
		return;

	Clear();
	mySectorGrid = aOther.mySectorGrid;
	mySectorGridSize = aOther.mySectorGridSize;

	for (Vertex* vertex : aOther.myVertices)
		CreateVertex(vertex->myPos);

	for (Triangle* triangle : aOther.myTriangles)
	{
		Vertex* v0 = myVertices[aOther.myVertices.Find(triangle->myVertices[0])];
		Vertex* v1 = myVertices[aOther.myVertices.Find(triangle->myVertices[1])];
		Vertex* v2 = myVertices[aOther.myVertices.Find(triangle->myVertices[2])];

		Edge* e0 = FindOrCreateEdge(v0, v1);
		Edge* e1 = FindOrCreateEdge(v1, v2);
		Edge* e2 = FindOrCreateEdge(v2, v0);
		CreateTriangle(e0, e1, e2);
	}
}

void Navmesh::GenerateDefaultGrid()
{
	Clear();

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
	Clear();

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

void Navmesh::GetAsTriangleMesh(TriangleMesh& aMesh) const
{
	aMesh.myVertices.RemoveAll();

	for (Triangle* triangle : myTriangles)
	{
		aMesh.myVertices.Add(triangle->myVertices[0]->myPos);
		aMesh.myVertices.Add(triangle->myVertices[1]->myPos);
		aMesh.myVertices.Add(triangle->myVertices[2]->myPos);
	}
}

void Navmesh::Render(unsigned int aFlags)
{
	Slush::Engine& engine = Slush::Engine::GetInstance();
	Slush::Renderer& renderer = Slush::Engine::GetInstance().GetWindow().GetRenderer();

	const Vector2f& mousePos = engine.GetInput().GetMousePositionf();

	int lineColor = 0xFF00FFFF;
	int faceColor = 0xAA00AAFF;

	if ((aFlags & RenderFlags::FACES) > 0)
	{
		const bool showMouseTest = (aFlags & RenderFlags::MOUSE_TEST) > 0;
		for (Triangle* triangle : myTriangles)
		{
			bool collision = showMouseTest && triangle->PointInside(mousePos);

			if (collision)
			{
				renderer.RenderTriangle(triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos, 0xFFFFFFFF);
			}
			else
			{
				renderer.RenderTriangle(triangle->myVertices[0]->myPos, triangle->myVertices[1]->myPos, triangle->myVertices[2]->myPos, faceColor);
			}

		}
	}

	if ((aFlags & RenderFlags::EDGES) > 0)
	{
		for (Edge* edge : myEdges)
		{
			renderer.RenderLine(edge->myVertices[0]->myPos, edge->myVertices[1]->myPos, lineColor);
		}
	}

	if ((aFlags & RenderFlags::OUTLINE) > 0)
	{
		for (Edge* edge : myEdges)
		{
			if (!edge->myTriangles[0] || !edge->myTriangles[1])
				renderer.RenderLine(edge->myVertices[0]->myPos, edge->myVertices[1]->myPos, lineColor);
		}
	}
}

void Navmesh::Clear()
{
	DropClearanceCaches();
	myTriangles.DeleteAll();
	myEdges.DeleteAll();
	myVertices.DeleteAll();
}

void Navmesh::AssignIndices() const
{
	for (int i = 0; i < myTriangles.Count(); ++i)
		myTriangles[i]->myIndex = i;

	for (int i = 0; i < myEdges.Count(); ++i)
		myEdges[i]->myIndex = i;
}

void Navmesh::DropClearanceCaches()
{
	myClearanceCaches.DeleteAll();
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

bool Navmesh::FindTrianglePath(const Vector2f& aStart, const Vector2f& aGoal, Triangle* aStartTriangle, Triangle* aGoalTriangle, float anAgentRadius, PathCorridor& outCorridor) const
{
	AssignIndices();
	ClearanceCache* clearanceCache = anAgentRadius > 0.f ? &GetClearanceCache(anAgentRadius) : nullptr;

	// The node for each (triangle, entry edge) pair, by Triangle::myIndex * 3 + the edge's slot in myEdges.
	FW_GrowingArray<int> nodeIndexByEntry;
	nodeIndexByEntry.Reserve(myTriangles.Count() * 3);
	for (int& nodeIndex : nodeIndexByEntry)
		nodeIndex = -1;

	FW_GrowingArray<AStarNode> nodes;

	AStarNode& startNode = nodes.Add();
	startNode.myTriangle = aStartTriangle;
	startNode.myPosition = aStart;
	startNode.myGCost = 0.f;
	startNode.myFCost = Length(aStart - aGoal);

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
				portal.myLeftWallOffset = GetWallCornerOffset(left, edge);
				portal.myRightWallOffset = GetWallCornerOffset(right, edge);

				prevLeft = left;
				prevRight = right;
			}

			return true;
		}

		Triangle* currentTriangle = nodes[bestIndex].myTriangle;
		Edge* entryEdge = nodes[bestIndex].myEdgeFromParent;
		Vector2f currentPosition = nodes[bestIndex].myPosition;
		float currentGCost = nodes[bestIndex].myGCost;

		for (int e = 0; e < 3; ++e)
		{
			Edge* edge = currentTriangle->myEdges[e];
			if (edge == entryEdge)
				continue;

			for (int t = 0; t < 2; ++t)
			{
				Triangle* neighbor = edge->myTriangles[t];
				if (neighbor == nullptr || neighbor == currentTriangle)
					continue;

				if (clearanceCache != nullptr && !CanCrossEdge(*clearanceCache, currentTriangle, entryEdge, edge))
					continue;

				// Cost runs between points on the crossed edges rather than triangle centres, so the chosen
				// chain is one the funnel can pull tight around corners (see FindPath).
				const Vector2f position = GetSearchPointOnPortal(edge, currentPosition, aGoal, anAgentRadius);
				const float tentativeGCost = currentGCost + Length(position - currentPosition);
				const float heuristic = Length(aGoal - position);

				// Each (triangle, entry edge) pair is its own search node: a node's position lies on its
				// entry edge, and with a radius, which edges a triangle may be left over depends on the
				// edge it was entered over. Otherwise a cheap entry that is pinched shut toward the goal
				// would hide a costlier open one.
				int entrySlot = 0;
				while (neighbor->myEdges[entrySlot] != edge)
					++entrySlot;

				int& existingIndex = nodeIndexByEntry[neighbor->myIndex * 3 + entrySlot];
				if (existingIndex != -1)
				{
					if (tentativeGCost >= nodes[existingIndex].myGCost)
						continue;

					nodes[existingIndex].myPosition = position;
					nodes[existingIndex].myGCost = tentativeGCost;
					nodes[existingIndex].myFCost = tentativeGCost + heuristic;
					nodes[existingIndex].myParentIndex = bestIndex;
					nodes[existingIndex].myEdgeFromParent = edge;
					nodes[existingIndex].myIsClosed = false;
				}
				else
				{
					existingIndex = nodes.Count();
					AStarNode& newNode = nodes.Add();
					newNode.myTriangle = neighbor;
					newNode.myPosition = position;
					newNode.myGCost = tentativeGCost;
					newNode.myFCost = tentativeGCost + heuristic;
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

bool Navmesh::StringPull(const Vector2f& aStart, const Vector2f& aGoal, const PathCorridor& aCorridor, float anAgentRadius, FW_GrowingArray<Vector2f>& outWaypoints) const
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
	if (anAgentRadius > 0.f)
	{
		ClearanceCache& clearanceCache = GetClearanceCache(anAgentRadius);
		for (const Portal& portal : aCorridor.myPortals)
		{
			portalLefts.Add(GetPushedPortalEnd(clearanceCache, portal.myLeft, portal.myLeftWallOffset));
			portalRights.Add(GetPushedPortalEnd(clearanceCache, portal.myRight, portal.myRightWallOffset));
		}
	}
	else
	{
		for (const Portal& portal : aCorridor.myPortals)
		{
			portalLefts.Add(portal.myLeft);
			portalRights.Add(portal.myRight);
		}
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

bool Navmesh::FindPath(const Vector2f& aStart, const Vector2f& aGoal, float anAgentRadius, FW_GrowingArray<Vector2f>& outWaypoints, PathCorridor* outCorridor) const
{
	Triangle* startTriangle = FindTriangleContaining(aStart);
	Triangle* goalTriangle = FindTriangleContaining(aGoal);
	if (startTriangle == nullptr || goalTriangle == nullptr)
		return false;

	PathCorridor foundCorridor;
	if (startTriangle != goalTriangle)
	{
		if (!FindTrianglePath(aStart, aGoal, startTriangle, goalTriangle, anAgentRadius, foundCorridor))
			return false;
	}

	// Build into a local corridor and only hand it to the caller once the search has
	// actually succeeded, so a reused *outCorridor is replaced rather than appended to
	// (guaranteeing the zero-portal same-triangle case) and stays untouched on failure.
	if (outCorridor != nullptr)
		*outCorridor = foundCorridor;
	return StringPull(aStart, aGoal, foundCorridor, anAgentRadius, outWaypoints);
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

namespace
{
	bool IsPointInsidePolygon(const Vector2f& aPoint, const FW_GrowingArray<Vector2f>& aPolygon)
	{
		bool isInside = false;
		for (int i = 0, previousIndex = aPolygon.Count() - 1; i < aPolygon.Count(); previousIndex = i++)
		{
			const Vector2f& vertex = aPolygon[i];
			const Vector2f& previousVertex = aPolygon[previousIndex];
			const bool crossesHorizontalRay = (vertex.y > aPoint.y) != (previousVertex.y > aPoint.y);
			if (crossesHorizontalRay && aPoint.x < (previousVertex.x - vertex.x) * (aPoint.y - vertex.y) / (previousVertex.y - vertex.y) + vertex.x)
				isInside = !isInside;
		}

		return isInside;
	}
}

void Navmesh::CutHole(const FW_GrowingArray<Vector2f>& aPolygon)
{
	if (aPolygon.Count() < 3)
		return;

	CutPolygon(aPolygon);
	DropClearanceCaches();
}

bool Navmesh::IsAreaFullyOnMesh(const FW_GrowingArray<Vector2f>& aPolygon) const
{
	if (aPolygon.Count() < 3)
		return false;

	for (const Vector2f& point : aPolygon)
	{
		bool isOnMesh = false;
		for (const Triangle* triangle : myTriangles)
		{
			if (triangle->PointInside(point))
			{
				isOnMesh = true;
				break;
			}
		}

		if (!isOnMesh)
			return false;
	}

	for (const Edge* edge : myEdges)
	{
		if (edge->myTriangles[0] != nullptr && edge->myTriangles[1] != nullptr)
			continue;

		FW_Intersection::LineSegment boundaryEdge;
		boundaryEdge.myStart = edge->myVertices[0]->myPos;
		boundaryEdge.myEnd = edge->myVertices[1]->myPos;
		if (IsPointInsidePolygon(boundaryEdge.myStart, aPolygon) || IsPointInsidePolygon(boundaryEdge.myEnd, aPolygon))
			return false;

		for (int i = 0; i < aPolygon.Count(); ++i)
		{
			FW_Intersection::LineSegment polygonEdge;
			polygonEdge.myStart = aPolygon[i];
			polygonEdge.myEnd = aPolygon[(i + 1) % aPolygon.Count()];
			if (FW_Intersection::LineSegmentVsLineSegment(boundaryEdge, polygonEdge))
				return false;
		}
	}

	return true;
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

namespace
{
	// Clearance checks treat a gap exactly as wide as the agent as open, so shave a hair off
	// to keep float error from closing it.
	const float ClearanceEpsilon = 0.001f;

	struct BlockedInterval
	{
		float myStart;
		float myEnd;
	};

	bool SegmentsCross(const Vector2f& aA, const Vector2f& aB, const Vector2f& aC, const Vector2f& aD)
	{
		const float abC = Cross(aB - aA, aC - aA);
		const float abD = Cross(aB - aA, aD - aA);
		const float cdA = Cross(aD - aC, aA - aC);
		const float cdB = Cross(aD - aC, aB - aC);
		return ((abC > 0.f) != (abD > 0.f)) && ((cdA > 0.f) != (cdB > 0.f)) && abC != 0.f && abD != 0.f && cdA != 0.f && cdB != 0.f;
	}

	float DistanceSegmentToSegmentSq(const Vector2f& aA, const Vector2f& aB, const Vector2f& aC, const Vector2f& aD)
	{
		if (SegmentsCross(aA, aB, aC, aD))
			return 0.f;

		float distanceSq = DistancePointToSegmentSq(aA, aC, aD);
		distanceSq = FW_Min(distanceSq, DistancePointToSegmentSq(aB, aC, aD));
		distanceSq = FW_Min(distanceSq, DistancePointToSegmentSq(aC, aA, aB));
		distanceSq = FW_Min(distanceSq, DistancePointToSegmentSq(aD, aA, aB));
		return distanceSq;
	}

	void AddBlockedInterval(float aStart, float anEnd, FW_GrowingArray<BlockedInterval>& outIntervals)
	{
		aStart = FW_Max(aStart, 0.f);
		anEnd = FW_Min(anEnd, 1.f);
		if (aStart >= anEnd)
			return;

		BlockedInterval& interval = outIntervals.Add();
		interval.myStart = aStart;
		interval.myEnd = anEnd;
	}

	// Narrows [ioMin, ioMax] to the t where aValue + aSlope * t lies strictly inside (aLow, aHigh).
	bool ClipLinearRange(float aValue, float aSlope, float aLow, float aHigh, float& ioMin, float& ioMax)
	{
		if (aSlope == 0.f)
			return aValue > aLow && aValue < aHigh;

		float tLow = (aLow - aValue) / aSlope;
		float tHigh = (aHigh - aValue) / aSlope;
		if (tLow > tHigh)
		{
			const float swap = tLow;
			tLow = tHigh;
			tHigh = swap;
		}

		ioMin = FW_Max(ioMin, tLow);
		ioMax = FW_Min(ioMax, tHigh);
		return ioMin < ioMax;
	}

	// The part of the portal segment aPortalStart + t * (aPortalEnd - aPortalStart), t in [0, 1],
	// that lies closer than aRadius to the wall segment, as up to three intervals that together
	// cover one connected range (the wall's capsule is convex).
	void AddWallBlockedIntervals(const Vector2f& aPortalStart, const Vector2f& aPortalEnd, const Vector2f& aWallStart, const Vector2f& aWallEnd, float aRadius, FW_GrowingArray<BlockedInterval>& outIntervals)
	{
		const Vector2f portalDir = aPortalEnd - aPortalStart;
		const float portalLength2 = Length2(portalDir);

		const Vector2f wallEnds[2] = { aWallStart, aWallEnd };
		for (const Vector2f& center : wallEnds)
		{
			const Vector2f fromCenter = aPortalStart - center;
			const float a = portalLength2;
			const float b = 2.f * Dot(fromCenter, portalDir);
			const float c = Length2(fromCenter) - aRadius * aRadius;
			const float discriminant = b * b - 4.f * a * c;
			if (a == 0.f || discriminant <= 0.f)
				continue;

			const float root = sqrtf(discriminant);
			AddBlockedInterval((-b - root) / (2.f * a), (-b + root) / (2.f * a), outIntervals);
		}

		const Vector2f wallDir = aWallEnd - aWallStart;
		const float wallLength = Length(wallDir);
		if (wallLength == 0.f)
			return;

		const Vector2f along = wallDir / wallLength;
		const Vector2f across{ -along.y, along.x };
		const Vector2f fromWallStart = aPortalStart - aWallStart;

		float tMin = 0.f;
		float tMax = 1.f;
		if (!ClipLinearRange(Dot(fromWallStart, along), Dot(portalDir, along), 0.f, wallLength, tMin, tMax))
			return;
		if (!ClipLinearRange(Dot(fromWallStart, across), Dot(portalDir, across), -aRadius, aRadius, tMin, tMax))
			return;

		AddBlockedInterval(tMin, tMax, outIntervals);
	}

	bool IsUnitRangeFullyCovered(const FW_GrowingArray<BlockedInterval>& someIntervals)
	{
		// Open intervals: one ending exactly where the next starts leaves that point free.
		float covered = 0.f;
		bool coversStart = false;
		for (const BlockedInterval& interval : someIntervals)
		{
			if (interval.myStart <= 0.f)
			{
				coversStart = true;
				covered = FW_Max(covered, interval.myEnd);
			}
		}

		if (!coversStart)
			return false;

		bool madeProgress = true;
		while (madeProgress && covered < 1.f)
		{
			madeProgress = false;
			for (const BlockedInterval& interval : someIntervals)
			{
				if (interval.myStart < covered && interval.myEnd > covered)
				{
					covered = interval.myEnd;
					madeProgress = true;
				}
			}
		}

		return covered >= 1.f;
	}
}

float Navmesh::GetDistanceToNearestWall(const Vector2f& aPosition) const
{
	Triangle* startTriangle = FindTriangleContaining(aPosition);
	if (startTriangle == nullptr)
		return -1.f;

	// Flood outward across open edges, skipping any edge already farther away than the closest wall
	// found so far. The straight line to the closest wall only crosses edges closer than that wall,
	// so it's always reached.
	float closestSq = -1.f;
	FW_GrowingArray<Triangle*> visitedTriangles;
	FW_GrowingArray<Triangle*> openTriangles;
	visitedTriangles.Add(startTriangle);
	openTriangles.Add(startTriangle);
	while (!openTriangles.IsEmpty())
	{
		Triangle* triangle = openTriangles.GetLast();
		openTriangles.RemoveLast();

		for (Edge* edge : triangle->myEdges)
		{
			const float distanceSq = DistancePointToSegmentSq(aPosition, edge->myVertices[0]->myPos, edge->myVertices[1]->myPos);
			if (closestSq >= 0.f && distanceSq >= closestSq)
				continue;

			if (IsWallEdge(edge))
			{
				closestSq = distanceSq;
				continue;
			}

			Triangle* neighbor = edge->myTriangles[0] == triangle ? edge->myTriangles[1] : edge->myTriangles[0];
			if (visitedTriangles.Contains(neighbor))
				continue;

			visitedTriangles.Add(neighbor);
			openTriangles.Add(neighbor);
		}
	}

	return closestSq >= 0.f ? sqrtf(closestSq) : -1.f;
}

bool Navmesh::IsWallEdge(const Edge* anEdge)
{
	return anEdge->myTriangles[0] == nullptr || anEdge->myTriangles[1] == nullptr;
}

bool Navmesh::IsWallVertex(const Vertex* aVertex)
{
	for (const Edge* edge : aVertex->myEdges)
	{
		if (IsWallEdge(edge))
			return true;
	}

	return false;
}

bool Navmesh::CanCrossEdge(ClearanceCache& aCache, Triangle* aTriangle, Edge* anEntryEdge, Edge* anExitEdge) const
{
	if (!HasPortalClearance(aCache, anExitEdge))
		return false;

	// The start triangle has no entry edge, so nothing is passed through inside it.
	if (anEntryEdge == nullptr)
		return true;

	return HasTransitClearance(aCache, aTriangle, anEntryEdge->GetSharedVertex(anExitEdge));
}

bool Navmesh::HasPortalClearance(ClearanceCache& aCache, Edge* aPortal) const
{
	ClearanceState& state = aCache.myPortalStates[aPortal->myIndex];
	if (state != ClearanceState::Unknown)
		return state == ClearanceState::Open;

	const Vector2f& portalStart = aPortal->myVertices[0]->myPos;
	const Vector2f& portalEnd = aPortal->myVertices[1]->myPos;
	const float blockingRadius = aCache.myRadius - ClearanceEpsilon;
	const float blockingRadiusSq = blockingRadius * blockingRadius;

	// Every wall closer than the radius to the portal is closer than that to the triangles on either side of it.
	Triangle* triangle = aPortal->myTriangles[0];
	GatherNearbyWalls(aCache, triangle);
	const int wallsStart = aCache.myNearbyWallsStart[triangle->myIndex];
	const int wallsEnd = wallsStart + aCache.myNearbyWallsCount[triangle->myIndex];

	FW_GrowingArray<BlockedInterval> blockedIntervals;
	for (int i = wallsStart; i < wallsEnd; ++i)
	{
		const Vector2f& wallStart = aCache.myNearbyWalls[i]->myVertices[0]->myPos;
		const Vector2f& wallEnd = aCache.myNearbyWalls[i]->myVertices[1]->myPos;
		if (DistanceSegmentToSegmentSq(portalStart, portalEnd, wallStart, wallEnd) < blockingRadiusSq)
			AddWallBlockedIntervals(portalStart, portalEnd, wallStart, wallEnd, blockingRadius, blockedIntervals);
	}

	state = IsUnitRangeFullyCovered(blockedIntervals) ? ClearanceState::Blocked : ClearanceState::Open;
	return state == ClearanceState::Open;
}

namespace
{
	const int MaxClearanceCaches = 4;
}

Navmesh::ClearanceCache& Navmesh::GetClearanceCache(float anAgentRadius) const
{
	for (ClearanceCache* cache : myClearanceCaches)
	{
		if (cache->myRadius == anAgentRadius)
			return *cache;
	}

	// A few radii cover every enemy size; past that (dragging the Debugger's radius slider, say) drop the oldest.
	if (myClearanceCaches.Count() >= MaxClearanceCaches)
		myClearanceCaches.DeleteNonCyclicAtIndex(0);

	ClearanceCache* cache = new ClearanceCache();
	cache->myRadius = anAgentRadius;

	cache->myPortalStates.Reserve(myEdges.Count());
	for (ClearanceState& state : cache->myPortalStates)
		state = ClearanceState::Unknown;

	cache->myTransitStates.Reserve(myTriangles.Count() * 3);
	for (ClearanceState& state : cache->myTransitStates)
		state = ClearanceState::Unknown;

	cache->myNearbyWallsStart.Reserve(myTriangles.Count());
	cache->myNearbyWallsCount.Reserve(myTriangles.Count());
	for (int i = 0; i < myTriangles.Count(); ++i)
	{
		cache->myNearbyWallsStart[i] = -1;
		cache->myNearbyWallsCount[i] = 0;
	}

	myClearanceCaches.Add(cache);
	return *cache;
}

namespace
{
	float DistanceSegmentToTriangleSq(const Vector2f& aStart, const Vector2f& anEnd, const Vector2f& aT0, const Vector2f& aT1, const Vector2f& aT2)
	{
		if (FW_Intersection::PointInsideTriangle(aStart, aT0, aT1, aT2) || FW_Intersection::PointInsideTriangle(anEnd, aT0, aT1, aT2))
			return 0.f;

		float distanceSq = DistanceSegmentToSegmentSq(aStart, anEnd, aT0, aT1);
		distanceSq = FW_Min(distanceSq, DistanceSegmentToSegmentSq(aStart, anEnd, aT1, aT2));
		distanceSq = FW_Min(distanceSq, DistanceSegmentToSegmentSq(aStart, anEnd, aT2, aT0));
		return distanceSq;
	}

	Vector2f ClosestPointOnSegment(const Vector2f& aPoint, const Vector2f& aStart, const Vector2f& anEnd)
	{
		const Vector2f dir = anEnd - aStart;
		const float length2 = Length2(dir);
		if (length2 == 0.f)
			return aStart;

		return aStart + dir * FW_Clamp(Dot(aPoint - aStart, dir) / length2, 0.f, 1.f);
	}

	// A closest pair of points between segments AB and CD (not unique when they're parallel).
	void GetClosestPointsBetweenSegments(const Vector2f& aA, const Vector2f& aB, const Vector2f& aC, const Vector2f& aD, Vector2f& outOnAB, Vector2f& outOnCD)
	{
		if (SegmentsCross(aA, aB, aC, aD))
		{
			const float abC = Cross(aB - aA, aC - aA);
			const float abD = Cross(aB - aA, aD - aA);
			outOnCD = aC + (aD - aC) * (abC / (abC - abD));
			outOnAB = outOnCD;
			return;
		}

		outOnAB = aA;
		outOnCD = ClosestPointOnSegment(aA, aC, aD);
		float bestSq = Length2(outOnCD - outOnAB);

		const Vector2f candidatesOnAB[3] = { aB, ClosestPointOnSegment(aC, aA, aB), ClosestPointOnSegment(aD, aA, aB) };
		const Vector2f candidatesOnCD[3] = { ClosestPointOnSegment(aB, aC, aD), aC, aD };
		for (int i = 0; i < 3; ++i)
		{
			const float distanceSq = Length2(candidatesOnCD[i] - candidatesOnAB[i]);
			if (distanceSq < bestSq)
			{
				bestSq = distanceSq;
				outOnAB = candidatesOnAB[i];
				outOnCD = candidatesOnCD[i];
			}
		}
	}

	float GetLargerWallDistanceSq(const Vector2f& aPoint, const Vector2f& aWallA0, const Vector2f& aWallA1, const Vector2f& aWallB0, const Vector2f& aWallB1)
	{
		return FW_Max(DistancePointToSegmentSq(aPoint, aWallA0, aWallA1), DistancePointToSegmentSq(aPoint, aWallB0, aWallB1));
	}

	const int GoldenSectionIterations = 25;

	// The minimum of aFunction, convex over [0, 1], by golden-section search: each step reuses one of
	// the previous step's two samples, so it takes about half the evaluations of a ternary search for
	// the same precision. Returns early with the first sample below aStopBelow, for callers that
	// only need to know whether the minimum is below it.
	template <typename Function>
	float FindConvexMinimum(const Function& aFunction, float aStopBelow)
	{
		const float shrink = 0.618034f; // 1 / the golden ratio
		float low = 0.f;
		float high = 1.f;
		float first = high - shrink;
		float second = low + shrink;
		float firstValue = aFunction(first);
		float secondValue = aFunction(second);
		for (int i = 0; i < GoldenSectionIterations; ++i)
		{
			if (firstValue < aStopBelow || secondValue < aStopBelow)
				break;

			if (firstValue < secondValue)
			{
				high = second;
				second = first;
				secondValue = firstValue;
				first = high - shrink * (high - low);
				firstValue = aFunction(first);
			}
			else
			{
				low = first;
				first = second;
				firstValue = secondValue;
				second = low + shrink * (high - low);
				secondValue = aFunction(second);
			}
		}

		return FW_Min(firstValue, secondValue);
	}

	// Whether some point of the triangle (aCorner, aSideA, aSideB) is closer than the square root of
	// aRadiusSq to both walls. The larger distance to the two walls is convex over the plane, so
	// nested searches find its minimum: the outer one over lines parallel to aSideA-aSideB, the inner
	// one along each line.
	bool IsTriangleCloseToBothWalls(const Vector2f& aCorner, const Vector2f& aSideA, const Vector2f& aSideB, const Vector2f& aWallA0, const Vector2f& aWallA1, const Vector2f& aWallB0, const Vector2f& aWallB1, float aRadiusSq)
	{
		const float minimum = FindConvexMinimum([&](float aLineFraction)
		{
			const Vector2f lineStart = aCorner + (aSideA - aCorner) * aLineFraction;
			const Vector2f lineEnd = aCorner + (aSideB - aCorner) * aLineFraction;
			return FindConvexMinimum([&](float aFraction)
			{
				return GetLargerWallDistanceSq(lineStart + (lineEnd - lineStart) * aFraction, aWallA0, aWallA1, aWallB0, aWallB1);
			}, aRadiusSq);
		}, aRadiusSq);

		return minimum < aRadiusSq;
	}
}

void Navmesh::GatherNearbyWalls(ClearanceCache& aCache, Triangle* aTriangle) const
{
	if (aCache.myNearbyWallsStart[aTriangle->myIndex] != -1)
		return;

	const Vector2f& t0 = aTriangle->myVertices[0]->myPos;
	const Vector2f& t1 = aTriangle->myVertices[1]->myPos;
	const Vector2f& t2 = aTriangle->myVertices[2]->myPos;
	const float blockingRadius = aCache.myRadius - ClearanceEpsilon;
	const float blockingRadiusSq = blockingRadius * blockingRadius;

	// The triangle's bounding box grown by the radius: an edge outside it is farther than the radius.
	const Vector2f searchMin{ FW_Min(t0.x, FW_Min(t1.x, t2.x)) - blockingRadius, FW_Min(t0.y, FW_Min(t1.y, t2.y)) - blockingRadius };
	const Vector2f searchMax{ FW_Max(t0.x, FW_Max(t1.x, t2.x)) + blockingRadius, FW_Max(t0.y, FW_Max(t1.y, t2.y)) + blockingRadius };

	aCache.myNearbyWallsStart[aTriangle->myIndex] = aCache.myNearbyWalls.Count();

	// Flood out across open edges, only through edges that may be within the radius of the triangle;
	// every wall that could be closer than that is reached this way. The cheap bounding-box test is
	// enough to decide where to flood; only walls get the exact distance.
	FW_GrowingArray<Triangle*> visitedTriangles;
	FW_GrowingArray<Triangle*> openTriangles;
	visitedTriangles.Add(aTriangle);
	openTriangles.Add(aTriangle);
	while (!openTriangles.IsEmpty())
	{
		Triangle* triangle = openTriangles.GetLast();
		openTriangles.RemoveLast();

		for (Edge* edge : triangle->myEdges)
		{
			const Vector2f& edgeStart = edge->myVertices[0]->myPos;
			const Vector2f& edgeEnd = edge->myVertices[1]->myPos;
			if (FW_Max(edgeStart.x, edgeEnd.x) < searchMin.x || FW_Min(edgeStart.x, edgeEnd.x) > searchMax.x
				|| FW_Max(edgeStart.y, edgeEnd.y) < searchMin.y || FW_Min(edgeStart.y, edgeEnd.y) > searchMax.y)
				continue;

			if (IsWallEdge(edge))
			{
				if (DistanceSegmentToTriangleSq(edgeStart, edgeEnd, t0, t1, t2) < blockingRadiusSq)
					aCache.myNearbyWalls.Add(edge);
				continue;
			}

			Triangle* neighbor = edge->myTriangles[0] == triangle ? edge->myTriangles[1] : edge->myTriangles[0];
			if (visitedTriangles.Contains(neighbor))
				continue;

			visitedTriangles.Add(neighbor);
			openTriangles.Add(neighbor);
		}
	}

	aCache.myNearbyWallsCount[aTriangle->myIndex] = aCache.myNearbyWalls.Count() - aCache.myNearbyWallsStart[aTriangle->myIndex];
}

bool Navmesh::HasTransitClearance(ClearanceCache& aCache, Triangle* aTriangle, Vertex* aCorner) const
{
	// Any path through the triangle between the two edges meeting at corner C crosses every segment
	// from C to the opposite edge AB. So the crossing is blocked exactly when the parts of the
	// triangle closer than the radius to some wall link up into a chain from C to AB. With C a wall
	// vertex and one wall across AB this is Demyen's "width < 2r" test; the chain form also catches
	// pinches between walls that aren't vertices of this triangle. Which of the two edges is the
	// entry doesn't matter, so the result is kept per corner.
	int cornerSlot = 0;
	while (aTriangle->myVertices[cornerSlot] != aCorner)
		++cornerSlot;

	ClearanceState& state = aCache.myTransitStates[aTriangle->myIndex * 3 + cornerSlot];
	if (state != ClearanceState::Unknown)
		return state == ClearanceState::Open;

	const Vector2f& cornerPos = aCorner->myPos;
	const Vector2f& sideA = aTriangle->myVertices[(cornerSlot + 1) % 3]->myPos;
	const Vector2f& sideB = aTriangle->myVertices[(cornerSlot + 2) % 3]->myPos;

	const float blockingRadius = aCache.myRadius - ClearanceEpsilon;
	const float blockingRadiusSq = blockingRadius * blockingRadius;

	GatherNearbyWalls(aCache, aTriangle);
	const int wallsStart = aCache.myNearbyWallsStart[aTriangle->myIndex];
	const int wallCount = aCache.myNearbyWallsCount[aTriangle->myIndex];

	FW_GrowingArray<bool> isReached;
	FW_GrowingArray<int> reachedOpen;
	for (int i = 0; i < wallCount; ++i)
	{
		const Edge* wall = aCache.myNearbyWalls[wallsStart + i];
		const bool touchesCorner = DistancePointToSegmentSq(cornerPos, wall->myVertices[0]->myPos, wall->myVertices[1]->myPos) < blockingRadiusSq;
		isReached.Add(touchesCorner);
		if (touchesCorner)
			reachedOpen.Add(i);
	}

	state = ClearanceState::Open;
	while (!reachedOpen.IsEmpty())
	{
		const int wallIndex = reachedOpen.GetLast();
		reachedOpen.RemoveLast();

		const Edge* wall = aCache.myNearbyWalls[wallsStart + wallIndex];
		const Vector2f& wallStart = wall->myVertices[0]->myPos;
		const Vector2f& wallEnd = wall->myVertices[1]->myPos;
		if (DistanceSegmentToSegmentSq(wallStart, wallEnd, sideA, sideB) < blockingRadiusSq)
		{
			state = ClearanceState::Blocked;
			break;
		}

		for (int otherIndex = 0; otherIndex < wallCount; ++otherIndex)
		{
			if (isReached[otherIndex])
				continue;

			const Edge* other = aCache.myNearbyWalls[wallsStart + otherIndex];
			const Vector2f& otherStart = other->myVertices[0]->myPos;
			const Vector2f& otherEnd = other->myVertices[1]->myPos;
			if (DistanceSegmentToSegmentSq(wallStart, wallEnd, otherStart, otherEnd) >= 4.f * blockingRadiusSq)
				continue;

			// Linked when some point of the triangle is closer than the radius to both walls. The
			// overlap usually contains the midpoint of the walls' closest points, so try that first.
			Vector2f onWall;
			Vector2f onOther;
			GetClosestPointsBetweenSegments(wallStart, wallEnd, otherStart, otherEnd, onWall, onOther);
			bool isLinked = FW_Intersection::PointInsideTriangle((onWall + onOther) / 2.f, cornerPos, sideA, sideB);
			if (!isLinked)
				isLinked = IsTriangleCloseToBothWalls(cornerPos, sideA, sideB, wallStart, wallEnd, otherStart, otherEnd, blockingRadiusSq);

			if (isLinked)
			{
				isReached[otherIndex] = true;
				reachedOpen.Add(otherIndex);
			}
		}
	}

	return state == ClearanceState::Open;
}

namespace
{
	const float MaxCornerOffsetPerRadius = 3.f;

	const int OffMeshSearchIterations = 20;

	float GetAngleBetween(const Vector2f& aFirst, const Vector2f& aSecond)
	{
		return acosf(FW_Clamp(Dot(GetNormalized(aFirst), GetNormalized(aSecond)), -1.f, 1.f));
	}
}

Vector2f Navmesh::GetWallCornerOffset(Vertex* aVertex, Edge* aPortal) const
{
	// Walk the triangles around aVertex from the portal in both directions until each side hits a
	// wall edge. Those two walls bound the free wedge the portal is in; the angles walked add up to
	// the wedge's interior angle.
	Edge* walls[2] = { nullptr, nullptr };
	float anglesToWall[2] = { 0.f, 0.f };
	for (int side = 0; side < 2; ++side)
	{
		Edge* edge = aPortal;
		Triangle* triangle = aPortal->myTriangles[side];
		while (triangle != nullptr)
		{
			Edge* nextEdge = triangle->GetOtherEdge(edge, aVertex);
			const Vertex* edgeOther = edge->myVertices[0] == aVertex ? edge->myVertices[1] : edge->myVertices[0];
			const Vertex* nextOther = nextEdge->myVertices[0] == aVertex ? nextEdge->myVertices[1] : nextEdge->myVertices[0];
			anglesToWall[side] += GetAngleBetween(edgeOther->myPos - aVertex->myPos, nextOther->myPos - aVertex->myPos);

			if (nextEdge == aPortal)
				return Vector2f();

			if (IsWallEdge(nextEdge))
			{
				walls[side] = nextEdge;
				break;
			}

			triangle = nextEdge->myTriangles[0] == triangle ? nextEdge->myTriangles[1] : nextEdge->myTriangles[0];
			edge = nextEdge;
		}
	}

	if (walls[0] == nullptr || walls[1] == nullptr)
		return Vector2f();

	// Every portal in the same wedge must get a bit-identical offset, so the funnel sees one corner
	// point rather than several a float error apart. So everything below is symmetric in the two
	// walls; only which side of them the wedge lies on comes from the walk.
	const Vector2f wallDir = GetNormalized((walls[0]->myVertices[0] == aVertex ? walls[0]->myVertices[1] : walls[0]->myVertices[0])->myPos - aVertex->myPos);
	const Vector2f otherWallDir = GetNormalized((walls[1]->myVertices[0] == aVertex ? walls[1]->myVertices[1] : walls[1]->myVertices[0])->myPos - aVertex->myPos);
	const bool isWedgeOverHalfTurn = anglesToWall[0] + anglesToWall[1] > FW_PI;

	Vector2f bisector = wallDir + otherWallDir;
	if (Length2(bisector) > 0.000001f)
	{
		bisector = GetNormalized(bisector);
		if (isWedgeOverHalfTurn)
			bisector = -bisector;
	}
	else
	{
		// A straight wall: point square off it, into the wedge the portal is in.
		const Vector2f portalDir = (aPortal->myVertices[0] == aVertex ? aPortal->myVertices[1] : aPortal->myVertices[0])->myPos - aVertex->myPos;
		const Vector2f along = wallDir - otherWallDir;
		bisector = GetNormalized(Vector2f{ -along.y, along.x });
		if (Dot(bisector, portalDir) < 0.f)
			bisector = -bisector;
	}

	// Pushed out by r / sin(half the wedge angle), both walls are r away; sin(half) is the same for
	// the wedge and for the angle between the walls measured the other way round.
	const float halfAngleSin = sinf(GetAngleBetween(wallDir, otherWallDir) / 2.f);
	const float offsetLength = halfAngleSin > 1.f / MaxCornerOffsetPerRadius ? 1.f / halfAngleSin : MaxCornerOffsetPerRadius;
	return bisector * offsetLength;
}

Vector2f Navmesh::GetPushedPortalEnd(ClearanceCache& aCache, const Vector2f& anEnd, const Vector2f& aWallOffset) const
{
	if (aWallOffset.x == 0.f && aWallOffset.y == 0.f)
		return anEnd;

	// Placing the pushed end checks it against the mesh, and the same corner comes up in path after path.
	for (const PushedPortalEnd& pushedEnd : aCache.myPushedPortalEnds)
	{
		if (pushedEnd.myEnd == anEnd && pushedEnd.myWallOffset == aWallOffset)
			return pushedEnd.myPushedEnd;
	}

	PushedPortalEnd& pushedEnd = aCache.myPushedPortalEnds.Add();
	pushedEnd.myEnd = anEnd;
	pushedEnd.myWallOffset = aWallOffset;
	pushedEnd.myPushedEnd = ComputePushedPortalEnd(anEnd, aWallOffset * aCache.myRadius);
	return pushedEnd.myPushedEnd;
}

Vector2f Navmesh::ComputePushedPortalEnd(const Vector2f& anEnd, const Vector2f& aFullOffset) const
{
	if (FindTriangleContaining(anEnd + aFullOffset) != nullptr)
		return anEnd + aFullOffset;

	// Something else is in the way along the bisector, closer than the full offset: stop halfway to
	// where it leaves the mesh, the best clearance between the two along that line.
	float onMesh = 0.f;
	float offMesh = 1.f;
	for (int i = 0; i < OffMeshSearchIterations; ++i)
	{
		const float middle = (onMesh + offMesh) / 2.f;
		if (FindTriangleContaining(anEnd + aFullOffset * middle) != nullptr)
			onMesh = middle;
		else
			offMesh = middle;
	}

	return anEnd + aFullOffset * (onMesh / 2.f);
}

Vector2f Navmesh::GetSearchPointOnPortal(Edge* aPortal, const Vector2f& aFrom, const Vector2f& aGoal, float anAgentRadius)
{
	// The usable stretch of the portal: each end on a wall vertex pulled in by the radius, the way
	// string-pulling later keeps the path off it. A* already checked the portal fits the radius;
	// if the pulled-in ends still cross, meet in the middle.
	const Vector2f start = aPortal->myVertices[0]->myPos;
	const Vector2f toEnd = aPortal->myVertices[1]->myPos - start;
	const float length = Length(toEnd);
	if (length <= 0.f)
		return start;

	float minDistance = IsWallVertex(aPortal->myVertices[0]) ? anAgentRadius : 0.f;
	float maxDistance = IsWallVertex(aPortal->myVertices[1]) ? length - anAgentRadius : length;
	if (minDistance > maxDistance)
	{
		minDistance = FW_Clamp((minDistance + maxDistance) / 2.f, 0.f, length);
		maxDistance = minDistance;
	}

	// The point on that stretch with the shortest way from aFrom on to aGoal. Along the portal's line
	// that's where the straight line from aFrom to aGoal crosses it, with aGoal mirrored across the
	// line first if it's on aFrom's side. The length is convex along the line, so clamping that point
	// to the stretch gives the shortest way within it.
	const Vector2f direction = toEnd / length;
	const float fromAlong = Dot(aFrom - start, direction);
	const float goalAlong = Dot(aGoal - start, direction);
	const float fromAcross = Cross(direction, aFrom - start);
	float goalAcross = Cross(direction, aGoal - start);
	if (fromAcross * goalAcross > 0.f)
		goalAcross = -goalAcross;

	// Both on the line: every point between them is as short.
	float crossingAlong = goalAlong;
	if (fromAcross != goalAcross)
		crossingAlong = fromAlong + (goalAlong - fromAlong) * (fromAcross / (fromAcross - goalAcross));

	return start + direction * FW_Clamp(crossingAlong, minDistance, maxDistance);
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
