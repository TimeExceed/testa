%% -*- erlang -*-

%% This file is picked from project testa [https://github.com/TimeExceed/testa.git]
%% Copyright (c) 2017, Taoda (tyf00@aliyun.com)
%% All rights reserved.

%% Redistribution and use in source and binary forms, with or without modification,
%% are permitted provided that the following conditions are met:

%% * Redistributions of source code must retain the above copyright notice, this
%%   list of conditions and the following disclaimer.

%% * Redistributions in binary form must reproduce the above copyright notice, this
%%   list of conditions and the following disclaimer in the documentation and/or
%%   other materials provided with the distribution.

%% * Neither the name of the {organization} nor the names of its
%%   contributors may be used to endorse or promote products derived from
%%   this software without specific prior written permission.

%% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
%% ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
%% WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
%% DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
%% ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
%% (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
%% LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
%% ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
%% (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
%% SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-module(testa_test).

multipleTb_(_, []) -> ok;
multipleTb_(Case, [H|T]) -> 
    case Case(H) of
        ok -> multipleTb_(Case, T);
        {error, Msg} -> {error, Msg}
    end.
            
multipleTb(Case) ->
    Ins = [{X, Y} || X <- lists:seq(1, 5), Y <- lists:seq(1, 5)],
    multipleTb_(Case, Ins).

gcd(X, Y) ->
    if Y == 0 -> X;
       true -> gcd(Y, X rem Y)
    end.

gcdtb_(_, []) -> ok;
gcdtb_(Case, [H|T]) -> 
    case Case(H) of
        ok -> gcdtb_(Case, T);
        {error, Msg} -> {error, Msg}
    end.

gcdtb(Case) ->
    Ins = [{X, Y} || X <- lists:seq(0, 12), Y <- lists:seq(0, 12), (X /= 0) or (Y /= 0)],
    gcdtb_(Case, Ins).
            
main(Args) ->
    Cases = #{
      "testTesta.correctAddition" => 
          testa:is(
            fun() -> 1 + 2 end,
            3),
      "testTesta.wrongAddition" =>
          testa:is(
           fun() -> 1 + 2 end,
           2),
      "testTesta.correctMultiple" =>
          testa:eq(
            fun({X,Y}) -> X*Y end,
            fun({X,Y}) -> lists:sum([X || _ <- lists:seq(1, Y)]) end,
            fun(X) -> multipleTb(X) end),
      "testTesta.wrongMultiple" =>
          testa:eq(
            fun({X,Y}) -> X+Y end,
            fun({X,Y}) -> lists:sum([X || _ <- lists:seq(1, Y)]) end,
            fun(X) -> multipleTb(X) end),
      "testTesta.correctGcdIs" =>
          testa:is(
            fun({X, Y}) -> gcd(X, Y) end,
            4,
            fun(Case) -> Case({8, 12}) end),
      "testTesta.correctGcd" =>
          testa:verify(
            fun({X,Y}) -> gcd(X, Y) end,
            fun(Result, {X, Y}) ->
                    if (X rem Result == 0) and (Y rem Result == 0) -> ok;
                       true -> {error, io_lib:format("~p/=gcd(~p,~p)", [Result, X, Y])}
                    end
            end,
            fun(X) -> gcdtb(X) end),
      "testTesta.wrongGcd" =>
          testa:verify(
            fun({X,Y}) -> gcd(X, Y) end,
            fun(Result, {X, Y}) ->
                    if (X rem Result /= 0) or (Y rem Result /= 0) -> ok;
                       true -> {error, io_lib:format("~p/=gcd(~p,~p)", [Result, X, Y])}
                    end
            end,
            fun(X) -> gcdtb(X) end)},
    testa:main(Args, Cases).

