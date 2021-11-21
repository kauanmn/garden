# Garden

Garden é um projeto 3D capaz de gerar aleatoriamente o cenário, com árvores de cores e tamanhos variados toda vez que é iniciado.

## Descrição técnica

O projeto foi desenvolvido utilizando as bibliotecas ABCg e Dear ImGui e a API gráfica OpenGL:
- A biblioteca **ABCg** possibilita a prototipagem rápida de aplicações gráficas interativas 3D em C++ capazes de rodar tanto no desktop quanto no navegador.
- O **OpenGL** é uma API gráfica que permite a renderização de gráficos 2D e 3D através de uma pipeline gráfica.


### Inicialização do cenário

O cenário é inicializado no método `OpenGLWindow::initializeGL` junto com as configurações da janela. Isso porque os outros métodos são chamados a cada frame de exibição por isso geramos as posições, cores e tamanhos das árvores nesse instante.

## Desenvolvedores

Kauan Manzato do Nascimento (RA: 11201721592)

Otávio Sanchez (RA: 11094713)

## License

[MIT](https://choosealicense.com/licenses/mit/)
