
int main(int argc, char *argv[])
{
    int count = 0;
    switch (argc)
    {
    case 1:
        count = 1;
        [[fallthrough]];
    case 2:
        count = 2;
        break;
    case 3:
        count = 3;
   }
}