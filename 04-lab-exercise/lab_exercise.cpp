// ================================================================
// Textured Earth Lab - Guided Starter Version
// ---------------------------------------------------------------
// This file is designed for you to COMPLETE during the lab.
// Lots of TODOs and HINTS are provided to make it easier.
//
// IMPORTANT:
// - Read the lab handout alongside this file.
// - Each "EXERCISE" below matches the handout sections.
// - Do NOT delete the TODO comments; fill code where indicated.
// ================================================================

#include <iostream>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd
#else
    #include <unistd.h>
    #define getcwd getcwd
#endif

// ---------------------------------------------------------------
// Vertex struct: already DONE for you
// position (px,py,pz), normal (nx,ny,nz), texture coordinates (u,v)
// ---------------------------------------------------------------
struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float u, v;
};

static const float PI = 3.14159265359f;

// ---------------------------------------------------------------
// Error callback for GLFW (already done)
// ---------------------------------------------------------------
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "[GLFW] Error (" << error << "): " << description << "\n";
}

// ---------------------------------------------------------------
// Shader helper functions (provided so you can focus on lab tasks)
// You do not need to modify these for the lab.
// ---------------------------------------------------------------
GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "[Shader] Compile error: " << log << "\n";
    } else {
        std::cerr << "[Shader] Compile OK (type " << type << ")\n";
    }
    return shader;
}

GLuint createProgram(const char* vsSrc, const char* fsSrc) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    GLuint prog = glCreateProgram();

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cerr << "[Program] Link error: " << log << "\n";
    } else {
        std::cerr << "[Program] Link OK, id=" << prog << "\n";
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ---------------------------------------------------------------
// Exercise 2 – Texture loading helper (Task 2.1 in handout)
// You DO NOT need to change this function; just CALL it later.
// ---------------------------------------------------------------
GLuint loadTexture(const char* path) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        std::cerr << "[Texture] CWD: " << cwd << "\n";
    }

    int w = 0, h = 0, chans = 0;
    // TODO (READ): This calls SOIL_load_image to read the file from disk.
    // You just need to know that img will point to the pixels if it works.
    unsigned char* img = SOIL_load_image(path, &w, &h, &chans, SOIL_LOAD_AUTO);
    if (!img) {
        std::cerr << "[Texture] SOIL_load_image error: " << SOIL_last_result() << "\n";
        return 0;
    }

    std::cerr << "[Texture] Loaded image " << w << "x" << h
              << " channels=" << chans << "\n";

    GLenum format = (chans == 4) ? GL_RGBA : GL_RGB;

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Upload image to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format,
                 w, h, 0, format,
                 GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Basic texture parameters (already chosen for you)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(img);

    std::cerr << "[Texture] Created GL texture id=" << texID << "\n";
    return texID;
}

// ---------------------------------------------------------------
// Exercise 3 – Sphere generation (Tasks 3.1 & 3.2)
// This function is already fully implemented for you.
// You should READ it and understand the loops.
// ---------------------------------------------------------------
void generateSphere(float radius, int stacks, int slices,
                    std::vector<Vertex>& vertices,
                    std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    std::cerr << "[Sphere] Generating sphere: radius=" << radius
              << " stacks=" << stacks << " slices=" << slices << "\n";

    vertices.reserve((stacks + 1) * (slices + 1));

    // Task 3.1: Build vertices using latitude/longitude.
    for (int i = 0; i <= stacks; ++i) {
        float v = static_cast<float>(i) / stacks;
        float phi = v * PI;
        float y = cosf(phi);
        float r = sinf(phi);

        for (int j = 0; j <= slices; ++j) {
            float u = static_cast<float>(j) / slices;
            float theta = u * 2.0f * PI;
            float x = r * cosf(theta);
            float z = r * sinf(theta);

            Vertex vert;
            vert.px = radius * x;
            vert.py = radius * y;
            vert.pz = radius * z;
            vert.nx = x;
            vert.ny = y;
            vert.nz = z;
            vert.u  = 1.0f - u;  // east-west wrap
            vert.v  = v;         // north-south

            vertices.push_back(vert);
        }
    }

    // Task 3.2: Build indices (two triangles per quad).
    int stride = slices + 1;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int i0 = i * stride + j;
            int i1 = (i + 1) * stride + j;
            int i2 = (i + 1) * stride + (j + 1);
            int i3 = i * stride + (j + 1);

            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    std::cerr << "[Sphere] Vertices: " << vertices.size()
              << "  Indices: " << indices.size() << "\n";
}

// ---------------------------------------------------------------
// Shaders used in Exercise 4 - You do NOT need to edit them.
// ---------------------------------------------------------------
const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out vec3 vNormal;
out vec2 vTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    vTex = aTex;
    gl_Position = proj * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
in vec3 vNormal;
in vec2 vTex;

out vec4 FragColor;

uniform sampler2D earthTex;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(N, L), 0.0);

    vec3 base = texture(earthTex, vTex).rgb;
    vec3 color = base * (0.2 + 0.8 * diff);
    FragColor = vec4(color, 1.0);
}
)";

// ================================================================
// MAIN FUNCTION
// ================================================================
int main() {
    std::cerr << "[Main] Starting\n";

    // -----------------------------------------------------------
    // EXERCISE 1 - Create a window and OpenGL context (5 marks)
    // -----------------------------------------------------------

    // TODO 1.1.1: Set the GLFW error callback.
    glfwSetErrorCallback(glfwErrorCallback);

    // TODO 1.1.2: Initialise GLFW and check it works.
    if (!glfwInit()) {
        std::cerr << "[Main] GLFW init failed\n";
        return -1;
    }
    std::cerr << "[Main] GLFW init OK\n";

    // TODO 1.1.3: Request OpenGL 3.3 core profile and create an 800x600 window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Textured Earth Lab", nullptr, nullptr);
    if (!window) {
        std::cerr << "[Main] Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    std::cerr << "[Main] Window created\n";

    // TODO 1.1.4: Make the context current and enable vsync.
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // TODO 1.1.5: Initialise GLEW and check for errors.
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "[Main] GLEW error: " << glewGetErrorString(glewErr) << "\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    std::cerr << "[Main] GLEW init OK\n";

    // TODO 1.2.1: Enable depth testing ONCE here.
    glEnable(GL_DEPTH_TEST);

    // -----------------------------------------------------------
    // EXERCISE 3 - Generate the sphere mesh (8 marks)
    // -----------------------------------------------------------
    std::vector<Vertex> sphereVerts;
    std::vector<unsigned int> sphereIdx;

    // TODO 3.1: Call generateSphere.
    generateSphere(1.0f, 72, 144, sphereVerts, sphereIdx);

    if (sphereVerts.empty() || sphereIdx.empty()) {
        std::cerr << "[Main] Sphere data is empty - aborting\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // TODO 3.2: Upload sphere data to OpenGL (VAO, VBO, EBO).
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    std::cerr << "[Main] VAO=" << vao << " VBO=" << vbo << " EBO=" << ebo << "\n";

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sphereVerts.size() * sizeof(Vertex),
                 sphereVerts.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sphereIdx.size() * sizeof(unsigned int),
                 sphereIdx.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    std::cerr << "[Main] VAO/VBO/EBO setup done\n";

    // -----------------------------------------------------------
    // EXERCISE 2 - Load a texture with SOIL (6 marks)
    // -----------------------------------------------------------

    // TODO 2.2.1: Call loadTexture once at startup.
    GLuint earthTex = loadTexture("earth.jpg");

    if (!earthTex) {
        std::cerr << "[Main] WARNING: earth.jpg not loaded - texture id=0\n";
    }

    // -----------------------------------------------------------
    // EXERCISE 4 - Shaders, matrices, and draw the Earth (8 marks)
    // -----------------------------------------------------------

    // TODO 4.1.1: Create the shader program.
    GLuint program = createProgram(vertexShaderSrc, fragmentShaderSrc);
    if (!program) {
        std::cerr << "[Main] Program is 0 - aborting\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // TODO 4.1.2: Get uniform locations.
    glUseProgram(program);
    GLint locModel = glGetUniformLocation(program, "model");
    GLint locView  = glGetUniformLocation(program, "view");
    GLint locProj  = glGetUniformLocation(program, "proj");
    GLint locTex   = glGetUniformLocation(program, "earthTex");

    std::cerr << "[Main] Uniform locations: model=" << locModel
              << " view=" << locView
              << " proj=" << locProj
              << " earthTex=" << locTex << "\n";

    // TODO 2.2.3 / 4.1.3: Tell the shader that earthTex uses texture unit 0.
    if (locTex >= 0) {
        glUniform1i(locTex, 0);
    } else {
        std::cerr << "[Main] WARNING: earthTex uniform not found\n";
    }
    glUseProgram(0);

    // TODO: Set a dark blue-ish clear colour.
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);

    std::cerr << "[Main] Entering main loop\n";

    // ============================================================
    // MAIN RENDER LOOP
    // ============================================================
    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;

        // TODO 1.2.2: Set the viewport and clear both colour and depth buffers.
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // -------------------------------------------------------
        // EXERCISE 4.2 - Set model, view, projection matrices.
        // -------------------------------------------------------
        float t = static_cast<float>(glfwGetTime());

        // TODO 4.2.1: Create a model matrix that rotates the sphere over time.
        glm::mat4 model = glm::rotate(glm::mat4(1.0f),
                                      t * 0.3f,
                                      glm::vec3(0.0f, 1.0f, 0.0f));

        // TODO 4.2.2: Create a view matrix.
        glm::mat4 view  = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // TODO 4.2.3: Create a projection matrix.
        glm::mat4 proj  = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(width) / static_cast<float>(height),
            0.1f, 100.0f
        );

        // TODO 4.2.4: Use the shader program and upload matrices.
        glUseProgram(program);
        if (locModel >= 0)
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
        if (locView >= 0)
            glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
        if (locProj >= 0)
            glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(proj));

        // TODO 2.2.2: Activate texture unit 0 and bind earthTex before drawing.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTex);

        // TODO 4.3.1: Bind the VAO and draw.
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereIdx.size()), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        // Swap buffers and poll input events.
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cerr << "[Main] Exiting main loop\n";

    // -----------------------------------------------------------
    // Cleanup
    // -----------------------------------------------------------
    glDeleteTextures(1, &earthTex);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    std::cerr << "[Main] Shutdown\n";
    return 0;
}