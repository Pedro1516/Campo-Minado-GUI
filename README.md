# Campo Minado em C com Raylib

Projeto de Campo Minado desenvolvido em linguagem C, utilizando a biblioteca gráfica Raylib para renderização, entrada de dados e gerenciamento de janela.

------------------------------------------------------------

## Requisitos

- Compilador GCC (ou compatível)
- Biblioteca Raylib instalada
- Sistema operacional Linux ou Windows (MinGW)

------------------------------------------------------------

## Compilação

### Linux

Utilize o comando abaixo para compilar o projeto no Linux:

```bash
gcc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main -s
```

O executável gerado será chamado "main".

------------------------------------------------------------

### Windows (MinGW)

Para compilar no Windows utilizando MinGW, execute:

```bash
gcc main.c -o jogo.exe -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread -static -s -mwindows
```

O executável gerado será chamado "jogo.exe".

------------------------------------------------------------

## Ícone no Windows

Para exibir um ícone no executável do Windows, é necessário utilizar um arquivo de recursos (.rc).

O arquivo "recursos.rc" pode ser encontrado na pasta:
programas auxiliares

Esse arquivo deve ser convertido para um arquivo objeto (.o) antes da compilação.

### Conversão do arquivo .rc para .o

Utilize o comando abaixo com o MinGW:

```bash
windres recursos.rc -O coff -F pe-x86-64 -o recursos.o
```

------------------------------------------------------------

### Compilação com ícone

Após a conversão, compile o projeto incluindo o arquivo de recursos:

```bash
gcc main.c recursos.o -o jogo.exe -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread -static -s -mwindows
```

 **Nota:** 
Lembre-se de mover o arquivo "recursos.o" para fora da pasta "programas auxiliares"
ou ajustar o caminho no comando de compilação.

------------------------------------------------------------

## Inclusão de Imagens no Binário

É possível compilar o executável (Linux ou Windows) com as imagens já embutidas,
sem depender de arquivos PNG externos.

Para isso, as imagens devem ser convertidas para arquivos .h.

------------------------------------------------------------

## Conversão de PNG para Header (.h)

O projeto inclui um utilitário chamado:
"conversor png para header.c"

### Compilação do conversor

```bash
gcc "conversor png para header.c" -o conversor.exe
```

------------------------------------------------------------

### Uso do conversor

```bash
./conversor.exe seu_sprite.png meuSprite > meu_sprite.h
```

Onde:
- seu_sprite.png : imagem original
- meuSprite      : nome da variável que armazenará a imagem
- meu_sprite.h   : arquivo de saída


**Nota:**
"meuSprite" será o nome da variável dentro do arquivo .h que contém a imagem.


------------------------------------------------------------

## Execução

Linux:
```bash
./main
```

Windows:
```bash
jogo.exe
```

------------------------------------------------------------

## Contribuição

Pull requests são bem-vindos.

Para mudanças maiores:
- Abra uma issue antes de enviar o pull request
- Atualize ou adicione testes quando apropriado

------------------------------------------------------------

## Licença

Este projeto está licenciado sob a licença MIT.

Mais informações:
https://choosealicense.com/licenses/mit/
