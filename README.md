## Estrutura do Projeto

O projeto está organizado nos seguintes arquivos e pastas principais:

- `main.cpp`: Arquivo principal que inicializa o programa, gerencia a leitura do arquivo `.obj`, constrói a estrutura de dados half-edge e processa as consultas do usuário. Ele também coordena a renderização do objeto 2D.
- `io/objreader.cpp`: Responsável por ler e salvar arquivos `.obj`, extraindo vértices e faces, e convertendo-os para a estrutura de dados utilizada pelo programa. O salvamento garante que os índices das faces estejam corretos após transformações.
- `renderer/renderer.cpp`: Implementa a renderização do objeto usando OpenGL, desenhando pontos e arestas conforme definidos na estrutura half-edge. Permite alternar entre algoritmos de linha (Paramétrico, Bresenham, Xiaolin Wu) e realizar transformações interativas (translação, rotação, escala, reflexão, cisalhamento).
- `dsa/TwoDHalfEdgeGeometry.h`: Define as classes e estruturas para o half-edge, vértices, arestas e faces, além de métodos para consultas, manipulação da geometria e cálculo robusto do centróide (centroide ponderado por área para malhas triangulares).
- `io/usr.cpp`: Interface de linha de comando para consultas interativas sobre a geometria (faces adjacentes, arestas, etc).


## Funcionamento Geral

Ao executar o programa, o fluxo principal é:

1. **Leitura do arquivo OBJ**: Utiliza funções de `io/objreader.cpp` para importar a geometria do objeto 2D, garantindo que os dados estejam corretos para a estrutura half-edge.
2. **Construção da estrutura Half-Edge**: Com auxílio das definições em `dsa/TwoDHalfEdgeGeometry.h`, monta a estrutura de dados que representa eficientemente a topologia do objeto, permitindo consultas rápidas e manipulação robusta.
3. **Renderização Interativa**: Chama funções de `renderer/renderer.cpp` para exibir o objeto na tela usando OpenGL. O usuário pode alternar entre algoritmos de linha, aplicar transformações geométricas (translação, rotação, escala, reflexão, cisalhamento) e visualizar labels de IDs.
4. **Consultas e Interface**: Permite ao usuário realizar consultas sobre faces, arestas e vértices, tanto pela interface gráfica quanto pela linha de comando (`io/usr.cpp`).
5. **Salvamento de Arquivo OBJ**: Após transformações, o objeto pode ser salvo corretamente em um novo arquivo `.obj`, preservando a topologia e a ordem dos vértices/faces.

Cada componente do projeto contribui para importar, estruturar, renderizar, transformar e consultar objetos gráficos 2D de forma eficiente e robusta.

## Como rodar:

### Windows
```bash
g++ main.cpp io/objreader.cpp renderer/renderer.cpp -o meu_programa.exe -lfreeglut -lopengl32 -lglu32 -Wno-deprecated
meu_programa.exe [arquivo].obj
```
Certifique-se de ter a `freeglut.dll` em seu repositório.

### Linux
```bash
g++ main.cpp io/objreader.cpp renderer/renderer.cpp -o meu_programa -lglut -lGLU -lGL
./meu_programa [arquivo].obj
```
## Página do projeto no GitHub
https://github.com/fabio-bays/compgraf/tree/atividade_4


