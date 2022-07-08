#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/model.h"
#include "graphics/light.h"

#include "graphics/models/cube.hpp"
#include "graphics/models/lamp.hpp"

#include "io/keyboard.h"
#include "io/mouse.h"
#include "io/joystick.h"
#include "io/screen.h"
#include "io/camera.h"

void processInput(double deltaTime);

float mixVal = 0.5f;

Screen screen;

Joystick mainJ(0);
Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 3.0f));

double deltaTime = 0.0f; // tme btwn frames
double lastFrame = 0.0f; // time of last frame

bool flashlightOn = false;

bool lightOn = false;
bool lampUp,lampDown, lampRight, lampLeft = false;
bool texture1, texture2, texture3 = false;

int main() {

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	int success;
	char infoLog[512];

	glfwInit();

	// openGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (!screen.init()) {
		std::cout << "Could not open window" << std::endl;
		glfwTerminate();
		return -1;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	screen.setParameters();

	// SHADERS===============================
	Shader shader("assets/object.vs", "assets/object.fs");
	Shader lampShader("assets/object.vs", "assets/lamp.fs");

	// MODELS==============================
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),

	};

	Cube cubes(cubePositions[0], glm::vec3(1.0f));
	cubes.init();

	// LIGHTS
	DirLight dirLight = { glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.1f), glm::vec3(0.4f), glm::vec3(0.5f) };

	glm::vec3 pointLightPositions = glm::vec3(0.7f, 0.2f, 2.0f);
	Lamp lamps;
		lamps = Lamp(glm::vec3(1.0f),
			glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
			1.0f, 0.07f, 0.032f,
			pointLightPositions, glm::vec3(0.25f));
		lamps.init();
	

	SpotLight s = {
		Camera::defaultCamera.cameraPos, Camera::defaultCamera.cameraFront,
		glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(20.0f)),
		1.0f, 0.07f, 0.032f,
		glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f)
	};

	mainJ.update();
	if (mainJ.isPresent()) {
		std::cout << mainJ.getName() << " is present." << std::endl;
	}
	while (!screen.shouldClose()) {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
			// printf and reset timer
			printf("%f fps\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}
		
		// calculate dt
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		// process input
		processInput(deltaTime);

		// render
		screen.update();

		// draw shapes
		shader.activate();

		shader.set3Float("viewPos", Camera::defaultCamera.cameraPos);
		if (texture1) {
			cubes.cleanup();
			char text[100] = "assets/texture1.png";
			cubes.setTexturename(text);
			cubes.init();
			
			texture1 = !texture1;
		}
		if (texture2) {
			cubes.cleanup();
			char text[100] = "assets/texture2.png";
			cubes.setTexturename(text);
			cubes.init();
			texture2 = !texture2;
		}
		if (texture3) {
			cubes.cleanup();
			char text[100] = "assets/texture.png";
			cubes.setTexturename(text);
			cubes.init();
			texture3 = !texture3;
		}
		if (lampUp) {
			lamps.pointLight.position.y += 0.1f;
			lamps.pos.y = lamps.pointLight.position.y;
			lampUp = !lampUp;
		}
		if (lampDown) {
			lamps.pointLight.position.y -= 0.1f;
			lamps.pos.y = lamps.pointLight.position.y;
			lampDown = !lampDown;
		}
		if (lampLeft) {
			lamps.pointLight.position.x -= 0.1f;
			lamps.pos.x = lamps.pointLight.position.x;
			lampLeft = !lampLeft;
		}
		if (lampRight) {
			lamps.pointLight.position.x += 0.1f;
			lamps.pos.x = lamps.pointLight.position.x;
			lampRight = !lampRight;
		}
		if (lightOn) {
			dirLight.render(shader);
			lamps.pointLight.render(shader, 0);
			shader.setInt("noPointLights", 1);
		}
		else
		{
			shader.setInt("noPointLights", 0);
		}
		if (flashlightOn) {
			s.position = Camera::defaultCamera.cameraPos;
			s.direction = Camera::defaultCamera.cameraFront;
			s.render(shader, 0);
			shader.setInt("noSpotLights", 1);
		}
		else {
			shader.setInt("noSpotLights", 0);
		}
		// create transformation
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = Camera::defaultCamera.getViewMatrix();
		projection = glm::perspective(
			glm::radians(Camera::defaultCamera.zoom), 
			(float)Screen::SCR_WIDTH / (float)Screen::SCR_HEIGHT, 0.1f, 100.0f);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		cubes.render(shader);
		lampShader.activate();
		lampShader.setMat4("view", view);
		lampShader.setMat4("projection",projection);
		lamps.render(lampShader);

		// send new frame to window
		screen.newFrame();
		glfwPollEvents();
	}
		cubes.cleanup();
		lamps.cleanup();
	glfwTerminate();
	return 0;
}
void processInput(double deltaTime) {
	if (Keyboard::key(GLFW_KEY_ESCAPE)) {
		screen.setShouldClose(true);
	}
	if (Keyboard::keyWentDown(GLFW_KEY_L)) {
		lightOn = !lightOn;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_O)) {
		flashlightOn = !flashlightOn;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_1)) {
		texture1 = !texture1;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_2)) {
		texture2 = !texture2;
	}
	if (Keyboard::keyWentDown(GLFW_KEY_3)) {
		texture3 = !texture3;
	}
	// move camera
	if (Keyboard::key(GLFW_KEY_W)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::FORWARD, deltaTime);
	}
	if (Keyboard::key(GLFW_KEY_S)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::BACKWARD, deltaTime);
	}
	if (Keyboard::key(GLFW_KEY_D)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::RIGHT, deltaTime);
	}
	if (Keyboard::key(GLFW_KEY_A)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::LEFT, deltaTime);
	}
	if (Keyboard::key(GLFW_KEY_SPACE)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::UP, deltaTime);
	}
	if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::DOWN, deltaTime);
	}
	if (Keyboard::key(GLFW_KEY_UP)) {
		lampUp = !lampUp;
	}
	if (Keyboard::key(GLFW_KEY_DOWN)) {
		lampDown = !lampDown;
	}
	if (Keyboard::key(GLFW_KEY_LEFT)) {
		lampLeft = !lampLeft;
	}
	if (Keyboard::key(GLFW_KEY_RIGHT)) {
		lampRight = !lampRight;
	}

}