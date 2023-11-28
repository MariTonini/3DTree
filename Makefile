# Makefile para compilar o código da árvore 3D

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -Wextra -std=c99

# Bibliotecas
LIBS = -lGL -lGLU -lglut -lm

# Nome do executável
TARGET = 3DTree

# Arquivos fonte
SOURCES = 3DTree.c

# Comando de compilação
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

# Comando para limpar os arquivos gerados durante a compilação
clean:
	rm -f $(TARGET)

