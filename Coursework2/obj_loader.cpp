#define TINYOBJLOADER_IMPLEMENTATION
#include "obj_loader.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <unordered_map>

std::vector<float> LoadMyObj(const std::string& inputfile) {
    std::vector<float> vertices;

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            float vx = attrib.vertices[3 * idx.vertex_index + 0];
            float vy = attrib.vertices[3 * idx.vertex_index + 1];
            float vz = attrib.vertices[3 * idx.vertex_index + 2];
            vertices.push_back(vx);
            vertices.push_back(vy);
            vertices.push_back(vz);
        }
    }

    return vertices;

}

std::vector<float> LoadMyObjWithNormals(const std::string& inputfile) {
    std::vector<float> vertices;

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            float vx = attrib.vertices[3 * idx.vertex_index + 0];
            float vy = attrib.vertices[3 * idx.vertex_index + 1];
            float vz = attrib.vertices[3 * idx.vertex_index + 2];

            float nx = 0.0f;
            float ny = 0.0f;
            float nz = 0.0f;
            if (idx.normal_index >= 0) {
                nx = attrib.normals[3 * idx.normal_index + 0];
                ny = attrib.normals[3 * idx.normal_index + 1];
                nz = attrib.normals[3 * idx.normal_index + 2];
            }

            vertices.push_back(vx);
            vertices.push_back(vy);
            vertices.push_back(vz);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    return vertices;
}

std::vector<float> LoadMyObjUV(const std::string& inputfile) {
    std::vector<float> vertices;

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            float vx = attrib.vertices[3 * idx.vertex_index + 0];
            float vy = attrib.vertices[3 * idx.vertex_index + 1];
            float vz = attrib.vertices[3 * idx.vertex_index + 2];
            vertices.push_back(vx);
            vertices.push_back(vy);
            vertices.push_back(vz);

            float tx = 0.0f;
            float ty = 0.0f;
            if (idx.texcoord_index >= 0) {
                tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                ty = attrib.texcoords[2 * idx.texcoord_index + 1];
            }
            vertices.push_back(tx);
            vertices.push_back(ty);
        }
    }

    return vertices;
}

std::vector<MeshSegment> LoadMeshByMaterial(const std::string& inputfile) {
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    config.mtl_search_path = "objs/";

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(inputfile, config)) {
        std::cerr << "TinyObjReader: " << reader.Error() << std::endl;
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& materials = reader.GetMaterials();

    std::unordered_map<int, MeshSegment> segments;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;

        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            int materialID = shape.mesh.material_ids[f];

            MeshSegment& segment = segments[materialID];
            segment.materialName = materialID >= 0 && materialID < materials.size()
                ? materials[materialID].name
                : "default";
            segment.textureFile = materialID >= 0 && materialID < materials.size()
                ? materials[materialID].diffuse_texname
                : "";

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                float vx = attrib.vertices[3 * idx.vertex_index + 0];
                float vy = attrib.vertices[3 * idx.vertex_index + 1];
                float vz = attrib.vertices[3 * idx.vertex_index + 2];

                float tx = 0.0f, ty = 0.0f;
                if (idx.texcoord_index >= 0) {
                    tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                }

                float nx = 0.0f, ny = 0.0f, nz = 0.0f;
                if (idx.normal_index >= 0) {
                    nx = attrib.normals[3 * idx.normal_index + 0];
                    ny = attrib.normals[3 * idx.normal_index + 1];
                    nz = attrib.normals[3 * idx.normal_index + 2];
                }

                segment.vertices.push_back(vx);
                segment.vertices.push_back(vy);
                segment.vertices.push_back(vz);
                segment.vertices.push_back(tx);
                segment.vertices.push_back(ty);
                segment.vertices.push_back(nx);
                segment.vertices.push_back(ny);
                segment.vertices.push_back(nz);

            }

            index_offset += fv;
        }
    }

    std::vector<MeshSegment> result;
    for (auto& kv : segments) {
        result.push_back(kv.second);
    }
    return result;
}
