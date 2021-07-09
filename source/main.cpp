#include <files.hpp>
#include <math.h>
#include <cam.hpp>
#include <figures.h>
#include <shader.hpp>
#include <files.hpp>
#include <model.hpp>

const u32 SCR_WIDTH  = 1280;
const u32 SCR_HEIGHT = 720;
const f32 ASPECT		 = (f32)SCR_WIDTH / (f32)SCR_HEIGHT;

Cam* cam;
i32 dx, dz;

f32  deltaTime	= 0.0f;
f32  lastFrame	= 0.0f;
bool wireframe	= false;

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam->processKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam->processKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam->processKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam->processKeyboard(RIGHT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		dx = -1;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		dx = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		dz = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		dz = -1;
	}
}
void key_callback(GLFWwindow*, int key, int, int act, int) {
	wireframe ^= key == GLFW_KEY_E && act == GLFW_PRESS;
}
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		cam->movePov(xpos, ypos);
	} else {
		cam->stopPov();
	}
}
void scroll_callback(GLFWwindow*, f64, f64 yoffset) {
	cam->processScroll((f32)yoffset);
}


struct Block {
	glm::vec3 pos;
	bool visible;
	Block(glm::vec3 _pos, bool _visible=true) 
		: pos(_pos), visible(_visible) {}
};


struct Ball {
	u32 tex;
	f32 radius;
	glm::vec3 pos;
	glm::mat4 model;

	f32 dx = 0; 
	f32 dz = 0; 

	Ball(std::string texture, Shader*& shader){
		tex = shader->loadTexture(texture);
		radius = 0.2f;
		pos = {0,0,0};
		model = glm::translate(glm::mat4(1.0f), pos);
		model = glm::scale(this->model, glm::vec3(radius));

	}
	~Ball() {
	}

	void update_pos() {

		pos.x += dx ;
		pos.z += dz ;

	}
	void Draw(Shader* shader, Model* ball) {
		pos.y = 0.5f + radius*2;
		model = glm::translate(glm::mat4(1.0f), pos);
		model = glm::scale(this->model, glm::vec3(radius*2.0f));
		shader->setMat4("model", model);
		glBindTexture(GL_TEXTURE_2D, tex);
		ball->Draw(shader);
	}

};

bool doesCubeIntersectSphere(glm::vec3 C1, glm::vec3 C2, glm::vec3 S, float R) {
	float dist_pow = R * R;
    /* assume C1 and C2 are element-wise sorted, if not, do that now */
    if (S.x < C1.x) dist_pow -= pow(S.x - C1.x, 2);
    else if (S.x > C2.x) dist_pow -= pow(S.x - C2.x, 2);
    if (S.y < C1.y) dist_pow -= pow(S.y - C1.y, 2);
    else if (S.y > C2.y) dist_pow -= pow(S.y - C2.y, 2);
    if (S.z < C1.z) dist_pow -= pow(S.z - C1.z, 2);
    else if (S.z > C2.z) dist_pow -= pow(S.z - C2.z, 2);
    return dist_pow > 0;
}

struct NPC {
	bool visible = true;
	float radius;
	glm::vec3 pos;
	glm::vec3 dir;
	i32 random_dir_c = 1000;
	
	NPC (f32 size) : radius(size) {
		std::cout << size << "\n";
		pos.x = rand()%30;
		pos.z = rand()%30;
		pos.y = 2;
		dir.y = 0;
		dir.x = (rand() % int((float)RAND_MAX*0.05f))/(float)RAND_MAX ;
		dir.z = (rand() % int((float)RAND_MAX*0.06f))/(float)RAND_MAX ; 
	}
	void randomize_dir() {
		if(1000 <= random_dir_c) {
			dir.y = 0;
			dir.x = (rand() % int((float)RAND_MAX*0.05f))/(float)RAND_MAX ;
			dir.z = (rand() % int((float)RAND_MAX*0.06f))/(float)RAND_MAX ; 
			std::cout << "updated\n";
		}  else random_dir_c = 0;
	}
	void update_pos() {
		if(!(0 <= pos.x + dir.x && pos.x + dir.x < 30)) {
			dir.x *= -1;
		}
		if(!(0 <= pos.z + dir.z && pos.z + dir.z < 30)) {
			dir.z *= -1;
		}
		pos += dir;
		random_dir_c ++;

	}
	void Draw(Shader* shader, Model* ball) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
		model = glm::scale(model, glm::vec3(radius));
		shader->setMat4("model", model);
		ball->Draw(shader);
	}

};

i32 main() {
	GLFWwindow* window = glutilInit(3, 3, SCR_WIDTH, SCR_HEIGHT, "Cubito");
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	cam = new Cam();	
	
	// modificar valores de acuerdo a donde los tenga guardados
	Files* files = new Files("", "resources/textures", "resources/objects");
	Shader* shader = new Shader(files, "shader.vert", "shader.frag");
	Shader* ball_shader = new Shader(files, "Ball.vert", "Ball.frag");

	u32 balltex = ball_shader->loadTexture("Balltexture.jpg");

	Ball ball("rubik.jpg", ball_shader);
	Model* ballx = new Model(files, "Ball/Ballmodel.obj");

	glm::vec3 lightPos	 = glm::vec3(1.0f);
	glm::vec3 lightColor	 = glm::vec3(0.21f, 0.62f, 0.32f);

	glEnable(GL_DEPTH_TEST);

	Cube* cubex = new Cube();
	u32 size[2] = {30, 30};
	std::vector<Block> positions;
	for (u32 i = 0; i < size[0]; ++i) {
		for (u32 j = 0; j < size[1]; ++j) {
			positions.push_back(Block({i*1.0f, 0.0f, j*1.0f}, true));
		}
	}

	std::vector<NPC> players;
	u32 ps = 10;
	for (u32 i = 0; i < ps; ++i) {
		players.push_back(NPC(0.05 + rand() % int(RAND_MAX * 0.95)/(float)RAND_MAX));
	}

	u32 cubeVao, lampVao, vbo, ebo;
	glGenVertexArrays(1, &cubeVao);
	glGenVertexArrays(1, &lampVao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ARRAY_BUFFER, cubex->getVSize()*sizeof(float),
			cubex->getVertices(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubex->getISize()*sizeof(u32),
			cubex->getIndices(), GL_STATIC_DRAW);

	// posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);
	// normales: ojo que es el 3er comp, por eso offset es 6
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(6));
	glEnableVertexAttribArray(1);
	// textures
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(9));
	glEnableVertexAttribArray(2);

	glBindVertexArray(lampVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);

	shader->loadTexture("container2.png", "xyzmat.diffuse");	
	shader->loadTexture("container2_specular.png", "xyzmat.specular"); 

	shader->activeTexture(0);
	shader->activeTexture(1);

	cam->pos = {-8.66733, 11.5225, 9.03402};
	cam->lookat = {0.703716, -0.710353, 0.0135148};
	
	ball.pos = {rand() % 30, 0, rand()%30};
	ball.radius = .5f;

	while (!glfwWindowShouldClose(window)) {
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	
		processInput(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPolygonMode(GL_FRONT_AND_BACK, wireframe? GL_LINE: GL_FILL);

		glm::mat4 proj = glm::perspective(cam->zoom, ASPECT, 0.1f, 100.0f);

		shader->use();
		lightPos.x = 2.0f*(cos(currentFrame) - sin(currentFrame));
		lightPos.z = 2.0f*(cos(currentFrame) + sin(currentFrame));

		ball.dx = dx * 0.03;
		ball.dz = dz * 0.03;
		shader->setVec3("xyz", lightPos);
		shader->setVec3("xyzColor", lightColor);
		shader->setVec3("xyzView", cam->pos);

		glm::mat4 model;
		shader->setMat4("proj", proj);
		shader->setMat4("view", cam->getViewM4());
		glBindVertexArray(cubeVao);

		shader->activeTexture(0);
		for (Block& block : positions) {
			if(!block.visible) continue;
			model = glm::mat4(1.0f);
			model = glm::translate(model, block.pos);
			shader->setMat4("model", model);
			glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

		} 

		ball.update_pos();
			
		ball.Draw(ball_shader, ballx);

		glBindTexture(GL_TEXTURE_2D, balltex);
		for(NPC& n : players) {
			if(!n.visible) continue;
			n.pos.y = 0.5f + n.radius;
			n.update_pos();
			n.Draw(ball_shader, ballx);
			bool intersects = pow((n.pos.x - ball.pos.x),2) + pow((n.pos.z - ball.pos.z),2) < pow(n.radius*2,2);
			if(intersects) {
				if(n.radius <= ball.radius) {
					n.visible = false;
					ball.radius *= 1.0 + (n.radius *0.5f);
				}

			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete cam;
	delete shader;
	delete files;
	delete ball_shader;

	return 0;
}