#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Estrutura para representar um nó da árvore
struct Node {
    int key;
    struct Node *left, *right;
};

// Definição de um ponteiro para a estrutura Node
typedef struct Node* Tree;

// Parâmetros de iluminação e material
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
GLfloat ambient[] = {0.3, 0.3, 0.3, 1.0};
GLfloat diffuse[] = {0.9, 0.6, 0.3, 1.0};
GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat highlightedSpecular[] = {1.0, 0.0, 0.0, 1.0};
GLfloat shininess = 50.0;

// Variáveis globais para a árvore e interação
Tree root = NULL;
Tree highlightedNode = NULL;
int searchKey = -1;
int searching = 0;
int highlightDuration = 3;

// Variáveis de transformação
float translationX = 0.0;
float translationY = 0.0;
float translationZ = 0.0;
float rotationAngle = 0.0;
float scale = 1.0;

// Função para desenhar uma esfera representando um nó
void drawSphere(double radius, int key, int isHighlighted) {
    GLUquadricObj *quadratic;
    quadratic = gluNewQuadric();
    gluQuadricDrawStyle(quadratic, GLU_FILL);

    if (isHighlighted) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, highlightedSpecular);
        glutSolidSphere(radius * 1.2, 32, 32);
    } else {
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glutSolidSphere(radius, 32, 32);
    }

    glRasterPos3f(-0.05, -0.05, radius + 0.1);
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, '0' + key);
}

// Função de temporizador para remover o destaque após um período
void update(int value) {
    if (highlightedNode != NULL) {
        highlightedNode = NULL;
        searching = 0;
        glutPostRedisplay();
    }
}

// Função para inserir um nó na árvore
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

// Função para remover um nó da árvore
Tree removeNode(Tree root, int key) {
    if (root == NULL)
        return root;

    if (key < root->key)
        root->left = removeNode(root->left, key);
    else if (key > root->key)
        root->right = removeNode(root->right, key);
    else {
        if (root->left == NULL) {
            Tree temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Tree temp = root->left;
            free(root);
            return temp;
        }

        Tree temp = root->right;
        while (temp->left != NULL)
            temp = temp->left;
        
        root->key = temp->key;
        root->right = removeNode(root->right, temp->key);
    }

    return root;
}

// Função para buscar um nó na árvore
Tree search(Tree root, int k) {
    if (root == NULL || root->key == k)
        return root;

    if (root->key > k)
        return search(root->left, k);
    else
        return search(root->right, k);
}

// Função para construir a árvore a partir de argumentos de linha de comando
void buildTree(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        int key = atoi(argv[i]);
        root = insert(root, key);
    }
}

// Função para desenhar a árvore em 3D
void drawTree(Tree root, double x, double y, double z, double scale) {
    if (root == NULL)
        return;

    int isHighlighted = 0;

    if (searching && root->key == searchKey) {
        isHighlighted = 1;
    } else if (highlightedNode != NULL && root->key == highlightedNode->key) {
        isHighlighted = 1;
    }

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);
    drawSphere(0.2, root->key, isHighlighted);
    glPopMatrix();

    if (root->left != NULL) {
        glColor3f(0, 0, 0);
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex3f(x, y, z);
        glVertex3f(x - 0.5, y - 0.5, z - 0.5);
        glEnd();
        drawTree(root->left, x - 0.5, y - 0.5, z - 0.5, scale);
    }

    if (root->right != NULL) {
        glColor3f(0, 0, 0);
        glLineWidth(2.0);
        glBegin(GL_LINES);
        glVertex3f(x, y, z);
        glVertex3f(x + 0.5, y - 0.5, z - 0.5);
        glEnd();
        drawTree(root->right, x + 0.5, y - 0.5, z - 0.5, scale);
    }
}

// Função de exibição
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glClearColor(0.5, 0.7, 1.0, 1.0);

    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 10);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    glTranslatef(translationX, translationY, translationZ);
    glRotatef(rotationAngle, 0.0, 1.0, 0.0);
    glScalef(scale, scale, scale);

    drawTree(root, 0, 0, 0, 1.0);

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glutSwapBuffers();
}

// Função de teclado para interação do usuário
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
            scale += 0.1;
            glutPostRedisplay();
            break;
        case '-':
            if (scale > 0.1) {
                scale -= 0.1;
                glutPostRedisplay();
            }
            break;
        case 'W':
        case 'w':
            translationY += 0.1;
            glutPostRedisplay();
            break;
        case 'S':
        case 's':
            translationY -= 0.1;
            glutPostRedisplay();
            break;
        case 'A':
        case 'a':
            translationX -= 0.1;
            glutPostRedisplay();
            break;
        case 'D':
        case 'd':
            translationX += 0.1;
            glutPostRedisplay();
            break;
        case 'Q':
        case 'q':
            rotationAngle += 5.0;
            glutPostRedisplay();
            break;
        case 'E':
        case 'e':
            rotationAngle -= 5.0;
            glutPostRedisplay();
            break;
        case 27:  // Tecla Esc
            exit(0);
            break;
    }
}

// Função principal
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s chave1 chave2 chave3 ... (pelo menos um valor de chave)\n", argv[0]);
        return 1;
    }

    buildTree(argc, argv);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("3D Tree");
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(highlightDuration * 1000, update, 0);
    glutMainLoop();

    return 0;
}

