#include "collision.h"
using namespace wsp;

// returns a 2d vector containing the vertices of a tilted rectangle (based on https://math.stackexchange.com/questions/2518607)
std::vector<std::vector<f32>> TiltedRectVertices(TiltedRect* rect) {
	return {
		{rect->x - rect->width * cos(rect->angle) - rect->height * sin(rect->angle), rect->y - rect->width * sin(rect->angle) + rect->height * cos(rect->angle)},
		{rect->x + rect->width * cos(rect->angle) - rect->height * sin(rect->angle), rect->y + rect->width * sin(rect->angle) + rect->height * cos(rect->angle)},
		{rect->x + rect->width * cos(rect->angle) + rect->height * sin(rect->angle), rect->y + rect->width * sin(rect->angle) - rect->height * cos(rect->angle)},
		{rect->x - rect->width * cos(rect->angle) + rect->height * sin(rect->angle), rect->y - rect->width * sin(rect->angle) - rect->height * cos(rect->angle)}
	};
}

// returns a sprite's vertices
std::vector<std::vector<f32>> GetVertices(Sprite* sprite) {
	TiltedRect rect;
	// x and y are centered based on sprite dimensions (stretching is automatically applied from center so no need to account for that)
	rect.x = sprite->GetX() + sprite->GetWidth() / 2;
	rect.y = sprite->GetY() + sprite->GetHeight() / 2;
	// width and height have stretching and custom collision rectangles accounted for
	rect.width = (f32) sprite->GetCollisionRectangle()->width * sprite->GetStretchWidth() / 2;
	rect.height = (f32) sprite->GetCollisionRectangle()->height * sprite->GetStretchHeight() / 2;
	// angle is doubled and converted to radians
	rect.angle = sprite->GetRotation() * 2.0 * (M_PI / 180.0);
	// vertices are calculated
	return TiltedRectVertices(&rect);
}

// returns a quad's vertices
std::vector<std::vector<f32>> GetVertices(Quad* quad) {
	TiltedRect rect;
	// width and height are calculated first since they're used for x and y (quads are simpler than sprites because they lack stretching and custom collision rectangles)
	rect.width = (f32) quad->GetWidth() / 2;
	rect.height = (f32) quad->GetHeight() / 2;
	// x and y are centered
	rect.x = quad->GetX() + rect.width;
	rect.y = quad->GetY() + rect.height;
	// angle is doubled and converted to radians
	rect.angle = quad->GetRotation() * 2.0 * (M_PI / 180.0);
	// vertices are calculated
	return TiltedRectVertices(&rect);
}

// returns the dot product of two 2d vectors defined by f32 arrays
f32 DotProduct(std::vector<f32> vector1, std::vector<f32> vector2) {
	return (vector1[0] * vector2[0] + vector1[1] * vector2[1]);
}

// returns the normalized version of the vector passed in by dividing each of its components by its magnitude
std::vector<f32> NormalizeVector(std::vector<f32> vector) {
	f32 magnitude = sqrt(pow(vector[0], 2) + pow(vector[1], 2));
	return {
		vector[0] / magnitude,
		vector[1] / magnitude
	};
}

// returns the 2 normal vectors to a vector
std::vector<std::vector<f32>> GetNormalVectors(std::vector<f32> vector) {
	std::vector<std::vector<f32>> normals = {
		{-vector[1], vector[0]}, // (-y, x)
		{vector[1], -vector[0]} // (y, -x)
	};
	return normals;
}

// returns an array of normalized edges given a polygon defined by an array of vertices
std::vector<std::vector<f32>> GetEdgesFromVertices(std::vector<std::vector<f32>> vertices) {
	int edgeCount = vertices.size();
	std::vector<std::vector<f32>> edges(edgeCount, std::vector<f32>(2)); // as many edges as there are vertices; 2 components per edge (x/y)
	for (int vertex = 0; vertex < edgeCount; vertex++) {
		edges[vertex][0] = vertices[(vertex + 1) % edgeCount][0] - vertices[vertex][0];
		edges[vertex][1] = vertices[(vertex + 1) % edgeCount][1] - vertices[vertex][1];
		edges[vertex] = NormalizeVector(edges[vertex]);
	}
	return edges;
}

// returns an array of the normals to an array of edges (note: normal count will be double edge count, since a vertex has 2 normals in opposite directions)
std::vector<std::vector<f32>> GetNormalsFromEdges(std::vector<std::vector<f32>> edges) {
	int edgeCount = edges.size();
	std::vector<std::vector<f32>> normals(edgeCount * 2, std::vector<f32>(2)); // 2x as many normals as there are edges, 2 components per normal (x/y)
	for (int vertex = 0; vertex < edgeCount; vertex++) {
		std::vector<std::vector<f32>> edgeNormals = GetNormalVectors(edges[vertex]);
		normals[vertex * 2 + 0] = edgeNormals[0];
		normals[vertex * 2 + 1] = edgeNormals[1];
	}
	return normals;
}

// returns the interval that a polygon (defined by a set of vertices) takes up on a given axis
std::vector<f32> GetIntervalOnAxis(std::vector<std::vector<f32>> vertices, std::vector<f32> axis) {
	std::vector<f32> interval(2);
	for (int vertex = 0; vertex < (int) vertices.size(); vertex++) { // loop through all vertices to find the min and max
		f32 d = DotProduct(axis, vertices[vertex]);
		if (vertex == 0) { // on the first iteration, set the min and max to the current vertex
			interval[0] = d;
			interval[1] = d;
		}
		if (interval[0] > d) interval[0] = d;
		if (interval[1] < d) interval[1] = d;
	}
	return interval;
}

// returns the overlap between two polygons (defined by sets of vertices) on a given axis
f32 GetOverlapOnAxis(std::vector<std::vector<f32>> vertices1, std::vector<std::vector<f32>> vertices2, std::vector<f32> axis) {
	f32 overlap = 0.0;
	std::vector<f32> interval1 = GetIntervalOnAxis(vertices1, axis);
	std::vector<f32> interval2 = GetIntervalOnAxis(vertices2, axis);
	if ((interval2[0] < interval1[1]) && (interval1[0] < interval2[1])) overlap = interval2[0] - interval1[1]; // there is overlap, so return amount of overlap (otherwise 0)
	return overlap;
}

// returns info (penetrating axis and amount on axis) regarding a collision (or the lack thereof) between two convex polygons (defined by sets of vertices)
std::vector<f32> Collision(std::vector<std::vector<f32>> vertices1, std::vector<std::vector<f32>> vertices2) {
	int vertexCount1 = vertices1.size();
	int vertexCount2 = vertices2.size();
	std::vector<std::vector<f32>> axes(vertexCount1 + vertexCount2, std::vector<f32>(2)); // (total vertices) vertices, 2 coords per vertex (x/y)
	std::vector<std::vector<f32>> normals1 = GetNormalsFromEdges(GetEdgesFromVertices(vertices1));
	std::vector<std::vector<f32>> normals2 = GetNormalsFromEdges(GetEdgesFromVertices(vertices2));
	std::vector<std::vector<f32>> collisionInfo(vertexCount1 + vertexCount2, std::vector<f32>(3)); // 1 set of info for each axis, 3 floats per info (x, y, distance)
	int penetratingIndex = 0;
	// only use every other normal, since half are just duplicates in opposite directions
	for (int i = 0; i < vertexCount1; i++) {
		axes[i] = normals1[i * 2];
	}
	for (int i = 0; i < vertexCount2; i++) {
		axes[vertexCount1 + i] = normals2[i * 2];
	}
	// axes have been evaluated, time to get overlap for each one and evaluate collision
	for (int i = 0; i < vertexCount1 + vertexCount2; i++) {
		collisionInfo[i][0] = axes[i][0];
		collisionInfo[i][1] = axes[i][1];
		collisionInfo[i][2] = GetOverlapOnAxis(vertices1, vertices2, axes[i]);
		if (abs(collisionInfo[i][2]) < abs(collisionInfo[penetratingIndex][2])) penetratingIndex = i; // update penetrating index if current axis has a smaller overlap interval
	}
	return collisionInfo[penetratingIndex];
}

// a few function overloads to make collision easy
std::vector<f32> Collision(Sprite* collider1, Sprite* collider2) {
	return Collision(GetVertices(collider1), GetVertices(collider2));
}
std::vector<f32> Collision(Sprite* collider1, Quad* collider2) {
	return Collision(GetVertices(collider1), GetVertices(collider2));
}
std::vector<f32> Collision(Quad* collider1, Sprite* collider2) {
	return Collision(GetVertices(collider1), GetVertices(collider2));
}
std::vector<f32> Collision(Quad* collider1, Quad* collider2) {
	return Collision(GetVertices(collider1), GetVertices(collider2));
}

// returns true if layers 1 and 2 are close enough to possibly collide (the parameters are gross but this lets me generalize it to all layers rather than, say, just sprites)
bool CollisionPossible(Layer* layer1, Layer* layer2, f32 rotation1, f32 rotation2, f32 stretchX1, f32 stretchY1, f32 stretchX2, f32 stretchY2) {
	// layer1
	f32 layer1BoundX = layer1->GetWidth() * (std::max(stretchX1, 1.0f));;
	f32 layer1BoundY = layer1->GetHeight() * (std::max(stretchY1, 1.0f));;
	f32 layer1X = layer1->GetX();
	f32 layer1Y = layer1->GetY();
	if (rotation1) { // if layer1 is rotated, consider its bounds just the entire possible area to which it can rotate
		layer1BoundX = std::max(layer1BoundX, layer1BoundY);
		layer1BoundY = layer1BoundY;
		layer1X = layer1X + layer1->GetWidth() / 2 - layer1BoundX / 2;
		layer1Y = layer1Y + layer1->GetHeight() / 2 - layer1BoundY / 2;
	}
	// layer2
	f32 layer2BoundX = layer2->GetWidth() * (std::max(stretchX2, 1.0f));
	f32 layer2BoundY = layer2->GetHeight() * (std::max(stretchY2, 1.0f));
	f32 layer2X = layer2->GetX();
	f32 layer2Y = layer2->GetY();
	if (rotation2) { // if layer2 is rotated, consider its bounds just the entire possible area to which it can rotate
		layer2BoundX = std::max(layer2BoundX, layer2BoundY);
		layer2BoundY = layer2BoundX;
		layer2X = layer2X + layer2->GetWidth() / 2 - layer2BoundX / 2;
		layer2Y = layer2Y + layer2->GetHeight() / 2 - layer2BoundY / 2;
	}
	// return
	bool rightOfLeftBound = layer2X >= layer1X - layer2BoundX;
	bool leftOfRightBound = layer2X <= layer1X + layer2BoundX + layer1BoundX;
	bool belowTopBound = layer2Y >= layer1Y - layer2BoundY;
	bool aboveBotBound = layer2Y <= layer1Y + layer2BoundY + layer1BoundY;
	return (rightOfLeftBound && leftOfRightBound && belowTopBound && aboveBotBound);
}

// some CollisionPossible overloads to make calling it easier
bool CollisionPossible(Sprite* sprite1, Sprite* sprite2) {
	Layer* layer1 = (Layer*) sprite1;
	Layer* layer2 = (Layer*) sprite2;
	f32 rotation1 = sprite1->GetRotation();
	f32 rotation2 = sprite2->GetRotation();
	f32 stretchX1 = sprite1->GetStretchWidth();
	f32 stretchX2 = sprite2->GetStretchWidth();
	f32 stretchY1 = sprite1->GetStretchHeight();
	f32 stretchY2 = sprite2->GetStretchHeight();
	return CollisionPossible(layer1, layer2, rotation1, rotation2, stretchX1, stretchX2, stretchY1, stretchY2);
}

bool CollisionPossible(Sprite* sprite, Quad* quad) {
	Layer* layer1 = (Layer*) sprite;
	Layer* layer2 = (Layer*) quad;
	f32 rotation1 = sprite->GetRotation();
	f32 rotation2 = quad->GetRotation();
	f32 stretchX = sprite->GetStretchWidth();
	f32 stretchY = sprite->GetStretchHeight();
	return CollisionPossible(layer1, layer2, rotation1, rotation2, stretchX, stretchY);
}

bool CollisionPossible(Quad* quad, Sprite* sprite) {
	return CollisionPossible(sprite, quad);
}

bool CollisionPossible(Quad* quad1, Quad* quad2) {
	Layer* layer1 = (Layer*) quad1;
	Layer* layer2 = (Layer*) quad2;
	f32 rotation1 = quad1->GetRotation();
	f32 rotation2 = quad2->GetRotation();
	return CollisionPossible(layer1, layer2, rotation1, rotation2);
}