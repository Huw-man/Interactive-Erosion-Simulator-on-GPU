// This is from stackoverflow 
// https://stackoverflow.com/questions/5915753/generate-a-plane-with-triangle-strips
// Thank you, random internet person

#include <GL/glew.h>

#ifndef PLANE_MESH_HPP
#define PLANE_MESH_HPP

struct PlaneMesh {
    size_t nVerts, nInds;
    float* verts;
    int  * inds;
    GLuint vbuf, ibuf;

private:
    inline int getVerticesCount( int width, int height ) {
        return width * height * 2;
    }

    inline int getIndicesCount( int width, int height ) {
        return (width*height) + (width-1)*(height-2);
    }

    inline float* getVertices( int width, int height ) {
        float *vertices = new float[ getVerticesCount( width, height ) ];
        int i = 0;

        for ( int row=0; row<height; row++ ) {
            for ( int col=0; col<width; col++ ) {
                vertices[i++] = ((float) col) / (width-1);
                vertices[i++] = ((float) row) / (height-1);
            }
        }

        return vertices;
    }

    inline int* getIndices( int width, int height ) {
        int *indices = new int[ getIndicesCount(width, height) ];
        int i = 0;

        for ( int row=0; row<height-1; row++ ) {
            if ( (row&1)==0 ) { // even rows
                for ( int col=0; col<width; col++ ) {
                    indices[i++] = col + row * width;
                    indices[i++] = col + (row+1) * width;
                }
            } else { // odd rows
                for ( int col=width-1; col>0; col-- ) {
                    indices[i++] = col + (row+1) * width;
                    indices[i++] = col - 1 + row * width;
                }
            }
        }
        //indices[i+1] = indices[i];
        //indices[i+2] = indices[i];
        /*
        if ( (mHeight&1) && mHeight>2 ) {
            mpIndices[i++] = (mHeight-1) * mWidth;
        }*/

        return indices;
    }

    void makeBuffers();

public:
    PlaneMesh(glm::ivec2 size) {
        this->nVerts   = getVerticesCount(size.x,size.y);
        this->nInds    = getIndicesCount(size.x,size.y);
        this->verts = getVertices(size.x,size.y);
        this->inds  = getIndices(size.x,size.y);
        makeBuffers();
    }
    void render();

};

#endif // PLANE_MESH_HPP