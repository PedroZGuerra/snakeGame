#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Definindo a largura e altura da janela
const int Largura = 1920;
const int Altura = 1080;
const float TAMANHO_GRADE = 0.05f; // Tamanho de cada célula no grid
const double INTERVALO_MOVIMENTO = 0.1; // Intervalo de tempo entre movimentos

// Definindo direções para a cobra
enum Direcao { CIMA, BAIXO, ESQUERDA, DIREITA, NENHUMA };
Direcao direcaoAtual = DIREITA; // Direção inicial do jogador
Direcao direcaoAI = NENHUMA; // Direção inicial da AI

// Estrutura representando um segmento da cobra
struct SegmentoCobra {
    float x, y;
};

// Vetores que armazenam os segmentos da cobra do jogador e da AI
std::vector<SegmentoCobra> cobra;
std::vector<SegmentoCobra> cobraAI;
bool comidaExiste = false; // Indicador se a comida está presente na tela
SegmentoCobra comida; // Posição da comida
double ultimoTempoMovimento = 0.0; // Controla o intervalo de movimento

// Função para gerar comida em uma posição aleatória no grid
void gerarComida() {
    comida.x = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
    comida.y = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
}

// Função para processar as entradas do jogador (teclas pressionadas)
void processarEntrada(GLFWwindow* janela) {
    if (glfwGetKey(janela, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(janela, true); // Fecha o jogo se ESC for pressionado
    }

    // Movimenta a cobra com as teclas W, S, A, D
    if (glfwGetKey(janela, GLFW_KEY_W) == GLFW_PRESS && direcaoAtual != BAIXO) {
        direcaoAtual = CIMA;
    }
    if (glfwGetKey(janela, GLFW_KEY_S) == GLFW_PRESS && direcaoAtual != CIMA) {
        direcaoAtual = BAIXO;
    }
    if (glfwGetKey(janela, GLFW_KEY_A) == GLFW_PRESS && direcaoAtual != DIREITA) {
        direcaoAtual = ESQUERDA;
    }
    if (glfwGetKey(janela, GLFW_KEY_D) == GLFW_PRESS && direcaoAtual != ESQUERDA) {
        direcaoAtual = DIREITA;
    }
}

// Atualiza a posição da cobra de acordo com a direção atual
void atualizarCobra(std::vector<SegmentoCobra>& cobra, Direcao& dir) {
    SegmentoCobra novaCabeca = cobra.front(); // Nova cabeça será criada com base na posição atual da frente

    // Movimenta a cobra conforme a direção
    switch (dir) {
    case CIMA: novaCabeca.y += TAMANHO_GRADE; break;
    case BAIXO: novaCabeca.y -= TAMANHO_GRADE; break;
    case ESQUERDA: novaCabeca.x -= TAMANHO_GRADE; break;
    case DIREITA: novaCabeca.x += TAMANHO_GRADE; break;
    }

    cobra.insert(cobra.begin(), novaCabeca); // Insere a nova cabeça

    // Verifica se a cobra comeu a comida
    if (novaCabeca.x < comida.x + TAMANHO_GRADE && novaCabeca.x > comida.x - TAMANHO_GRADE &&
        novaCabeca.y < comida.y + TAMANHO_GRADE && novaCabeca.y > comida.y - TAMANHO_GRADE) {
        comidaExiste = false; // A cobra comeu, não remove a cauda
    }
    else {
        cobra.pop_back(); // Se não comeu, remove a cauda (movimento normal)
    }

    // Verifica colisão com o próprio corpo
    for (size_t i = 1; i < cobra.size(); ++i) {
        if (novaCabeca.x == cobra[i].x && novaCabeca.y == cobra[i].y) {
            // Se colidir, reinicia a cobra
            cobra.clear();
            cobra.push_back({ 0.0f, 0.0f });
            dir = DIREITA; // Reseta a direção do jogador
            return;
        }
    }

    // Verifica colisão com as bordas da tela
    if (novaCabeca.x > 1.0f || novaCabeca.x < -1.0f || novaCabeca.y > 1.0f || novaCabeca.y < -1.0f) {
        cobra.clear();
        cobra.push_back({ 0.0f, 0.0f });
        dir = DIREITA; // Reseta a direção do jogador
    }

    // Se não houver comida, gera uma nova
    if (!comidaExiste) {
        gerarComida();
        comidaExiste = true;
    }
}

// Desenha um segmento de cobra ou comida em uma cor específica
void desenharSegmento(float x, float y, float r, float g, float b) {
    glColor3f(r, g, b); // Define a cor do segmento
    glBegin(GL_QUADS); // Desenha um quadrado
    glVertex2f(x - TAMANHO_GRADE, y - TAMANHO_GRADE);
    glVertex2f(x + TAMANHO_GRADE, y - TAMANHO_GRADE);
    glVertex2f(x + TAMANHO_GRADE, y + TAMANHO_GRADE);
    glVertex2f(x - TAMANHO_GRADE, y + TAMANHO_GRADE);
    glEnd();
}

// Desenha toda a cobra com a cor gradiente
void desenharCobra(const std::vector<SegmentoCobra>& cobra, float r, float g, float b) {
    desenharSegmento(cobra.front().x, cobra.front().y, r, g, b); // Desenha a cabeça

    // Desenha o corpo com uma variação de cor
    for (size_t i = 1; i < cobra.size(); ++i) {
        float valorCor = static_cast<float>(i) / cobra.size();
        desenharSegmento(cobra[i].x, cobra[i].y, r * valorCor, g * valorCor, b * valorCor); // Corpo com cor gradiente
    }
}

// Desenha a comida
void desenharComida() {
    if (comidaExiste) {
        desenharSegmento(comida.x, comida.y, 1.0f, 0.0f, 0.0f); // Comida em vermelho
    }
}

// Função para mover a AI (computador) em direção à comida
void moverAI() {
    if (cobraAI.empty()) return;

    SegmentoCobra cabecaAI = cobraAI.front();

    // Movimento horizontal em direção à comida
    if (fabs(cabecaAI.x - comida.x) > TAMANHO_GRADE) {
        if (cabecaAI.x < comida.x) {
            direcaoAI = DIREITA;
        }
        else if (cabecaAI.x > comida.x) {
            direcaoAI = ESQUERDA;
        }
    }
    // Movimento vertical em direção à comida
    else if (fabs(cabecaAI.y - comida.y) > TAMANHO_GRADE) {
        if (cabecaAI.y < comida.y) {
            direcaoAI = CIMA;
        }
        else if (cabecaAI.y > comida.y) {
            direcaoAI = BAIXO;
        }
    }

    atualizarCobra(cobraAI, direcaoAI); // Atualiza a posição da AI

    // Verifica se a AI comeu a comida
    if (cabecaAI.x < comida.x + TAMANHO_GRADE && cabecaAI.x > comida.x - TAMANHO_GRADE &&
        cabecaAI.y < comida.y + TAMANHO_GRADE && cabecaAI.y > comida.y - TAMANHO_GRADE) {
        comidaExiste = false; // Se comeu, remove a comida
    }

    // Verifica colisão da AI com as bordas da tela
    if (cobraAI.front().x > 1.0f || cobraAI.front().x < -1.0f || cobraAI.front().y > 1.0f || cobraAI.front().y < -1.0f) {
        cobraAI.clear();
        cobraAI.push_back({ 0.5f, 0.0f });
        direcaoAI = NENHUMA;
    }
}

// Função principal
int main() {
    srand(static_cast<unsigned>(time(0))); // Semente para geração aleatória

    if (!glfwInit()) {
        std::cout << "Erro ao inicializar o GLFW" << std::endl;
        return 1;
    }

    GLFWwindow* Janela = glfwCreateWindow(Largura, Altura, "Jogo da Cobrinha", nullptr, nullptr);

    if (!Janela) {
        std::cout << "Erro ao criar janela" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(Janela);
    glfwSwapInterval(1);
    glewInit(); 

    // Inicializa a cobra do jogador e da AI
    cobra.push_back({ 0.0f, 0.0f });
    cobraAI.push_back({ 0.5f, 0.0f });

    gerarComida(); // Gera a primeira comida

    // Loop principal
    while (!glfwWindowShouldClose(Janela)) {
        processarEntrada(Janela); // Processa as entradas do jogador

        double tempoAtual = glfwGetTime();
        if (tempoAtual - ultimoTempoMovimento >= INTERVALO_MOVIMENTO) {
            atualizarCobra(cobra, direcaoAtual); // Atualiza a cobra do jogador
            moverAI(); // Move a AI
            ultimoTempoMovimento = tempoAtual;
        }

        // Limpa a tela e desenha os elementos
        glClearColor(0.8f, 0.7f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        desenharCobra(cobra, 0.0f, 1.0f, 0.0f); // Desenha a cobra do jogador (verde)
        desenharCobra(cobraAI, 1.0f, 0.5f, 0.0f); // Desenha a cobra da AI (laranja)
        desenharComida(); // Desenha a comida

        // Verifica colisão entre o jogador e a AI
        if (!cobra.empty() && !cobraAI.empty()) {
            if (cobra.front().x == cobraAI.front().x && cobra.front().y == cobraAI.front().y) {
                // Reseta a cobra do jogador se colidir com a AI
                cobra.clear();
                cobra.push_back({ 0.0f, 0.0f });
                direcaoAtual = DIREITA;
            }
        }

        glfwSwapBuffers(Janela); 
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
