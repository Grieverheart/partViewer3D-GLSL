#include "include/grid.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>

//Base IntersectionObject class
class IntersectionObject{
public:
    virtual ~IntersectionObject(void) = 0;
	virtual bool raycast(glm::vec3, glm::vec3, float &t) = 0;
	virtual glm::vec3 normal(void)const = 0;
    virtual const AABB& get_AABB(void)const = 0;
};

IntersectionObject::~IntersectionObject(void){
}

bool AABB::raycast(glm::vec3 o, glm::vec3 ray_dir, float &t)const{
	glm::vec3 invDir = 1.0f / ray_dir;
	float t1 = (bounds_[0].x - o.x) * invDir.x;
	float t2 = (bounds_[1].x - o.x) * invDir.x;
	float t3 = (bounds_[0].y - o.y) * invDir.y;
	float t4 = (bounds_[1].y - o.y) * invDir.y;
	float t5 = (bounds_[0].z - o.z) * invDir.z;
	float t6 = (bounds_[1].z - o.z) * invDir.z;
	
	float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
	
	if(tmax < 0.0f){
		t = tmax;
		return false;
	}
	
	if(tmin > tmax){
		t = tmax;
		return false;
	}
	
	t = tmin;
	return true;
}

//Sphere derivative
class Sphere: public IntersectionObject{
public:
	Sphere(glm::vec3 pos, float radius):
        radius_(radius), pos_(pos),
        mAABB{glm::vec3(-radius) + pos, glm::vec3(radius) + pos}
    {}

	bool raycast(glm::vec3 o, glm::vec3 ray_dir, float &t){
        glm::vec3 direction = pos_ - o;
        float B = glm::dot(ray_dir, direction);
        float det = B * B - glm::dot(direction, direction) + radius_ * radius_;
        if(det < 0.0f) return false;
        float t0 = B + sqrt(det);
        float t1 = B - sqrt(det);
        bool retValue = false;
        if((t0 < t) && (t0 > 0.0001f)){
            t = t0;
            raycastion_point_ = o + t * ray_dir;
            retValue = true;
        }
        if((t1 < t) && (t1 > 0.0001f)){
            t = t1;
            raycastion_point_ = o + t * ray_dir;
            retValue = true;
        }
        return retValue;
    }

    const AABB& get_AABB(void)const{
        return mAABB;
    }

	glm::vec3 normal(void)const{
		return glm::normalize(raycastion_point_ - pos_);
	};
private:
	float radius_;
	glm::vec3 pos_;
	glm::vec3 raycastion_point_; //Holds the point of raycastion on the sphere for returning the normal
	AABB mAABB;
};

//Plane derivative
//class Plane: public IntersectionObject{
//public:
//	Plane(glm::vec3 normal, glm::vec3 point, Material& material){
//        normal_ = glm::normalize(normal);
//        mPoint = point;
//        mMaterial = material;
//        mType = PLANE;
//        mAABB.setExtends(glm::vec3(-10000.0f), glm::vec3(10000.0f)); //Does not really matter
//    }
//
//	bool raycast(Ray ray, float &t){
//        float denominator = glm::dot(normal_, ray.dir);
//        if(fabs(denominator) < 0.0001f) return false;
//        float numerator = glm::dot(normal_, (mPoint - ray.r0));
//        if(fabs(numerator) > 0.0001f){
//            float t1 = numerator / denominator;
//            if(t1 < t && t1 > 0.0001f){
//                t = t1;
//                return true;
//            }
//        }
//        return false;
//    }
//
//	glm::vec3 normal(void)const{
//		return normal_;
//	};
//private:
//	glm::vec3 normal_;
//	glm::vec3 mPoint;
//};

//Triangle derivative
class Triangle: public IntersectionObject{
public:
	Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2):
        normal_(glm::normalize(glm::cross(v1 - v0, v2 - v0))),
        vertices_{v0, v1, v2}
    {
        glm::vec3 min(10000.0f);
        glm::vec3 max(-10000.0f);
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                if(vertices_[i][j] < min[j]) min[j] = vertices_[i][j];
                if(vertices_[i][j] > max[j]) max[j] = vertices_[i][j];
            }
        }

        mAABB = {min, max};
    }

#if 0 //This culls back-facing triangles
	bool raycast(glm::vec3 o, glm::vec3 ray_dir, float &t){
        glm::vec3 AC = vertices_[2] - vertices_[0];
        glm::vec3 AB = vertices_[1] - vertices_[0];
        glm::vec3 P = glm::cross(ray_dir, AC);
        float det = glm::dot(AB, P);
        if(det < 0.0f) return false;
        glm::vec3 T = o - vertices_[0];
        float u = glm::dot(T, P);
        if(u < 0.0f || u > det) return false;
        glm::vec3 Q = glm::cross(T, AB);
        float v = glm::dot(ray_dir, Q);
        if(v < 0.0f || u + v > 1.0f) return false;
        float t1 = glm::dot(AC, Q);
        float invDet = 1.0f / det;
        t1 *= invDet;
        u  *= invDet;
        v  *= invDet;
        if(t1 > t || t1 < 0.0f) return false;
        t = t1;
        return true;
    }
#else
	bool raycast(glm::vec3 o, glm::vec3 ray_dir, float &t){
        glm::vec3 AC = vertices_[2] - vertices_[0];
        glm::vec3 AB = vertices_[1] - vertices_[0];
        glm::vec3 P = glm::cross(ray_dir, AC);
        float det = glm::dot(AB, P);
        if(det == 0.0f) return false; //TODO: This shoud test if det is close to zero instead
        float invDet = 1.0f / det;
        glm::vec3 T = o - vertices_[0];
        float u = glm::dot(T, P) * invDet;
        if(u < 0.0f || u > 1.0f) return false;
        glm::vec3 Q = glm::cross(T, AB);
        float v = glm::dot(ray_dir, Q) * invDet;
        if(v < 0.0f || u + v > 1.0f) return false;
        float t1 = glm::dot(AC, Q) * invDet;
        if(t1 > t || t1 < 0.0f) return false;
        t = t1;
        return true;
    }
#endif

    const AABB& get_AABB(void)const{
        return mAABB;
    }

	glm::vec3 normal(void)const{
		return normal_;
	};
private:
	glm::vec3 normal_;
	glm::vec3 vertices_[3]; // We use a pointer so that we can use triangles inside meshes without having duplicate vertices
	AABB mAABB;
	// glm::vec3 N, N1, N2;
	// float d, d1, d2;
};

Grid::Grid(const SimConfig& config):
    cells_(nullptr)
{
    // Find shape out_radius
    float out_radii[config.n_shapes]{};
    for(int shape_id = 0; shape_id < config.n_shapes; ++shape_id){
        if(config.shapes[shape_id].type == Shape::MESH){
            const Shape::Mesh& mesh = config.shapes[shape_id].mesh;
            float max = out_radii[shape_id] * out_radii[shape_id];
            for(int vid = 1; vid < mesh.n_vertices; ++vid){
                float dot = glm::dot(mesh.vertices[vid]._coord, mesh.vertices[vid]._coord);
                if(dot > max){
                    max = dot;
                }
            }
            out_radii[shape_id] = sqrt(max);
        }
        else if(config.shapes[shape_id].type == Shape::SPHERE){
            out_radii[shape_id] = 1.0;
        }
    }

    glm::vec3 offset(
        -(config.box[0][0] + config.box[0][1] + config.box[0][2]) / 2.0,
        -(config.box[1][1] + config.box[1][2]) / 2.0,
        -config.box[2][2] / 2.0
    );

	// Find Scene Extends and allocate IntersectionObjects
    {
        glm::vec3 min(10000.0f);
        glm::vec3 max(-10000.0f);
        for(int i = 0; i < config.n_part; ++i){
            int shape_id = config.particles[i].shape_id;
            glm::vec3 pos = config.particles[i].pos + offset;
            float size = config.particles[i].size;
            for(int j = 0; j < 3; ++j){
                if(pos[j] - size * out_radii[shape_id] < min[j]) min[j] = pos[j] - size * out_radii[shape_id];
                if(pos[j] + size * out_radii[shape_id] > max[j]) max[j] = pos[j] + size * out_radii[shape_id];
            }

            if(config.shapes[shape_id].type == Shape::MESH){
                for(int fid = 0; fid < config.shapes[shape_id].mesh.n_vertices; fid += 3){
                    const Vertex* vertices = config.shapes[shape_id].mesh.vertices + fid;
                    glm::vec3 vertex_pos[] = {vertices[0]._coord, vertices[1]._coord, vertices[2]._coord};
                    glm::vec4 rot = config.particles[i].rot;
                    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0), rot.x, glm::vec3(rot.y, rot.z, rot.w));
                    for(int vid = 0; vid < 3; ++vid){
                        vertex_pos[vid] = pos + glm::mat3(rotation_matrix) * (size * vertex_pos[vid]);
                    }
                    Triangle* triangle = new Triangle(vertex_pos[0], vertex_pos[1], vertex_pos[2]);
                    items_.push_back({triangle, i});
                }
            }
            else{
                Sphere* sphere = new Sphere(pos, 0.5);
                items_.push_back({sphere, i});
            }
        }

        // Add 0.1 so that we don't get out of bounds
        scene_bounds_ = {min - 0.1f, max + 0.1f};
    }
	
	//Calculate grid properties
	glm::vec3 gridSize = scene_bounds_.bounds_[1] - scene_bounds_.bounds_[0];
	float cubeRoot = pow((5.0f * items_.size()) / (gridSize[0] * gridSize[1] * gridSize[2]), 0.333);
	for(int i = 0; i < 3; i++){
		float temp = gridSize[i] * cubeRoot;
		temp = std::max(1.0f, std::min(temp, 128.0f)); //Minimum of 1 cell and maximum of 128 cells in each dimension
		n_cells_[i] = (int)temp;
	}
	cell_size_ = gridSize / glm::vec3(n_cells_[0], n_cells_[1], n_cells_[2]);
	
	//Alocate memory
	cells_ = new Cell[n_cells_[0] * n_cells_[1] * n_cells_[2]];
	//Insert Objects in grid
	for(const auto& item: items_){
        //convert AABB to cell coordinates
        glm::vec3 min = item.object_->get_AABB().bounds_[0];
        glm::vec3 max = item.object_->get_AABB().bounds_[1];
        min = (min - scene_bounds_.bounds_[0]) / cell_size_;
        max = (max - scene_bounds_.bounds_[0]) / cell_size_;
        for(int z = int(min.z); z <= int(max.z); z++){
            for(int y = int(min.y); y <= int(max.y); y++){
                for(int x = int(min.x); x <= int(max.x); x++){
                    int index = x + y * n_cells_[0] + z * n_cells_[0] * n_cells_[1];
                    if(index >= n_cells_[0] * n_cells_[1] * n_cells_[2]){
                        printf("Out of bounds! Vec(%d, %d, %d)\n", x, y, z);
                        printf("Scene size: Vec(%d, %d, %d)", n_cells_[0], n_cells_[2], n_cells_[2]);
                    }
                    cells_[index].push_back(&item);
                }
            }
        }
	}
	
}

Grid::~Grid(void){
    delete[] cells_;
    for(auto item: items_) delete item.object_;
}

template<typename T>
inline T clamp(T input, T min, T max){
	return std::max(min, std::min(input, max));
}

bool Grid::raycast(glm::vec3 o, glm::vec3 ray_dir, int& pid){
    float t = FLT_MAX;
	glm::vec3 invDir = 1.0f / ray_dir;
	glm::vec3 deltaT, nextCrossingT;
	glm::ivec3 exitCell, step;
	
	float tmin = 10000.0f;
    glm::vec3 origin = o;
	if(!scene_bounds_.raycast(o, ray_dir, tmin)) return false;
	if(tmin > 0.0f) origin = origin + ray_dir * tmin; //If origin outside box, set origin to hit point
	else tmin = 0.0f;

	//Convert ray origin to cell coordinates
	glm::vec3 rayOrigCell = origin - scene_bounds_.bounds_[0];
	glm::ivec3 cell = glm::ivec3(rayOrigCell / cell_size_);
	for(int i = 0; i < 3; ++i){
		cell[i] = clamp(cell[i], 0, n_cells_[i] - 1);
		if(ray_dir[i] < 0.0f){
			deltaT[i] = -cell_size_[i] * invDir[i];
			nextCrossingT[i] = tmin + (cell[i] * cell_size_[i] - rayOrigCell[i]) * invDir[i];
			exitCell[i] = -1;
			step[i] = -1;
		}
		else{
			deltaT[i] = cell_size_[i] * invDir[i];
			nextCrossingT[i] = tmin + ((cell[i] + 1) * cell_size_[i] - rayOrigCell[i]) * invDir[i];
			exitCell[i] = n_cells_[i];
			step[i] = 1;
		}
	}
	
	//Traverse the cells using 3d-DDA
	float retValue = false;
    int n_tests = 0;
	while(1){
		int index = cell[0] + cell[1] * n_cells_[0] + cell[2] * n_cells_[0] * n_cells_[1];
        for(auto item: cells_[index]){
            ++n_tests;
			if(item->object_->raycast(o, ray_dir, t)){
                retValue = true;
                pid = item->pid_;
            }
		}
		unsigned char k = 
			((nextCrossingT[0] < nextCrossingT[1]) << 2) +
			((nextCrossingT[0] < nextCrossingT[2]) << 1) +
			((nextCrossingT[1] < nextCrossingT[2]));
		static const unsigned char map[8] = {2, 1, 2, 1, 2, 2, 0, 0};
		unsigned char axis = map[k];
		if(t < nextCrossingT[axis]) break;
		cell[axis] += step[axis];
		if(cell[axis] == exitCell[axis]) break;
		nextCrossingT[axis] += deltaT[axis];
	}

    printf("Tested %d triangles\n", n_tests);
	return retValue;
}

