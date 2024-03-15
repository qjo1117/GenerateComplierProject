class Info
{
    private var a = 10;
};

function Func(a, b)
{
    return a * 3 + b * 2;
}

function main() {
    var val2 = 100;
    var val = Info;
    val.a = 30;
    print Func('Hello ', "World");
}