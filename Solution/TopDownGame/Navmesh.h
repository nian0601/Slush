#pragma once
#include "FW_Intersection.h"
#include <Core\Assets\AssetParser.h>

class Navmesh
{
public:
	struct TriangleMesh
	{
		// [n, n+1, n+2] builds a triangle, vertices are not shared
		FW_GrowingArray<Vector2f> myVertices;
	};

	enum RenderFlags
	{
		FACES = 1 << 0,
		EDGES = 1 << 1,
		MOUSE_TEST = 1 << 2,
		OUTLINE = 1 << 3,

		ALL = FACES | EDGES | MOUSE_TEST,
	};

public:
	Navmesh();
	Navmesh(const Navmesh&) = delete;
	Navmesh& operator=(const Navmesh&) = delete;
	~Navmesh();
	void CopyFrom(const Navmesh& aOther);

	void GenerateDefaultGrid();

	void Save(Slush::AssetParser::Handle aRootHandle) const;
	void Load(Slush::AssetParser::Handle aRootHandle);

	void GetAsTriangleMesh(TriangleMesh& aMesh) const;

	void Render(unsigned int aFlags = RenderFlags::OUTLINE);

	struct Portal
	{
		Vector2f myLeft;
		Vector2f myRight;

		// For an end on a wall vertex: where to push that end, per unit of agent radius, to keep the
		// radius from the walls meeting there. Zero for an open-floor end.
		Vector2f myLeftWallOffset;
		Vector2f myRightWallOffset;
	};

	struct PathCorridor
	{
		FW_GrowingArray<Portal> myPortals;
	};

	bool BuildEdgeCenterPath(const Vector2f& aStart, const Vector2f& aGoal, const PathCorridor& aCorridor, FW_GrowingArray<Vector2f>& outWaypoints) const;
	bool StringPull(const Vector2f& aStart, const Vector2f& aGoal, const PathCorridor& aCorridor, float anAgentRadius, FW_GrowingArray<Vector2f>& outWaypoints) const;

	// Agent clearance: anAgentRadius is checked per triangle crossing at query time, so one
	// mesh serves every agent size and nothing is rebuilt or eroded per size. Portal (edge)
	// length alone is wrong on this mesh both ways: short edges between open-floor vertices
	// would falsely block, and a long edge past a narrow pinch would falsely pass. Instead
	// each crossing checks the exit edge has a stretch at least the radius from every wall,
	// and that walls don't pinch shut the passage through the triangle around the entry and
	// exit edges' shared corner. The second is Demyen's triangle width, adapted to count only
	// walls as obstacles (this mesh is built from a grid, so most vertices are open floor) and
	// generalized to pinches between walls that aren't vertices of the triangle itself.
	// See Demyen & Buro, "Efficient Triangulation-Based Pathfinding" (AAAI 2006):
	// https://cdn.aaai.org/AAAI/2006/AAAI06-148.pdf
	// String-pulling then keeps the radius too: before the funnel runs, each portal end on a
	// wall vertex is pushed out along the bisector of the walls meeting there until both are
	// the radius away, so the path runs the radius off straight walls and turns around each
	// corner at one such point. One point instead of an arc keeps the path a plain polyline
	// for movement code; enemies are small enough that the sharp turn reads fine.
	// The funnel can only straighten the triangle chain A* picked, so the search measures cost
	// between points on the crossed edges (each the point on the edge's usable stretch with the
	// shortest way on to the goal) rather than between triangle centres, which tie on this grid
	// mesh and can pick a chain a whole cell away from the corner, or zigzag across open floor.
	// Clearance depends only on the mesh and the radius, so each radius's results (and the walls near
	// each triangle they're built from) are cached as the search asks for them, and dropped when the
	// mesh changes: the first query after a cut pays for what it touches, later ones mostly reuse it.
	// Run TopDownGame.exe -pathfindbenchmark to time FindPath on level_main (Debug/pathfind_benchmark.txt).
	// A radius of 0 is a point agent: no crossing is checked and no portal end is pushed.
	bool FindPath(const Vector2f& aStart, const Vector2f& aGoal, float anAgentRadius, FW_GrowingArray<Vector2f>& outWaypoints, PathCorridor* outCorridor = nullptr) const;

	bool IsAreaFullyOnMesh(const FW_GrowingArray<Vector2f>& aPolygon) const;
	// Distance to the closest wall (outer or cut boundary), or a negative value when the point is off the mesh.
	float GetDistanceToNearestWall(const Vector2f& aPosition) const;
	static constexpr float ourBoxSnapDistance = 8.f;
	Vector2f ResolveBoxSnap(const Vector2f& aPosition, const Vector2f& aHalfSize) const;
	void CutHole(const FW_GrowingArray<Vector2f>& aPolygon);

	int GetTriangleCount() const;
	int GetVertexCount() const;
	bool HasVertexNear(const Vector2f& aPosition, float anEpsilon) const;

private:
	struct Vertex;
	struct Edge;
	struct Triangle;

	struct Vertex
	{
		Vector2f myPos;

		FW_GrowingArray<Edge*> myEdges;
	};

	struct Edge
	{
		void AddTriangle(Triangle* aTriangle);
		bool RemoveTriangle(Triangle* aTriangle);
		Vertex* GetSharedVertex(Edge* aEdge) const;
		Vertex* GetOtherUniqueVertex(Vertex* aVertex, Edge* aEdge) const;

		Vertex* myVertices[2];
		Triangle* myTriangles[2];

		// Position in Navmesh::myEdges, refreshed by AssignIndices() before a query uses it.
		int myIndex = -1;
	};
	
	struct Triangle
	{
		~Triangle();

		bool PointInside(const Vector2f& aPoint) const;
		Edge* FindIntersectingEdge(const FW_Intersection::Ray& aRay) const;
		Edge* GetEdgeWithoutVertex(Vertex* aVertex) const;
		Edge* GetOtherEdge(Edge* aEdge, Vertex* aVertex) const;
		Vertex* GetOtherVertex(Edge* aEdge) const;

		Vector2f GetCenterPosition() const;

		Vertex* myVertices[3];
		Edge* myEdges[3];

		// Position in Navmesh::myTriangles, refreshed by AssignIndices() before a query uses it.
		int myIndex = -1;
	};
	
	struct CutEdge
	{
		Edge* myEdge;
		Vector2f myCutPosition;
	};

	struct AStarNode
	{
		Triangle* myTriangle = nullptr;
		Edge* myEdgeFromParent = nullptr;
		// Where the search reaches this node, on myEdgeFromParent (aStart for the start node).
		Vector2f myPosition;
		int myParentIndex = -1;
		float myGCost = 0.f;
		float myFCost = 0.f;
		bool myIsClosed = false;
	};

	enum class ClearanceState : char
	{
		Unknown,
		Open,
		Blocked,
	};

	struct PushedPortalEnd
	{
		Vector2f myEnd;
		Vector2f myWallOffset;
		Vector2f myPushedEnd;
	};

	// Clearance results for one agent radius. They depend only on the mesh and the radius, so they're
	// kept across queries (every enemy of a size asks the same questions) and dropped whenever the
	// mesh changes. Filled in lazily, as the search asks.
	struct ClearanceCache
	{
		float myRadius = 0.f;
		FW_GrowingArray<ClearanceState> myPortalStates; // By Edge::myIndex
		FW_GrowingArray<ClearanceState> myTransitStates; // By Triangle::myIndex * 3 + the corner's slot in myVertices

		// The walls closer than the radius to each triangle, as a range of myNearbyWalls; start -1 until gathered.
		FW_GrowingArray<int> myNearbyWallsStart; // By Triangle::myIndex
		FW_GrowingArray<int> myNearbyWallsCount;
		FW_GrowingArray<Edge*> myNearbyWalls;

		FW_GrowingArray<PushedPortalEnd> myPushedPortalEnds;
	};

	void Clear();
	void AssignIndices() const;
	void DropClearanceCaches();
	ClearanceCache& GetClearanceCache(float anAgentRadius) const;
	void GatherNearbyWalls(ClearanceCache& aCache, Triangle* aTriangle) const;

	Triangle* FindTriangleContaining(const Vector2f& aPosition) const;
	bool FindTrianglePath(const Vector2f& aStart, const Vector2f& aGoal, Triangle* aStartTriangle, Triangle* aGoalTriangle, float anAgentRadius, PathCorridor& outCorridor) const;

	static bool IsWallEdge(const Edge* anEdge);
	static bool IsWallVertex(const Vertex* aVertex);
	bool CanCrossEdge(ClearanceCache& aCache, Triangle* aTriangle, Edge* anEntryEdge, Edge* anExitEdge) const;
	bool HasPortalClearance(ClearanceCache& aCache, Edge* aPortal) const;
	bool HasTransitClearance(ClearanceCache& aCache, Triangle* aTriangle, Vertex* aCorner) const;
	Vector2f GetWallCornerOffset(Vertex* aVertex, Edge* aPortal) const;
	Vector2f GetPushedPortalEnd(ClearanceCache& aCache, const Vector2f& anEnd, const Vector2f& aWallOffset) const;
	Vector2f ComputePushedPortalEnd(const Vector2f& anEnd, const Vector2f& aFullOffset) const;
	static Vector2f GetSearchPointOnPortal(Edge* aPortal, const Vector2f& aFrom, const Vector2f& aGoal, float anAgentRadius);

	Vertex* GetVertex(int x, int y) const;
	Edge* GetEdgeWithVertex(Vertex* aV, Edge* aE1, Edge* aE2) const;
	Edge* GetEdgeWithoutVertex(Vertex* aV, Edge* aE1, Edge* aE2) const;

	void CreateQuad(Vertex*& aTopLeftVertex, Edge*& aLeftEdge, Edge*& aTopEdge);

	Vertex* CreateVertex(const Vector2f aPosition);
	void DeleteVertexIfNeeded(Vertex* aVertex);

	Edge* CreateEdge(Vertex* aV1, Vertex* aV2);
	Edge* FindOrCreateEdge(Vertex* aV1, Vertex* aV2);
	void DeleteEdgeIfNeeded(Edge* aEdge);

	Triangle* CreateTriangle(Edge* aE1, Edge* aE2, Edge* aE3);
	void DeleteTriangle(Triangle* aTriangle);

	void CutPolygon(const FW_GrowingArray<Vector2f>& aPolygonPoints);
	void EnsureCutterVerticesExist(const FW_GrowingArray<Vector2f>& aPolygonPoints);
	void Cut(const Vector2f& aV1, const Vector2f& aV2);
	void CollectCutEdges(const FW_Intersection::LineSegment& aCuttingLine, FW_GrowingArray<CutEdge>& outCutEdges) const;
	void CutTriangle(Triangle* aTriangle, Edge* aCutEdge, Vertex* aCutVertex, Edge* aNewEdge1, Edge* aNewEdge2);

	Vertex* InsertVertexInTriangle(Triangle* aTriangle, const Vector2f& aPosition);
	Vertex* SplitEdgeAtPosition(Edge* aEdge, const Vector2f& aPosition);
	Vertex* FindNearbyVertex(const Vector2f& aPosition, float anEpsilon) const;
	Edge* FindNearbyEdge(const Vector2f& aPosition, float anEpsilon) const;

	bool IsInsideCutArea(const FW_GrowingArray<Vector2f>& aCutPositions, Triangle* aTriangle);

	FW_GrowingArray<Vertex*> myVertices;
	FW_GrowingArray<Edge*> myEdges;
	FW_GrowingArray<Triangle*> myTriangles;

	mutable FW_GrowingArray<ClearanceCache*> myClearanceCaches;

	Vector2i mySectorGrid{ 15, 7 };
	Vector2i mySectorGridSize{ 128, 128 };
	
	//Vector2i mySectorGrid{ 2, 2 };
	//Vector2i mySectorGridSize{ 512, 512 };
};
