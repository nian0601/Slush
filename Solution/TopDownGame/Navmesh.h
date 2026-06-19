#pragma once

class Navmesh
{
public:
	Navmesh();

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
		Edge* myEdges[3];
	};
	FW_GrowingArray<Triangle*> myTriangles;

	Vertex* GetVertex(int x, int y) const;
	Vector2i mySectorGrid{ 15, 9 };
	Vector2i mySectorGridSize{ 128, 128 };
};
