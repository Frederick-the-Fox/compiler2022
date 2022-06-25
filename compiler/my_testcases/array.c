void init(int arr[][10][10])
{
    int i = 0;
    while (i < 10)
    {
        int j = 0;
        while (j < 10)
        {
            int k = 0;
            while (k < 10)
            {
                arr[i][j][k] = 0;
                k = k + 1;
            }
            j = j + 1;
        }
        i = i + 1;
    }
}

int f1(int a0[])
{
    return a0[0];
}

int main()
{
    int arr[10][10][10], sum = 0;
    init(arr);
    sum = sum + f1(arr[0][0]);
    putint(sum);
    putch(10);
    return 0;
}