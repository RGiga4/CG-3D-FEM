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
void compK(std::vector<float>* M, std::vector<Trip>* tripletList, float* pos, unsigned int i, unsigned int j, unsigned int k, unsigned int l);

int
main(int, char* argv[]) {
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

	//unsigned int n_Vet = 8;
	//unsigned int n_Tet = 5;
	unsigned int n_Vet = 4;
	unsigned int n_Tet = 1;

	Phy Body;

	float vertex_body[] = {
		 0.0f,  0.0f, 0.0f,
		 1.0f,  0.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		 0.0f,  0.0f, 1.0f,
		 /*0.0f,  0.0f, 0.0f,
		 1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		 0.0f,  0.0f, 1.0f,
		 1.0f,  0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		 0.0f,  1.0f, 1.0f,*/
	};
	unsigned int indices_body[] = {
		0, 1, 2, 3,
		/*0, 1, 3, 4,
		1, 4, 5, 6,
		1, 2, 3, 6,
		3, 4, 6, 7,
		1, 3, 4, 6*/
	};
	// vertex data
	/*float vertex_data[] = {
		 0.0f,  0.0f, 0.0f,
		 1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		 0.0f,  0.0f, 1.0f,
		 1.0f,  0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		 0.0f,  1.0f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f
	};*/
	float vertex_data[] = {
		 0.0f,  0.0f, 0.0f,
		 1.0f,  0.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
		 0.0f,  0.0f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
		 1.0f, 0.5f, 0.2f, 1.0f,
	};
	unsigned int indices[] = {
		0, 1, 3, 0, 1, 4, 0, 3, 1, 1, 3, 4,
	};
	//int fix[] = { 0,1,2, 3,4,5, 6,7,8, 9,10,11 };//muss sortiert sein
	//int n_fix = 12;
	int fix[] = { 0,1,2, 3,4,5, 6,7,8 };
	int n_fix = 9;// size fo fix


	Body.setVertex(vertex_body, n_Vet);
	Body.setIndex(indices_body, n_Tet);
	Body.setFix(fix, n_fix);

	getHullTet(&indices[0], 0, 1, 2, 3);


	
	/*std::vector<Trip> tripletList;
	tripletList.reserve(n_Tet*144);//es ex doppelte eintarge indices_body

	std::vector<float> M(3 * n_Vet);

	for (int it = 0; it < n_Tet * 4; it += 4) {
		Body.compK(&M ,&tripletList, vertex_body, indices_body[it + 0], indices_body[it + 1], indices_body[it + 2], indices_body[it + 3]);
		
	
	}
	
	

	Eigen::SparseMatrix<float> mat(3*n_Vet, 3*n_Vet);
	//Eigen::SparseMatrix<float> mat(3 * 5, 3 * 5);
	mat.setFromTriplets(tripletList.begin(), tripletList.end());

	glm::vec4 tmp(0.0, 0.0, 0.0, 1.0);
	
	//std::cout << tmp.x <<" "<< tmp.y << " "<< tmp.z << " "<< tmp.w << " " << std::endl;
	
	

	std::vector<int> remap(3*n_Vet);

	for (int i = 0; i < 3*n_Vet; i++) {
		int k = i;
		for (int j = 0; j < n_fix; j++) {
			if (fix[j] < i)k--;
			if (fix[j] == i) {
				k = -1;
				break;
			}

		}
		remap[i] = k;
		
	}

	std::vector<Trip> List;
	List.reserve(n_Tet * 144);

	std::vector<Trip> List_A;// List for matrix A
	List_A.reserve(n_Tet * 144 + 3 * n_Vet);*/

	float delta_t = 1.0 / 60.0;
	/*float t2 = delta_t * delta_t;
	

	for (int i = 0; i < tripletList.size(); i++) {//reduzieren von K
		
		int col = remap[tripletList[i].col()];
		int row = remap[tripletList[i].row()];
		
		if (col == -1 || row == -1)continue;
		
		float val = tripletList[i].value();
		//std::cout << col << " " << row << " " << val << std::endl;
		List.push_back(Trip(row, col, val));
		List_A.push_back(Trip(row, col, val*t2));
	}

	Eigen::SparseMatrix<float> K(3 * n_Vet - n_fix, 3 * n_Vet - n_fix);
	Eigen::SparseMatrix<float> A(3 * n_Vet - n_fix, 3 * n_Vet - n_fix);

	K.setFromTriplets(List.begin(), List.end());
	

	std::vector<int> new_index(3 * n_Vet - n_fix);
	
	int ptr = 0; //ezeugen von remap dateien
	for (int i = 0; i < 3 * n_Vet; i++) {
		if (remap[i] == -1) continue;
		new_index[ptr] = i;
		//std::cout << ptr << " " << i << std::endl;
		ptr++;
		
	}
	Eigen::VectorXf v(3 * n_Vet - n_fix);//3*8 -12 = 12
	Eigen::VectorXf x(3 * n_Vet - n_fix);
	Eigen::VectorXf f_u(3 * n_Vet - n_fix);
	Eigen::VectorXf mv(3 * n_Vet - n_fix);
	Eigen::VectorXf b(3 * n_Vet - n_fix);*/
	Eigen::VectorXf f_ext(3 * n_Vet - n_fix);

	//f_ext << 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0;
	f_ext << 0.0, 0.0, 1.0;
	Body.setfext(&f_ext);

	/*for (int i = 0; i < 3 * n_Vet - n_fix; i++)// kopieren von reduzierem x
		x[i] = vertex_body[new_index[i]];
	for (int i = 0; i < 3 * n_Vet - n_fix; i++)
		v[i] = 0.0;

	
	for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
		int row = new_index[i];
		//std::cout << row <<" "<< i<< std::endl;
		List_A.push_back(Trip(i, i, (1.0+delta_t*0.2)*M[row]));
		
	
	}
	f_u = K * x;//un defomiert
	A.setFromTriplets(List_A.begin(), List_A.end());


	//std::cout <<  A << std::endl;
	Eigen::ConjugateGradient<Eigen::SparseMatrix<float> > solver;
	solver.compute(A);

	Eigen::VectorXf new_v;

	//update loop
	
	for (int i = 0; i < 3 * n_Vet-n_fix; i++) {
		int row = new_index[i];
		
		mv[i] = M[row] * v[i];
		//std::cout << i<<" "<<row << std::endl;
	}
	b = mv - delta_t * (K * x - f_u - f_ext);
	if (solver.info() != Eigen::Success) {
		// decomposition failed
		std::cout << "error in solve" << std::endl;
	}
	new_v = solver.solve(b);
	x = x + delta_t * new_v;//Euler intgration
	v = new_v;

	std::cout << new_v << std::endl;*/

	Body.init();

	//std::cout << x << std::endl;
	
	std::cout << Body.update() << std::endl;

	f_ext << 0.0, 0.0, 0.0;
	Body.setfext(&f_ext);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(vertex_data), vertex_data);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(24 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

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

	while (glfwWindowShouldClose(window) == false) {
		now = std::chrono::system_clock::now();
		
		float delta_time = getTimeDelta();

		//std::cout << delta_time << std::endl;
		if (delta_time + time1 > 1000.0) {
			time1 += delta_time - 1000.0;
			start_time = std::chrono::system_clock::now();
			std::cout << "Tik" << std::endl;
			std::cout << Body.update() << std::endl;
		}
		
		//Sleep(700);

		/*for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
			int row = new_index[i];

			mv[i] = M[row] * v[i];
			//std::cout << i<<" "<<row << std::endl;
		}
		b = mv - delta_t * (K * x - f_u - f_ext);
		if (solver.info() != Eigen::Success) {
			// decomposition failed
			std::cout << "error in solve" << std::endl;
		}
		new_v = solver.solve(b);
		x = x + delta_t * new_v;//Euler intgration
		v = new_v;
		std::cout << x << " \n-----\n" <<std::endl;*/


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
		glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void*)0);

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
void compK(std::vector<float>* M, std::vector<Trip>* tripletList, float* pos, unsigned int i, unsigned int j, unsigned int k, unsigned int l) {
	glm::mat4 coord(1, pos[3 * i], pos[3 * i+1], pos[3 * i+2],   // 1st column colum mayor!!
				1, pos[3 * j], pos[3 * j + 1], pos[3 * j + 2],   // 2nd column
				1, pos[3 * k], pos[3 * k + 1], pos[3 * k + 2],   // 3rd column
				1, pos[3 * l], pos[3 * l + 1], pos[3 * l + 2]);
	float Vol = glm::determinant(coord)/(6.0*4.0);
	if (Vol < 0.0)Vol = -Vol;

	for (int aa = 0; aa < 3; aa++) {
		(*M)[3 * i + aa] += Vol;
		(*M)[3 * j + aa] += Vol;
		(*M)[3 * k + aa] += Vol;
		(*M)[3 * l + aa] += Vol;
	}

	glm::mat4 C = glm::inverse(coord);
	float a1 = C[1][0];
	float a2 = C[1][1];
	float a3 = C[1][2];
	float a4 = C[1][3];
	float b1 = C[2][0];
	float b2 = C[2][1];
	float b3 = C[2][2];
	float b4 = C[2][3];
	float c1 = C[3][0];
	float c2 = C[3][1];
	float c3 = C[3][2];
	float c4 = C[3][3];
	
	//for (int i = 0; i<4; i++)
		//std::cout << C[i][0] << " " << C[i][1] << " " << C[i][2] << " " << C[i][3] << std::endl;
	//std::cout << a1 << " " << a2 << " " << a3 << " " << a4 << std::endl;


	MatrixXd B = MatrixXd::Constant(6, 12, 0);//clom mayor!!
	unsigned int off = 0;
	B(0, 0 + off) = a1;
	B(1, 1 + off) = b1;
	B(2, 2 + off) = c1;
	B(3, 0 + off) = b1;
	B(3, 1 + off) = a1;
	B(4, 1 + off) = c1;
	B(4, 2 + off) = b1;
	B(5, 0 + off) = c1;
	B(5, 2 + off) = a1;

	off = 3;
	B(0, 0 + off) = a2;
	B(1, 1 + off) = b2;
	B(2, 2 + off) = c2;
	B(3, 0 + off) = b2;
	B(3, 1 + off) = a2;
	B(4, 1 + off) = c2;
	B(4, 2 + off) = b2;
	B(5, 0 + off) = c2;
	B(5, 2 + off) = a2;

	off = 6;
	B(0, 0 + off) = a3;
	B(1, 1 + off) = b3;
	B(2, 2 + off) = c3;
	B(3, 0 + off) = b3;
	B(3, 1 + off) = a3;
	B(4, 1 + off) = c3;
	B(4, 2 + off) = b3;
	B(5, 0 + off) = c3;
	B(5, 2 + off) = a3;

	off = 9;
	B(0, 0 + off) = a4;
	B(1, 1 + off) = b4;
	B(2, 2 + off) = c4;
	B(3, 0 + off) = b4;
	B(3, 1 + off) = a4;
	B(4, 1 + off) = c4;
	B(4, 2 + off) = b4;
	B(5, 0 + off) = c4;
	B(5, 2 + off) = a4;

	float nu = 0.3;
	float young = 100;

	MatrixXd E = MatrixXd::Constant(6, 6, 0);
	E(0, 0) = 1.0 - nu;
	E(0, 1) = nu;
	E(0, 2) = nu;
	E(1, 0) = nu;
	E(1, 1) = 1.0-nu;
	E(1, 2) = nu;
	E(2, 0) = nu;
	E(2, 1) = nu;
	E(2, 2) = 1.0-nu;

	E(3, 3) = 0.5 - nu;
	E(4, 4) = 0.5 - nu;
	E(5, 5) = 0.5 - nu;
	
	E = (young / ((1.0 + nu) * (1 - 2.0 * nu))) * E;

	MatrixXd K = B.transpose() * E * B;

	//std::cout << B.transpose() * E * B<< std::endl;
	//std::cout << E << std::endl;
	int i_off[] = { i, j, k, l }; //index offset
	for (int t_a = 0; t_a < 4; t_a++) {
		for (int t_b = 0; t_b < 4; t_b++) {

			for (int t_i = 0; t_i < 3; t_i++) {
				for (int t_j = 0; t_j < 3; t_j++) {

					tripletList->push_back(Trip(3 * i_off[t_b] + t_j, 3 * i_off[t_a] + t_i,
												K(3 * t_b + t_j, 3 * t_a + t_i  )));

				}

			}

		}

	}
	
}
