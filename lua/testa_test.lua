local testa = require 'testa'

local testTesta = {}

testTesta.correctAddition = testa.is(
    function ()
        return 1 + 2
    end,
    3
)

testTesta.wrongAddition = testa.is(
    function ()
        return 2 + 2
    end,
    3
)

testTesta.correctMultiple = testa.eq(
    function (x, y)
        return x * y
    end,
    function (x, y)
        local r = 0
        for i = 1, y do 
            r = r + x
        end
        return r
    end,
    function (case)
        for i = 1, 5 do
            for j = 1, 5 do
                local ok, msg = case(i, j)
                if not ok then
                    return false, msg
                end
            end
        end
        return true
    end
)

testTesta.wrongMultiple = testa.eq(
    function (x, y)
        return x + y
    end,
    function (x, y)
        local r = 0
        for i = 1, y do 
            r = r + x
        end
        return r
    end,
    function (case)
        for i = 1, 5 do
            for j = 1, 5 do
                local ok, msg = case(i, j)
                if not ok then
                    return false, msg
                end
            end
        end
        return true
    end
)

local function gcd(a, b)
    if a == 0 and b == 0 then
        error("undefined on (0,0)")
    end
    if a == 0 then
        return b
    else
        return gcd(b % a, a)
    end
end

local function gcdtb(case)
    for i = 0, 12 do
        for j = 0, 12 do
            if i ~= 0 or j ~= 0 then
                local ok, msg = case(i, j)
                if not ok then
                    return false, msg
                end
            end
        end
    end
    return true
end

testTesta.correctGcd = testa.verify(
    gcd,
    function (result, i, j)
        if i % result == 0 and j % result == 0 then
            return true
        else
            return false, string.format('%d~=gcd(%d,%d)', result, i, j)
        end
    end,
    gcdtb
)

testTesta.wrongGcd = testa.verify(
    gcd,
    function (result, i, j)
        if i % result ~= 0 or j % result ~= 0 then
            return true
        else
            return false, string.format('%d~=gcd(%d,%d)', result, i, j)
        end
    end,
    gcdtb
)

testa.main({TestTesta = testTesta})
