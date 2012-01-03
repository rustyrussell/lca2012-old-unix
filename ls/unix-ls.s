.globl  _inf
.comm   _inf,1006
.globl  _aflg
.comm   _aflg,2
.globl  _dflg
.comm   _dflg,2
.globl  _lflg
.comm   _lflg,2
.globl  _sflg
.comm   _sflg,2
.globl  _tflg
.comm   _tflg,2
.globl  _uflg
.comm   _uflg,2
.globl  _iflg
.comm   _iflg,2
.globl  _fflg
.comm   _fflg,2
.globl  _gflg
.comm   _gflg,2
.globl  _fout
.comm   _fout,2
.globl  _rflg
.data
_rflg:
1
.globl  _year
.comm   _year,2
.globl  _flags
.comm   _flags,2
.globl  _uidfil
.data
_uidfil:
177777
.globl  _lastuid
.data
_lastuid:
177777
.globl  _tbuf
.comm   _tbuf,20
.globl  _tblocks
.comm   _tblocks,2
.globl  _statreq
.comm   _statreq,2
.globl  _lastp
.data
_lastp:
_end
.globl  _rlastp
.data
_rlastp:
_end
.globl  _dotp
.data
_dotp:
L1
.globl  _main
.data
L10001:L8
L4
L4
L10
L4
L17
L11
L4
L16
L4
L4
L12
L4
L4
L4
L4
L4
L13
L9
L14
L15
.text
_main:
~~main:
~lb=177730
~argc=4
~argv=6
~ep=r4
~ep1=r3
~slastp=r2
~i=177770
~j=177766
~t=177726
jsr     r5,csv
sub     $44,sp
mov     $1,(sp)
jsr     pc,*$_dup
mov     r0,_fout
mov     r5,(sp)
add     $-16,(sp)
jsr     pc,*$_time
mov     -16(r5),r0
add     $-365,r0
mov     r0,_year
dec     4(r5)
jle     L3
mov     6(r5),r0
cmpb    $55,*2(r0)
jne     L3
add     $2,6(r5)
jbr     L4
L20001:mov      *6(r5),r0
movb    (r0),r0
sub     $141,r0
cmp     r0,$24
jhi     L4
asl     r0
jmp     *L10001(r0)
L8:inc  _aflg
jbr     L4
L9:inc  _sflg
L20006:inc      _statreq
jbr     L4
L10:inc _dflg
jbr     L4
L11:inc _gflg
jbr     L4
L12:inc _lflg
jbr     L20006
L13:mov $-1,_rflg
jbr     L4
L14:inc _tflg
jbr     L20006
L15:inc _uflg
jbr     L4
L16:inc _iflg
jbr     L4
L17:inc _fflg
L4:add  $1,*6(r5)
mov     *6(r5),r0
tstb    (r0)
jne     L20001
dec     4(r5)
L3:tst  _fflg
jeq     L19
inc     _aflg
clr     _lflg
clr     _sflg
clr     _tflg
clr     _statreq
L19:tst _lflg
jeq     L20
mov     $L21,-52(r5)
tst     _gflg
jeq     L22
mov     $L23,-52(r5)
L22:clr (sp)
mov     -52(r5),-(sp)
jsr     pc,*$_open
tst     (sp)+
mov     r0,_uidfil
L20:tst 4(r5)
jne     L24
inc     4(r5)
mov     $-2+_dotp,6(r5)
L24:clr -10(r5)
jbr     L25
L20003:mov      $1,(sp)
add     $2,6(r5)
mov     *6(r5),-(sp)
jsr     pc,*$_gstat
tst     (sp)+
mov     r0,r4
jeq     L27
mov     *6(r5),(r4)
bis     $1000,22(r4)
L27:inc -10(r5)
L25:cmp 4(r5),-10(r5)
jgt     L20003
mov     $_compar,(sp)
mov     $36,-(sp)
mov     _lastp,r1
sub     $_end,r1
sxt     r0
div     $36,r0
mov     r0,-(sp)
mov     $_end,-(sp)
jsr     pc,*$_qsort
add     $6,sp
mov     _lastp,r2
mov     $_end,r4
jbr     L28
L20005:bit      $-100000,22(r4)
jeq     L10003
tst     _dflg
jeq     L10002
L10003:tst      _fflg
jeq     L31
L10002:cmp      $1,4(r5)
jge     L32
mov     (r4),(sp)
mov     $L33,-(sp)
jsr     pc,*$_printf
tst     (sp)+
L32:mov r2,_lastp
mov     (r4),(sp)
jsr     pc,*$_readdir
tst     _fflg
jne     L34
mov     $_compar,(sp)
mov     $36,-(sp)
mov     _lastp,r1
sub     r2,r1
sxt     r0
div     $36,r0
mov     r0,-(sp)
mov     r2,-(sp)
jsr     pc,*$_qsort
add     $6,sp
L34:tst _statreq
jeq     L35
mov     _tblocks,(sp)
mov     $L36,-(sp)
jsr     pc,*$_printf
tst     (sp)+
L35:mov r2,r3
L37:cmp _lastp,r3
jlos    L30
mov     r3,(sp)
jsr     pc,*$_pentry
add     $36,r3
jbr     L37
L31:mov r4,(sp)
jsr     pc,*$_pentry
L30:add $36,r4
L28:cmp r2,r4
jhi     L20005
jsr     pc,_flush
jmp     cret
.globl  _pentry
.text
_pentry:
~~pentry:
~ap=4
~cp=r2
~p=r3
~t=r4
jsr     r5,csv
mov     4(r5),r3
cmp     $-1,20(r3)
jeq     L41
tst     _iflg
jeq     L42
mov     20(r3),(sp)
mov     $L43,-(sp)
jsr     pc,*$_printf
tst     (sp)+
L42:tst _lflg
jeq     L44
mov     22(r3),(sp)
jsr     pc,*$_pmode
movb    24(r3),r0
mov     r0,(sp)
mov     $L45,-(sp)
jsr     pc,*$_printf
tst     (sp)+
movb    25(r3),r4
tst     _gflg
jeq     L46
movb    26(r3),r4
L46:bic $-400,r4
mov     $_tbuf,(sp)
mov     r4,-(sp)
jsr     pc,*$_getname
tst     (sp)+
tst     r0
jne     L47
mov     $_tbuf,(sp)
mov     $L48,-(sp)
jbr     L20008
L47:mov r4,(sp)
mov     $L50,-(sp)
L20008:jsr      pc,*$_printf
tst     (sp)+
bit     $60000,22(r3)
jeq     L51
movb    30(r3),r0
mov     r0,(sp)
bic     $-400,(sp)
movb    31(r3),r0
mov     r0,-(sp)
bic     $-400,(sp)
mov     $L52,-(sp)
jsr     pc,*$_printf
cmp     (sp)+,(sp)+
jbr     L53
L51:mov 30(r3),(sp)
movb    27(r3),r0
mov     r0,-(sp)
jsr     pc,*$_locv
tst     (sp)+
mov     r0,(sp)
mov     $L54,-(sp)
jsr     pc,*$_printf
tst     (sp)+
L53:mov r3,(sp)
add     $32,(sp)
jsr     pc,*$_ctime
mov     r0,r2
cmp     _year,32(r3)
jlos    L55
mov     r2,(sp)
add     $24,(sp)
mov     r2,-(sp)
add     $4,(sp)
mov     $L56,-(sp)
jsr     pc,*$_printf
cmp     (sp)+,(sp)+
jbr     L59
L55:mov r2,(sp)
add     $4,(sp)
mov     $L58,-(sp)
jbr     L20010
L44:tst _sflg
jeq     L59
mov     30(r3),(sp)
movb    27(r3),r0
mov     r0,-(sp)
jsr     pc,*$_nblock
tst     (sp)+
mov     r0,(sp)
mov     $L61,-(sp)
L20010:jsr      pc,*$_printf
tst     (sp)+
L59:bit $1000,22(r3)
jeq     L62
mov     (r3),(sp)
mov     $L63,-(sp)
jbr     L20012
L62:mov r3,(sp)
mov     $L65,-(sp)
L20012:jsr      pc,*$_printf
tst     (sp)+
L41:jmp cret
.globl  _getname
.text
_getname:
~~getname:
~c=177766
~i=177762
~j=177770
~n=177764
~buf=6
~uid=4
jsr     r5,csv
sub     $10,sp
cmp     _lastuid,4(r5)
jne     L67
L20015:clr      r0
L66:jmp cret
L67:mov _uidfil,_inf
clr     (sp)
clr     -(sp)
mov     _inf,-(sp)
jsr     pc,*$_seek
cmp     (sp)+,(sp)+
clr     2+_inf
mov     $-1,_lastuid
L70:clr -16(r5)
clr     -10(r5)
clr     -14(r5)
jbr     L71
L20014:tst      -12(r5)
jge     L73
mov     $-1,r0
jbr     L66
L73:cmp $72,-12(r5)
jne     L74
inc     -10(r5)
mov     $60,-12(r5)
L74:tst -10(r5)
jne     L75
mov     -16(r5),r0
add     6(r5),r0
movb    -12(r5),(r0)
inc     -16(r5)
L75:cmp $2,-10(r5)
jne     L71
mov     -14(r5),r1
mul     $12,r1
add     -12(r5),r1
add     $-60,r1
mov     r1,-14(r5)
L71:mov $_inf,(sp)
jsr     pc,*$_getc
mov     r0,-12(r5)
cmp     $12,r0
jne     L20014
cmp     4(r5),-14(r5)
jne     L70
mov     -16(r5),r0
add     6(r5),r0
clrb    (r0)
inc     -16(r5)
mov     4(r5),_lastuid
jbr     L20015
.globl  _nblock
.text
_nblock:
~~nblock:
~size=6
~size0=4
~n=r4
jsr     r5,csv
mov     $1000,(sp)
mov     6(r5),-(sp)
mov     4(r5),-(sp)
jsr     pc,*$_ldiv
cmp     (sp)+,(sp)+
mov     r0,r4
bit     $777,6(r5)
jeq     L78
inc     r4
L78:cmp $10,r4
jge     L79
mov     r4,r0
add     $377,r0
ash     $-10,r0
add     r0,r4
L79:mov r4,r0
jmp     cret
.globl  _m0
.data
_m0:
3
100000
144
40000
142
20000
143
55
.globl  _m1
.data
_m1:
1
400
162
55
.globl  _m2
.data
_m2:
1
200
167
55
.globl  _m3
.data
_m3:
2
4000
163
100
170
55
.globl  _m4
.data
_m4:
1
40
162
55
.globl  _m5
.data
_m5:
1
20
167
55
.globl  _m6
.data
_m6:
2
2000
163
10
170
55
.globl  _m7
.data
_m7:
1
4
162
55
.globl  _m8
.data
_m8:
1
2
167
55
.globl  _m9
.data
_m9:
1
1
170
55
.globl  _m10
.data
_m10:
1
10000
164
40
.globl  _m
.data
_m:
_m0
_m1
_m2
_m3
_m4
_m5
_m6
_m7
_m8
_m9
_m10
.globl  _pmode
.text
_pmode:
~~pmode:
~mp=r4
~aflag=4
jsr     r5,csv
mov     4(r5),_flags
mov     $_m,r4
L20017:mov      (r4)+,(sp)
jsr     pc,*$_select
cmp     $26+_m,r4
jhi     L20017
jmp     cret
.globl  _select
.text
_select:
~~select:
~ap=r3
~n=r4
~pairp=4
jsr     r5,csv
mov     4(r5),r3
mov     (r3)+,r4
jbr     L84
L20019:bit      (r3)+,_flags
jne     L85
add     $2,r3
L84:dec r4
jge     L20019
L85:mov (r3),(sp)
jsr     pc,*$_putchar
jmp     cret
.globl  _makename
.text
_makename:
~~makename:
.bss
L87:.=.+144
.text
~dp=r4
~file=6
~fp=r3
~i=r2
~dfile=L87
~dir=4
jsr     r5,csv
mov     $L87,r4
mov     4(r5),r3
jbr     L88
L20021:movb     (r3)+,(r4)+
L88:tstb        (r3)
jne     L20021
movb    $57,(r4)+
mov     6(r5),r3
clr     r2
L20023:movb     (r3)+,(r4)+
inc     r2
cmp     $16,r2
jgt     L20023
clrb    (r4)
mov     $L87,r0
jmp     cret
.globl  _readdir
.text
_readdir:
~~readdir:
.bss
L94:.=.+20
.text
~ep=r2
~dentry=L94
~j=r3
~p=r4
~dir=4
jsr     r5,csv
mov     $_inf,(sp)
mov     4(r5),-(sp)
jsr     pc,*$_fopen
tst     (sp)+
tst     r0
jge     L95
mov     4(r5),(sp)
mov     $L96,-(sp)
jsr     pc,*$_printf
tst     (sp)+
jbr     L93
L95:clr _tblocks
L97:mov $L94,r4
clr     r3
L20025:mov      $_inf,(sp)
jsr     pc,*$_getc
movb    r0,(r4)+
inc     r3
cmp     $20,r3
jgt     L20025
tst     L94
jeq     L97
tst     _aflg
jne     L10004
cmpb    $56,2+L94
jeq     L97
L10004:cmp      $-1,L94
jeq     L98
clr     (sp)
mov     $2+L94,-(sp)
mov     4(r5),-(sp)
jsr     pc,_makename
cmp     (sp)+,(sp)+
mov     r0,-(sp)
jsr     pc,*$_gstat
tst     (sp)+
mov     r0,r2
cmp     $-1,20(r2)
jeq     L102
mov     L94,20(r2)
L102:clr        r3
L103:cmp        $16,r3
jle     L97
mov     r2,r0
add     r3,r0
movb    2+L94(r3),(r0)
inc     r3
jbr     L103
L98:mov _inf,(sp)
jsr     pc,*$_close
L93:jmp cret
.globl  _gstat
.text
_gstat:
~~gstat:
~file=4
~argfl=6
~rep=r4
~statb=177726
jsr     r5,csv
sub     $44,sp
mov     _lastp,r0
add     $36,r0
cmp     _rlastp,r0
jhi     L107
mov     $1000,(sp)
jsr     pc,*$_sbrk
add     $1000,_rlastp
L107:mov        _lastp,r4
add     $36,_lastp
clr     22(r4)
clr     20(r4)
tst     6(r5)
jne     L10005
tst     _statreq
jeq     L108
L10005:mov      r5,(sp)
add     $-52,(sp)
mov     4(r5),-(sp)
jsr     pc,*$_stat
tst     (sp)+
tst     r0
jge     L109
mov     4(r5),(sp)
mov     $L110,-(sp)
jsr     pc,*$_printf
tst     (sp)+
mov     $-1,-50(r5)
clrb    -41(r5)
clr     -40(r5)
clr     -46(r5)
tst     6(r5)
jeq     L109
sub     $36,_lastp
clr     r0
L106:jmp        cret
L109:mov        -50(r5),20(r4)
bic     $-100000,-46(r5)
mov     -46(r5),r0
bic     $-60001,r0
cmp     $60000,r0
jne     L112
bic     $20000,-46(r5)
jbr     L113
L112:mov        -46(r5),r0
bic     $-60001,r0
cmp     $40000,r0
jne     L113
bic     $60000,-46(r5)
bis     $-100000,-46(r5)
L113:bic        $10000,-46(r5)
bit     $1000,-46(r5)
jeq     L115
bis     $10000,-46(r5)
L115:bic        $1000,-46(r5)
mov     -46(r5),22(r4)
movb    -43(r5),25(r4)
movb    -42(r5),26(r4)
movb    -44(r5),24(r4)
movb    -41(r5),27(r4)
mov     -40(r5),30(r4)
bit     $60000,22(r4)
jeq     L116
tst     _lflg
jeq     L116
mov     -36(r5),30(r4)
L116:mov        -12(r5),32(r4)
mov     -10(r5),34(r4)
tst     _uflg
jeq     L117
mov     -16(r5),32(r4)
mov     -14(r5),34(r4)
L117:mov        -40(r5),(sp)
movb    -41(r5),r0
mov     r0,-(sp)
jsr     pc,*$_nblock
tst     (sp)+
add     r0,_tblocks
L108:mov        r4,r0
jbr     L106
.globl  _compar
.text
_compar:
~~compar:
~ap1=4
~ap2=6
~i=r2
~j=177770
~p1=r4
~p2=r3
jsr     r5,csv
tst     -(sp)
mov     4(r5),r4
mov     6(r5),r3
tst     _dflg
jne     L119
mov     22(r4),r0
bic     $76777,r0
cmp     $-77000,r0
jne     L120
mov     22(r3),r0
bic     $76777,r0
cmp     $-77000,r0
jeq     L119
mov     $1,r0
jbr     L118
L20027:mov      $-1,r0
jbr     L118
L20030:clr      r2
cmp     32(r4),32(r3)
jhis    L125
L20032:inc      r2
jbr     L126
L125:cmp        32(r4),32(r3)
jhi     L20028
cmp     34(r4),34(r3)
jlo     L20032
cmp     34(r4),34(r3)
jlos    L126
L20028:dec      r2
L126:mov        r2,r1
mul     _rflg,r1
L20031:mov      r1,r0
L118:jmp        cret
L120:mov        22(r3),r0
bic     $76777,r0
cmp     $-77000,r0
jeq     L20027
L119:tst        _tflg
jne     L20030
bit     $1000,22(r4)
jeq     L132
mov     (r4),r4
jbr     L133
L132:mov        r4,r4
L133:bit        $1000,22(r3)
jeq     L134
mov     (r3),r3
jbr     L136
L134:mov        r3,r3
L136:movb       (r4)+,r0
movb    (r3)+,r1
sub     r1,r0
mov     r0,-10(r5)
jne     L10006
tstb    -1(r4)
jne     L136
L10006:mov      _rflg,r1
mul     -10(r5),r1
jbr     L20031
.globl
.data
L1:.byte 56,0
L21:.byte 57,145,164,143,57,160,141,163,163,167,144,0
L23:.byte 57,145,164,143,57,147,162,157,165,160,0
L33:.byte 12,45,163,72,12,0
L36:.byte 164,157,164,141,154,40,45,144,12,0
L43:.byte 45,65,144,40,0
L45:.byte 45,62,144,40,0
L48:.byte 45,55,66,56,66,163,0
L50:.byte 45,55,66,144,0
L52:.byte 45,63,144,54,45,63,144,0
L54:.byte 45,67,163,0
L56:.byte 40,45,55,67,56,67,163,40,45,55,64,56,64,163,40,0
L58:.byte 40,45,55,61,62,56,61,62,163,40,0
L61:.byte 45,64,144,40,0
L63:.byte 45,163,12,0
L65:.byte 45,56,61,64,163,12,0
L96:.byte 45,163,40,165,156,162,145,141,144,141,142,154,145,12,0
L110:.byte 45,163,40,156,157,164,40,146,157,165,156,144,12,0
