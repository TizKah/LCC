# grupo7-P1
Práctica 1

## Ejercicio 10
Si en vez de utilizar un loop infinito utilizamos la funcion `pause` lo que logramos es
que luego que el proceso padre o hijo recibe una señal, este no vuelve a entrar en un loop,
sino que continua la ejecucion normal del programa luego de `pause`.

## Ejercicio 11
### b)
Garantizar que dos pedidos no reciban el mismo numero se puede realizar de varias formas:
- realizar las respuestas de manera secuencial,
- o bloquear el acceso a memoria de otros procesos cuando uno accede a la misma.

### c)
La ventaja que trae `select` es que el programa recibe y guarda las conexiones 
(a traves de los file descriptors) y los levanta y utiliza a medida que son necesitados
o utilizados.
