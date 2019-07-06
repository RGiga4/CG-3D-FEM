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
	unsigned int vertexShader = compileShader("pudding_min.vert", GL_VERTEX_SHADER);
	unsigned int fragmentShader = compileShader("pudding_min.frag", GL_FRAGMENT_SHADER);
	unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
	// after linking the program the shader objects are no longer needed
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	//unsigned int n_Vet = 8;
	//unsigned int n_Tet = 5;


	Phy Body;


	/*int fix[] = { 0,1,2, 3,4,5, 6,7,8, 9,10,11 };
	int n_fix = 12;*/

	Body.loaddata("..\\blender\\Pudding\\pudding.txt");
	Body.myloadScene("..\\blender\\Pudding\\thepudding.obj", true, glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));


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


	/*Body.setVertex(vertex_body, n_Vet);
	Body.setIndex(indices_body, n_Tet);
	Body.setFix(fix, n_fix);*/

	//getHullTet(&indices[0], 0, 1, 2, 3);





	float delta_t = 1.0 / 60.0;

	Body.sett(delta_t);

	//Eigen::VectorXf f_ext(3 * n_Vet - n_fix);

	//f_ext << 0.0, 0.0, 10.0, 0.0, 0.0, 10.0, 0.0, 0.0, 10.0, 0.0, 0.0, 10.0;
	//f_ext << 0.0, 0.0, 1.0;
	//Body.setfext(&f_ext);
	//Body.setfextZ();
	Eigen::VectorXf f_ext(3 * Body.n_Vet - Body.n_fix);
	Body.f_ext = f_ext;

	Body.setfextZ(-100.);


	Body.init();

	//std::cout << x << std::endl;

	Body.update();



	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	int nodes = Body.nodes;
	int faces = Body.faces;

	unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, Body.size_vbo, Body.vbo_data);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * nodes * sizeof(float)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(6 * nodes * sizeof(float)));
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

	unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, Body.size_ibo, Body.ibo_data);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	std::cout << "size " << Body.size_ibo << " " << std::endl;

	// rendering loop

	start_time = std::chrono::system_clock::now();
	auto now = std::chrono::system_clock::now();
	float time1 = 0.0;

	int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
	glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
	glUniform3fv(light_dir_loc, 1, &light_dir[0]);

	//float* arr = Body.vbo_data;

	/*for (int i = 0; i < nodes; i++) {
		std::cout <<"pos "<<i<<" "<<arr[3*i+0] <<" "<< arr[3 * i + 1] << " " << arr[3 * i + 2] << " " << std::endl;
		std::cout << "nrm " << i << " " << arr[3 * i + 0+3*nodes] << " " << arr[3 * i + 1 + 3 * nodes] << " " << arr[3 * i + 2 + 3 * nodes] << " " << std::endl;

	}*/

	while ((glfwWindowShouldClose(window) == false) && true) {
		now = std::chrono::system_clock::now();

		float delta_time = getTimeDelta();

		//std::cout << delta_time << std::endl;
		if (delta_time + time1 > 16.0) {
			std::cout << "tick" << std::endl;
			time1 += delta_time - 16.0;
			start_time = std::chrono::system_clock::now();
			//std::cout << "Tik" << std::endl;
			Body.update();
			Body.updatevbo();
			//std::cout << getTimeDelta() << std::endl;
			//std::cout << "up" << std::endl;

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			memcpy(ptr, Body.vbo_data, 3 * nodes * sizeof(float));
			glUnmapBuffer(GL_ARRAY_BUFFER);
		}

		//Sleep(700);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// set background color...
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		// and fill screen with it (therefore clearing the window)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// render something...






		glm::mat4 view_matrix = cam.view_matrix();
		glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
		glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

		glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 3 * Body.faces, GL_UNSIGNED_INT, (void*)0);

		// swap buffers == show rendered content
		glfwSwapBuffers(window);
		// process window events
		glfwPollEvents();


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

