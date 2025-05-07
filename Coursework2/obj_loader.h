#pragma once
// obj_loader.h
#pragma once
#include <string>
#include <vector>
struct MeshSegment {
    std::vector<float> vertices; 
    std::string materialName;
    std::string textureFile;      
};

std::vector<MeshSegment> LoadMeshByMaterial(const std::string& inputfile);
std::vector<float> LoadMyObjWithNormals(const std::string& inputfile);
std::vector<float> LoadMyObj(const std::string& inputfile);
struct Vertex {
	float x, y, z;
	float u, v;
};