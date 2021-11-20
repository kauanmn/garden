#include "openglwindow.hpp"
#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>
#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>



// explicit specialization of std::hash for Vertex
namespace std {
	template <>
	struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const noexcept {
			const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
			return h1;
		}
	};
} // namespace std

void OpenGLWindow::handleEvent(SDL_Event& ev) {
	if (ev.type == SDL_KEYDOWN) {
		if (ev.key.keysym.sym == SDLK_UP)    m_tiltSpeed =  1.0f;
		if (ev.key.keysym.sym == SDLK_DOWN)  m_tiltSpeed = -1.0f;
		if (ev.key.keysym.sym == SDLK_LEFT)  m_panSpeed  = -1.0f;
		if (ev.key.keysym.sym == SDLK_RIGHT) m_panSpeed  =  1.0f;



		if (ev.key.keysym.sym == SDLK_w) m_dollySpeed =  1.0f;
		if (ev.key.keysym.sym == SDLK_s) m_dollySpeed = -1.0f;
		if (ev.key.keysym.sym == SDLK_a) m_truckSpeed = -1.0f;
		if (ev.key.keysym.sym == SDLK_d) m_truckSpeed =  1.0f;
	}
	if (ev.type == SDL_KEYUP) {
		if (ev.key.keysym.sym == SDLK_UP)    m_tiltSpeed = 0.0f;
		if (ev.key.keysym.sym == SDLK_DOWN)  m_tiltSpeed = 0.0f;
		if (ev.key.keysym.sym == SDLK_LEFT)  m_panSpeed  = 0.0f;
		if (ev.key.keysym.sym == SDLK_RIGHT) m_panSpeed  = 0.0f;



		if (ev.key.keysym.sym == SDLK_w) m_dollySpeed = 0.0f;
		if (ev.key.keysym.sym == SDLK_s) m_dollySpeed = 0.0f;
		if (ev.key.keysym.sym == SDLK_a) m_truckSpeed = 0.0f;
		if (ev.key.keysym.sym == SDLK_d) m_truckSpeed = 0.0f;
	}
	/*
	if (ev.type == SDL_MOUSEMOTION) {
		m_tiltSpeed = (static_cast<float>(ev.motion.yrel))*1.5f;
		update();
		m_tiltSpeed = 0.0f;
	}
	*/
}





void OpenGLWindow::initializeGL() {
	abcg::glClearColor(0,0,0,1);

	// enable depth buffering
	abcg::glEnable(GL_DEPTH_TEST);

	// create program
	m_program = createProgramFromFile(getAssetsPath() + "lookat.vert",
									  getAssetsPath() + "lookat.frag");
	m_ground.initializeGL(m_program);


	// load model
	loadModelFromFile(getAssetsPath() + "arbusto.obj");

	// generate VBO
	abcg::glGenBuffers(1, &m_VBO);
	abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
					   m_vertices.data(), GL_STATIC_DRAW);
	abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

	// generate EBO
	abcg::glGenBuffers(1, &m_EBO);
	abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					   sizeof(m_indices[0]) * m_indices.size(), m_indices.data(),
					   GL_STATIC_DRAW);
	abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// create VAO
	abcg::glGenVertexArrays(1, &m_VAO);

	// bind vertex attributes to current VAO
	abcg::glBindVertexArray(m_VAO);


	abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	const GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};
	abcg::glEnableVertexAttribArray(positionAttribute);
	abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), nullptr);
	abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

	abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	// end of binding to current VAO
	abcg::glBindVertexArray(0);


	auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  	m_randomEngine.seed(seed);
	std::uniform_int_distribution<int> intDistribution(1, 10);

	for(int i = 0; i < bushN; i++){		
		bushPositionX[i] = float(intDistribution(m_randomEngine)) * 1.0f;
		bushPositionY[i] = float(intDistribution(m_randomEngine)) * 1.0f;
		bushColorRed[i] = float(intDistribution(m_randomEngine)) * 0.01f;
		bushColorGreen[i] = float(intDistribution(m_randomEngine)) * 0.1f;
		bushSize[i] = float(intDistribution(m_randomEngine)) * 0.001f;
		bushPositionXOut[i] = intDistribution(m_randomEngine) > 5;
		bushPositionYOut[i] = intDistribution(m_randomEngine) > 5;
	}

	resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::loadModelFromFile(std::string_view path) {
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(path.data())) {
		if (!reader.Error().empty()) {
			throw abcg::Exception{abcg::Exception::Runtime(
					fmt::format("Failed to load model {} ({})", path, reader.Error()))};
		}
		throw abcg::Exception{
				abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
	}

	if (!reader.Warning().empty()) {
		fmt::print("Warning: {}\n", reader.Warning());
	}

	const auto& attrib{reader.GetAttrib()};
	const auto& shapes{reader.GetShapes()};

	m_vertices.clear();
	m_indices.clear();

	// A key:value map with key=Vertex and value=index
	std::unordered_map<Vertex, GLuint> hash{};

	// Loop over shapes
	for (const auto& shape : shapes) {
		// Loop over indices
		for (const auto offset : iter::range(shape.mesh.indices.size())) {
			// Access to vertex
			const tinyobj::index_t index{shape.mesh.indices.at(offset)};

			// Vertex position
			const int startIndex{3 * index.vertex_index};
			const float vx{attrib.vertices.at(startIndex + 0)};
			const float vy{attrib.vertices.at(startIndex + 1)};
			const float vz{attrib.vertices.at(startIndex + 2)};

			Vertex vertex{};
			vertex.position = {vx, vy, vz};

			// If hash doesn't contain this vertex
			if (hash.count(vertex) == 0) {
				// Add this index (size of m_vertices)
				hash[vertex] = m_vertices.size();
				// Add this vertex
				m_vertices.push_back(vertex);
			}

			m_indices.push_back(hash[vertex]);
		}
	}
}

void OpenGLWindow::paintGL() {
	update();

	// clear color buffer and depth buffer
	abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

	abcg::glUseProgram(m_program);

	// get location of uniform variables (could be precomputed)
	const GLint  viewMatrixLoc{abcg::glGetUniformLocation(m_program,  "viewMatrix")};
	const GLint  projMatrixLoc{abcg::glGetUniformLocation(m_program,  "projMatrix")};
	const GLint modelMatrixLoc{abcg::glGetUniformLocation(m_program, "modelMatrix")};
	const GLint       colorLoc{abcg::glGetUniformLocation(m_program, "color")};




	// set uniform variables for viewMatrix and projMatrix
	// these matrices are used for every scene object
	abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
	abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);


	abcg::glBindVertexArray(m_VAO);



	for(int i = 0; i < bushN; i++){
		glm::mat4 model{1.0f};
		{
			float positionX =  bushPositionXOut[i] ? -bushPositionX[i] : bushPositionX[i];
			float positionY =  bushPositionYOut[i] ? -bushPositionY[i] : bushPositionY[i];
			model = glm::translate(model, glm::vec3(positionX, 0.0f, positionY));
			model = glm::rotate   (model, glm::radians(bushPositionY[i] * 9.0f), glm::vec3(0,1,0));
			model = glm::scale    (model, glm::vec3(bushSize[i]));

			abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
			abcg::glUniform4f(colorLoc, bushColorRed[i], bushColorGreen[i], 0.0f, 1.0f);
			abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
		}

		
	}

	abcg::glBindVertexArray(0);

	// draw ground
	m_ground.paintGL();

	abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() { abcg::OpenGLWindow::paintUI(); }

void OpenGLWindow::resizeGL(int width, int height) {
	m_viewportWidth  = width;
	m_viewportHeight = height;

	m_camera.computeProjectionMatrix(width, height);
}

void OpenGLWindow::terminateGL() {
	m_ground.terminateGL();

	abcg::glDeleteProgram(m_program);
	abcg::glDeleteBuffers(1, &m_EBO);
	abcg::glDeleteBuffers(1, &m_VBO);
	abcg::glDeleteVertexArrays(1, &m_VAO);
}

void OpenGLWindow::update() {
	const float deltaTime{static_cast<float>(getDeltaTime())};

	// update LookAt camera
	m_camera.dolly(m_dollySpeed * deltaTime);
	m_camera.truck(m_truckSpeed * deltaTime);
	m_camera.pan(m_panSpeed * deltaTime);
	m_camera.tilt(m_tiltSpeed * deltaTime);
}