int pow(int x, int y)
{
    if (y == 0)
        return 1;
    return x * pow(x, y - 1);
}

int a = 2, b = 2;
int main()
{
    return pow(a, b);
}