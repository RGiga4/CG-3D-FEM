#include "Phy.hpp"

void Phy::init() {
	std::vector<Trip> tripletList;
	tripletList.reserve(n_Tet * 144);//es ex doppelte eintarge indices_body

	std::vector<float> M(3 * n_Vet);

	for (int it = 0; it < n_Tet * 4; it += 4) {
		compK(&M, &tripletList, vertex_body, indices_body[it + 0], indices_body[it + 1], indices_body[it + 2], indices_body[it + 3]);


	}
	this->M = M;


	Eigen::SparseMatrix<float> mat(3 * n_Vet, 3 * n_Vet);
	//Eigen::SparseMatrix<float> mat(3 * 5, 3 * 5);
	mat.setFromTriplets(tripletList.begin(), tripletList.end());

	



	std::vector<int> remap(3 * n_Vet);

	for (int i = 0; i < 3 * n_Vet; i++) {
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
	List_A.reserve(n_Tet * 144 + 3 * n_Vet);

	delta_t = 1.0 / 60.0;
	float t2 = delta_t * delta_t;


	for (int i = 0; i < tripletList.size(); i++) {//reduzieren von K

		int col = remap[tripletList[i].col()];
		int row = remap[tripletList[i].row()];

		if (col == -1 || row == -1)continue;

		float val = tripletList[i].value();
		//std::cout << col << " " << row << " " << val << std::endl;
		List.push_back(Trip(row, col, val));
		List_A.push_back(Trip(row, col, val * t2));
	}

	Eigen::SparseMatrix<float> K(3 * n_Vet - n_fix, 3 * n_Vet - n_fix);
	Eigen::SparseMatrix<float> A(3 * n_Vet - n_fix, 3 * n_Vet - n_fix);

	K.setFromTriplets(List.begin(), List.end());

	this->K = K;

	std::vector<int> new_index(3 * n_Vet - n_fix);

	int ptr = 0; //ezeugen von remap dateien
	for (int i = 0; i < 3 * n_Vet; i++) {
		if (remap[i] == -1) continue;
		new_index[ptr] = i;
		//std::cout << ptr << " " << i << std::endl;
		ptr++;

	}

	this->new_index = new_index;

	Eigen::VectorXf v(3 * n_Vet - n_fix);//3*8 -12 = 12
	Eigen::VectorXf x(3 * n_Vet - n_fix);
	Eigen::VectorXf f_u(3 * n_Vet - n_fix);
	Eigen::VectorXf mv(3 * n_Vet - n_fix);
	Eigen::VectorXf b(3 * n_Vet - n_fix);
	

	for (int i = 0; i < 3 * n_Vet - n_fix; i++)// kopieren von reduzierem x
		x[i] = vertex_body[new_index[i]];
	for (int i = 0; i < 3 * n_Vet - n_fix; i++)
		v[i] = 0.0;

	this->x = x;
	this->v = v;

	for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
		int row = new_index[i];
		//std::cout << row <<" "<< i<< std::endl;
		List_A.push_back(Trip(i, i, (1.0 + delta_t * 0.2) * M[row]));


	}
	f_u = K * x;//un defomiert
	this->f_u = f_u;
	A.setFromTriplets(List_A.begin(), List_A.end());
	this->A = A;
	
	solver.compute(this->A);

	compLamda();
	
}

Eigen::VectorXf Phy::update() {
	Eigen::VectorXf mv(3 * n_Vet - n_fix);
	Eigen::VectorXf new_v(3 * n_Vet - n_fix);
	Eigen::VectorXf b(3 * n_Vet - n_fix);
	for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
		int row = new_index[i];
		
		mv[i] = M[row] * v[i];
		//std::cout << i<<" "<<row<< std::endl;
		
	}
	
	b = mv - delta_t * (K * x - f_u - (f_ext));
	
	if (solver.info() != Eigen::Success) {
		// decomposition failed
		std::cout << "error in solve" << std::endl;
	}
	new_v = solver.solve(b);
	//std::cout << new_v << " " << std::endl;
	x = x + delta_t * new_v;//Euler intgration

	for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
		deformsmall[i] += delta_t * new_v[i];
		//if(i>=0 && i<3)std::cout <<i<<" "<< new_v[i] << " " << std::endl;
	}

	v = new_v;

	//std::cout << "v "<<new_v << std::endl;

	return x;

}

void Phy::setVertex(float* vertex_body, int len) {
	this->vertex_body = vertex_body;
	n_Vet = len;
}
void Phy::setIndex(unsigned int* indices_body, int len) {
	this->indices_body = indices_body;
	n_Tet = len;
}
void Phy::setFix(int* fix, int len) {
	this->fix = fix;
	n_fix = len;
}
void Phy::setfext(Eigen::VectorXf f_ext) {
	this->f_ext = f_ext;
}
void Phy::setfextNull() {
	for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
		f_ext[i] = 0.0;
	}

}
void Phy::setfextZ(float r) {
	
	for (int i = 0; i < 3 * n_Vet - n_fix; i++) {
		if (i % 3 == 0)f_ext[i] = r;
		else f_ext[i] = 0.0;
	}
	
}
void Phy::sett(float t) {
	this->delta_t = t;
}

void Phy::compK(std::vector<float>* M, std::vector<Trip>* tripletList, float* pos, unsigned int i, unsigned int j, unsigned int k, unsigned int l) {
	glm::mat4 coord(1, pos[3 * i], pos[3 * i + 1], pos[3 * i + 2],   // 1st column colum mayor!!
		1, pos[3 * j], pos[3 * j + 1], pos[3 * j + 2],   // 2nd column
		1, pos[3 * k], pos[3 * k + 1], pos[3 * k + 2],   // 3rd column
		1, pos[3 * l], pos[3 * l + 1], pos[3 * l + 2]);
	float Vol = glm::determinant(coord) / (6.0 * 4.0);
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
	float young = 3.0;

	MatrixXd E = MatrixXd::Constant(6, 6, 0);
	E(0, 0) = 1.0 - nu;
	E(0, 1) = nu;
	E(0, 2) = nu;
	E(1, 0) = nu;
	E(1, 1) = 1.0 - nu;
	E(1, 2) = nu;
	E(2, 0) = nu;
	E(2, 1) = nu;
	E(2, 2) = 1.0 - nu;

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
						K(3 * t_b + t_j, 3 * t_a + t_i)));

				}

			}

		}

	}

}
void Phy::loaddata(const char* filename) {
	std::ifstream infile;
	infile.open(filename);

	int mode = 0;
	int ak_Vet = 0, ak_Tet = 0, ak_fix = 0;
	//int n_Vet, n_Tet, n_fix;

	float* Ver_data;
	unsigned int* Tet_data;
	int* fix_data;

	if (infile.is_open()) {
		std::string line;
		while (getline(infile, line)) {

			//printf("%s\n", line.c_str());

			std::string::size_type sz = 0;     // alias of size_t

			if (mode == 0) {

				//n_Ver = std::stoi(line.substr(sz), &sz);



				//n_Tet = std::stoi(line.substr(sz), &sz);

				sscanf(line.c_str(), "%i, %i, %i", &n_Vet, &n_Tet, &n_fix);

				Ver_data = static_cast<float*>(malloc(sizeof(float) * n_Vet * 3));
				deformsmall = static_cast<float*>(malloc(sizeof(float) * n_Vet * 3));
				Tet_data = static_cast<unsigned int*>(malloc(sizeof(unsigned int) * n_Tet *4));
				fix_data = static_cast<int*>(malloc(sizeof(int) * n_fix * 3));

				mode = 1;
				continue;
			}
			if (mode == 1) {//points
				int k;
				float x, y, z;

				sscanf(line.c_str(), "%i, %f, %f, %f", &k, &x, &y, &z);

				Ver_data[3 * ak_Vet] = x;
				Ver_data[3 * ak_Vet +1] = y;
				Ver_data[3 * ak_Vet+2] = z;

				deformsmall[3 * ak_Vet] = 0.;
				deformsmall[3 * ak_Vet+1] = 0.;
				deformsmall[3 * ak_Vet+2] = 0.;

				//std::cout << "Hie: " << x << " " << y << " " << z << std::endl;
				ak_Vet++;
				if (ak_Vet == n_Vet) {
					mode = 2;
					continue;
				}

			}
			if (mode == 2) {//Tet
				int h;
				unsigned int i, j, k, l;

				sscanf(line.c_str(), "%i, %i, %i, %i, %i", &h, &i, &j, &k, &l);

				Tet_data[4 * ak_Tet] = i-1;
				Tet_data[4 * ak_Tet+1] = j-1;
				Tet_data[4 * ak_Tet+2] = k-1;
				Tet_data[4 * ak_Tet+3] = l-1;

				//std::cout << "Hiet: " << i << " " << j << " " << k << " "<< l << std::endl;

				ak_Tet++;
				if (ak_Tet == n_Tet) {
					mode = 3;
					continue;
				}

			}
			if (mode == 3) {//fix

				int i;

				sscanf(line.c_str(), "%i, ", &i);

				fix_data[3*ak_fix] = 3*(i-1);
				fix_data[3 * ak_fix+1] = 3 * (i-1) + 1;
				fix_data[3 * ak_fix+2] = 3 * (i-1) + 2;

				ak_fix++;
				if (ak_fix == n_fix) {
					mode = 4;
					continue;
				}

			}

		}
		infile.close();
		//std::cout << "Data: " << ak_Vet << " " << ak_Tet << " " << ak_fix << " "  << std::endl;
		this->vertex_body = Ver_data;
		this->indices_body = Tet_data;
		this->fix = fix_data;
		this->n_fix = 3 * this->n_fix;
	}
}
void Phy::myloadScene(const char* filename, bool smooth, const glm::vec4& color) {
	Assimp::Importer importer;
	int process = aiProcess_JoinIdenticalVertices;
	if (smooth) {
		process |= aiProcess_GenSmoothNormals;
	}
	else {
		process |= aiProcess_GenNormals;
	}
	const aiScene* scene = importer.ReadFile(filename, process);
	

	//std::vector<geometry> objects;
	std::function<void(aiNode*, glm::mat4)> traverse;
	traverse = [&](aiNode * node, glm::mat4 t) {
		aiMatrix4x4 aim = node->mTransformation;
		glm::mat4 new_t(
			aim.a1, aim.b1, aim.c1, aim.d1,
			aim.a2, aim.b2, aim.c2, aim.d2,
			aim.a3, aim.b3, aim.c3, aim.d3,
			aim.a4, aim.b4, aim.c4, aim.d4
		);
		t = new_t * t;

		aiMatrix4x4 local_t;
		if (node->mNumMeshes > 0) {
			for (uint32_t i = 0; i < 1; ++i) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

				/*geometry m{};
				m.positions.resize(mesh->mNumVertices);
				m.normals.resize(mesh->mNumVertices);
				m.colors.resize(mesh->mNumVertices, glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
				m.faces.resize(mesh->mNumFaces);*/

				nodes = mesh->mNumVertices;
				faces = mesh->mNumFaces;
				size_vbo = nodes * 10 * sizeof(float);
				size_ibo = nodes * 3 * sizeof(unsigned int);
				vbo_data = static_cast<float*>(malloc(nodes * 10*sizeof(float)));
				deformbig = static_cast<float*>(malloc(nodes * 3 * sizeof(float)));
				undeformbig = static_cast<float*>(malloc(nodes * 3 * sizeof(float)));
				ibo_data = static_cast<unsigned int*>(malloc(nodes * 3*sizeof(unsigned int)));

				std::cout << "nodes: " <<nodes << std::endl;

				for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
					glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
					glm::vec3 nrm(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

					//m.positions[i] = pos;
					//m.normals[i] = nrm;
					//m.colors[i] = color;

					vbo_data[3 * i + 0] = pos[0];
					vbo_data[3 * i + 1] = pos[1];
					vbo_data[3 * i + 2] = pos[2];
					vbo_data[3 * i + 3*nodes] = nrm[0];
					vbo_data[3 * i + 3*nodes +1] = nrm[1];
					vbo_data[3 * i + 3 * nodes+2] = nrm[2];

					//vbo_data[4 * i + 6 * nodes] = color[0];
					vbo_data[4 * i + 6 * nodes] = 0.;
					vbo_data[3 * i + 6 * nodes] = color[0];

					vbo_data[4 * i + 6 * nodes+1] = color[1];
					vbo_data[4 * i + 6 * nodes+2] = color[2];
					vbo_data[4 * i + 6 * nodes+3] = color[3];

					undeformbig[3 * i + 0] = pos[0];
					undeformbig[3 * i + 1] = pos[1];
					undeformbig[3 * i + 2] = pos[2];

					deformbig[3 * i + 0] = 0.;
					deformbig[3 * i + 1] = 0.;
					deformbig[3 * i + 2] = 0.;

					//std::cout << "pos: " << pos[0]<<" "<<pos[1]<<" "<<pos[2]<< std::endl;
					//std::cout << "nrm: " << nrm[0] << " " << nrm[1] << " " << nrm[2] << std::endl;
				}

				for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
					glm::uvec3 face(mesh->mFaces[i].mIndices[0],
						mesh->mFaces[i].mIndices[1],
						mesh->mFaces[i].mIndices[2]);
					//m.faces[i] = face;
					ibo_data[i * 3 + 0] = face[0];
					ibo_data[i * 3 + 1] = face[1];
					ibo_data[i * 3 + 2] = face[2];
					//std::cout << "face: " << face[0]<<" "<<face[1]<<" "<<face[2]<< std::endl;
				}

				/*glGenVertexArrays(1, &m.vao);
				//glBindVertexArray(m.vao);
				//m.vbo = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->mNumVertices * 10 * sizeof(float), vbo_data);
				//m.ibo = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, mesh->mNumFaces * 3 * sizeof(unsigned int), ibo_data);
				//glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
				glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
				//m.transform = t;
				//m.vertex_count = 3 * mesh->mNumFaces;
				//objects.push_back(m);*/

				
			}
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			traverse(node->mChildren[i], t);
		}
		
	};
	
	traverse(scene->mRootNode, glm::identity<glm::mat4>());
	
}
void Phy::compLamda() {
	//funkt wiel x undefomiert ist
	std::vector<std::tuple< int, int, float> > out;

	for (int i = 0; i < nodes; i++) {
		float x1 = undeformbig[3 * i + 0];//ween das gecalled wird ist vbo = undeform
		float y1 = undeformbig[3 * i + 1];
		float z1 = undeformbig[3 * i + 2];

		float sum = 0.000001;
		std::vector<std::tuple< int, float> > tmp;

		for (int j = 0; j < n_Vet-(n_fix/3); j++) {// oh no das ist schlecht, weil es gib komisches remap der indecies
			
			float x2 = x[3 * j + 0];
			float y2 = x[3 * j + 1];
			float z2 = x[3 * j + 2];

			float d2 = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2);

			

			if (d2 < 1.) {
				float lamda = exp(-(d2));
				sum += lamda;
				//if(lamda>0.9)std::cout << lamda << std::endl;
				tmp.push_back(std::make_tuple(j, lamda));
			}
		
		}
		for (auto it = tmp.begin(); it != tmp.end(); it++) {
			out.push_back(std::make_tuple(i, std::get<0>(*it) , std::get<1>(*it)/sum));
			//std::cout << i <<" "<< std::get<0>(*it)<<" "<< std::get<1>(*it) / sum << std::endl;
		}
	
	}
	this->lamda = out;
}
void Phy::updatevbo() {

	for (int i = 0; i < 3*nodes; i++) {
		deformbig[i] = 0.;
	}

	for (auto it = lamda.begin(); it != lamda.end(); it++) {
		int i = std::get<0>(*it);//index big
		int j = std::get<1>(*it);//index small
		float w = std::get<2>(*it);
		//if(w>0.8)std::cout << i << " " << j << " " << w << std::endl;

		deformbig[i] += w * deformsmall[j];

	}

	for (int i = 0; i < 3 * nodes; i++) {
		vbo_data[i] = undeformbig[i] + deformbig[i];
	}

}