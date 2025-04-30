-module(turnos).
-export([server/0, counter/1, wait_connect/2, get_request/2, test/0, client/0]).

get_request(Socket, CounterPid) ->
  receive
    {tcp, _SocketRecv, "NUEVO"} ->
      CounterPid ! {put, self()},
      receive
        {ok, N} ->
          gen_tcp:send(Socket, integer_to_list(N)),
          get_request(Socket, CounterPid);
        _ ->
          gen_tcp:send(Socket, "error:counter_error"),
          io:fwrite("Counter error~n")
      end;
    {tcp, _SocketRecv, "CHAU"} ->
      gen_tcp:send(Socket, "conn_ended");
    {tcp, _SocketRecv, _} ->
      gen_tcp:send(Socket, "error:unknown_command"),
      get_request(Socket, CounterPid)
  end.

wait_connect(ListenSocket, CounterPid) ->
  {ok, Socket} = gen_tcp:accept(ListenSocket),
  spawn (fun () -> wait_connect (ListenSocket, CounterPid) end),
  get_request(Socket, CounterPid).

counter(N) ->
  receive
    {put, Pid} ->
      Pid ! {ok, N},
      counter(N+1);
    _ ->
      io:fwrite("Counter error~n")
  end.

server() ->
  io:fwrite("server start~n"),
  {ok, ListenSocket} = gen_tcp:listen(8000, [{reuseaddr, true}]),
  CounterPid = spawn(turnos, counter, [0]),
  spawn(turnos, wait_connect, [ListenSocket, CounterPid]),
  io:fwrite("server end~n"),
  ok.

client() ->
  io:fwrite("client start~n"),
  {ok, ServerSocket} = gen_tcp:connect("localhost", 8000, []),
  gen_tcp:send(ServerSocket, "NUEVO"),
  receive
    {tcp, _, N} ->
      io:fwrite("Received: ~p~n", [N]);
    _ ->
      io:fwrite("Bad response~n")
  end,
  gen_tcp:send(ServerSocket, "CHAU"),
  receive
    {tcp, _, "conn_ended"} ->
      io:fwrite("Connection ended~n");
    _ ->
      io:fwrite("Error~n")
  end,
  gen_tcp:close(ServerSocket),
  io:fwrite("client end~n"),
  ok.

test() ->
  io:fwrite("start~n"),
  server(),
  client(),
  io:fwrite("end~n"),
  ok.