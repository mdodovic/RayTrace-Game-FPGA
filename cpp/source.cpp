#include <glfw3.h>
#include <cmath>
#include <iostream>
#include <time.h>
#include <Windows.h>
#include <thread>
#include <chrono>


using namespace std;

#ifndef INF
#define INF std::numeric_limits<float>::infinity()
#endif
const GLint WIDTH = 640, HEIGHT = 480;
GLubyte pixel[HEIGHT][WIDTH][3];

inline float random(float seed) {
	return ((429493445 * (long long int)((seed + 1) * 1000000) + 907633385) % 4294967296) / 4294967296.;
}
inline float rt(float k) {
	return k * k;
}

class color {
public:
	GLubyte r, g, b;
	color(GLubyte rr = 0, GLubyte gg = 0, GLubyte bb = 0) { r = rr; g = gg; b = bb; }
	color operator*(float p) {
		if (0 < p && p < 1) {
			float rr = r;
			float gg = g;
			float bb = b;
			return color(rr*p, gg*p, bb*p);
		}
		return *this;
	}

	bool operator==(color p) {
		if (r == p.r && g == p.g && b == p.b)
			return true;
		else
			return false;
	}


	bool operator!=(color p) {
		if (r != p.r || g != p.g || b != p.b)
			return true;
		else
			return false;
	}
};

class vector3 {
public:
	float x, y, z;
	vector3(float xx = 0, float yy = 0, float zz = 0) {
		x = xx;
		y = yy;
		z = zz;
	}
	vector3 operator-(vector3 p) {
		return vector3(x - p.x, y - p.y, z - p.z);
	}
	vector3 operator*(float p) {
		return vector3(x*p, y*p, z*p);
	}
	friend vector3 operator*(vector3 x, vector3 y);
	vector3& operator+=(vector3 p) {
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}
	vector3& operator-=(vector3 p) {
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}
	vector3 operator/(float p) {
		return vector3(x / p, y / p, z / p);
	}
	vector3 operator-() {
		return vector3(-x, -y, -z);
	}
	vector3 operator+(vector3 p) {
		return vector3(x + p.x, y + p.y, z + p.z);
	}
	vector3 normalize();
	float abs();

	void ispis() { cout << "(" << x << " ," << y << " ," << z << ")" << endl; };

	static vector3 toDot(vector3 pos, vector3 dir, float t);
};

vector3 operator*(vector3 x, vector3 y) {
	return vector3(x.y*y.z - x.z*y.y, x.z*y.x - x.x*y.z, x.x*y.y - x.y*y.x);
}

vector3 vector3::normalize() {
	float len = this->abs();
	return (*this) / len;
}

float vector3::abs() {
	return sqrt(rt(x) + rt(y) + rt(z));
}

vector3 vector3::toDot(vector3 pos, vector3 dir, float t) {
	vector3 temp;
	temp.x = pos.x + dir.x * t;
	temp.y = pos.y + dir.y * t;
	temp.z = pos.z + dir.z * t;
	return temp;
}

class planet {
public:
	color c;
	vector3 pos;
	vector3 vel;
	float r;
	planet *next;
	string name;
	float mass;
	planet(color col, vector3 p, vector3 v, float m, float rad, string s) { c = col; pos = p; vel = v;  r = rad; next = 0; name = s; };
	static void insert(planet*& p, color col, vector3 pos, vector3 v, float m, float rad, const string s);

};
void planet::insert(planet*& p, color col, vector3 pos, vector3 v, float m, float rad, const string s) {
	planet *temp = new planet(col, pos, v, m, rad, s);
	temp->next = p;
	p = temp;
}

vector3 sun;
planet *planets;

class camera {

public:
	vector3 pos;
	vector3 forward;
	vector3 right;
	vector3 up;
	camera(vector3 p) { pos = p; forward = vector3(0, 0, -1); right = vector3(1, 0, 0); up = vector3(0, 1, 0); };
	color rayTrace(vector3 dir, planet *p, float& e);
	static float sunRay(vector3 pos, vector3 dir, planet *p);
};

float camera::sunRay(vector3 pos, vector3 dir, planet *p) {
	vector3 temp = vector3((pos.x - p->pos.x), (pos.y - p->pos.y), (pos.z - p->pos.z));
	float a = rt(dir.x) + rt(dir.y) + rt(dir.z);
	float b = 2 * (dir.x*temp.x + dir.y*temp.y + dir.z*temp.z);
	float c = rt(temp.x) + rt(temp.y) + rt(temp.z) - rt(p->r);
	float d = rt(b) - 4 * a*c;
	if (d < 0) {
		return 1;
	}
	else if (d == 0) {
		return 1;
	}
	else {
		d = sqrt(d);
		float t1 = (-b + d) / (2 * a);
		float t2 = (-b - d) / (2 * a);
		if (abs(t1) < 0.001)
			t1 = 0;
		if (abs(t2) < 0.001)
			t2 = 0;
		//t1 >= 0 && t2 >= 0
		//if (t1 == 0 && t2 == 0) 
		//	return 1; 
		if (t1 != 0 && t2 != 0) {
			if (t1 < 1 && t2 < 1 && t1 > 0 && t2 > 0)
				return 0;
		}
		else if (t1 > 0 || t2 > 0) {
			float ret = (vector3::toDot(pos, dir, t2) - vector3::toDot(pos, dir, t1)).abs();
			return 101 / (100 + rt(ret));
		}
		return 1;
	}
}


color camera::rayTrace(vector3 dir, planet *p, float& eLength) {
	//ray = pos + dir * time; time = (ray-pos)/dir;
	//t?
	vector3 temp = vector3((pos.x - p->pos.x), (pos.y - p->pos.y), (pos.z - p->pos.z));
	float a = rt(dir.x) + rt(dir.y) + rt(dir.z);
	float b = 2 * (dir.x*temp.x + dir.y*temp.y + dir.z*temp.z);
	float c = rt(temp.x) + rt(temp.y) + rt(temp.z) - rt(p->r);
	float d = rt(b) - 4 * a*c;
	if (d < 0) {
		//return color(255, 255, 255);
		eLength = INF;
		float len = dir.abs();
		float temp = random((random(dir.x / len) + random(dir.y / len) + random(dir.z / len)) / 3.);
		if (temp < 0.0005) {
			return color(255, 255, 255);
		}
		return color();
	}
	else if (d == 0) {
		eLength = INF;
		return color();
	}
	else {
		d = sqrt(d);
		float t1 = (-b + d) / (2 * a);
		float t2 = (-b - d) / (2 * a);
		vector3 dosunca;
		vector3 dirsunca;
		float sumplus;
		if (t2 >= 0) {
			eLength = t2 * dir.abs();
			dosunca = vector3::toDot(pos, dir, t2);
			dirsunca = (sun - dosunca);
			planet *tempp = planets;
			sumplus = 1;
			while (tempp != NULL) {
				if (tempp->name != "S") {
					sumplus *= sunRay(dosunca, dirsunca, tempp);
					if (sumplus == 0)
						return color();
				}
				tempp = tempp->next;
			}
			return p->c*sumplus;
		}
		else if (t1 >= 0) {
			eLength = t1 * dir.abs();
			dosunca = vector3::toDot(pos, dir, t1);
			dirsunca = (sun - dosunca);
			planet *tempp = planets;
			sumplus = 1;
			while (tempp != NULL) {
				if (tempp->name != "S") {
					sumplus *= sunRay(dosunca, dirsunca, tempp);
					if (sumplus == 0)
						return color();
				}
				tempp = tempp->next;
			}
			return p->c*sumplus;
		}
		else {
			eLength = INF;
			return color();
		}

	}
}

float PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		cout << "QueryPerformanceFrequency failed!\n";

	PCFreq = float(li.QuadPart);

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
}
float GetCounter()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return float(li.QuadPart - CounterStart) / PCFreq;
}

void multiThread(camera cam, float sinOfF, float pixelRatio, float piyelRatio, float RATIO, int lowx, int lowy, int highx, int highy) {
	for (int i = lowx; i < highx; i++) {
		for (int j = lowy; j < highy; j++) {
			vector3 tempface = cam.forward + cam.right*((2 * sinOfF*i*pixelRatio - sinOfF)*RATIO) + cam.up*(2 * sinOfF*j*piyelRatio - sinOfF);
			planet *tempp = planets;
			color tempc, minC = color(), white = color(255, 255, 255), black = color(0, 0, 0);
			float errorMin = INF, errorTmp;

			while (tempp != NULL) {
				tempc = cam.rayTrace(tempface, tempp, errorTmp);
				if (tempc == white && errorTmp == INF && errorMin == INF && minC == black) {
					minC = white;
				}
				else if (errorTmp < errorMin) {
					errorMin = errorTmp;
					minC = tempc;
				}
				tempp = tempp->next;
			}
			pixel[j][i][0] = minC.r;
			pixel[j][i][1] = minC.g;
			pixel[j][i][2] = minC.b;
		}
	}
}
camera cam(vector3(0, 0, 2500));
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {


	}
	if (true) {
		vector3 temp;
		vector3 back;
		vector3 left;
		vector3 down;
		float angle = 0.01;
		float aangle = 1 - angle;
		switch (key) {
		case GLFW_KEY_UP:
			back = -cam.forward;
			cam.up = (cam.up * aangle + back * angle).normalize();
			cam.forward = (cam.up*cam.right).normalize();
			break;
		case GLFW_KEY_DOWN:
			down = -cam.up;
			cam.up = (cam.up * aangle + cam.forward * angle).normalize();
			cam.forward = (cam.up*cam.right).normalize();
			break;
		case GLFW_KEY_LEFT:
			left = -cam.right;
			cam.right = (cam.right * aangle + cam.forward * angle).normalize();
			cam.forward = (cam.up*cam.right).normalize();
			break;
		case GLFW_KEY_RIGHT:
			back = -cam.forward;
			cam.right = (cam.right * aangle + back * angle).normalize();
			cam.forward = (cam.up*cam.right).normalize();
			break;
		case GLFW_KEY_W:
			cam.pos += cam.forward * 25;
			break;
		case GLFW_KEY_S:
			cam.pos -= cam.forward * 25;
			break;
		}
	}

}
int main() {



	ios_base::sync_with_stdio(0);
	cin.tie(0);
	cout.tie(0);

	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "ORT", nullptr, nullptr);

	if (nullptr == window) {
		cout << "FAIL";
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetKeyCallback(window, key_callback);

	sun = vector3(0, 0, 0);

	planets = new planet(color(250, 100, 200), vector3(0, 500, 0), vector3(0, 0, 0), 100, 50, "P");
	planet::insert(planets, color(100, 100, 0), vector3(0, 1000, 0), vector3(0, 0, 0), 100, 200, "P");
	//planet::insert(planets, color(255, 255, 255), vector3(0, 0, -200), vector3(0, 0, 0), 100, 50., "P");
	//planets = new planet(color(0, 200, 0), vector3(0, 0, 0), vector3(0, 0, 0), 100, 100, "P");
	planet::insert(planets, color(255, 255, 0), vector3(0, 0, 0), vector3(0, 0, 0), 100, 50., "S");

	float pixelRatio = 1 / float(WIDTH);
	float piyelRatio = 1 / float(HEIGHT);
	float RATIO = float(WIDTH) / float(HEIGHT);
	const float sinOfF = 0.5;
	StartCounter();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		//StartCounter();
		thread Plocica1(multiThread, cam, sinOfF, pixelRatio, piyelRatio, RATIO, 0, 0, WIDTH / 2, HEIGHT / 2);
		thread Plocica2(multiThread, cam, sinOfF, pixelRatio, piyelRatio, RATIO, WIDTH / 2, 0, WIDTH, HEIGHT / 2);
		thread Plocica3(multiThread, cam, sinOfF, pixelRatio, piyelRatio, RATIO, 0, HEIGHT / 2, WIDTH / 2, HEIGHT);
		thread Plocica4(multiThread, cam, sinOfF, pixelRatio, piyelRatio, RATIO, WIDTH / 2, HEIGHT / 2, WIDTH, HEIGHT);

		Plocica1.join();
		Plocica2.join();
		Plocica3.join();
		Plocica4.join();
		/*for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				vector3 tempface = vector3(cam.forward.x + cam.right.x*(2 * sinOfF*i*pixelRatio - sinOfF)*RATIO + cam.up.x*(2 * sinOfF*j*piyelRatio - sinOfF), cam.forward.y + cam.right.y*(2 * sinOfF*i*pixelRatio - sinOfF)*RATIO + cam.up.y*(2 * sinOfF*j*piyelRatio - sinOfF), cam.forward.z + cam.right.z*(2 * sinOfF*i*pixelRatio - sinOfF)*RATIO + cam.up.x*(2 * sinOfF*j*piyelRatio - sinOfF));
				planet *tempp = planets;
				color tempc, minC = color(), white = color(255, 255, 255), black = color(0, 0, 0);
				float errorMin = INF, errorTmp;

				while (tempp != NULL) {
					tempc = cam.rayTrace(tempface, tempp, errorTmp);
					if (tempc == white && errorTmp == INF && minC == black) {
						minC = white;
					}
					else if (errorTmp < errorMin) {
						errorMin = errorTmp;
						minC = tempc;
					}
					tempp = tempp->next;
				}
				pixel[j][i][0] = minC.r;
				pixel[j][i][1] = minC.g;
				pixel[j][i][2] = minC.b;

			}

		}*/

		//cout << GetCounter() << endl;
		// pixel matrix filled

		planet *tempp = planets;
		float copet = 1000;
		float copet2 = 0.1;
		while (tempp != NULL) {
			if (tempp->name != "S") {
				tempp->pos.x = sin(GetCounter()*copet2) * copet;
				tempp->pos.y = cos(GetCounter()*copet2) * copet;
				copet /= 2;
				copet2 *= 2;
			}
			tempp = tempp->next;
		}



		//planet *tempp = planets;
		//float copet = 1000;
		//float copet2 = 0.01;
		glRasterPos2f(-1, -1);
		glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		//glEnd();

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return EXIT_SUCCESS;
}
