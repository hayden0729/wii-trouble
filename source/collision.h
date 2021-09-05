#ifndef TANK_COLLISION_H
#define TANK_COLLISION_H

#include <stdlib.h>
#include <gccore.h>
#include <wiisprite.h>
#include <math.h>
#include <vector>

using namespace wsp;

// rectangle structure for determining collision (note: height/width represent true height/width divided by 2, also angle is in radians)
struct TiltedRect {
	f32 x;
	f32 y;
	f32 width;
	f32 height;
	f32 angle;
};

// returns a 2d vector containing the vertices of a tilted rectangle (based on https://math.stackexchange.com/questions/2518607)
std::vector<std::vector<f32>> TiltedRectVertices(TiltedRect* rect);

// returns a sprite's vertices
std::vector<std::vector<f32>> GetVertices(Sprite* sprite);

// returns a quad's vertices
std::vector<std::vector<f32>> GetVertices(Quad* quad);

// returns the dot product of two 2d vectors defined by f32 arrays
f32 DotProduct(std::vector<f32> vector1, std::vector<f32> vector2);

// returns the normalized version of the vector passed in by dividing each of its components by its magnitude
std::vector<f32> NormalizeVector(std::vector<f32> vector);

// returns the 2 normal vectors to a vector
std::vector<std::vector<f32>> GetNormalVectors(std::vector<f32> vector);

// returns an array of normalized edges given a polygon defined by an array of vertices
std::vector<std::vector<f32>> GetEdgesFromVertices(std::vector<std::vector<f32>> vertices);

// returns an array of the normals to an array of edges (note: normal count will be double edge count, since a vertex has 2 normals in opposite directions)
std::vector<std::vector<f32>> GetNormalsFromEdges(std::vector<std::vector<f32>> edges);

// returns the interval that a polygon (defined by a set of vertices) takes up on a given axis
std::vector<f32> GetIntervalOnAxis(std::vector<std::vector<f32>> vertices, std::vector<f32> axis);

// returns the overlap between two polygons (defined by sets of vertices) on a given axis
f32 GetOverlapOnAxis(std::vector<std::vector<f32>> vertices1, std::vector<std::vector<f32>> vertices2, std::vector<f32> axis);

// returns info (penetrating axis and amount on axis) regarding a collision (or the lack thereof) between two convex polygons (defined by sets of vertices)
std::vector<f32> Collision(std::vector<std::vector<f32>> vertices1, std::vector<std::vector<f32>> vertices2);

// a few function overloads to make collision easy
std::vector<f32> Collision(Sprite* collider1, Sprite* collider2);
std::vector<f32> Collision(Sprite* collider1, Quad* collider2);
std::vector<f32> Collision(Quad* collider1, Sprite* collider2);
std::vector<f32> Collision(Quad* collider1, Quad* collider2);

// returns true if layers 1 and 2 may be colliding (note: all the parameters are kinda gross but this lets me generalize it to all layers rather than, say, just sprites)
bool CollisionPossible(Layer* layer1, Layer* layer2, f32 rotation1 = 0.0, f32 rotation2 = 0.0, f32 stretchX1 = 0.0, f32 stretchY1 = 0.0, f32 stretchX2 = 0.0, f32 stretchY2 = 0.0);

// some CollisionPossible overloads to make calling it easier
bool CollisionPossible(Sprite* sprite1, Sprite* sprite2);

bool CollisionPossible(Sprite* sprite, Quad* quad);

bool CollisionPossible(Quad* quad, Sprite* sprite);

bool CollisionPossible(Quad* quad1, Quad* quad2);

#endif