// Test various -*- C++ -*- things.

// ====================== simple class structures  =======================

struct default_public_struct {
 // defaults to public:
  int a;
  int b;
};

struct explicit_public_struct {
 public:
  int a;
  int b;
};

struct protected_struct {
 protected:
  int a;
  int b;
};

struct private_struct {
 private:
  int a;
  int b;
};

struct mixed_protection_struct {
 public:
  int a;
  int b;
 private:
  int c;
  int d;
 protected:
  int e;
  int f;
 public:
  int g;
 private:
  int h;
 protected:
  int i;
};

class public_class {
 public:
  int a;
  int b;
};

class protected_class {
 protected:
  int a;
  int b;
};

class default_private_class {
 // defaults to private:
  int a;
  int b;
};

class explicit_private_class {
 private:
  int a;
  int b;
};

class mixed_protection_class {
 public:
  int a;
  int b;
 private:
  int c;
  int d;
 protected:
  int e;
  int f;
 public:
  int g;
 private:
  int h;
 protected:
  int i;
};

// ========================= simple inheritance ==========================

class A {
 public:
  int a;
  int x;
};

A g_A;

class B : public A {
 public:
  int b;
  int x;
};

B g_B;

class C : public A {
 public:
  int c;
  int x;
};

C g_C;

class D : public B, public C {
 public:
  int d;
  int x;
};

D g_D;

class E : public D {
 public:
  int e;
  int x;
};

E g_E;

void inheritance2 (void)
{
}

void inheritance1 (void)
{
  int ival;
  int *intp;

  // {A::a, A::x}

  g_A.A::a = 1;
  g_A.A::x = 2;

  // {{A::a,A::x},B::b,B::x}

  g_B.A::a = 3;
  g_B.A::x = 4;
  g_B.B::b = 5;
  g_B.B::x = 6;

  // {{A::a,A::x},C::c,C::x}

  g_C.A::a = 7;
  g_C.A::x = 8;
  g_C.C::c = 9;
  g_C.C::x = 10;

  // {{{A::a,A::x},B::b,B::x},{{A::a,A::x},C::c,C::x},D::d,D::x}

  // The following initialization code is non-portable, but allows us
  // to initialize all members of g_D until we can fill in the missing
  // initialization code with legal C++ code.

  for (intp = (int *) &g_D, ival = 11;
       intp < ((int *) &g_D + sizeof (g_D) / sizeof (int));
       intp++, ival++)
    {
      *intp = ival;
    }

  // Overlay the nonportable initialization with legal initialization.

  // ????? = 11;  (g_D.A::a = 11; is ambiguous)
  // ????? = 12;  (g_D.A::x = 12; is ambiguous)
  g_D.B::b = 13;
  g_D.B::x = 14;
  // ????? = 15;
  // ????? = 16;
  g_D.C::c = 17;
  g_D.C::x = 18;
  g_D.D::d = 19;
  g_D.D::x = 20;


  // {{{{A::a,A::x},B::b,B::x},{{A::a,A::x},C::c,C::x},D::d,D::x}},E::e,E::x}

  // The following initialization code is non-portable, but allows us
  // to initialize all members of g_D until we can fill in the missing
  // initialization code with legal C++ code.

  for (intp = (int *) &g_E, ival = 21;
       intp < ((int *) &g_E + sizeof (g_E) / sizeof (int));
       intp++, ival++)
  {
    *intp = ival;
  }

  // Overlay the nonportable initialization with legal initialization.

  // ????? = 21;  (g_E.A::a = 21; is ambiguous)
  // ????? = 22;  (g_E.A::x = 22; is ambiguous)
  g_E.B::b = 23;
  g_E.B::x = 24;
  // ????? = 25;
  // ????? = 26;
  g_E.C::c = 27;
  g_E.C::x = 28;
  g_E.D::d = 29;
  g_E.D::x = 30;
  g_E.E::e = 31;
  g_E.E::x = 32;

  inheritance2 ();	
}

// ======================== virtual base classes=========================

class vA {
 public:
  int va;
  int vx;
};

vA g_vA;

class vB : public virtual vA {
 public:
  int vb;
  int vx;
};

vB g_vB;

class vC : public virtual vA {
 public:
  int vc;
  int vx;
};

vC g_vC;

class vD : public virtual vB, public virtual vC {
 public:
  int vd;
  int vx;
};

vD g_vD;

class vE : public virtual vD {
 public:
  int ve;
  int vx;
};

vE g_vE;

void inheritance4 (void)
{
}

void inheritance3 (void)
{
  int ival;
  int *intp;

  // {vA::va, vA::vx}

  g_vA.vA::va = 1;
  g_vA.vA::vx = 2;

  // {{vA::va, vA::vx}, vB::vb, vB::vx}

  g_vB.vA::va = 3;
  g_vB.vA::vx = 4;
  g_vB.vB::vb = 5;
  g_vB.vB::vx = 6;

  // {{vA::va, vA::vx}, vC::vc, vC::vx}

  g_vC.vA::va = 7;
  g_vC.vA::vx = 8;
  g_vC.vC::vc = 9;
  g_vC.vC::vx = 10;

  // {{{{vA::va, vA::vx}, vB::vb, vB::vx}, vC::vc, vC::vx}, vD::vd,vD::vx}

  g_vD.vA::va = 11;
  g_vD.vA::vx = 12;
  g_vD.vB::vb = 13;
  g_vD.vB::vx = 14;
  g_vD.vC::vc = 15;
  g_vD.vC::vx = 16;
  g_vD.vD::vd = 17;
  g_vD.vD::vx = 18;


  // {{{{{vA::va,vA::vx},vB::vb,vB::vx},vC::vc,vC::vx},vD::vd,vD::vx},vE::ve,vE::vx}

  g_vD.vA::va = 19;
  g_vD.vA::vx = 20;
  g_vD.vB::vb = 21;
  g_vD.vB::vx = 22;
  g_vD.vC::vc = 23;
  g_vD.vC::vx = 24;
  g_vD.vD::vd = 25;
  g_vD.vD::vx = 26;
  g_vE.vE::ve = 27;
  g_vE.vE::vx = 28;

  inheritance4 ();	
}

// ======================================================================

class Base1 {
 public:
  int x;
  Base1(int i) { x = i; }
};

class Foo
{
 public:
  int x;
  int y;
  static int st;
  Foo (int i, int j) { x = i; y = j; }
  int operator! ();
  operator int ();
  int times (int y);
};

class Bar : public Base1, public Foo {
 public:
  int z;
  Bar (int i, int j, int k) : Base1 (10*k), Foo (i, j) { z = k; }
};

int Foo::operator! () { return !x; }

int Foo::times (int y) { return x * y; }

int Foo::st = 100;

int Foo::operator int() { return x; }

Foo foo(10, 11);
Bar bar(20, 21, 22);

typedef struct {
  int one;
  int two;
} tagless_struct;
tagless_struct v_tagless;

/* Try to get the compiler to allocate a class in a register.  */
class small {
 public:
  int x;
  int method ();
};
small::method ()
{
  return x + 5;
}
void marker_reg1 () {}

int
register_class ()
{
  /* We don't call any methods for v, so gcc version cygnus-2.3.3-930220
     might put this variable in a register.  This is a lose, though, because
     it means that GDB can't call any methods for that variable.  */
  register small v;

  v.x = 77;
  marker_reg1 ();
  return v.x + 5;
}

int
main()
{
  inheritance1 ();
  inheritance3 ();
  register_class ();
  int Foo::* pmi = &Foo::y;

  /* Make sure the AIX linker doesn't remove the variable.  */
  v_tagless.one = 5;

  return (int)foo.*pmi;
}
