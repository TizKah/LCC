
# a) ¿Qué similitudes y diferencias encontró en cuanto a la implementación?
# b) Compare la performance y la robustez. ¿Qué pasa si un proceso muere en cada caso?
Ambos códigos implementan la suma por consenso utilizando algoritmos de paso de mensajes. Cada proceso tiene un valor inicial y el objetivo es calcular la suma global distribuidamente mediante el envío y recepción de mensajes entre procesos. Además, ambos verifican que el número de procesos sea una potencia de 2 para garantizar un funcionamiento correcto.

Sin embargo, difieren en varios aspectos clave. En cuanto a la implementación, el código MPI utiliza la biblioteca MPI para la comunicación entre procesos, mientras que el código Erlang aprovecha el lenguaje Erlang para la misma tarea. En cuanto a la estructura, el código MPI es más lineal y directo, mientras que el código Erlang es más modular y recurso, facilitando una estructura más adaptable y escalable.

En términos de depuración, el código Erlang ofrece una función específica para imprimir mensajes adicionales que ayudan a entender el flujo del programa, lo cual es una ventaja considerable para el desarrollo y la depuración. No obstante, el manejo de errores difiere, ya que el código Erlang no aborda explícitamente casos como la muerte de un proceso, mientras que el código MPI también presenta ciertas limitaciones en la robustez al no manejar explícitamente situaciones críticas que podrían afectar el flujo del programa.

En cuanto al rendimiento y la robustez, es difícil realizar una comparación precisa sin pruebas específicas, pero se podría anticipar que el código MPI podría tener un mejor rendimiento debido a la optimización de la biblioteca MPI para la comunicación entre procesos. Sin embargo, en términos de robustez, el código Erlang podría demostrar mayor resistencia al continuar con los procesos activos ante fallos no previstos.

# Test mpi:

  Command being timed: "mpirun -np 8 ./a.out"
  User time (seconds): 0.35
  System time (seconds): 0.22
  Percent of CPU this job got: 159%
  Elapsed (wall clock) time (h:mm:ss or m:ss): 0:00.36
  Average shared text size (kbytes): 0
  Average unshared data size (kbytes): 0
  Average stack size (kbytes): 0
  Average total size (kbytes): 0
  Maximum resident set size (kbytes): 16244
  Average resident set size (kbytes): 0
  Major (requiring I/O) page faults: 550
  Minor (reclaiming a frame) page faults: 9374
  Voluntary context switches: 7443
  Involuntary context switches: 84
  Swaps: 0
  File system inputs: 32
  File system outputs: 0
  Socket messages sent: 0
  Socket messages received: 0
  Signals delivered: 0
  Page size (bytes): 4096
  Exit status: 0

# Test erlang:

  Command being timed: "erl -noshell -eval consensus:start(), init:stop()."
  User time (seconds): 0.13
  System time (seconds): 0.09
  Percent of CPU this job got: 19%
  Elapsed (wall clock) time (h:mm:ss or m:ss): 0:01.16
  Average shared text size (kbytes): 0
  Average unshared data size (kbytes): 0
  Average stack size (kbytes): 0
  Average total size (kbytes): 0
  Maximum resident set size (kbytes): 39968
  Average resident set size (kbytes): 0
  Major (requiring I/O) page faults: 916
  Minor (reclaiming a frame) page faults: 7040
  Voluntary context switches: 782
  Involuntary context switches: 8
  Swaps: 0
  File system inputs: 0
  File system outputs: 0
  Socket messages sent: 0
  Socket messages received: 0
  Signals delivered: 0
  Page size (bytes): 4096
  Exit status: 0