#include <chrono>
#include <random>
#include <glm/gtx/transform.hpp>
#include <stb_image.h>

#include "common.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "buffer.hpp"
#include "Phy.hpp"


#include <imgui.hpp>
#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <Eigen/Sparse>




using Eigen::MatrixXd;
typedef Eigen::Triplet<float> Trip;

const int WINDOW_WIDTH = 800;
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
std::chrono::time_point<std::chrono::system_clock> start_time;

void
resizeCallback(GLFWwindow* window, int width, int height);
float getTimeDelta();

const int slowness_factor = 3;

void getHullTet(unsigned int* arr, unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3);



int main(int, char* argv[]) {
	GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	camera cam(window);

	// load and compile shaders and link program
	unsigned int vertexShader = compileShader("basic_colors2.vert", GL_VERTEX_SHADER);
	unsigned int fragmentShader = compileShader("basic_colors2.frag", GL_FRAGMENT_SHADER);
	unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
	// after linking the program the shader objects are no longer needed
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	unsigned int n_Vet = 8;
	unsigned int n_Tet = 5;
	

	Phy Body;

	float vertex_body[] = {//Physik data
		 0.0f,  0.0f, 0.0f,
		 1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		 0.0f,  0.0f, 1.0f,
		 1.0f,  0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		 0.0f,  1.0f, 1.0f,
	};
	unsigned int indices_body[] = {
		0, 1, 3, 4,
		1, 4, 5, 6,
		1, 2, 3, 6,
		3, 4, 6, 7,
		1, 3, 4, 6
	};
	int fix[] = { 0,1,2, 3,4,5, 6,7,8, 9,10,11 };
	int n_fix = 12;

	Body.loaddata("C:\\Users\\roman\\Desktop\\cgintro\\framework\\blender\\Pudding\\pudding.txt");
	// vertex data
	float vertex_data[] = {
		 0.0f,  0.0f, 0.0f,
		 1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		 0.0f,  0.0f, 1.0f,
		 1.0f,  0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		 0.0f,  1.0f, 1.0f,

		 0.0f,  0.0f, 0.0f,
		 0.0f,  0.0f, 0.0f,
		 0.0f,  0.0f, 0.0f,
		 0.0f,  0.0f, 0.0f,
		 0.0f,  0.0f, 0.0f,
		 0.0f,  0.0f, 0.0f,
		 0.0f,  0.0f, 0.0f,
		 0.0f,  0.0f, 0.0f,

		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f
	};
	
	for (int i = 0; i < 8; i++) {
		glm::vec3 nor(vertex_data[3 * i]- 0.5, vertex_data[3 * i+1] - 0.5, vertex_data[3 * i+2] - 0.5);
		nor = glm::normalize(nor);
		vertex_data[24 + 3 * i] = nor[0];
		vertex_data[24 + 3 * i + 1] = nor[1];
		vertex_data[24 + 3 * i + 2] = nor[2];
		//std::cout << nor[0] << " " << nor[1] << " " << nor[2] << std::endl;
	
	}

	unsigned int indices[] = {
		0, 1, 2,  0, 2, 3, 
		0, 1, 5,  0, 4, 5,
		1, 2, 5,  2, 5, 6,
		4, 5, 6,  4, 6, 7,
		0, 3, 4,  3, 4, 7,
		3, 2, 7,  7, 6, 2,
	};
	
	//int fix[] = { 0,1,2, 3,4,5, 6,7,8 };
	//int n_fix = 9;// size fo fix


	Body.setVertex(vertex_body, n_Vet);
	Body.setIndex(indices_body, n_Tet);
	Body.setFix(fix, n_fix);

	//getHullTet(&indices[0], 0, 1, 2, 3);


	
	

	float delta_t = 1.0 / 60.0;

	Body.sett(delta_t);
	
	Eigen::VectorXf f_ext(3 * n_Vet - n_fix);

	f_ext << 0.0, 0.0, 10.0, 0.0, 0.0, 10.0, 0.0, 0.0, 10.0, 0.0, 0.0, 10.0;
	//f_ext << 0.0, 0.0, 1.0;
	Body.setfext(&f_ext);

	

	Body.init();

	//std::cout << x << std::endl;
	
	std::cout << Body.update() << std::endl;

	f_ext << 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
	Body.setfext(&f_ext);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertex_data), vertex_data);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(24 * sizeof(float)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(48 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glUseProgram(shaderProgram);
	int model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
	int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
	int proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");

	proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
	float angle = -0.25 * M_PI;
	glm::mat4 model_matrix = glm::rotate(angle, glm::vec3(1.f, 0.f, 0.f));

	unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(indices), indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// rendering loop

	start_time = std::chrono::system_clock::now();
	auto now = std::chrono::system_clock::now();
	float time1 = 0.0;

	int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
	glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
	glUniform3fv(light_dir_loc, 1, &light_dir[0]);

	while (glfwWindowShouldClose(window) == false) {
		now = std::chrono::system_clock::now();
		
		float delta_time = getTimeDelta();

		//std::cout << delta_time << std::endl;
		if (delta_time + time1 > 16.0) {
			time1 += delta_time - 16.0;
			start_time = std::chrono::system_clock::now();
			//std::cout << "Tik" << std::endl;
			Body.update();
			//std::cout << getTimeDelta() << std::endl;
		}
		
		//Sleep(700);

		


		// set background color...
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		// and fill screen with it (therefore clearing the window)
		glClear(GL_COLOR_BUFFER_BIT);

		// render something...
		
		
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
			vertex_data[Body.new_index[i]] = Body.x[i];
		}
		memcpy(ptr, vertex_data, sizeof(vertex_data));
		glUnmapBuffer(GL_ARRAY_BUFFER);
		


		glm::mat4 view_matrix = cam.view_matrix();
		glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

		glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);
		
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);

		// swap buffers == show rendered content
		glfwSwapBuffers(window);
		// process window events
		glfwPollEvents();
		//break;
	}


	glfwTerminate();

	return 0;
}



void resizeCallback(GLFWwindow*, int width, int height)
{
	// set new width and height as viewport size
	glViewport(0, 0, width, height);
	proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}

float getTimeDelta() {
	auto now = std::chrono::system_clock::now();
	return static_cast<float>((std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count()));
}
void getHullTet(unsigned int* arr, unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {
	unsigned int t[12] = { a0, a1, a2, a0, a1, a3, a0, a2, a1, a1, a2, a3 };
	for (int i = 0; i < 12; i++) {
		arr[i] = t[i];
	}
	
}

