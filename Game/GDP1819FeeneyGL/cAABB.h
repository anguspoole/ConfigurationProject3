#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <map>
#include <vector>

// Checkpoint 5
class cAABB
{
public:
	//	cAABB();

		// Setters
	void setMinXYZ(glm::vec3 newMinXYZ)
	{
		this->minXYZ = newMinXYZ;
	}
	void setMaxXYZ(glm::vec3 newMaxXYZ)
	{
		this->maxXYZ = newMaxXYZ;
	}
	void setCube(bool isCube) {
		this->isCube = isCube;
	}
	void setLength(float length) {
		this->length = length;
	}
	// Getters 
	glm::vec3 getMinXYZ(void)
	{
		return this->minXYZ;
	}

	glm::vec3 getMaxXYZ(void)
	{
		return this->maxXYZ;
	}
	float getCentre(void)
	{
		return centre;
	}
	float getLength(void) {
		return length;
	}
	bool getCube(bool isCube) {
		return this->isCube;
	}
	unsigned long long getID(void);

	// Main mixXYZ and maxXYZ
	static unsigned long long generateID(glm::vec3 minXYZ, glm::vec3 maxXYZ);

	// Original
	//static unsigned long long generateID(glm::vec3 minXYZ, float AABBsideLength);


	// Store triangle "lookup" info
	struct sAABB_Triangle
	{
		glm::vec3 verts[3];
		// Normal, etc.
	};
	std::vector< sAABB_Triangle > vecTriangles;		// CON: Triangles stored in multiple places, maybe?
//	std::vector< cTriangle* > vec_pTriangles;   // CON: Need central store of triangles
//	std::vector< cThingWeNeedToTestForCollision > 

private:

	// these two:
	glm::vec3 minXYZ;
	glm::vec3 maxXYZ;
	//
	float sideLegth;
	float length;
	float centre;
	bool isCube;

	static unsigned long long generateID(glm::vec3 minXYZ);
};

class cAABBHierarchy
{
public:
	std::map< unsigned long long,  /*ID of the AABB*/
		cAABB* > m_mapAABBs;
private:		// Will be private
	// 


	// If pointers to the "central store of triangles", 
	//  THIS would be the central store... 
	//	std::vector< cTriangle > vecTriangles;


};
