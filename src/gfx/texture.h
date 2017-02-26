#ifndef VM_GFX_TEXTURE_H
#define VM_GFX_TEXTURE_H
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

namespace vm {

struct Texture {
    virtual void set_parameter(GLenum parameter, int value);
    virtual void set_parameter(GLenum parameter, float value);
    virtual void generate_mipmaps();
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual int get_depth() const = 0;
    virtual GLenum get_type() const = 0;
    virtual GLuint id() const = 0;
    virtual void clear(float r, float g, float b, float a) const = 0;
};

struct TextureDesc2d {
    int width;
    int height;
    GLenum internal_format;
};

class Texture2d : public Texture {
    GLuint m_id;
    TextureDesc2d m_desc;

public:
    Texture2d(const Texture2d &) = delete;
    Texture2d &operator=(const Texture2d &) = delete;

    Texture2d(const TextureDesc2d &desc);
    ~Texture2d();

    virtual int get_width() const {
        return m_desc.width;
    }

    virtual int get_height() const {
        return m_desc.height;
    }

    virtual int get_depth() const {
        return 1;
    }

    virtual GLenum get_type() const {
        return GL_TEXTURE_2D;
    }

    virtual GLuint id() const {
        return m_id;
    }
};

struct TextureDesc3d {
    int width;
    int height;
    int depth;
    GLenum internal_format;
};

class Texture3d {
    GLuint m_id;
    TextureDesc3d m_desc;

public:
    Texture3d(const Texture3d &) = delete;
    Texture3d &operator=(const Texture3d &) = delete;

    Texture3d(const TextureDesc3d &desc);
    ~Texture3d();

    virtual int get_width() const {
        return m_desc.width;
    }

    virtual int get_height() const {
        return m_desc.height;
    }

    virtual int get_depth() const {
        return m_desc.depth;
    }

    virtual GLenum get_type() const {
        return GL_TEXTURE_3D;
    }

    virtual GLuint id() const {
        return m_id;
    }
};

} // namespace vm

#endif
