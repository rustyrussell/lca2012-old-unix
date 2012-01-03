#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>

/*
 * list file or directory
 */

struct buffered_file {
	int	fdes;
	int	nleft;
	char	*nextc;
	char	buff[512];
} inf;

#if 0
struct ibuf {
	int	idev;
	int	inum;
	int	iflags;
	char	inl;
	char	iuid;
	char	igid;
	char	isize0;
	int	isize;
	int	iaddr[8];
	time_t	iatime;
	time_t	imtime;
};
#endif

struct lbuf {
	char	lname[257];
	int	lnum;
	int	lflags;
	int	lnl;
	int	luid;
	int	lgid;
	char	lsize0;
	int	lsize;
	time_t	lmtime;
};

struct lbufx {
	char	*namep;
};

int	aflg, dflg, lflg, sflg, tflg, uflg, iflg, fflg, gflg;
int	fout;
int	rflg = 1;
unsigned year;
int	flags;
int	uidfil = -1;
int	lastuid	= -1;
char	tbuf[16];
int	tblocks;
int	statreq;
struct	lbuf	*lastp;
struct	lbuf	*rlastp;
char	*dotp = ".";

int printf(const char *format, ...);
extern void *stdout;
int fflush(void *stream);
char *ctime(const time_t *timep);
void *sbrk(long increment);

/* part of V6 standard library. */
static char *locv(int hi, int lo)
{
	static char buffer[32];
	int sprintf(char *, const char *format, ...);
	sprintf(buffer, "%u", (hi << 16) | lo);
	return buffer;
}

static int ldiv(int hi, int lo, int div)
{
	return ((hi << 16) | lo) / div;
}

static void seek(int fd, int off, int whence)
{
	lseek(fd, 0, 0);
}

/* This is in the standard library for V6, in asm! */
static int fopen_(char *file, struct buffered_file *bf)
{
	bf->fdes = open(file, O_RDWR);
	bf->nleft = 0;
	bf->nextc = bf->buff;
	return bf->fdes;
}

static int getc_(struct buffered_file *bf)
{
	if (bf->nleft) {
		bf->nleft--;
		return *(bf->nextc++);
	}
	bf->nextc = bf->buff;
	bf->nleft = read(bf->fdes, &bf->buff, sizeof(bf->buff));
	if (bf->nleft > 0) {
		bf->nleft--;
		return *(bf->nextc++);
	}
	return -1;
}

/* pre-declare */
struct lbuf *gstat();
int select_(int *pairp);

#define	IFMT	060000
#define	DIRX	0100000
#define	CHR	020000
#define	BLK	040000
#define	ISARG	01000
#define	LARGE	010000
#define	STXT	010000
#define	SUID	04000
#define	SGID	02000
#define	ROWN	0400
#define	WOWN	0200
#define	XOWN	0100
#define	RGRP	040
#define	WGRP	020
#define	XGRP	010
#define	ROTH	04
#define	WOTH	02
#define	XOTH	01
#define	RSTXT	01000

main(argc, argv)
char **argv;
{
	int i, j;
	register struct lbuf *ep, *ep1;
	register struct lbuf *slastp, *firstp;
	struct lbuf lb;
	int t;
	int compar();

	firstp = rlastp = lastp = sbrk(0);
	fout = dup(1);
	time(&lb.lmtime);
	year = (lb.lmtime >> 16) - 245; /* 6 months ago */
	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'a':
			aflg++;
			continue;

		case 's':
			sflg++;
			statreq++;
			continue;

		case 'd':
			dflg++;
			continue;

		case 'g':
			gflg++;
			continue;

		case 'l':
			lflg++;
			statreq++;
			continue;

		case 'r':
			rflg = -1;
			continue;

		case 't':
			tflg++;
			statreq++;
			continue;

		case 'u':
			uflg++;
			continue;

		case 'i':
			iflg++;
			continue;

		case 'f':
			fflg++;
			continue;

		default:
			continue;
		}
		argc--;
	}
	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		statreq = 0;
	}
	if(lflg) {
		char *t = "/etc/passwd";
		if(gflg)
			t = "/etc/group";
		uidfil = open(t, 0);
	}
	if (argc==0) {
		argc++;
		argv = &dotp - 1;
	}
	for (i=0; i < argc; i++) {
		if ((ep = gstat(*++argv, 1))==0)
			continue;
		((struct lbufx *)ep)->namep = *argv;
		ep->lflags |= ISARG;
	}
	qsort(firstp, lastp - firstp, sizeof *lastp, compar);
	slastp = lastp;
	for (ep = firstp; ep<slastp; ep++) {
		if (ep->lflags&DIRX && dflg==0 || fflg) {
			if (argc>1)
				printf("\n%s:\n", ((struct lbufx *)ep)->namep);
			lastp = slastp;
			readdir_(((struct lbufx *)ep)->namep);
			if (fflg==0)
				qsort(slastp,lastp - slastp,sizeof *lastp,compar);
			if (statreq)
				printf("total %d\n", tblocks);
			for (ep1=slastp; ep1<lastp; ep1++)
				pentry(ep1);
		} else 
			pentry(ep);
	}
	fflush(stdout);
	return 0;
}

pentry(ap)
struct lbuf *ap;
{
	register t;
	register struct lbuf *p;
	register char *cp;

	p = ap;
	if (p->lnum == -1)
		return;
	if (iflg)
		printf("%5d ", p->lnum);
	if (lflg) {
		pmode(p->lflags);
		printf("%2d ", p->lnl);
		t = p->luid;
		if(gflg)
			t = p->lgid;
#if 0
		t &= 0377;
#endif
		if (getname(t, tbuf)==0)
			printf("%-6.6s", tbuf);
		else
			printf("%-6d", t);
		if (p->lflags & (BLK|CHR)) {
			struct { char dminor, dmajor;} *dev = (void *)&p->lsize;
			printf("%3d,%3d", dev->dmajor&0377,
			    dev->dminor&0377);
		} else
			printf("%7s", locv(p->lsize0, p->lsize));
		cp = ctime(&p->lmtime);
		if((p->lmtime >> 16) < year)
			printf(" %-7.7s %-4.4s ", cp+4, cp+20); else
			printf(" %-12.12s ", cp+4);
	} else if (sflg)
		printf("%4d ", nblock(p->lsize0, p->lsize));
	if (p->lflags&ISARG)
		printf("%s\n", ((struct lbufx *)p)->namep);
	else
		printf("%-14s\n", p->lname);
}

getname(uid, buf)
int uid;
char buf[];
{
	int j, c, n, i;

	if (uid==lastuid)
		return(0);
	inf.fdes = uidfil;
	seek(inf.fdes, 0, 0);
	inf.nleft = 0;
	lastuid = -1;
	do {
		i = 0;
		j = 0;
		n = 0;
		while((c=getc_(&inf)) != '\n') {
			if (c<0)
				return(-1);
			if (c==':') {
				j++;
				c = '0';
			}
			if (j==0)
				buf[i++] = c;
			if (j==2)
				n = n*10 + c - '0';
		}
	} while (n != uid);
	buf[i++] = '\0';
	lastuid = uid;
	return(0);
}

nblock(size0, size)
unsigned size0, size;
{
	register int n;

	n = ldiv(size0, size, 512);
	if (size&0777)
		n++;
	if (n>8)
		n =+ (n+255)/256;
	return(n);
}

int	m0[] = { 3, DIRX, 'd', BLK, 'b', CHR, 'c', '-'};
int	m1[] = { 1, ROWN, 'r', '-' };
int	m2[] = { 1, WOWN, 'w', '-' };
int	m3[] = { 2, SUID, 's', XOWN, 'x', '-' };
int	m4[] = { 1, RGRP, 'r', '-' };
int	m5[] = { 1, WGRP, 'w', '-' };
int	m6[] = { 2, SGID, 's', XGRP, 'x', '-' };
int	m7[] = { 1, ROTH, 'r', '-' };
int	m8[] = { 1, WOTH, 'w', '-' };
int	m9[] = { 1, XOTH, 'x', '-' };
int	m10[] = { 1, STXT, 't', ' ' };

int	*m[] = { m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10};

pmode(aflag)
{
	register int **mp;

	flags = aflag;
	for (mp = &m[0]; mp < &m[11];)
		select_(*mp++);
}

int select_(int *pairp)
{
	register int n, *ap;

	ap = pairp;
	n = *ap++;
	while (--n>=0 && (flags&*ap++)==0)
		ap++;
	putchar(*ap);
}

char *makename(dir, file)
char *dir, *file;
{
	static char dfile[1000];
	register char *dp, *fp;
	register int i;

	dp = dfile;
	fp = dir;
	while (*fp)
		*dp++ = *fp++;
	*dp++ = '/';
	fp = file;
	for (i=0; i<256; i++)
		*dp++ = *fp++;
	*dp = 0;
	return(dfile);
}

readdir_(dir)
char *dir;
{
#if 0
	static struct {
		int	dinode;
		char	dname[14];
	} dentry;
	register char *p;
	register int j;
	register struct lbuf *ep;

	if (fopen(dir, &inf) < 0) {
		printf("%s unreadable\n", dir);
		return;
	}
	tblocks = 0;
	for(;;) {
		p = &dentry;
		for (j=0; j<16; j++)
			*p++ = getc(&inf);
		if (dentry.dinode==0
		 || aflg==0 && dentry.dname[0]=='.')
			continue;
		if (dentry.dinode == -1)
			break;
		ep = gstat(makename(dir, dentry.dname), 0);
		if (ep->lnum != -1)
			ep->lnum = dentry.dinode;
		for (j=0; j<14; j++)
			ep->lname[j] = dentry.dname[j];
	}
	close(inf.fdes);
#else
	struct dentry {
		unsigned long	dinode;
		unsigned long	doff;
		unsigned short	dreclen;
		char		dname[1];
	} *p;
	register int j;
	register struct lbuf *ep;

	inf.fdes = open(dir, 0);
	if (inf.fdes < 0) {
		printf("%s unreadable\n", dir);
		return;
	}
	tblocks = 0;
	for(;;) {
		int n;
#ifdef __dietlibc__
		n = getdents(inf.fdes, &inf.buff, sizeof(inf.buff));
#else
		n = syscall(SYS_getdents, inf.fdes, &inf.buff, sizeof(inf.buff));
#endif		
		if (n <= 0)
			break;

		for (p = (struct dentry *)inf.buff;
		     (char *)p < inf.buff + n;
		     p = (void *)p + p->dreclen) {
			if (p->dinode==0
			    || aflg==0 && p->dname[0]=='.')
				continue;
			ep = gstat(makename(dir, p->dname), 0);
			if (ep->lnum != -1)
				ep->lnum = p->dinode;
			for (j=0; j<p->dreclen - 2 - (p->dname - (char*)p); j++)
				ep->lname[j] = p->dname[j];
		}
	}
	close(inf.fdes);
#endif	
}

struct lbuf *gstat(file, argfl)
char *file;
{
	struct stat statb;
	register struct lbuf *rep;

	if (lastp+1 >= rlastp) {
		rlastp = sbrk(512) + 512;
	}
	rep = lastp;
	lastp++;
	rep->lflags = 0;
	rep->lnum = 0;
	if (argfl || statreq) {
		if (stat(file, &statb)<0) {
			printf("%s not found\n", file);
			statb.st_ino = -1;
			statb.st_size = 0;
			statb.st_mode = 0;
			if (argfl) {
				lastp--;
				return(0);
			}
		}
		rep->lnum = statb.st_ino;
		statb.st_mode &= ~DIRX;
		if ((statb.st_mode&IFMT) == 060000) {
			statb.st_mode &= ~020000;
		} else if ((statb.st_mode&IFMT)==040000) {
			statb.st_mode &= ~IFMT;
			statb.st_mode |= DIRX;
		}
		statb.st_mode &= ~ LARGE;
		if (statb.st_mode & RSTXT)
			statb.st_mode |= STXT;
		statb.st_mode &= ~ RSTXT;
		rep->lflags = statb.st_mode;
		rep->luid = statb.st_uid;
		rep->lgid = statb.st_gid;
		rep->lnl = statb.st_nlink;
		rep->lsize0 = statb.st_size >> 16;
		rep->lsize = statb.st_size & 0xFFFF;
		if (rep->lflags & (BLK|CHR) && lflg)
			rep->lsize = statb.st_rdev;
		rep->lmtime = statb.st_mtime;
		if(uflg) {
			rep->lmtime = statb.st_atime;
		}
		tblocks =+ nblock(0, statb.st_size);
	}
	return(rep);
}

compar(ap1, ap2)
struct lbuf *ap1, *ap2;
{
	register struct lbuf *p1, *p2;
	register int i;
	int j;
	char *p1c, *p2c;

	p1 = ap1;
	p2 = ap2;
	if (dflg==0) {
		if ((p1->lflags&(DIRX|ISARG)) == (DIRX|ISARG)) {
			if ((p2->lflags&(DIRX|ISARG)) != (DIRX|ISARG))
				return(1);
		} else {
			if ((p2->lflags&(DIRX|ISARG)) == (DIRX|ISARG))
				return(-1);
		}
	}
	if (tflg) {
		i = 0;
		if (p2->lmtime > p1->lmtime)
			i++;
		else if (p2->lmtime < p1->lmtime)
			i--;
		return(i*rflg);
	}
	if (p1->lflags&ISARG)
		p1c = ((struct lbufx *)p1)->namep;
	else
		p1c = p1->lname;
	if (p2->lflags&ISARG)
		p2c = ((struct lbufx *)p2)->namep;
	else
		p2c = p2->lname;
	for (;;)
		if ((j = *p1c++ - *p2c++) || p1c[-1]==0)
			return(rflg*j);
	return(0);
}
