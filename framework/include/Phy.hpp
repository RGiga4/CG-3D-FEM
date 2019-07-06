#pragma once

#include "common.hpp"
#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <Eigen/Sparse>

#include <iostream>
#include <fstream>
#include <string>

using Eigen::MatrixXd;
typedef Eigen::Triplet<float> Trip;

class Phy {
public:
	int n_Tet, n_Vet, n_fix;
	float* vertex_body;
	unsigned int* indices_body;
	int* fix;

	Eigen::VectorXf v;//3*8 -12 = 12
	Eigen::VectorXf x;
	Eigen::VectorXf f_u;
	//Eigen::VectorXf* mv;
	//Eigen::VectorXf* b;
	Eigen::VectorXf f_ext;

	Eigen::ConjugateGradient<Eigen::SparseMatrix<float> > solver;
	Eigen::SparseMatrix<float> A;
	Eigen::SparseMatrix<float> K;
	float delta_t;

	std::vector<float> M;
	std::vector<int> new_index;

	//Eigen::VectorXf del_x;

	void setVertex(float* vertex_body, int len);
	void setIndex(unsigned int* indices_body, int len);
	void setFix(int* fix, int n_fix);
	void sett(float t);
	void setfext(Eigen::VectorXf f_ext);
	void setfextNull();
	void setfextZ(float r);
	void compK(std::vector<float>* M, std::vector<Trip>* tripletList, float* pos, unsigned int i, unsigned int j, unsigned int k, unsigned int l);
	void init();
	Eigen::VectorXf update();
	void loaddata(const char* filename);
	void myloadScene(const char* filename, bool smooth, const glm::vec4& color);

	float* vbo_data;
	unsigned int* ibo_data;
	int nodes;
	int faces;
	int size_vbo, size_ibo;

	float* deformsmall;
	float* deformbig;
	float* undeformbig;

	void compLamda();
	std::vector<std::tuple< int, int, float> > lamda;
	void updatevbo();
};

