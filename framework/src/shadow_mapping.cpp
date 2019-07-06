#include "common.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "camera.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 600;
const int SHADOW_WIDTH = 2000;
const int SHADOW_HEIGHT = 2000;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;

int window_width = WINDOW_WIDTH;
int window_height = WINDOW_HEIGHT;

glm::mat4 proj_matrix;
unsigned int fbo = 0;
unsigned int framebuffer_tex = 0;
unsigned int depth_rbo = 0;
glm::vec2 half_size_near;

unsigned int lamp_framebuffer_tex = 0;
unsigned int lamp_depth_fbo = 0;

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

unsigned int
create_texture_rgba32f(int width, int height) {
    unsigned int handle;
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, GL_RGBA32F, width, height);

    return handle;
}

unsigned int
setup_fullscreen_quad() {
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
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

    return VAO;
}

void build_framebuffer(unsigned int& framebuffer_tex, unsigned int& depth_rbo, unsigned int& fbo, int width, int height) {
    if (framebuffer_tex) {
        glDeleteTextures(1, &framebuffer_tex);
    }

    if (depth_rbo) {
        glDeleteRenderbuffers(1, &depth_rbo);
    }

    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
    }

    framebuffer_tex = create_texture_rgba32f(width, height);
    glCreateRenderbuffers(1, &depth_rbo);
    glNamedRenderbufferStorage(depth_rbo, GL_DEPTH_COMPONENT32, width, height);

    glCreateFramebuffers(1, &fbo);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, framebuffer_tex, 0);
    glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbo);
    if(glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Incomplete FBO!");
        std::terminate();
    }
}

// Builds a framebuffer where the scene is rendered in light space
void build_depth_framebuffer(unsigned int& framebuffer_tex, unsigned int& depth_fbo, int width, int height) {
	if (framebuffer_tex) {
		glDeleteTextures(1, &framebuffer_tex);
	}

	if (depth_fbo) {
		glDeleteRenderbuffers(1, &depth_fbo);
	}

	glGenFramebuffers(1, &depth_fbo);

	glGenTextures(1, &framebuffer_tex);
	glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer_tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);

	// load and compile shaders and link program

	// Shader for depth texture in lamp's space
	unsigned int vertexShaderDepth = compileShader("shadow_mapping_depth.vert", GL_VERTEX_SHADER);
	unsigned int fragmentShaderDepth = compileShader("shadow_mapping_depth.frag", GL_FRAGMENT_SHADER);
	unsigned int shaderProgramDepth = linkProgram(vertexShaderDepth, fragmentShaderDepth);
	glDeleteShader(vertexShaderDepth);
	glDeleteShader(fragmentShaderDepth);

    /*
	unsigned int vertexShaderGeometry = compileShader("deferred_geometry.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShaderGeometry = compileShader("deferred_geometry.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgramGeometry = linkProgram(vertexShaderGeometry, fragmentShaderGeometry);
    glDeleteShader(fragmentShaderGeometry);
    glDeleteShader(vertexShaderGeometry);
	*/
	std::vector<geometry> objects = loadScene("cornell_box_modified.obj", true);
	
	/*
    build_framebuffer(framebuffer_tex, depth_rbo, fbo, WINDOW_WIDTH, WINDOW_HEIGHT);
	*/
	build_depth_framebuffer(lamp_framebuffer_tex, lamp_depth_fbo, SHADOW_WIDTH, SHADOW_HEIGHT);

    unsigned int quad = setup_fullscreen_quad();
    unsigned int vertexShaderCompose = compileShader("shadow_mapping_compose.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShaderCompose = compileShader("shadow_mapping_compose.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgramCompose = linkProgram(vertexShaderCompose, fragmentShaderCompose);
    glDeleteShader(fragmentShaderCompose);
    glDeleteShader(vertexShaderCompose);

    glUseProgram(shaderProgramCompose);

	int model_mat_loc = glGetUniformLocation(shaderProgramCompose, "model_mat");
	int view_mat_loc = glGetUniformLocation(shaderProgramCompose, "view_mat");
	int proj_mat_loc = glGetUniformLocation(shaderProgramCompose, "proj_mat");
	int lamp_tex_loc = glGetUniformLocation(shaderProgramCompose, "shadowMap");

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
    float fov_tan = std::tan(0.5f * FOV);
    half_size_near = glm::vec2(fov_tan, fov_tan);


	int light_space_matrix_loc = glGetUniformLocation(shaderProgramDepth, "light_space_matrix");
	int depth_model_matrix_loc = glGetUniformLocation(shaderProgramDepth, "model");

	int compose_light_space_matrix_loc = glGetUniformLocation(shaderProgramCompose, "light_space_matrix");

    // rendering loop
	while (glfwWindowShouldClose(window) == false) {
		// DEPTH MAP PASS
		glBindFramebuffer(GL_FRAMEBUFFER, lamp_depth_fbo);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::mat4 light_projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, NEAR_VALUE, FAR_VALUE);
		glm::mat4 light_view = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 light_space_matrix = light_projection * light_view;

		glUseProgram(shaderProgramDepth);
		glUniformMatrix4fv(light_space_matrix_loc, 1, GL_FALSE, &light_space_matrix[0][0]);

		for (unsigned int i = 0; i < objects.size(); i++) {
			glm::mat4 model_matrix = objects[i].transform;
			glUniformMatrix4fv(depth_model_matrix_loc, 1, GL_FALSE, &model_matrix[0][0]);

			objects[i].bind();
			glDrawElements(GL_TRIANGLES, objects[i].vertex_count, GL_UNSIGNED_INT, (void *) 0);
		}

        /*
		// GEOMETRY PASS
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glUseProgram(shaderProgramGeometry);
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        for (unsigned i = 0; i < objects.size(); ++i) {
            glm::mat4 model_matrix = objects[i].transform;
            glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);

            objects[i].bind();
            glDrawElements(GL_TRIANGLES, objects[i].vertex_count, GL_UNSIGNED_INT, (void*) 0);
        }
		*/

		glm::mat4 view_matrix = cam.view_matrix();

        // COMPOSE PASS
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glUseProgram(shaderProgramCompose);

		glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);
		glUniformMatrix4fv(compose_light_space_matrix_loc, 1, GL_FALSE, &light_space_matrix[0][0]);

		glBindTexture(GL_TEXTURE_2D, lamp_framebuffer_tex);
		glUniform1i(lamp_tex_loc, 0);

		for (unsigned i = 0; i < objects.size(); ++i) {
			glm::mat4 model_matrix = objects[i].transform;
			glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);

			objects[i].bind();
			glDrawElements(GL_TRIANGLES, objects[i].vertex_count, GL_UNSIGNED_INT, (void*)0);
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteFramebuffers(1, &fbo);
    for (unsigned int i = 0; i < objects.size(); ++i) {
        objects[i].destroy();
    }

    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);

    float aspect = static_cast<float>(width) / height;
    half_size_near.x = aspect * half_size_near.y;
}
