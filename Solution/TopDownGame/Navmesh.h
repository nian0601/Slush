#pragma once

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
		Vertex* myVertices[2];
		
		FW_GrowingArray<Triangle*> myTriangles;
	};
	FW_GrowingArray<Edge*> myEdges;

	struct Triangle
	{
		Vertex* myVertices[3];
		Edge* myEdges[3];
	};
	FW_GrowingArray<Triangle*> myTriangles;

	Vertex* GetVertex(int x, int y) const;
	void CreateEdge(Vertex* aV1, Vertex* aV2);
	void CreateTriangle(Edge* aE1, Edge* aE2, Edge* aE3, Vertex* aV1, Vertex* aV2, Vertex* aV3);

	Vector2i mySectorGrid{ 15, 7 };
	Vector2i mySectorGridSize{ 128, 128 };

	//Vector2i mySectorGrid{ 2, 2 };
	//Vector2i mySectorGridSize{ 512, 512 };

	bool myHasAnchor = false;
	Vector2f myMouseAnchor;
};
