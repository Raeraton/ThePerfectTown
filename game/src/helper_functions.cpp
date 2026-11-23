#include <render/my/helper_functions.hpp>

constexpr int ERROR_BUFFER_SIZE = 4096;
static char ERROR_BUFFER[ERROR_BUFFER_SIZE];


std::string load_shader_code(const char* path)
{

    namespace fs = std::filesystem;

    if (!fs::exists(path)) {
        std::stringstream err_stream;
        err_stream << "Shader file does not exist: " << path;
        std::snprintf( ERROR_BUFFER, ERROR_BUFFER_SIZE, "%s", err_stream.str().c_str() );
        throw ERROR_BUFFER;
    }

    if (!fs::is_regular_file(path)) {
        std::stringstream err_stream;
        err_stream << "Shader path is not a regular file: " << path;
        std::snprintf( ERROR_BUFFER, ERROR_BUFFER_SIZE, "%s", err_stream.str().c_str() );
        throw ERROR_BUFFER;
    }

    std::ifstream file(path);

    if( !file.is_open() ){
        std::stringstream err_stream;
        err_stream << "unable to open " << path;
        std::snprintf( ERROR_BUFFER, ERROR_BUFFER_SIZE, "%s", err_stream.str().c_str() );
        throw ERROR_BUFFER;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

GLuint create_shader_program( const char* vertex_shader_path, const char* fragment_shader_path ){
    std::string vertexSource = load_shader_code(vertex_shader_path);
    std::string fragmentSource = load_shader_code(fragment_shader_path);

    const char* vertexCode = vertexSource.c_str();
    const char* fragmentCode = fragmentSource.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCode, nullptr);
    glCompileShader(vertexShader);

    GLint success = GL_FALSE;
    char infoLog[1024];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        std::stringstream errstream;
        errstream << "Vertex shader compilation failed: {" << infoLog << '}';
        std::snprintf(ERROR_BUFFER, ERROR_BUFFER_SIZE, "%s", errstream.str().c_str());
        glDeleteShader(vertexShader);
        throw ERROR_BUFFER;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCode, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
        std::stringstream errstream;
        errstream << "Fragment shader compilation failed: {" << infoLog << '}';
        std::snprintf(ERROR_BUFFER, ERROR_BUFFER_SIZE, "%s", errstream.str().c_str());
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        throw ERROR_BUFFER;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::stringstream errstream;
        errstream << "Shader program linking failed: {" << infoLog << '}';
        std::snprintf(ERROR_BUFFER, ERROR_BUFFER_SIZE, "%s", errstream.str().c_str());
        glDeleteProgram(shaderProgram);
        throw ERROR_BUFFER;
    }

    return shaderProgram;
}


GLint get_uniform_location( GLuint shader_program, const char* unif_name ){
    GLint location = glGetUniformLocation( shader_program, unif_name );
    if( location == -1 ){
        std::snprintf(  ERROR_BUFFER, ERROR_BUFFER_SIZE,
                        "uniform not found %s in shader program %d", unif_name, shader_program);
        throw ERROR_BUFFER;
    }
    return location;
}

std::vector<std::string> list_available_uniforms(GLuint program){
    GLint count;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

    std::vector<std::string> out;
    for (GLint i = 0; i < count; ++i)
    {
        char name[256];
        GLsizei length;
        GLint size;
        GLenum type;

        glGetActiveUniform(
            program,
            i,
            sizeof(name),
            &length,
            &size,
            &type,
            name
        );

        out.push_back(name);
    }
    return out;
}


void set_uniform( GLint loc, const glm::mat4& mat ){
    glUniformMatrix4fv(
        loc,
        1,
        GL_FALSE,
        glm::value_ptr(mat)
    );
}

void set_uniform( GLint loc, const glm::vec3& vec ){
    glUniform3f( loc, vec.x, vec.y, vec.z );
}

void set_uniform( GLint loc, float f ){
    glUniform1f( loc, f );
}

void set_uniform( GLint loc, const glm::vec2& vec ){
    glUniform2f( loc, vec.x, vec.y);
}





void create_render_target( GLuint* fbo, GLuint* texture, GLuint* depth, int w, int h ){
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        w,
        h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        *texture,
        0
    );

    glGenRenderbuffers(1, depth);
    glBindRenderbuffer(GL_RENDERBUFFER, *depth);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT24,
        w,
        h
    );

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        *depth
    );

        // Check framebuffer validity
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE) {
        throw "Framebuffer is incomplete!";
    }

}

