#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Para a função sleep()

struct Node {
    int key;
    struct Node *left, *right;
};

typedef struct Node* Tree;

GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  // Posição da fonte de luz
GLfloat ambient[] = {0.3, 0.3, 0.3, 1.0};   // Componente ambiente do material
GLfloat diffuse[] = {0.9, 0.6, 0.3, 1.0};  // Componente difuso do material
GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};   // Componente especular do material
GLfloat shininess = 50.0;                      // Exponente de especularidade

Tree root = NULL;           // Declaração da árvore no escopo global
Tree highlightedNode = NULL; // Nó destacado na busca interativa
int searchKey = -1;          // Chave da busca interativa
int searching = 0;           // Estado de busca (0: não buscando, 1: buscando)
int highlightDuration = 3;   // Duração do destaque em segundos

float translationX = 0.0;
float translationY = 0.0;
float translationZ = 0.0;
float rotationAngle = 0.0;  // Ângulo de rotação em graus
float scale = 1.0;  // Fator de escala

void drawSphere(double radius, int key, int isHighlighted) {
    GLUquadricObj *quadratic;
    quadratic = gluNewQuadric();
    gluQuadricDrawStyle(quadratic, GLU_FILL);

    if (isHighlighted) {
        glColor3f(1.0, 0.0, 0.0); // Cor vermelha para o nó destacado
        glutSolidSphere(radius * 1.2, 32, 32);  // Aumenta ligeiramente o tamanho para destacar
        glColor3f(1.0, 1.0, 1.0); // Restaura a cor branca para o número
    } else {
        glColor3f(0.7, 0.7, 0.7); // Cor cinza para nós não destacados
        glutSolidSphere(radius, 32, 32);
    }

    glRasterPos3f(-0.05, -0.05, radius + 0.1);
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, '0' + key);
}

void update(int value) {
    if (highlightedNode != NULL) {
        highlightedNode = NULL;  // Limpa o destaque
        searching = 0;           // Finaliza o estado de busca
        glutPostRedisplay();     // Atualiza a tela
    }
}

Tree insert(Tree root, int key) {
    if (root == NULL) {
        Tree newNode = (Tree)malloc(sizeof(struct Node));
        newNode->key = key;
        newNode->left = newNode->right = NULL;
        return newNode;
    }

    if (key < root->key)
        root->left = insert(root->left, key);
    else if (key > root->key)
        root->right = insert(root->right, key);

    return root;
}

Tree removeNode(Tree root, int key) {
    if (root == NULL)
        return root;

    if (key < root->key)
        root->left = removeNode(root->left, key);
    else if (key > root->key)
        root->right = removeNode(root->right, key);
    else {
        // Caso 1: Nó sem ou com um filho
        if (root->left == NULL) {
            Tree temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Tree temp = root->left;
            free(root);
            return temp;
        }

        // Caso 2: Nó com dois filhos
        Tree temp = root->right;
        while (temp->left != NULL)
            temp = temp->left;
        
        root->key = temp->key;
        root->right = removeNode(root->right, temp->key);
    }

    return root;
}

Tree search(Tree root, int k) {
    if (root == NULL || root->key == k)
        return root;

    if (root->key > k)
        return search(root->left, k);
    else
        return search(root->right, k);
}

void buildTree(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        int key = atoi(argv[i]);
        root = insert(root, key);
    }
}

void drawTree(Tree root, double x, double y, double z, double scale) {
    if (root == NULL)
        return;

    int isHighlighted = 0;

    // Verifica se o nó atual está destacado
    if (searching && root->key == searchKey) {
        isHighlighted = 1;
    } else if (highlightedNode != NULL && root->key == highlightedNode->key) {
        isHighlighted = 1;
    }

    // Desenha a esfera representando o nó
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);
    drawSphere(0.2, root->key, isHighlighted);
    glPopMatrix();

    // Desenha os ramos
    if (root->left != NULL) {
        glColor3f(0.2, 0.8, 0.2); // Cor dos ramos (verde)
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex3f(x, y, z);
        glVertex3f(x - 0.5, y - 0.5, z - 0.5);
        glEnd();
        drawTree(root->left, x - 0.5, y - 0.5, z - 0.5, scale);
    }

    if (root->right != NULL) {
        glColor3f(0.2, 0.8, 0.2); // Cor dos ramos (verde)
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex3f(x, y, z);
        glVertex3f(x + 0.5, y - 0.5, z - 0.5);
        glEnd();
        drawTree(root->right, x + 0.5, y - 0.5, z - 0.5, scale);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glClearColor(0.5, 0.7, 1.0, 1.0);

    // Configura a câmera
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

    // Configura a matriz de projeção com perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 10);  // Angulo de visão, razão de aspecto, distância próxima e distância distante
    glMatrixMode(GL_MODELVIEW);

    // Configura a luz e materiais
    glEnable(GL_LIGHTING);  // Habilita a iluminação
    glEnable(GL_LIGHT0);    // Habilita a luz 0 (fonte de luz direcional)

    // Configura a luz
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    // Configura as propriedades do material
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    // Aplica translação, rotação e escala à árvore
    glTranslatef(translationX, translationY, translationZ);
    glRotatef(rotationAngle, 0.0, 1.0, 0.0);
    glScalef(scale, scale, scale);

    // Desenha a árvore
    drawTree(root, 0, 0, 0, 1.0);

    // Desabilita a iluminação
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'I':
        case 'i': {
            int newKey;
            printf("Digite a chave para inserir: ");
            scanf("%d", &newKey);
            root = insert(root, newKey);
            glutPostRedisplay();
            break;
        }
        case 'R':
        case 'r': {
            int removeKey;
            printf("Digite a chave para remover: ");
            scanf("%d", &removeKey);
            root = removeNode(root, removeKey);
            glutPostRedisplay();
            break;
        }
        case 'F':
        case 'f':
            printf("Digite a chave para buscar: ");
            scanf("%d", &searchKey);
            highlightedNode = search(root, searchKey);
            searching = 1;
            glutPostRedisplay();
            break;
        case '+':
            scale += 0.1;  // Aumenta a escala
            glutPostRedisplay();
            break;
        case '-':
            if (scale > 0.1) {
                scale -= 0.1;  // Diminui a escala (garante que não seja menor que 0.1)
                glutPostRedisplay();
            }
            break;
        case 'W':
        case 'w':
            translationY += 0.1;  // Translação para cima
            glutPostRedisplay();
            break;
        case 'S':
        case 's':
            translationY -= 0.1;  // Translação para baixo
            glutPostRedisplay();
            break;
        case 'A':
        case 'a':
            translationX -= 0.1;  // Translação para a esquerda
            glutPostRedisplay();
            break;
        case 'D':
        case 'd':
            translationX += 0.1;  // Translação para a direita
            glutPostRedisplay();
            break;
        case 'Q':
        case 'q':
            rotationAngle += 5.0;  // Rotação para a esquerda
            glutPostRedisplay();
            break;
        case 'E':
        case 'e':
            rotationAngle -= 5.0;  // Rotação para a direita
            glutPostRedisplay();
            break;
        case 27:  // Tecla Esc
            exit(0);
            break;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s chave1 chave2 chave3 ... (pelo menos um valor de chave)\n", argv[0]);
        return 1;
    }

    buildTree(argc, argv);  // Adiciona nós à árvore a partir dos argumentos de linha de comando

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("3D Tree");
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);  // Registra a função de teclado
    glClearColor(1.0, 1.0, 1.0, 1.0); // Cor de fundo branca
    glutTimerFunc(highlightDuration * 1000, update, 0);  // Configura a função de temporizador para remover o destaque
    glutMainLoop();

    return 0;
}

