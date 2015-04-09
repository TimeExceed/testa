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
    print(string.format('%s [--help|-h] [--list] [CASENAME]', arg[0]))
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
        for suite, cases in pairs(tbl) do
            for name, func in pairs(cases) do
                print(string.format('%s.%s', suite, name))
            end
        end
        os.exit(0)
    elseif action == 'case' then
        local suite, name = string.match(case, '(%w+)[.](%w+)')
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
    -- for suite, cases in pairs(tbl) do
    --     for name, func in pairs(cases) do
    --         local ok, msg = func()
    --         if ok then
    --             print(string.format('%s.%s passed', suite, name))
    --         else
    --             print(string.format('%s.%s failed: %s', suite, name, msg))
    --         end
    --     end
    -- end
end

return {
    is = is,
    eq = eq,
    verify = verify,

    dummytb = dummytb,

    main = main
}
