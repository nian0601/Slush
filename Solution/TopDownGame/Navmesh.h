#pragma once
#include "FW_Intersection.h"

class Navmesh
{
public:
	Navmesh();
	~Navmesh();

	void Update();
	void Render();

	struct Portal
	{
		Vector2f myLeft;
		Vector2f myRight;
	};

	struct PathCorridor
	{
		FW_GrowingArray<Portal> myPortals;
	};

	bool BuildEdgeCenterPath(const Vector2f& aStart, const Vector2f& aGoal, const PathCorridor& aCorridor, FW_GrowingArray<Vector2f>& outWaypoints) const;
	bool StringPull(const Vector2f& aStart, const Vector2f& aGoal, const PathCorridor& aCorridor, FW_GrowingArray<Vector2f>& outWaypoints) const;
	bool FindPath(const Vector2f& aStart, const Vector2f& aGoal, FW_GrowingArray<Vector2f>& outWaypoints, PathCorridor& outCorridor) const;

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
		int myParentIndex = -1;
		float myGCost = 0.f;
		float myFCost = 0.f;
		bool myIsClosed = false;
	};

	Triangle* FindTriangleContaining(const Vector2f& aPosition) const;
	bool FindTrianglePath(const Vector2f& aStart, Triangle* aStartTriangle, Triangle* aGoalTriangle, PathCorridor& outCorridor) const;

	Vertex* GetVertex(int x, int y) const;
	Edge* GetEdgeWithVertex(Vertex* aV, Edge* aE1, Edge* aE2) const;
	Edge* GetEdgeWithoutVertex(Vertex* aV, Edge* aE1, Edge* aE2) const;

	void CreateQuad(Vertex*& aTopLeftVertex, Edge*& aLeftEdge, Edge*& aTopEdge);

	Vertex* CreateVertex(const Vector2f aPosition);
	void DeleteVertexIfNeeded(Vertex* aVertex);

	Edge* CreateEdge(Vertex* aV1, Vertex* aV2);
	void DeleteEdgeIfNeeded(Edge* aEdge);

	Triangle* CreateTriangle(Edge* aE1, Edge* aE2, Edge* aE3);
	void DeleteTriangle(Triangle* aTriangle);

	void PerformCut();
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

	Vector2i mySectorGrid{ 15, 7 };
	Vector2i mySectorGridSize{ 128, 128 };
	
	//Vector2i mySectorGrid{ 2, 2 };
	//Vector2i mySectorGridSize{ 512, 512 };

	FW_GrowingArray<Vector2f> myCutPositions;
};
