struct C {
    char m_str[10];

    C() { this == m_str; }
};

int main(int argc, char *argv[])
{
    C c;
    return argv == &c;
}
