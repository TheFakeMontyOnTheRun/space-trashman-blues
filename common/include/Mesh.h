#ifndef MESH_H
#define MESH_H

struct Mesh {
    uint16_t triangleCount;
    uint8_t *uvCoords;
    FixP_t *geometry;
    struct Texture *texture;
    uint8_t colour;
    struct Vec3* vertices;
    uint16_t *indices;
    uint16_t indexCount;
    void *nativeBuffer;
    void *nativeVertexBuffer;
    void *nativeTexCoordBuffer;
    void *nativeIndicesBuffer;
};

void loadMesh(struct Mesh *mesh, char *filename);

#endif /*MESH_H*/
