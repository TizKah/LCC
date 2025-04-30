-module(hello).
-export([init/0, upper_aux/0, hello_aux/1, greeting/0, upper/0, hello/1]).

-define(GREETING, "Hola").

greeting() -> ?GREETING.

hello_aux(UpperPid) ->
    link(UpperPid),
    hello(greeting()).

hello(Greeting) ->
    receive after 500 ->
        ok
    end,
    io:fwrite("~s ~p~n", [Greeting, case rand:uniform(10) of 10 -> 1/uno; _ -> self() end]),
    hello(?MODULE:greeting()).

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
