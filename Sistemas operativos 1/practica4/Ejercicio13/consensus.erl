%% Inicia el módulo
-module(consensus).
-export([start/0, sum_process/2]).

%% Configuración de la bandera global
% Para activar el debug con los prints compilar con c(consensus, [{d, debug}]).
-ifdef(debug).
-define(DEBUG_PRINT(X), io:format("~s", [X])).
-else.
-define(DEBUG_PRINT(X), ok).
-endif.

start() ->
    %% Número de nucleos/nodos
    NumProcesses = 16,
    %% Activa el modo debug si se define la bandera DEBUG
    ?DEBUG_PRINT("Debug mode activated.~n"),
    spawn_processes(NumProcesses).

spawn_processes(NumProcesses) ->
    %% Crear y arrancar procesos
    Pids = [spawn(consensus, sum_process, [NumProcesses, Id]) || Id <- lists:seq(0, NumProcesses-1)],
    io:format("Spawned processes: ~p~n", [Pids]),
    lists:foreach(fun(Pid) -> Pid ! {start, Pids} end, Pids).

sum_process(NumProcesses, Id) ->
    io:format("Process ~p started~n", [Id]),
    receive
        {start, Pids} ->
            Sum = sum_loop(NumProcesses, Id, Pids, Id, 1),
            if Id =:= 0 ->
                %% Proceso con Id = 0 realiza un broadcast de Sum
                lists:foreach(fun(Pid) -> Pid ! {broadcast, Sum} end, Pids);
            true -> ok
            end,
            receive
                {broadcast, TotalSum} ->
                    io:format("Process ~p has total sum = ~p~n", [Id, TotalSum])
            end
    end.

sum_loop(N, Id, Pids, Sum, Step) when Step < N ->
    ?DEBUG_PRINT(io:format("Process ~p at step ~p with sum ~p~n", [Id, Step, Sum])),
    %% Determinar si enviar o recibir según el paso
    if
        Id rem (2 * Step) == 0 ->
            ?DEBUG_PRINT(io:format("Process ~p receiving from ~p~n", [Id, Id + Step])),
            receive
                {value, ReceivedValue} ->
                    NewSum = Sum + ReceivedValue,
                    sum_loop(N, Id, Pids, NewSum, Step * 2)
            end;
        Id rem Step == 0 ->
            ToPidIndex = Id - Step,
            ToPid = lists:nth((ToPidIndex rem length(Pids)) + 1, Pids),
            ?DEBUG_PRINT(io:format("Process ~p sending to ~p~n", [Id, ToPidIndex])),
            ToPid ! {value, Sum},
            sum_loop(N, Id, Pids, Sum, Step * 2);
        true ->
            sum_loop(N, Id, Pids, Sum, Step * 2)
    end;
sum_loop(_, _, _, Sum, _) ->
    Sum.
