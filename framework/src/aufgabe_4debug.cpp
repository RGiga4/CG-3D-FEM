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

void print(glm::vec3 x) {

	std::cout << "print: " << x.x << ", " << x.y << ", " << x.z << std::endl;
}

bool
intersect_plane(ray const& r,
                glm::vec3 const& p0,
                glm::vec3 const& p1,
                glm::vec3 const& p2,
                intersection* isect) {// r ein Strahl und p0,p1,p2 ein Dreieck
										//Seiteneffekt auf isect object: lambda, position, normal
										//true falls SChnittpunkt exsetier zwischen FAr und Near
    // TASK 1a: Compute correct t

	glm::vec3 a = p1 - p0;
	glm::vec3 b = p2 - p0;

	glm::vec3 normal = glm::vec3(a[1] * b[2]- a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[2]);//glm::cross
	normal = glm::normalize(normal);

	float cos_alp = glm::dot(r.direction, normal);

	if(cos_alp<0.0001 && cos_alp > -0.0001)
		return false;

    float t = glm::dot(p0-r.origin, normal);
	t = t / cos_alp;
	
    if (t < NEAR_VALUE || t > FAR_VALUE) {
        return false;
    }

    isect->lambda = t;
    isect->position = r.origin + t * r.direction;
    // TASK 1b: Set correct normal
    isect->normal = glm::vec3(0.f, 0.f, 1.f);
    return true;
}


bool
edge_test(glm::vec3 const& p0,
          glm::vec3 const& p1,
          glm::vec3 const& x,
          glm::vec3 const& face_normal) {//true: falls x bezüglich der Kante zwischen p0 und p1 innerhalb des Dreiecks liegt
    // TASK 2: Compute correct edge check
	
	
	float t = glm::dot(face_normal, glm::cross(p1 - p0, x - p0));// ist cos*c c>0
	
	//std::cout << t << std::endl;
	
	if (t > 0.f )//cos>0
		return true;
    return false;
}

bool
intersect_triangle(ray const& r,
	intersection* isect,
	glm::vec3 const& p0,
	glm::vec3 const& p1,
	glm::vec3 const& p2) {//debug vesion
    /*geometry const& object = objects[isect->object];
    unsigned int triangle_idx = isect->face;
    unsigned int i0 = object.faces[triangle_idx].x;
    unsigned int i1 = object.faces[triangle_idx].y;
    unsigned int i2 = object.faces[triangle_idx].z;
    glm::vec3 const& p0 = object.positions[i0];
    glm::vec3 const& p1 = object.positions[i1];
    glm::vec3 const& p2 = object.positions[i2];*/
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

	std::cout << "b0: " << b0 << ", b1: " << b1 << ", b2: " << b2 << std::endl;
	std::cout << "sum: " << b0 + b1 + b2 << std::endl;
    /*isect->normal = b0 * object.normals[i0]
                  + b1 * object.normals[i1]
                  + b2 * object.normals[i2];

    // interpolate parameters
    float lum = std::max(glm::dot(isect->normal, glm::normalize(light_pos - x)), 0.1f);
    isect->color = lum * b0 * object.colors[i0]
                 + lum * b1 * object.colors[i1]
                 + lum * b2 * object.colors[i2];*/


    return true;
}

/*bool
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
}*/

int
main(int, char* argv[]) {
	ray test_ray = ray();
	test_ray.origin = glm::vec3(0.f, 0.f, -2.f );
	test_ray.direction = glm::normalize(glm::vec3(0.f, 0.f, 1.f));
	intersection test_inter = intersection();

	float e = 0.01;

	glm::vec3 p0 = glm::vec3(0.f-e, 0.f-e, 1.f);
	glm::vec3 p1 = glm::vec3(1.f+2*e, 0.f-e, 1.f);
	glm::vec3 p2 = glm::vec3(0.f-e, 1.f+2*e, 1.f);

	intersect_plane(test_ray, p0, p1, p2, &test_inter);

	std::cout << "lambda: " << test_inter.lambda <<std::endl;
	std::cout << "pos: " << test_inter.position.x << ", " << test_inter.position.y << ", " << test_inter.position.z << std::endl;
	std::cout << "nor: " << test_inter.normal.x << ", " << test_inter.normal.y << ", " << test_inter.normal.z << std::endl;

	glm::vec3 x = test_inter.position;
	glm::vec3 n = test_inter.normal;

	std::cout << "Edge-p0-p1: " << edge_test(p0, p1, x, n) << std::endl;
	std::cout << "Edge-p1-p2: " << edge_test(p1, p2, x, n) << std::endl;
	std::cout << "Edge-p2-p0: " << edge_test(p2, p0, x, n) << std::endl;

	intersect_triangle(test_ray, &test_inter, p0, p1, p2);


}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}
