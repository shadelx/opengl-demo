#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"    

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource  ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    std::string line;

    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                //set mode to vextex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                //set mode to fragment
                type = ShaderType::FRAGMENT;

            }
        }
        else {
            ss[(int)type] << line << "\n";
        }
    }

    return { ss[0].str(),ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length); 
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "failed to compile shader" << (type == GL_VERTEX_SHADER ?
            "vertex" : "fragment") << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;

}

static  unsigned int CreateShader(const std::string vertexShader, const std::string& fragmentShader) {
    unsigned int  program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;


}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //syncronize fp
    glfwSwapInterval(1);

    // created after window context is created
    if (glewInit() != GLEW_OK)
        std::cout << "error!!" << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
        -0.5f, -0.5f, // 0
         0.5f, -0.5f, // 1
         0.5f, 0.5f,   // 2
        -0.5f, 0.5f //3
    };

    //define index buffer
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    //define vertex buffers and vertex array
    VertexArray va;
    VertexBuffer vb(positions, 4 * 2 * sizeof(float));    
    va.AddBuffer();

    BufferLayout layout;
    layout.Push<float>(3);
    va.AddLayout(layout);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    //index buffer declaration
    IndexBuffer ib(indices, 6);

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);


    //define uniform
    int location = glGetUniformLocation(shader, "u_Color");
    ASSERT(location != -1);
    glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f);

    //unbind code from vertex array video
    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    float r = 0.0f;
    float increment = 0.05f; 
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //order of things
        glUseProgram(shader);
        glUniform4f(location, r, 0.3f, 0.8f, 1.0f);
        
        glBindVertexArray(vao);
        va.Bind();
        ib.Bind();
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;
        r += increment;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}