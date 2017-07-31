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

-module(testa).

-export([is/2, is/3, eq/3, verify/3]).
-export([main/2]).

%%====================================================================
%% API functions
%%====================================================================

-spec is(fun(() -> any()), any()) -> fun(() -> ok | {error, string()}).

is(Fn, ExpVal) ->
    fun() ->
            RealVal = Fn(),
            if RealVal == ExpVal -> ok;
               true -> {error, io_lib:format("Expect=~p Got=~p", [ExpVal, RealVal])}
            end
    end.

is(Fn, ExpVal, Tb) ->
    fun() ->
            Tb(fun(X) ->
                       RealVal = Fn(X),
                       if RealVal == ExpVal -> ok;
                          true -> {error, io_lib:format("Expect=~p Got=~p", [ExpVal, RealVal])}
                       end
               end)
    end.

-spec eq(
        fun((In) -> any()), 
        fun((In) -> any()),
        fun((fun((In) -> ok | {error, string()})) -> ok | {error, string()})) 
        -> fun(() -> ok | {error, string()}).

eq(TrialFn, OracleFn, Tb) ->
    fun() -> Tb(
               fun(In) ->
                       RealVal = TrialFn(In),
                       ExpVal = OracleFn(In),
                       if RealVal =:= ExpVal -> ok;
                          true -> {error, io_lib:format("Expect=~p Got=~p Input=~p", [ExpVal, RealVal, In])}
                       end
               end)
    end.

verify(TrialFn, OracleFn, Tb) ->
    fun() -> Tb(
               fun(In) ->
                       Result = TrialFn(In),
                       OracleFn(Result, In)
               end)
    end.

-spec main([string()], #{string()=>fun(() -> ok | {error, string()})}) -> none().

main(Args, Cases) ->
    if length(Args) /= 1 -> 
            usage(),
            halt(1);
       true -> 
            case Args of
                ["--help"] -> 
                    usage(),
                    halt(0);
                ["-h"] -> 
                    usage(),
                    halt(0);
                ["--show-cases"] ->
                    Keys = maps:keys(Cases),
                    io:format("~s~n", [string:join(Keys, "\n")]),
                    halt(0);
                [Arg] -> 
                    case maps:find(Arg, Cases) of
                        error ->
                            io:format(standard_error, "unknown case: ~s~n", [Arg]),
                            halt(1);
                        {ok, Case} ->
                            case Case() of
                                ok -> halt(0);
                                {error, Msg} -> 
                                    io:format(standard_error, "Case fail: ~s~n", [Msg]),
                                    halt(1)
                            end
                    end
            end
    end.

%%====================================================================
%% Internal functions
%%====================================================================

usage() ->
    io:format("CASENAME        a case name that will be executed~n"),
    io:format("--show-cases    a list of case names, one name per line~n"),
    io:format("--help,-h       this help message~n").


