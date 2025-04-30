-module(hello).
-export([init/0, hello_aux/1]).
-export([upper_aux/0]).

hello_aux(UpperPid) ->
    link(UpperPid),
    hello().

hello() ->
    receive after 500 ->
        ok
    end,
    io:fwrite("Hello ~p~n", [case rand:uniform(10) of 10 -> 1/uno; _ -> self() end]),
    hello().

upper_aux() ->
    process_flag(trap_exit, true),
    upper().

upper() ->
    receive
        {'EXIT', From, Reason} ->
            io:format("Process error: ~p~n", [{'EXIT', From, Reason}]),
            spawn(?MODULE, hello_aux, [self()]),
            upper()
    end.

init() ->
    UpperPid = spawn(?MODULE, upper_aux, []),
    HelloPid = spawn(?MODULE, hello_aux, [UpperPid]),
    UpperPid ! {helloPid, HelloPid}.
