#include "include/grid.h"

//Base Object class
class Object{
public:
	virtual bool intersect(Ray ray, float &t) = 0;
	virtual glm::vec3 normal(void)const = 0;
    virtual const AABB& get_AABB(void)const = 0;
	AABB mAABB;
};

//Sphere derivative
class Sphere: public Object{
public:
	Sphere(glm::vec3 position, float radius, Material& material){
        mPosition = position;
        mRadius = radius;
        mType = SPHERE;
        mMaterial = material;
        mAABB.setExtends(glm::vec3(-radius) + position, glm::vec3(radius) + position);
    }

	bool intersect(Ray ray, float &t){
        glm::vec3 direction = mPosition - ray.r0;
        float B = glm::dot(ray.dir, direction);
        float det = sqrf(B) - glm::dot(direction, direction) + sqrf(mRadius);
        if(det < 0.0f) return false;
        float t0 = B + sqrt(det);
        float t1 = B - sqrt(det);
        bool retValue = false;
        if((t0 < t) && (t0 > 0.0001f)){
            t = t0;
            mIntersection = ray.r0 + t * ray.dir;
            retValue = true;
        }
        if((t1 < t) && (t1 > 0.0001f)){
            t = t1;
            mIntersection = ray.r0 + t * ray.dir;
            retValue = true;
        }
        return retValue;
    }

	glm::vec3 normal(void)const{
		return glm::normalize(mIntersection - mPosition);
	};
private:
	float mRadius;
	glm::vec3 mPosition;
	glm::vec3 mIntersection; //Holds the point of intersection on the sphere for returning the normal
};

//Plane derivative
class Plane: public Object{
public:
	Plane(glm::vec3 normal, glm::vec3 point, Material& material){
        mNormal = glm::normalize(normal);
        mPoint = point;
        mMaterial = material;
        mType = PLANE;
        mAABB.setExtends(glm::vec3(-10000.0f), glm::vec3(10000.0f)); //Does not really matter
    }

	bool intersect(Ray ray, float &t){
        float denominator = glm::dot(mNormal, ray.dir);
        if(fabs(denominator) < 0.0001f) return false;
        float numerator = glm::dot(mNormal, (mPoint - ray.r0));
        if(fabs(numerator) > 0.0001f){
            float t1 = numerator / denominator;
            if(t1 < t && t1 > 0.0001f){
                t = t1;
                return true;
            }
        }
        return false;
    }

	glm::vec3 normal(void)const{
		return mNormal;
	};
private:
	glm::vec3 mNormal;
	glm::vec3 mPoint;
};

//Triangle derivative
class Triangle: public Object{
public:
	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Material& material){
        mVertices[0] = v0;
        mVertices[1] = v1;
        mVertices[2] = v2;
        mNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        mMaterial = material;
        mType = TRIANGLE;
        
        glm::vec3 min(10000.0f);
        glm::vec3 max(-10000.0f);
        for(uint i = 0; i < 3; i++){
            for(uint j = 0; j < 3; j++){
                if(mVertices[i][j] < min[j]) min[j] = mVertices[i][j];
                if(mVertices[i][j] > max[j]) max[j] = mVertices[i][j];
            }
        }

        mAABB.setExtends(min, max);
    }

	bool intersect(Ray ray, float &t){
        glm::vec3 AC = mVertices[2] - mVertices[0];
        glm::vec3 AB = mVertices[1] - mVertices[0];
        glm::vec3 P = glm::cross(ray.dir, AC);
        float det = glm::dot(AB, P);
        if(det < 0.0f) return false;
        float invDet = 1.0f / det;
        glm::vec3 T = ray.r0 - mVertices[0];
        glm::vec3 Q = glm::cross(T, AB);
        float t1 = glm::dot(AC, Q) * invDet;
        if(t1 > t || t1 < 0.0f) return false;
        float u = glm::dot(T, P) * invDet;
        if(u < 0.0f || u > 1.0f) return false;
        float v = glm::dot(ray.dir, Q) * invDet;
        if(v < 0.0f || u + v > 1.0f) return false;
        t = t1;
        return true;
    }

	glm::vec3 normal(void)const{
		return mNormal;
	};
private:
	glm::vec3 mNormal;
	glm::vec3 mVertices[3]; // We use a pointer so that we can use triangles inside meshes without having duplicate vertices
	// glm::vec3 N, N1, N2;
	// float d, d1, d2;
};

Grid::Grid(const SimConfig* config){
}
