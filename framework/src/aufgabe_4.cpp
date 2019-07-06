#include "common.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "raytracer.hpp"

const int WINDOW_WIDTH =  800;
const int WINDOW_HEIGHT = 800;
const float FOV = 45.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 100.f;

glm::mat4 proj_matrix;
glm::vec3 light_pos;
std::vector<geometry> objects;

void
resizeCallback(GLFWwindow* window, int width, int height);


bool
intersect_plane(ray const& r,
                glm::vec3 const& p0,
                glm::vec3 const& p1,
                glm::vec3 const& p2,
                intersection* isect) {
	// r ein Strahl und p0,p1,p2 ein Dreieck
	//Seiteneffekt auf isect object: lambda, position, normal
	//true falls SChnittpunkt exsetier zwischen FAr und Near
	
    // TASK 1a: Compute correct t

	glm::vec3 a = p1 - p0;
	glm::vec3 b = p2 - p0;

	glm::vec3 normal = glm::vec3(a[1] * b[2]- a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);//glm::cross
	//glm::vec3 normal = glm::cross(a, b);
	normal = glm::normalize(normal);

	float cos_alp = glm::dot(r.direction, normal);

	
	if(cos_alp<0.0001f && cos_alp > -0.0001f)
		return false;

    float t = glm::dot(p0-r.origin, normal);
	t = t / cos_alp;
	
    if (t < NEAR_VALUE || t > FAR_VALUE) {
        return false;
    }

    isect->lambda = t;
    isect->position = r.origin + t * r.direction;
    // TASK 1b: Set correct normal
	isect->normal = normal;
    return true;
}

bool
edge_test(glm::vec3 const& p0,
	glm::vec3 const& p1,
	glm::vec3 const& x,
	glm::vec3 const& face_normal) {
	//true: falls x bezüglich der Kante zwischen p0 und p1 innerhalb des Dreiecks liegt
	//falls normale richtig

// TASK 2: Compute correct edge check


	float t = glm::dot(face_normal, glm::cross(p1 - p0, x - p0));// ist cos*c c>0


	if (t > 0.f)//cos>0
		return true;
	return false;
}

bool
intersect_triangle(ray const& r,
                   intersection* isect) {
	//seiteneffect in isect auf, normal, color, (lamda)
    geometry const& object = objects[isect->object];
    unsigned int triangle_idx = isect->face;
    unsigned int i0 = object.faces[triangle_idx].x;
    unsigned int i1 = object.faces[triangle_idx].y;
    unsigned int i2 = object.faces[triangle_idx].z;
    glm::vec3 const& p0 = object.positions[i0];
    glm::vec3 const& p1 = object.positions[i1];
    glm::vec3 const& p2 = object.positions[i2];
    if (!intersect_plane(r, p0, p1, p2, isect)) {
        return false;
    }

    glm::vec3& x = isect->position;
    glm::vec3& n = isect->normal;

    // first line
    if (!edge_test(p0, p1, x, n) ||
        !edge_test(p1, p2, x, n) ||
        !edge_test(p2, p0, x, n)) {
        return false;
    }

    // TASK 3: Compute correct barycentric coordinates

	glm::vec3 A_p0 = glm::cross(p2 - p1, x - p1);

	

	float A_tot = glm::length(glm::cross(p2 - p0, p1 - p0));
	glm::vec3 A_p1 = glm::cross(p0 - p2, x - p2);
	glm::vec3 A_p2 = glm::cross(p1 - p0, x - p0);

    float b0 = glm::length(A_p0)/A_tot;
	
    float b1 = glm::length(A_p1) / A_tot;
    float b2 = glm::length(A_p2) / A_tot;
	
    isect->normal = b0 * object.normals[i0]
                  + b1 * object.normals[i1]
                  + b2 * object.normals[i2];

    // interpolate parameters
    float lum = std::max(glm::dot(isect->normal, glm::normalize(light_pos - x)), 0.1f);
    isect->color = lum * b0 * object.colors[i0]
                 + lum * b1 * object.colors[i1]
                 + lum * b2 * object.colors[i2];


    return true;
}

bool
intersect_scene(ray const& r, intersection* isect) {
    bool found_intersection = false;
    intersection test_isect = *isect;
    for (unsigned int i = 0; i < objects.size(); ++i) {
        test_isect.object = i;
        for (unsigned int f = 0; f < objects[i].faces.size(); ++f) {
            test_isect.face = f;
            if (intersect_triangle(r, &test_isect)) {
                found_intersection = true;
                if (test_isect.lambda < isect->lambda) {
                    *isect = test_isect;
                }
            }
        }
    }

    return found_intersection;
}

int
main(int, char* argv[]) {
    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    camera cam(window);
    light_pos = glm::vec3(0.0, 1.8, 0.0);


    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("raytrace_render.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("raytrace_render.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    objects = loadScene("cornell_box.obj", true);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
    // fix view matrix for the cornell box
    glm::mat4 view_matrix = glm::transpose(glm::mat4(
        0.998201, 0, -0.0599639, 0.00662615,
        0.00474432, 0.996865, 0.0789773, -0.733012,
        0.0597759, -0.0791196, 0.995071, -2.25093,
        0, 0, 0, 1));

    raytracer tracer(WINDOW_WIDTH, WINDOW_WIDTH, proj_matrix, intersect_scene);
	
    tracer.trace(view_matrix, "raytraced.ppm");

    glUseProgram(shaderProgram);
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model_mat");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view_mat");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "proj_mat");
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_pos");
    glUniform3fv(light_dir_loc, 1, &light_pos[0]);
    glEnable(GL_DEPTH_TEST);

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glm::mat4 view_matrix = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_matrix[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);

        for (unsigned i = 0; i < objects.size(); ++i) {
            glm::mat4 model_matrix = objects[i].transform;
            glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_matrix[0][0]);

            objects[i].bind();
            glDrawElements(GL_TRIANGLES, objects[i].vertex_count, GL_UNSIGNED_INT, (void*) 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (unsigned int i = 0; i < objects.size(); ++i) {
        objects[i].destroy();
    }


    glfwTerminate();
}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
