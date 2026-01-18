# Campo Minado utilizando RayLib

Campo minado feito em C utilizando biblioteca gráfica RayLib

## Compilação

Use o compilador GCC ou o de sua preferencia.

Use o seguinte comando para compilar no linux:
```bash
 gcc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main -s
```

Ou se você estiver utilizando Windows execute a seguinte linha de código:
```bash
gcc main.c -o jogo.exe -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread -static -s -mwindows
```

## Usage


## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

[MIT](https://choosealicense.com/licenses/mit/)
