## Estrutura do Projeto

O projeto está organizado nos seguintes arquivos e pastas principais:

- `main.cpp`: Arquivo principal que inicializa o programa, gerencia a leitura do arquivo `.obj`, constrói a estrutura de dados half-edge e processa as consultas do usuário. Ele também coordena a renderização do objeto 2D.
- `io/objreader.cpp`: Responsável por ler arquivos `.obj` e extrair os vértices, arestas e faces, convertendo-os para a estrutura de dados utilizada pelo programa.
- `renderer/renderer.cpp`: Implementa a renderização do objeto usando OpenGL, desenhando pontos e arestas conforme definidos na estrutura half-edge.
- `geometry/TwoDHalfEdgeGeometry.h`: Define as classes e estruturas para o half-edge, vértices, arestas e faces, além de métodos para consultas e manipulação da geometria.

## Funcionamento do `main.cpp`

Ao executar o programa, o `main.cpp` realiza as seguintes etapas:

1. **Leitura do arquivo OBJ**: Utiliza funções de `io/objreader.cpp` para importar a geometria do objeto 2D.
2. **Construção da estrutura Half-Edge**: Com auxílio das definições em `geometry/TwoDHalfEdgeGeometry.h`, monta a estrutura de dados que representa eficientemente a topologia do objeto.
3. **Renderização**: Chama funções de `renderer/renderer.cpp` para exibir o objeto na tela usando OpenGL.
4. **Consultas interativas**: Permite ao usuário realizar consultas sobre faces, arestas e vértices, utilizando métodos definidos na estrutura half-edge.

Dessa forma, cada componente do projeto contribui para importar, estruturar, renderizar e consultar objetos gráficos 2D de forma eficiente.

## Como rodar:
### Windows
```bash
g++ main.cpp io/objreader.cpp renderer/renderer.cpp -o meu_programa.exe -lfreeglut -lopengl32 -lglu32 -Wno-deprecated
meu_programa.exe [arquivo].obj
```
Certifique-se de ter a ```freeglut.dll``` em seu repositório.

### Linux
```bash
g++ main.cpp io/objreader.cpp renderer/renderer.cpp -o meu_programa -lglut -lGLU -lGL
meu_programa [arquivo].obj
```
## Página do projeto no github
https://github.com/fabio-bays/compgraf/tree/main



