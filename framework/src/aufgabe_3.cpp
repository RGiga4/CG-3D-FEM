#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include "camera.hpp"
#include "mesh.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;
const float FOV = 45.f;
const float NEAR_ = 0.1f;
const float FAR_ = 100.f;
const float SUN_EARTH_DISTANCE = 5.f;
const float EARTH_MOON_DISTANCE = 2.f;

const int slowness_factor = 3;

glm::mat4 proj_matrix;
std::chrono::time_point<std::chrono::system_clock> start_time;

float getTimeDelta();

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);

    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("mesh_render.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("mesh_render.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    geometry sun = loadMesh("sphere.obj", false, glm::vec4(1.f, 0.6f, 0.f, 1.f));
    geometry earth = loadMesh("sphere.obj", false, glm::vec4(0.f, 0.0f, 1.f, 1.f));
    geometry moon = loadMesh("sphere.obj", false, glm::vec4(0.5f, 0.5f, 0.5f, 1.f));

    glUseProgram(shaderProgram);
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_, FAR_);
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    glEnable(GL_DEPTH_TEST);

    start_time = std::chrono::system_clock::now();

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 view_matrix = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        float time_year = getTimeDelta();
        float time_month = std::fmod(time_year * 12.f, 1.f);

        // render sun
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &sun.transform[0][0]);
        sun.bind();
        glDrawElements(GL_TRIANGLES, sun.vertex_count, GL_UNSIGNED_INT, (void*) 0);

        // render earth
        earth.transform = sun.transform *
                          glm::rotate<float>(2 * M_PI * time_year, glm::vec3(0.f, 1.f, 0.f)) *
                          glm::translate(glm::vec3(SUN_EARTH_DISTANCE, 0.f, 0.f)) *
                          glm::scale<float>(glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &earth.transform[0][0]);
        earth.bind();
        glDrawElements(GL_TRIANGLES, earth.vertex_count, GL_UNSIGNED_INT, (void*) 0);

        // TODO: use EARTH_MOON_DISTANCE and time_month variables to transform and render moon
		moon.transform = earth.transform * sun.transform *
			glm::rotate<float>(2 * M_PI * time_month, glm::vec3(0.f, 1.f, 0.f)) *
			glm::translate(glm::vec3(EARTH_MOON_DISTANCE, 0.f, 0.f)) *
			glm::scale<float>(glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &moon.transform[0][0]);
		moon.bind();
		glDrawElements(GL_TRIANGLES, moon.vertex_count, GL_UNSIGNED_INT, (void*) 0);

        // swap buffers == show rendered content
        glfwSwapBuffers(window);
        // process window events
        glfwPollEvents();
    }


    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_, FAR_);
}

float getTimeDelta() {
    auto now = std::chrono::system_clock::now();
    return static_cast<float>((std::chrono::duration_cast<std::chrono::milliseconds>(now-start_time).count() % (5000 * slowness_factor)) / 5000.f) / slowness_factor;
}
