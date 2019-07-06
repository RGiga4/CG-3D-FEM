#include <random>
#include <glm/gtx/transform.hpp>
#include <stb_image.h>

#include "common.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "buffer.hpp"

#include <imgui.hpp>

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;
const int TEXTURE_WIDTH = 20;
const int TEXTURE_HEIGHT = 20;

#ifndef M_PI
#define M_PI 3.14159265359
#endif

glm::mat4 proj_matrix;

void
resizeCallback(GLFWwindow* window, int width, int height);

float*
load_texture_data(std::string filename, int* width, int* height) {
    int channels;
    unsigned char* file_data = stbi_load(filename.c_str(), width, height, &channels, 3);

    int w = *width;
    int h = *height;

    float* data = new float[4 * w * h];
    for (int j=0; j < h; ++j) {
        for (int i=0; i < w; ++i) {
            data[j*w*4 + i*4 + 0] = static_cast<float>(file_data[j*w*3 + i*3 + 0]) / 255;
            data[j*w*4 + i*4 + 1] = static_cast<float>(file_data[j*w*3 + i*3 + 1]) / 255;
            data[j*w*4 + i*4 + 2] = static_cast<float>(file_data[j*w*3 + i*3 + 2]) / 255;
            data[j*w*4 + i*4 + 3] = 1.f;
        }
    }

    delete [] file_data;

    return data;
}

float*
random_texture_data(int width, int height) {
    float* data = new float[4 * width * height];
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    for (int j=0; j<height; ++j) {
        for (int i=0; i<width; ++i) {
            data[j*width*4 + i*4 + 0] = dist(rng);
            data[j*width*4 + i*4 + 1] = dist(rng);
            data[j*width*4 + i*4 + 2] = dist(rng);
            data[j*width*4 + i*4 + 3] = 1.f;
        }
    }

    return data;
}

float*
gradient_texture_data(int width, int height) {
    float* data = new float[4 * width * height];
    for (int j=0; j<height; ++j) {
        float red = static_cast<float>(j) / (height-1);
        for (int i=0; i<width; ++i) {
            float green = static_cast<float>(i) / (width-1);
            data[j*width*4 + i*4 + 0] = red;
            data[j*width*4 + i*4 + 1] = green;
            data[j*width*4 + i*4 + 2] = 0.f;
            data[j*width*4 + i*4 + 3] = 1.f;
        }
    }

    return data;
}

unsigned int
create_texture_rgba32f(int width, int height, float* data) {
    unsigned int handle;
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, GL_RGBA32F, width, height);
    glTextureSubImage2D(handle, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);

    return handle;
}

void
set_texture_filter_mode(unsigned int texture, GLenum mode) {
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mode);
}

void
set_texture_wrap_mode(unsigned int texture, GLenum mode) {
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, mode);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, mode);
}


int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);

    init_imgui(window);

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("uebung_6.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("uebung_6.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    // vertex data
    float vertices[] = {
        -1.0f, -1.0f, 0.f,
         1.0f, -1.0f, 0.f,
         1.0f,  1.0f, 0.f,
        -1.0f,  1.0f, 0.f
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0
    };

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glUseProgram(shaderProgram);
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");
    int tex_loc = glGetUniformLocation(shaderProgram, "tex");
    int show_edge_wrap_loc = glGetUniformLocation(shaderProgram, "show_edge_wrap");

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
    float angle = -0.25 * M_PI;
    glm::mat4 model_matrix = glm::rotate(angle, glm::vec3(1.f, 0.f, 0.f));

    float* random_tex_data = random_texture_data(TEXTURE_WIDTH, TEXTURE_HEIGHT);
    float* gradient_tex_data = gradient_texture_data(TEXTURE_WIDTH, TEXTURE_HEIGHT);
    int image_width, image_height;
	std::string DATA_ROOT = "../data/";
    float* image_tex_data = load_texture_data(DATA_ROOT + "blue_painted_planks.png", &image_width, &image_height);

    unsigned int gradient_tex = create_texture_rgba32f(TEXTURE_WIDTH, TEXTURE_HEIGHT, gradient_tex_data);
    unsigned int random_tex = create_texture_rgba32f(TEXTURE_WIDTH, TEXTURE_HEIGHT, random_tex_data);
    unsigned int image_tex = create_texture_rgba32f(image_width, image_height, image_tex_data);

    glBindTextureUnit(0, gradient_tex);
    glBindTextureUnit(1, random_tex);
    glBindTextureUnit(2, image_tex);

    delete [] random_tex_data;
    delete [] gradient_tex_data;
    delete [] image_tex_data;

    glEnable(GL_DEPTH_TEST);

    int chosen_texture = 0;
    const char* texture_choices[] = { "Gradient", "Random", "Loaded Image" };
    int bilinear = 0;
    const char* sampling_methods[] = { "Nearest Neighbor", "Bilinear" };
    int wrap_mode = 0;
    const char* wrap_modes[] = { "GL_CLAMP_TO_EDGE", "GL_CLAMP_TO_BORDER", "GL_MIRRORED_REPEAT", "GL_REPEAT", "GL_MIRROR_CLAMP_TO_EDGE" };
    bool show_edge_wrap = false;

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        glfwPollEvents();
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // define UI
        imgui_new_frame(400, 200);
        ImGui::Begin("Texturing");
        ImGui::Combo("Sampling Method", &bilinear, sampling_methods, 2);
        ImGui::Combo("Wrap Mode", &wrap_mode, wrap_modes, 5);
        ImGui::Combo("Texture", &chosen_texture, texture_choices, 3);
        ImGui::Checkbox("Show Edge Wrap", &show_edge_wrap);
        ImGui::End();

        GLenum filter_mode = bilinear ? GL_LINEAR : GL_NEAREST;
        set_texture_filter_mode(gradient_tex, filter_mode);
        set_texture_filter_mode(random_tex, filter_mode);
        set_texture_filter_mode(image_tex, filter_mode);

        GLenum tex_wrap_mode;
        switch (wrap_mode) {
            case 0:  tex_wrap_mode = GL_CLAMP_TO_EDGE;
                     break;
            case 1:  tex_wrap_mode = GL_CLAMP_TO_BORDER;
                     break;
            case 2:  tex_wrap_mode = GL_MIRRORED_REPEAT;
                     break;
            case 3:  tex_wrap_mode = GL_REPEAT;
                     break;
            default: tex_wrap_mode = GL_MIRROR_CLAMP_TO_EDGE;
                     break;
        }
        set_texture_wrap_mode(gradient_tex, tex_wrap_mode);
        set_texture_wrap_mode(random_tex, tex_wrap_mode);
        set_texture_wrap_mode(image_tex, tex_wrap_mode);

        glUniform1i(tex_loc, chosen_texture);
        glUniform1i(show_edge_wrap_loc, static_cast<int>(show_edge_wrap));

        glm::mat4 view_matrix = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*) 0);

        // render UI
        imgui_render();
        glfwSwapBuffers(window);
    }

    cleanup_imgui();
    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
