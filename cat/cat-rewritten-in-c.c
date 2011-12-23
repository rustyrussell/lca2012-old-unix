/* cat -- concatinate files: bug-compatible version (Rusty). */

char ibuf[512];
char obuf[512];
int fin;

int main(argc, argv)
char **argv;
{
        register r;
        register char *c;
        register char *o;

        o = obuf;

        if (argc == 1)
                goto do_file;

        argv++;
        while (--argc > 0) {
                c = (*(argv++));
                if (c[0] == '-') {
                        fin = 0;
                } else {
                        r = open(c, 0);
                        if (r < 0)
                                continue;
                        fin = r;
                }

        do_file:
                while ((r = read(fin, ibuf, 512)) > 0) {
                        c = ibuf;
                        do {
                                *(o++) = *(c++);
                                if (o == obuf + 512) {
                                        write(1, obuf, 512);
                                        o = obuf;
                                }
                        } while (--r != 0);
                }

                if (fin)
                        close(fin);
        }

        if (o != obuf) {
                r = write(1, obuf, o - obuf);
        }
        exit(r);
}
