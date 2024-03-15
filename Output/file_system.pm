class Info
{
    private var a = 10;
};

function Func(a, b)
{
    return a * 3 + b * 2;
}

function main() {
    var val = Info;
    val.a = 10;
    print Func('Hello ', "World");
}