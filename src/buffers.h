/*
 * glbuffer.h
 *
 *  Created on: Jul 16, 2015
 *      Author: Dylan
 */

#ifndef GLBUFFER_H_
#define GLBUFFER_H_

enum BufferType: GLenum {
    ARRAY = GL_ARRAY_BUFFER,
    ELEMENT_ARRAY = GL_ELEMENT_ARRAY_BUFFER,
    UNIFORM = GL_UNIFORM_BUFFER,
    READ = GL_COPY_READ_BUFFER,
    WRITE = GL_COPY_WRITE_BUFFER
};

enum DrawType: GLenum {
    STATIC = GL_STATIC_DRAW,
    DYNAMIC = GL_DYNAMIC_DRAW,
    STREAM = GL_STREAM_DRAW
};

template<typename DataType> class GLBuffer {
protected:
    int memoryUsage = 0;
public:
    size_t size = 0; // The number of valid elements
    GLuint ID = 0;
    bool created = false;
    BufferType bufferType;
    DrawType drawType;

    static_assert(std::is_standard_layout<DataType>::value,
                "Invalid DataType specified! "
                "The DataType for this GL Buffer is Non-POD (plain-old data), "
                "use of this DataType will lead to undefined behavior.");

    GLBuffer(BufferType bufferType, DrawType drawType): bufferType(bufferType), drawType(drawType) {}

    virtual ~GLBuffer() { deleteBuffer(); }
    virtual void generateBuffer() = 0;

    virtual void bindBuffer(BufferType bufType) { glBindBuffer(bufType, ID); }
    virtual void bindBuffer() final { bindBuffer(bufferType); }

    virtual void deleteBuffer() final {
        if(created) {
            glDeleteBuffers(1, &ID);
            created = false;

            memoryUsage = 0;
        }
    }
};

// This guy is just here to support resizing and stuff. You can't actually instantiate him.
template<typename DataType>
class GLVector: public GLBuffer<DataType> {
private:
    DataType* data;
    float resizeFactor;
    size_t maxSize;
    bool noResize;
private:
    // No data must be sent from CPU to GPU, so this is a surprisingly fast operation
    void copyBuffer(int copySize) {
        GLuint toDelete = this->ID;
        this->ID = 0;

        glGenBuffers(1, &this->ID);
        this->bindBuffer();
        glBufferData(this->bufferType, this->maxSize * sizeof(DataType), NULL, this->drawType);

        glBindBuffer(toDelete, 	READ);
        glBindBuffer(this->ID,	WRITE);
        glCopyBufferSubData(READ, WRITE, 0, 0, copySize * sizeof(DataType));

        glDeleteBuffers(1, &toDelete);
    }

    void upSize(int newSize) {
        int oldSize = maxSize;
        maxSize = newSize;

        DataType* temp = new DataType[newSize];
        for(int i = 0; i < oldSize; i++)
            temp[i] = data[i];
        for(int i = oldSize; i < newSize; i++)
            temp[i] = DataType();
        delete[] data;
        data = temp;

        if(this->created) generateBuffer();
    }

    void downSize(int newSize) {
        maxSize = newSize;

        DataType* temp = new DataType[newSize];
        for(int i = 0; i < newSize; i++)
            temp[i] = data[i];
        delete[] data;
        data = temp;

        if(this->created)  generateBuffer();
    }

protected:
    GLVector(BufferType bufType, DrawType drawType, size_t maxSize = 32, float resizeFactor = 2, bool noResize = false):
        GLBuffer<DataType>(bufType, drawType), resizeFactor(resizeFactor), maxSize(maxSize), noResize(noResize) {
        data = new DataType[maxSize];
        for(int i = 0; i < maxSize; i++) data[i] = DataType();
    }

    virtual void checkForResize() {
        if(!noResize) {
            while(this->size >= maxSize) upSize(maxSize * resizeFactor + 1);
            while(this->size > 32 && this->size < maxSize / (resizeFactor * resizeFactor)) downSize(maxSize / resizeFactor);
        }
    }

    virtual void set(int index, DataType val) { data[index] = val; }
public:
    size_t getMaxSize() { return maxSize; }
    virtual ~GLVector() { delete[] data; }
    const DataType* getData() { return &data[0]; }

    virtual void generateBuffer() final {
        this->deleteBuffer();
        glGenBuffers(1, &this->ID);
        this->bindBuffer();
        glBufferData(this->bufferType, this->maxSize * sizeof(DataType), this->getData(), this->drawType);
        this->created = true;

        this->memoryUsage = this->maxSize * sizeof(DataType);
    }

    virtual void streamData() final {
        this->bindBuffer();
        glBufferData(this->bufferType, this->getMaxSize() * sizeof(DataType), NULL, this->drawType); // Buffer orphaning, a common way to improve streaming perf.
        glBufferSubData(this->bufferType, 0, this->size * sizeof(DataType), this->getData());
    }

    virtual void updatePortion(size_t index, size_t size) final {
        this->bindBuffer();
        glBufferSubData(this->bufferType, index * sizeof(DataType), size * sizeof(DataType), this->getData() + index);
    }

    virtual void copyPortion(size_t readIndex, size_t writeIndex, size_t size) final {
        this->bindBuffer();
        glCopyBufferSubData(this->bufferType, this->bufferType, readIndex * sizeof(DataType), writeIndex * sizeof(DataType), size * sizeof(DataType));
    }
};

template<typename DataType> class StackBuffer: public GLVector<DataType> {
private:
    int minUpdateIndex = 0; // the minimum value reached since last buffer update

    int lastFilledIndex() { return this->size - 1; }
    int firstUnfilledIndex() { return this->size; }
public:
    StackBuffer(BufferType bufferType, DrawType drawType):
        GLVector<DataType>(bufferType, drawType) {}

    void push(DataType val) {
        int index = firstUnfilledIndex();
        this->size++;
        this->checkForResize();
        this->set(index, val);
    }

    void push(std::vector<DataType> vals) {
        for(int i = 0; i < vals.size(); i++) push(vals[i]);
    }

    void pop() {
        this->size--;
        this->checkForResize();
        if(firstUnfilledIndex() < minUpdateIndex)
            minUpdateIndex = firstUnfilledIndex();
    }

    void updateBuffer() {
        this->updatePortion(minUpdateIndex,firstUnfilledIndex()-minUpdateIndex);
        minUpdateIndex = lastFilledIndex();
    }

    void setRegion(size_t start, std::vector<DataType> nData) {
        for(int i = 0; i < (int)nData.size(); i++) {
            GLVector<DataType>::set(start+i,nData[i]);
        }
        this->updatePortion(start,nData.size());
    }
};


#endif /* GLBUFFER_H_ */
