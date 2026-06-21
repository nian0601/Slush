#pragma once
#include "FW_Intersection.h"

class Navmesh
{
public:
	Navmesh();
	~Navmesh();

	void Update();
	void Render();

private:
	struct Vertex;
	struct Edge;
	struct Triangle;

	struct Vertex
	{
		Vector2f myPos;

		FW_GrowingArray<Edge*> myEdges;
	};
	FW_GrowingArray<Vertex*> myVertices;

	struct Edge
	{
		void RemoveTriangle(Triangle* aTriangle);
		Vertex* GetSharedVertex(Edge* aEdge) const;
		Vertex* GetOtherUniqueVertex(Vertex* aVertex, Edge* aEdge) const;

		Vertex* myVertices[2];
		FW_GrowingArray<Triangle*> myTriangles;
	};
	FW_GrowingArray<Edge*> myEdges;

	struct Triangle
	{
		~Triangle();

		bool PointInside(const Vector2f& aPoint) const;
		Edge* FindIntersectingEdge(const FW_Intersection::Ray& aRay) const;
		Edge* GetEdgeWithoutVertex(Vertex* aVertex) const;

		Vertex* myVertices[3];
		Edge* myEdges[3];
	};
	FW_GrowingArray<Triangle*> myTriangles;

	Vertex* GetVertex(int x, int y) const;
	Edge* GetEdgeWithVertex(Vertex* aV, Edge* aE1, Edge* aE2) const;
	Edge* GetEdgeWithoutVertex(Vertex* aV, Edge* aE1, Edge* aE2) const;

	void CreateQuad(Vertex*& aTopLeftVertex, Edge*& aLeftEdge, Edge*& aTopEdge);
	Edge* CreateEdge(Vertex* aV1, Vertex* aV2);
	Triangle* CreateTriangle(Edge* aE1, Edge* aE2, Edge* aE3);

	void Cut(const Vector2f& aV1, const Vector2f& aV2);

	Vector2i mySectorGrid{ 15, 7 };
	Vector2i mySectorGridSize{ 128, 128 };
	
	//Vector2i mySectorGrid{ 2, 2 };
	//Vector2i mySectorGridSize{ 512, 512 };

	bool myHasAnchor = false;
	Vector2f myMouseAnchor;
};
