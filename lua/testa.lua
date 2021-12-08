-- This file is picked from project testa [https://github.com/TimeExceed/testa.git]
-- Copyright (c) 2013, Taoda (tyf00@aliyun.com)
-- All rights reserved.

-- Redistribution and use in source and binary forms, with or without modification,
-- are permitted provided that the following conditions are met:

-- * Redistributions of source code must retain the above copyright notice, this
--   list of conditions and the following disclaimer.

-- * Redistributions in binary form must reproduce the above copyright notice, this
--   list of conditions and the following disclaimer in the documentation and/or
--   other materials provided with the distribution.

-- * Neither the name of the {organization} nor the names of its
--   contributors may be used to endorse or promote products derived from
--   this software without specific prior written permission.

-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
-- ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
-- WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
-- DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
-- ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
-- ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
-- SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

local function dummytb(verifier)
    return verifier()
end

local function is(casefunc, expect, tb)
    if not tb then
        tb = dummytb
    end
    return function ()
        return tb(function ()
            local actual = casefunc()
            if actual == expect then
                return true
            else
                return false, string.format('expect: %s actual %s', 
                    tostring(expect), tostring(actual))
            end
        end)
    end
end

local function eq(casefunc, oracle, tb)
    if not tb then
        tb = dummytb
    end
    return function ()
        return tb(function (...)
            local actual = casefunc(...)
            local expect = oracle(...)
            if actual == expect then
                return true
            else
                return false, string.format('expect: %s actual: %s, by applying %s', 
                    tostring(expect), tostring(actual),
                    table.concat({...}, ', '))
            end
        end)
    end
end

local function verify(casefunc, oracle, tb)
    if not tb then
        tb = dummytb
    end
    return function ()
        return tb(function (...)
            local result = casefunc(...)
            return oracle(result, ...)
        end)
    end
end


local function printHelp()
    print(string.format('%s [--help|-h] [--show-cases] [CASENAME]', arg[0]))
    print('CASENAME', 'a case name that will be executed')
    print('--show-cases', 'a list of case names, one name per line')
    print('--help,-h', 'this help message')
end

local function parseArgs()
    for _, v in ipairs(arg) do
        if v == '-h' or v == '--help' then
            return 'help'
        end
    end
    if arg[1] == '--show-cases' then
        return 'list'
    end
    if #arg == 1 and string.match(arg[1], '%w+[.]%w+') then
        return 'case', arg[1]
    end
    return 'unknownarg', arg
end

local function main(tbl)
    action, case = parseArgs()
    if action == 'help' then
        printHelp()
        os.exit(0)
    elseif action == 'list' then
        local res = {}
        for suite, cases in pairs(tbl) do
            for name, func in pairs(cases) do
                table.insert(res, string.format('{"name": "%s.%s"}', suite, name))
            end
        end
        print('[')
        print(table.concat(res, ',\n'))
        print(']')
        os.exit(0)
    elseif action == 'case' then
        local suite, name = string.match(case, '([_%w]+)[.]([_%w]+)')
        if not tbl[suite] or not tbl[suite][name] then
            print(string.format('unknown case: %s', case))
            os.exit(1)
        else
            local func = tbl[suite][name]
            local ok, msg = func()
            if ok then
                os.exit(0)
            else
                io.stdout:write(msg)
                os.exit(1)
            end
        end
    else
        print(string.format('unknown args: %s', table.concat(arg, ' ')))
        os.exit(1)
    end
end

return {
    is = is,
    eq = eq,
    verify = verify,

    dummytb = dummytb,

    main = main
}
