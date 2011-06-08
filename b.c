int f(int x)
{
    int val;
    val = x * x + 2;
    return val;
}

int main(int argc, char *argv[])
{
    int rc;
    rc = f(argc);
    return rc;
}
