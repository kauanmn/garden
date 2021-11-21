# Garden

Garden é um projeto 3D capaz de gerar aleatoriamente o cenário, com árvores de cores e tamanhos variados. É possível se movimentar pelo mapa usando as teclas `WASD` e movimentar/rotacionar a câmera usando o mouse ou as setas do teclado.

O objetivo do projeto é praticar os seguintes conceitos vistos em aula:
- Câmera LookAt
- Matrizes de transformação
- Perspectiva
- Espaços do OpenGL
- Conceitos básicos de álgebra linear



## Descrição técnica

O projeto foi desenvolvido utilizando as bibliotecas ABCg e Dear ImGui e a API gráfica OpenGL:
- A biblioteca **ABCg** possibilita a prototipagem rápida de aplicações gráficas interativas 3D em C++ capazes de rodar tanto no desktop quanto no navegador.
- O **OpenGL** é uma API gráfica que permite a renderização de gráficos 2D e 3D através de uma pipeline gráfica.
Com isso, foi criado uma aplicação simples que renderiza uma cena 3D e calcula a movimentação da câmera LookAt.

A aplicação foi baseada no exercício prática na aula 7.7 LookAt, por isso, este texto vai descrever apenas as mudanças no projeto e os principais pontos dele.


### Vertex shader
O vertex shader foi alterado para que a névoa incluída na aula fosse removida, tornando o cenário completamente visível.
``` c++
void main() {
	vec4 posEyeSpace = viewMatrix * modelMatrix * vec4(inPosition, 1);
	fragColor = color;
	gl_Position = projMatrix * posEyeSpace;
}
```

<br>

### Câmera
A principal mudança com relação ao projeto LookAt é que há uma nova rotação de câmera no projeto Garden! A função `Camera::tilt()` permite que a câmera rotacione para cima e para baixo. Essa função permitiu praticar os conceitos vistos em matrizes de transformação, espaços do OpenGL, álgebra linear e câmera LookAt.

A implementação da função é feita abaixo:
``` C++
void Camera::tilt(float speed) {
	glm::mat4 transform{glm::mat4(1.0f)};

	const glm::vec3 forward   {glm::normalize(m_at - m_eye)};
	const glm::vec3 forward_xz{forward.x, 0.0f, forward.z};
	const glm::vec3 crossVec  {glm::cross(forward_xz, glm::vec3{0.0f, 1.0f, 0.0f})};

	// rotate camera around its local y axis
	transform = glm::translate(transform, m_eye);
	transform = glm::rotate(transform, -speed, crossVec);
	transform = glm::translate(transform, -m_eye);

	m_at = transform * glm::vec4(m_at, 1.0f);

	computeViewMatrix();
}
```

Perceba que, primeiramente, foi feita uma "projeção" do vetor ```forward```, que é o vetor em que a câmera aponta, no plano xz. Então fizemos um produto vetorial com o vetor ```forward``` no plano xz com o eixo y. Isso retorna o eixo que será usado para rotacionarmos.

Por que isso é necessário? Quando a câmera translada e rotaciona para os lados, o eixo que precisamos rotacionar ao redor deixa de ser o eixo x. E é esse novo eixo calculado ```crossVec``` que iremos rotacionar a câmera.

![image](https://user-images.githubusercontent.com/23441506/142777755-5524ed8f-bc15-4047-9eac-6482592493bc.png)


<br>


### Inicialização do cenário
A inicialização do cenário é feita com ```OpenGLWindow::initializeGL```. Aqui é onde carregamos o modelo da árvore que será usada, criamos os buffers e geramos as características das árvores, que é a adição feita no método.

As posições, as cores e o tamanho das árvores é gerada no seguinte trecho:
``` C++
auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
m_randomEngine.seed(seed);
std::uniform_int_distribution<int> intDistribution(1, 10);

for(int i = 0; i < tree.nObjects; i++){		
	tree.positionX[i] = float(intDistribution(m_randomEngine)) * 1.0f;
	tree.positionY[i] = float(intDistribution(m_randomEngine)) * 1.0f;
	tree.colorRed[i] = float(intDistribution(m_randomEngine)) * 0.08f;
	tree.colorGreen[i] = float(intDistribution(m_randomEngine)) * 0.1f;
	tree.size[i] = float(intDistribution(m_randomEngine)) * 0.05f;
	tree.positionXOut[i] = intDistribution(m_randomEngine) > 5;
	tree.positionYOut[i] = intDistribution(m_randomEngine) > 5;
}
```


<br>


### Terreno
O terreno é um chão plano de cor sólida centralizado no plano xz. Suas características foram definidas em ```Ground::paintGL```
``` C++
for (const auto z : iter::range(-N, N+1)) {
	for (const auto x : iter::range(-N, N+1)) {
		// set model matrix
		glm::mat4 model{1.0f};
		model = glm::translate(model, glm::vec3(x, 0.0f, z));
		abcg::glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, &model[0][0]);

		// cor terreno
		abcg::glUniform4f(m_colorLoc, 0.8f, 0.5f, 0.0f, 1.0f);

		abcg::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}
```

<br>


### Controles
As respostas aos eventos dos dispositivos de entrada continuam sendo feitas na função `OpenGLWindow::handleEvent(SDL_Event& ev)`. Foi feito um remapeamento das teclas com o seguinte trecho:
``` C++
if (ev.type == SDL_KEYDOWN) {
	if (ev.key.keysym.sym == SDLK_UP)    m_tiltSpeed = -1.0f;
	if (ev.key.keysym.sym == SDLK_DOWN)  m_tiltSpeed =  1.0f;
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
```

O suporte ao mouse foi introduzido com o seguinte bloco de código, usando o suporte do SDL ao evento de movimento do mouse `SDL_MOUSEMOTION`:
``` C++
if (ev.type == SDL_MOUSEMOTION) {
	m_tiltSpeed = (static_cast<float>(ev.motion.yrel))*1.5f;
	m_panSpeed = (static_cast<float>(ev.motion.xrel))*2.0f;
	update();
	m_tiltSpeed = 0.0f;
	m_panSpeed = 0.0f;
}
```

## Desenvolvedores

Kauan Manzato do Nascimento (RA: 11201721592)

Otávio Sanchez (RA: 11094713)

## License

[MIT](https://choosealicense.com/licenses/mit/)
