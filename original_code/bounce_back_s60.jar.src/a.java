import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

public class a {
  private CrystalMidlet o;
  
  private h E;
  
  private g v;
  
  public int D;
  
  public int i;
  
  public int n;
  
  public int H;
  
  public int s;
  
  public int h;
  
  public int G;
  
  public int z;
  
  public int d;
  
  public int u;
  
  public int J;
  
  public int c;
  
  private int m;
  
  private int k;
  
  public int A;
  
  public int g;
  
  public Image[] w;
  
  private boolean[][][] f;
  
  public boolean x;
  
  public boolean t;
  
  public boolean I;
  
  public boolean F;
  
  public boolean e;
  
  public boolean p;
  
  public boolean j;
  
  public boolean C;
  
  public boolean l;
  
  public boolean q;
  
  public int B;
  
  public int b;
  
  public byte a;
  
  public byte r;
  
  public static Image[] y;
  
  public a(int paramInt1, int paramInt2, int paramInt3, h paramh, CrystalMidlet paramCrystalMidlet) {
    this.E = paramh;
    this.o = paramCrystalMidlet;
    c c = new c("/res/b");
    byte[] arrayOfByte = c.a();
    byte b = 0;
    ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(arrayOfByte);
    DataInputStream dataInputStream = new DataInputStream(byteArrayInputStream);
    try {
      b = dataInputStream.readByte();
      this.f = new boolean[b][][];
      for (byte b1 = 0; b1 < b; b1++) {
        byte b2 = dataInputStream.readByte();
        byte b3 = dataInputStream.readByte();
        this.f[b1] = new boolean[b2][b3];
        for (byte b4 = 0; b4 < b3; b4++) {
          for (byte b5 = 0; b5 < b2; b5++)
            this.f[b1][b5][b4] = dataInputStream.readBoolean(); 
        } 
      } 
    } catch (IOException iOException) {}
    if (y != null) {
      this.w = y;
    } else {
      this.w = new Image[25];
      for (byte b1 = 0; b1 < 25; b1++) {
        arrayOfByte = c.a();
        this.w[b1] = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
      } 
      y = this.w;
    } 
    a(paramInt1, paramInt2, paramInt3);
  }
  
  public void a(int paramInt1, int paramInt2, int paramInt3) {
    i();
    this.v = this.E.Z;
    this.D = paramInt1;
    this.i = paramInt2;
    this.g = paramInt3;
    this.E.ae = 0;
    this.E.M = true;
    this.h = 0;
    this.s = 0;
    this.G = 0;
    this.z = 0;
    this.b = 0;
    this.B = 0;
    this.H = this.D / 16;
    this.n = this.i / 16;
    this.t = (paramInt3 != 0);
    this.x = false;
    this.e = false;
    this.F = false;
    this.I = false;
    this.p = false;
    this.q = false;
    this.j = false;
    this.C = false;
    this.l = false;
    this.a = 0;
    this.r = 0;
    this.k = -1;
    this.A = 0;
    this.d = this.w[this.g].getWidth();
    this.u = this.w[this.g].getHeight();
    this.J = this.d >> 1;
    this.c = this.u >> 1;
  }
  
  public void e() {
    i();
    this.D = this.H * 16 + 8;
    this.i = this.n * 16 + 8;
    this.o.h--;
    this.E.ae = 0;
    this.E.M = true;
    this.h = 0;
    this.s = 0;
    this.G = 0;
    this.z = 0;
    this.b = 0;
    this.B = 0;
    this.x = false;
    this.e = false;
    this.F = false;
    this.I = false;
    this.p = false;
    this.q = false;
    this.j = false;
    this.C = false;
    this.l = false;
    this.a = 0;
    this.r = 0;
    this.k = -1;
    this.A = 0;
    this.g = 0;
    this.d = this.w[this.g].getWidth();
    this.u = this.w[this.g].getHeight();
    this.J = this.d >> 1;
    this.c = this.u >> 1;
    if (this.t) {
      a(11);
      this.I = true;
    } 
  }
  
  public void k() {
    this.e = true;
    this.A = 25;
    this.o.b(0);
    this.o.c(0);
  }
  
  public void a(int paramInt1, int paramInt2) {
    this.v.R[paramInt1][paramInt2] = 105;
    for (byte b = 0; b < this.E.B.length; b++) {
      if (this.E.B[b] == 0) {
        this.E.B[b] = 15;
        this.E.v[b] = paramInt2;
        this.E.J[b] = paramInt1;
        break;
      } 
    } 
  }
  
  public void c(int paramInt) {
    if (paramInt == 8 || paramInt == 4 || paramInt == 2 || paramInt == 1)
      this.m |= paramInt; 
  }
  
  public void b(int paramInt) {
    if (paramInt == 8 || paramInt == 4 || paramInt == 2 || paramInt == 1)
      this.m &= paramInt ^ 0xFFFFFFFF; 
  }
  
  public void i() {
    this.m &= 0xFFFFFFF0;
  }
  
  private boolean a(int paramInt1, int paramInt2, int paramInt3, int paramInt4, boolean paramBoolean) {
    int i = paramInt1;
    int j = paramInt2;
    int k = paramInt1 + paramInt3;
    int m = paramInt2 + paramInt4;
    if (paramBoolean)
      this.k = -1; 
    for (byte b = 0; b < this.E.j; b++) {
      int n = this.E.a(b, false);
      int i1 = this.E.b(b, false);
      int i2 = this.E.f[b][1] * 16 + this.E.ag[b][0];
      int i3 = this.E.f[b][0] * 16 + this.E.ag[b][1];
      int i4 = i2 + n;
      int i5 = i3 + i1;
      if (this.E.a(i, j, k, m, i2, i3, i4, i5)) {
        if (paramBoolean)
          this.k = b; 
        return true;
      } 
    } 
    return false;
  }
  
  private boolean a(int paramInt1, int paramInt2, boolean paramBoolean) {
    int i = this.g;
    if ((i >= 0 && i <= 8) || (i >= 20 && i <= 22)) {
      i = 0;
    } else if (i >= 9 && i <= 19) {
      i = 1;
    } 
    int j = (this.f[i]).length;
    int k = (this.f[i][0]).length;
    int m = paramInt1 - (j >> 1);
    int n = paramInt2 - (k >> 1);
    boolean bool = this.v.a(m, n, j, k, this.f[i], paramBoolean);
    return (a(m, n, j, k, paramBoolean) || bool);
  }
  
  public void a(int paramInt) {
    this.g = paramInt;
    int i = this.d;
    int j = this.u;
    this.d = this.w[this.g].getWidth();
    this.u = this.w[this.g].getHeight();
    this.J = this.d >> 1;
    this.c = this.u >> 1;
    if (a(this.D, this.i, false)) {
      int k = Math.abs(this.d - i) + 1;
      int m = Math.abs(this.u - j) + 1;
      for (byte b1 = 1; b1 <= k; b1++) {
        if (!a(this.D + b1, this.i, false)) {
          this.D += b1;
          return;
        } 
        if (!a(this.D - b1, this.i, false)) {
          this.D -= b1;
          return;
        } 
      } 
      for (byte b2 = 1; b2 <= m; b2++) {
        if (!a(this.D, this.i + b2, false)) {
          this.i += b2;
          return;
        } 
        if (!a(this.D, this.i - b2, false)) {
          this.i -= b2;
          return;
        } 
      } 
      for (byte b3 = 1; b3 <= m; b3++) {
        for (byte b = 1; b <= k; b++) {
          if (!a(this.D + b, this.i + b3, false)) {
            this.i += b3;
            this.D += b;
            return;
          } 
          if (!a(this.D + b, this.i - b3, false)) {
            this.i -= b3;
            this.D += b;
            return;
          } 
          if (!a(this.D - b, this.i + b3, false)) {
            this.i += b3;
            this.D -= b;
            return;
          } 
          if (!a(this.D - b, this.i - b3, false)) {
            this.i -= b3;
            this.D -= b;
            return;
          } 
        } 
      } 
    } 
  }
  
  private void b(int paramInt1, int paramInt2) {
    byte b2;
    byte b1 = this.v.R[paramInt1][paramInt2];
    int i = b1 & Byte.MAX_VALUE;
    int j = b1 & 0x80;
    switch (i) {
      case 34:
        this.o.n += 200;
        this.E.C += 200;
        b2 = this.v.R[this.n][this.H];
        if ((b2 & Byte.MAX_VALUE) == 35)
          this.v.R[this.n][this.H] = (byte)(0x0 | b2 & 0x80); 
        this.n = paramInt1;
        this.H = paramInt2;
        this.t = this.I;
        this.o.b(2);
        this.v.R[paramInt1][paramInt2] = (byte)(0x23 | j);
        break;
      case 12:
        this.o.n += 1000;
        this.E.C += 1000;
        if (this.o.h < 5) {
          this.o.h++;
          this.E.M = true;
        } 
        this.o.b(2);
        this.v.R[paramInt1][paramInt2] = (byte)(0x0 | j);
        break;
      case 30:
        this.o.n += 2500;
        this.E.C += 2500;
        this.o.b(2);
        this.v.R[paramInt1][paramInt2] = (byte)(0x0 | j);
        break;
      case 93:
      case 94:
      case 97:
      case 98:
      case 101:
      case 102:
        this.o.n += 500;
        this.E.W--;
        this.E.M = true;
        this.o.b(1);
        break;
    } 
  }
  
  private void g() {
    if (this.r == 1)
      if (this.A == 2) {
        a(20);
      } else if (this.A == 1) {
        a(21);
      } else if (this.A == 0) {
        a(22);
        this.F = true;
        this.b = 550;
        this.r = 0;
      }  
    if (this.r == 2)
      if (this.A == 2) {
        a(21);
      } else if (this.A == 1) {
        a(20);
      } else if (this.A == 0) {
        a(0);
        this.F = false;
        this.q = false;
        this.r = 0;
      }  
    if (this.r == 3)
      if (this.A == 2) {
        a(10);
      } else if (this.A == 1) {
        a(9);
      } else if (this.A == 0) {
        a(0);
        this.I = false;
        this.r = 1;
        this.A = 3;
      }  
  }
  
  private void a() {
    if (this.a == 1)
      if (this.A == 2) {
        a(9);
      } else if (this.A == 1) {
        a(10);
      } else if (this.A == 0) {
        a(11);
        this.I = true;
        this.q = false;
        this.a = 0;
      }  
    if (this.a == 2)
      if (this.A == 2) {
        a(10);
      } else if (this.A == 1) {
        a(9);
      } else if (this.A == 0) {
        a(0);
        this.I = false;
        this.q = false;
        this.a = 0;
      }  
  }
  
  private void j() {
    if (this.A == 2) {
      a(this.g + 1);
    } else if (this.A == 1) {
      a(this.g - 1);
    } else if (this.A == 0) {
      if (this.I) {
        a(11);
      } else {
        a(0);
      } 
      this.q = false;
    } 
  }
  
  public void l() {
    if (!this.q) {
      if (this.I) {
        this.a = 2;
      } else {
        this.a = 1;
      } 
      this.q = true;
      this.A = 3;
    } 
  }
  
  public void b() {
    if (!this.q) {
      if (this.I) {
        this.r = 3;
      } else {
        this.r = 1;
      } 
      this.q = true;
      this.A = 3;
    } 
  }
  
  public void c() {
    if (!this.F && this.a == 0 && this.r == 0) {
      this.A = 0;
      this.q = false;
      if (this.I) {
        a(11);
      } else {
        a(0);
      } 
    } 
  }
  
  public void c(int paramInt1, int paramInt2) {
    byte b = this.v.R[paramInt1][paramInt2];
    int i = b & Byte.MAX_VALUE;
    int j = b & 0x80;
    switch (i) {
      case 39:
        if (!this.j)
          this.o.b(6); 
        this.p = false;
        this.C = false;
        this.j = true;
        this.B = 450;
        break;
      case 26:
        if (!this.C)
          this.o.b(6); 
        this.p = false;
        this.C = true;
        this.j = false;
        this.B = 450;
        break;
      case 15:
        if (!this.p)
          this.o.b(6); 
        this.p = true;
        this.C = false;
        this.j = false;
        this.x = false;
        this.B = 450;
        break;
      case 22:
        if (!this.F) {
          this.o.b(5);
        } else {
          this.b = 550;
        } 
        c();
        b();
        break;
      case 18:
        if (!this.I && !this.F && this.a == 0) {
          c();
          l();
          this.o.b(3);
        } 
        break;
      case 11:
        if (this.I && !this.F && this.a == 0) {
          c();
          l();
          this.o.b(4);
        } 
        break;
    } 
  }
  
  public void d(int paramInt) {
    a(paramInt);
    this.A = 3;
    this.q = true;
  }
  
  private int h() {
    int i = -125;
    if (this.I) {
      i = -180;
    } else if (this.F) {
      i = -95;
    } 
    if (this.C)
      i += i >> 2; 
    if (this.l)
      i -= i >> 2; 
    return i;
  }
  
  private int f() {
    int i = 0;
    if (this.I) {
      i = -120;
    } else if (this.F) {
      i = -63;
    } else {
      i = -83;
    } 
    if (this.C)
      i += i >> 2; 
    return i;
  }
  
  private int a(int paramInt, boolean paramBoolean) {
    if (paramBoolean) {
      paramInt = h();
      this.x = false;
      this.G = 0;
    } else if (paramInt > 30) {
      if (this.G == 0) {
        this.G = -paramInt;
        int i = f();
        if (this.G < i)
          this.G = i; 
      } 
      paramInt = this.G;
      this.G >>= 1;
      if (this.G > -10) {
        this.x = true;
        paramInt = 30;
        this.G = 0;
      } 
    } else if (paramInt > -30) {
      this.x = true;
      paramInt = 30;
      this.G = 0;
    } 
    return paramInt;
  }
  
  public void d() {
    boolean[] arrayOfBoolean = null;
    if (this.E.s != null)
      arrayOfBoolean = new boolean[this.E.s.length]; 
    if (this.e) {
      this.A--;
      if (this.A == 0)
        if (this.o.h == 0) {
          this.E.H.f();
        } else {
          e();
        }  
      return;
    } 
    if (this.E.o && this.E.P < this.D && this.D < this.E.P + 32 && this.E.R < this.i && this.i < this.E.R + 48) {
      this.E.o = false;
      this.o.b(7);
      this.o.c(1);
      this.E.ap = true;
      return;
    } 
    int i = this.s;
    int j = this.h;
    int k = this.z;
    if (this.A != 0) {
      this.A--;
      if (this.r != 0)
        g(); 
      if (this.a != 0)
        a(); 
      if (this.q && !this.F)
        j(); 
    } 
    if (this.B != 0) {
      this.B--;
      int i4 = this.B / 18;
      if (this.E.ae != i4) {
        this.E.ae = i4;
        this.E.M = true;
      } 
      if (this.B == 0) {
        this.p = false;
        this.j = false;
        this.C = false;
      } 
    } 
    if (this.b != 0) {
      this.b--;
      int i4 = this.b / 18;
      if (this.E.ae != i4) {
        this.E.ae = i4;
        this.E.M = true;
      } 
      if (this.b == 0) {
        this.r = 2;
        this.A = 3;
      } 
    } 
    boolean bool1 = ((this.m & 0x8) != 0 && (this.m & 0x4) == 0) ? true : false;
    int m = this.D / 16;
    int n = this.i / 16;
    byte b = this.v.R[n][m];
    int i1 = b & Byte.MAX_VALUE;
    this.l = ((b & 0x80) != 0);
    if (bool1 && this.x) {
      j = h();
      this.x = false;
    } 
    if (!this.x || (this.l && this.I)) {
      byte b5 = 80;
      byte b6 = 9;
      if (this.l) {
        if (!this.F)
          b5 = 20; 
        if (this.I) {
          b6 = -6;
          this.x = false;
        } else {
          b6 = 7;
        } 
      } 
      j += b6;
      if (j > b5) {
        j = b5;
      } else if (this.l && this.I && -j > b5) {
        j = -b5;
      } 
    } 
    boolean bool2 = false;
    if (i1 == 45 || i1 == 51 || i1 == 53 || i1 == 53 || i1 == 67 || i1 == 71 || i1 == 75) {
      if (j >= -100)
        if (this.p) {
          j += (i1 == 75) ? 50 : 100;
        } else {
          j -= (i1 == 75) ? 50 : 100;
        }  
      this.x = false;
      bool2 = true;
    } 
    if (this.p)
      j = -j; 
    int i2 = Math.abs(j) / 10;
    byte b1 = (j == 0) ? 0 : ((j < 0) ? -1 : 1);
    boolean bool3 = ((b1 && !this.p) || (b1 && this.p)) ? true : false;
    if (i2 > 14)
      i2 = 14; 
    if (this.p)
      j = -j; 
    for (byte b2 = 0; b2 < i2; b2++) {
      boolean bool = a(this.D, this.i + b1, true);
      n = (this.i + b1) / 16;
      i1 = this.v.R[n][m] & Byte.MAX_VALUE;
      if (i1 == 101 || i1 == 102 || i1 == 103 || i1 == 104)
        for (byte b5 = 0; b5 < this.v.Y.length; b5++) {
          if (this.v.Y[b5] == -1) {
            this.v.Y[b5] = m;
            this.v.P[b5] = n;
            bool = true;
            break;
          } 
        }  
      if (!bool) {
        this.i += b1;
        this.x = false;
      } else {
        boolean bool4 = false;
        int i4 = this.v.Y.length;
        for (byte b5 = 0; b5 < i4; b5++) {
          int i9;
          int i10;
          byte b7;
          byte b8;
          byte b9;
          byte b6 = 0;
          int i5 = b6 & Byte.MAX_VALUE;
          int i6 = b6 & 0x80;
          int i7 = this.v.P[b5];
          int i8 = this.v.Y[b5];
          if (i7 != -1 && i8 != -1) {
            b6 = this.v.R[i7][i8];
            i5 = b6 & Byte.MAX_VALUE;
            i6 = b6 & 0x80;
          } else if (this.k != -1) {
            switch (this.E.ao[this.k]) {
              case 0:
                if (this.F) {
                  i5 = 200;
                  break;
                } 
                i5 = 201;
                break;
              case 1:
                if (this.F) {
                  i5 = 200;
                  break;
                } 
                i5 = 202;
                break;
              case 2:
                i5 = 200;
                break;
            } 
          } 
          switch (i5) {
            case 0:
              break;
            case 201:
            case 202:
              k();
              return;
            case 1:
            case 62:
            case 63:
            case 64:
            case 65:
            case 85:
            case 86:
            case 87:
            case 88:
            case 89:
            case 90:
            case 91:
            case 92:
              if (this.F) {
                if (bool3) {
                  this.x = true;
                  bool4 = true;
                  j = 30;
                  break;
                } 
                j = 30;
                break;
              } 
              k();
              return;
            case 94:
            case 96:
              if (!this.I && this.D - this.J == i8 * 16) {
                this.i += b1;
                if (i5 == 94 && this.i == i7 * 16 + 8) {
                  b(i7, i8);
                  this.v.R[i7][i8] = (byte)(0x60 | i6);
                } 
                break;
              } 
              if (!a(this.D + 1, this.i + b1, false)) {
                this.i += b1;
                this.D++;
                break;
              } 
              if (!a(this.D - 1, this.i + b1, false)) {
                this.i += b1;
                this.D--;
                break;
              } 
              if (bool3) {
                j = a(j, bool1);
                break;
              } 
              j = 0;
              break;
            case 101:
            case 102:
            case 103:
            case 104:
              if (!a(this.D, this.i + b1, false)) {
                this.i += b1;
                if ((i5 == 102 || i5 == 101) && this.i == i7 * 16 + 8) {
                  b(i7, i8);
                  if (i5 == 102) {
                    this.v.R[i7][i8 - 1] = (byte)(this.v.R[i7][i8 - 1] & 0x80);
                    this.v.R[i7][i8 - 1] = (byte)(this.v.R[i7][i8 - 1] | 0x67);
                    this.v.R[i7][i8] = (byte)(0x68 | i6);
                    break;
                  } 
                  if (i5 == 101) {
                    this.v.R[i7][i8] = (byte)(0x67 | i6);
                    this.v.R[i7][i8 + 1] = (byte)(this.v.R[i7][i8 + 1] & 0x80);
                    this.v.R[i7][i8 + 1] = (byte)(this.v.R[i7][i8 + 1] | 0x68);
                  } 
                } 
                break;
              } 
              if (!a(this.D + 1, this.i + b1, false)) {
                this.i += b1;
                this.D++;
                if ((i5 == 102 || i5 == 101) && this.i == i7 * 16 + 8) {
                  b(i7, i8);
                  if (i5 == 102) {
                    this.v.R[i7][i8 - 1] = (byte)(this.v.R[i7][i8 - 1] & 0x80);
                    this.v.R[i7][i8 - 1] = (byte)(this.v.R[i7][i8 - 1] | 0x67);
                    this.v.R[i7][i8] = (byte)(0x68 | i6);
                    break;
                  } 
                  if (i5 == 101) {
                    this.v.R[i7][i8] = (byte)(0x67 | i6);
                    this.v.R[i7][i8 + 1] = (byte)(this.v.R[i7][i8 + 1] & 0x80);
                    this.v.R[i7][i8 + 1] = (byte)(this.v.R[i7][i8 + 1] | 0x68);
                  } 
                } 
                break;
              } 
              if (!a(this.D - 1, this.i + b1, false)) {
                this.i += b1;
                this.D--;
                if ((i5 == 102 || i5 == 101) && this.i == i7 * 16 + 8) {
                  b(i7, i8);
                  if (i5 == 102) {
                    this.v.R[i7][i8 - 1] = (byte)(this.v.R[i7][i8 - 1] & 0x80);
                    this.v.R[i7][i8 - 1] = (byte)(this.v.R[i7][i8 - 1] | 0x67);
                    this.v.R[i7][i8] = (byte)(0x68 | i6);
                    break;
                  } 
                  if (i5 == 101) {
                    this.v.R[i7][i8] = (byte)(0x67 | i6);
                    this.v.R[i7][i8 + 1] = (byte)(this.v.R[i7][i8 + 1] & 0x80);
                    this.v.R[i7][i8 + 1] = (byte)(this.v.R[i7][i8 + 1] | 0x68);
                  } 
                } 
                break;
              } 
              if (bool3) {
                j = a(j, bool1);
                break;
              } 
              j = 0;
              break;
            case 12:
            case 30:
            case 34:
              b(this.v.P[b5], this.v.Y[b5]);
              break;
            case 11:
            case 15:
            case 18:
            case 22:
            case 26:
            case 39:
              c(this.v.P[b5], this.v.Y[b5]);
            default:
              if (bool3) {
                int i12;
                byte b10;
                byte b11;
                byte b12;
                if (!this.p) {
                  if (i5 == 6 || i5 == 5)
                    i5 = 2; 
                  if (i5 == 116 || i5 == 115)
                    i5 = 110; 
                } else {
                  if (i5 == 3 || i5 == 4)
                    i5 = 2; 
                  if (i5 == 113 || i5 == 114)
                    i5 = 110; 
                } 
                switch (i5) {
                  case 3:
                  case 6:
                  case 52:
                  case 113:
                  case 116:
                    if (i5 != 6 && i5 != 3)
                      k = 0; 
                    if (j >= 80 && !this.q) {
                      if (this.p) {
                        d(this.g + 7);
                        break;
                      } 
                      d(this.g + 5);
                      break;
                    } 
                    if (j > 40 && !this.x) {
                      if (this.G == 0) {
                        this.G = -(j >> 1);
                        int i13 = f() >> 1;
                        if (this.G < i13)
                          this.G = i13; 
                      } 
                      j = this.G;
                      if (i <= 0)
                        i = this.G; 
                      this.G = 3 * this.G >> 2;
                      if (this.G > -10) {
                        this.x = true;
                        j = 30;
                        this.G = 0;
                      } 
                      break;
                    } 
                    if (i == 0) {
                      if ((!this.l || !this.I) && !a(this.D - 1, this.i + b1, false)) {
                        this.i += b1;
                        this.D--;
                        this.x = true;
                        j = 30;
                        this.G = 0;
                        bool4 = true;
                      } 
                      break;
                    } 
                    if (j > -30) {
                      this.x = true;
                      j = 30;
                      this.G = 0;
                    } 
                    break;
                  case 4:
                  case 5:
                  case 114:
                  case 115:
                    if (i5 != 5 && i5 != 4)
                      k = 0; 
                    if (j >= 80 && !this.q) {
                      if (this.p) {
                        d(this.g + 5);
                        break;
                      } 
                      d(this.g + 7);
                      break;
                    } 
                    if (j > 30 && !this.x) {
                      if (this.G == 0) {
                        this.G = -(j >> 1);
                        int i13 = f() >> 1;
                        if (this.G < i13)
                          this.G = i13; 
                      } 
                      j = this.G;
                      if (i <= 0)
                        i = -this.G; 
                      this.G = 3 * this.G >> 2;
                      if (this.G > -10) {
                        this.x = true;
                        j = 30;
                        this.G = 0;
                      } 
                      break;
                    } 
                    if (i == 0) {
                      if ((!this.l || !this.I) && !a(this.D + 1, this.i + b1, false)) {
                        this.i += b1;
                        this.D++;
                        this.x = true;
                        j = 30;
                        this.G = 0;
                        bool4 = true;
                      } 
                      break;
                    } 
                    if (j > -30) {
                      this.x = true;
                      j = 30;
                      this.G = 0;
                    } 
                    break;
                  case 2:
                    i11 = 0;
                    if (!a(this.D + 1, this.i + b1, false)) {
                      this.i += b1;
                      this.D++;
                      i11 = 1;
                    } else if (!a(this.D - 1, this.i + b1, false)) {
                      this.i += b1;
                      this.D--;
                      i11 = 1;
                    } 
                    if (i11)
                      break; 
                    if (j > 40 && !this.q) {
                      d(this.g + 1);
                      k = this.z;
                      break;
                    } 
                    if (bool1) {
                      this.G = 0;
                      k = this.z;
                      if (k == 0)
                        k = -30; 
                      j = h() + k;
                      this.x = false;
                      k -= 30;
                      if (k < -83)
                        k = -83; 
                      break;
                    } 
                    if (j > 30 && !this.x) {
                      if (this.G == 0) {
                        this.G = -j;
                        int i13 = f();
                        if (this.G < i13)
                          this.G = i13; 
                      } 
                      j = this.G;
                      this.G = 3 * this.G >> 2;
                      if (this.G > -10) {
                        this.x = true;
                        j = 30;
                        this.G = 0;
                      } 
                      break;
                    } 
                    if (j > -30) {
                      this.x = true;
                      j = 30;
                      this.G = 0;
                    } 
                    break;
                  case 200:
                    i11 = this.E.f[this.k][0] * 16 + this.E.ag[this.k][1];
                    i12 = this.E.f[this.k][1] * 16 + this.E.ag[this.k][0];
                    if (this.E.ao[this.k] == 2) {
                      b10 = 11;
                      b7 = 24;
                    } else if (this.E.ao[this.k] == 0) {
                      b10 = 32;
                      b7 = 32;
                    } else {
                      b10 = 16;
                      b7 = 16;
                    } 
                    b11 = this.E.s[this.k][0];
                    b12 = this.E.s[this.k][1];
                    if (b12 != 0) {
                      if ((!this.p && this.i - this.c >= i11) || (this.p && this.i + this.c <= i11 + b10)) {
                        int i13 = i11 + b10 + this.c;
                        if (this.p)
                          i13 = i11 - this.c; 
                        int i14 = this.k;
                        if (!a(this.D, i13, false)) {
                          this.i = i13;
                          j = 30;
                          bool4 = true;
                          break;
                        } 
                        if (this.F) {
                          if (!arrayOfBoolean[i14])
                            if (this.E.s[i14][1] != 0) {
                              this.E.s[i14][1] = (byte)-this.E.s[i14][1];
                            } else {
                              this.E.s[i14][0] = (byte)-this.E.s[i14][0];
                            }  
                          arrayOfBoolean[i14] = true;
                          break;
                        } 
                        k();
                        return;
                      } 
                      if (bool1) {
                        j = h();
                        this.x = false;
                        if (!this.q)
                          d(this.g + 1); 
                        break;
                      } 
                      if (this.x) {
                        int i13 = i11 - this.c;
                        if (this.p)
                          i13 = i11 + b10 + this.c; 
                        int i14 = this.k;
                        if (!a(this.D, i13, true)) {
                          this.i = i13;
                          i2 = 0;
                          j = 30;
                          bool4 = true;
                          break;
                        } 
                        if (this.F) {
                          if (!arrayOfBoolean[i14]) {
                            if (this.E.s[i14][1] != 0) {
                              this.E.s[i14][1] = (byte)-this.E.s[i14][1];
                            } else {
                              this.E.s[i14][0] = (byte)-this.E.s[i14][0];
                            } 
                            arrayOfBoolean[i14] = true;
                          } 
                          break;
                        } 
                        if (this.k == -1) {
                          k();
                          return;
                        } 
                      } 
                      break;
                    } 
                    if (b11 != 0) {
                      int i13 = this.D;
                      int i14 = this.i;
                      if (this.D > i12 + b7) {
                        i13 = i12 + b7 + this.J;
                      } else if (this.D < i12) {
                        i13 = i12 - this.J;
                      } 
                      if ((!this.p && this.i < i11) || (this.p && this.i > i11 + b10)) {
                        i13 = this.D;
                        i14 = i11 - this.c;
                        if (this.p)
                          i14 = i11 + b10 + this.c; 
                      } 
                      int i15 = this.k;
                      if (!a(i13, i14, true)) {
                        this.D = i13;
                        this.i = i14;
                        bool4 = true;
                        break;
                      } 
                      if (this.F) {
                        if (!arrayOfBoolean[i15]) {
                          this.E.s[i15][0] = (byte)-this.E.s[i15][0];
                          arrayOfBoolean[i15] = true;
                        } 
                        break;
                      } 
                      if (this.k == -1) {
                        k();
                        return;
                      } 
                    } 
                    break;
                } 
                k = 0;
                int i11 = 0;
                if (!a(this.D + 1, this.i + b1, false)) {
                  this.i += b1;
                  this.D++;
                  i11 = 1;
                } else if (!a(this.D - 1, this.i + b1, false)) {
                  this.i += b1;
                  this.D--;
                  i11 = 1;
                } 
                if ((i5 == 7 || i5 == 8) && this.F)
                  a(i7, i8); 
                if (i11 != 0)
                  break; 
                if (j > 40 && !this.q)
                  d(this.g + 1); 
                if (!this.l || !this.I)
                  j = a(j, bool1); 
                if (i5 == 84 || i5 == 108 || i5 == 83) {
                  if (!a(this.D + 1, this.i, true)) {
                    this.D++;
                    break;
                  } 
                  if (!this.F) {
                    i7 = this.v.P[0];
                    i8 = this.v.Y[0];
                    if (i7 != -1 && i8 != -1) {
                      b6 = this.v.R[i7][i8];
                      i5 = b6 & Byte.MAX_VALUE;
                    } 
                    if (i5 == 85 || i5 == 86 || i5 == 87 || i5 == 88 || i5 == 89 || i5 == 63 || i5 == 64 || i5 == 92)
                      k(); 
                    b5 = 0;
                    i4 = this.v.Y.length;
                    bool4 = false;
                  } 
                  break;
                } 
                if (i5 == 79 || i5 == 109 || i5 == 82) {
                  if (!a(this.D - 1, this.i, true)) {
                    this.D--;
                    break;
                  } 
                  if (!this.F) {
                    i7 = this.v.P[0];
                    i8 = this.v.Y[0];
                    if (i7 != -1 && i8 != -1) {
                      b6 = this.v.R[i7][i8];
                      i5 = b6 & Byte.MAX_VALUE;
                    } 
                    if (i5 == 85 || i5 == 86 || i5 == 87 || i5 == 88 || i5 == 89 || i5 == 63 || i5 == 64 || i5 == 92)
                      k(); 
                    b5 = 0;
                    i4 = this.v.Y.length;
                    bool4 = false;
                  } 
                } 
                break;
              } 
              if (!this.p) {
                if (i5 == 113 || i5 == 114)
                  i5 = 110; 
                if (i5 == 3 || i5 == 4)
                  i5 = 2; 
              } else {
                if (i5 == 116 || i5 == 115)
                  i5 = 110; 
                if (i5 == 6 || i5 == 5)
                  i5 = 2; 
              } 
              switch (i5) {
                case 3:
                case 6:
                case 113:
                case 116:
                  if (bool2) {
                    if (!a(this.D - 1, this.i + b1, false)) {
                      this.i += b1;
                      this.D--;
                    } 
                  } else if (j <= -40 && !this.q) {
                    if (this.p) {
                      d(this.g + 5);
                    } else {
                      d(this.g + 7);
                    } 
                  } else {
                    if (j != 0)
                      i = j >> 1; 
                    j = 0;
                  } 
                  bool4 = true;
                  break;
                case 4:
                case 5:
                case 114:
                case 115:
                  if (bool2) {
                    if (!a(this.D + 1, this.i + b1, false)) {
                      this.i += b1;
                      this.D++;
                    } 
                  } else if (j <= -40 && !this.q) {
                    if (this.p) {
                      d(this.g + 7);
                    } else {
                      d(this.g + 5);
                    } 
                  } else {
                    if (j != 0)
                      i = -(j >> 1); 
                    j = 0;
                  } 
                  bool4 = true;
                  break;
                case 200:
                  i9 = this.E.f[this.k][0] * 16 + this.E.ag[this.k][1];
                  i10 = this.E.f[this.k][1] * 16 + this.E.ag[this.k][0];
                  if (this.E.ao[this.k] == 2) {
                    byte b10 = 11;
                    b7 = 24;
                  } else if (this.E.ao[this.k] == 0) {
                    byte b10 = 32;
                    b7 = 32;
                  } else {
                    byte b10 = 16;
                    b7 = 16;
                  } 
                  b8 = this.E.s[this.k][0];
                  b9 = this.E.s[this.k][1];
                  if (b9 != 0) {
                    if ((!this.p && this.i - this.c <= i9) || (this.p && this.i + this.c >= i9 + 11)) {
                      if (b5 != 0 || this.v.Y[b5 + 1] != -1) {
                        int i13 = this.k;
                        if (this.F) {
                          if (!arrayOfBoolean[i13])
                            if (this.E.s[i13][1] != 0) {
                              this.E.s[i13][1] = (byte)-this.E.s[i13][1];
                            } else {
                              this.E.s[i13][0] = (byte)-this.E.s[i13][0];
                            }  
                          arrayOfBoolean[i13] = true;
                          break;
                        } 
                        k();
                        return;
                      } 
                      break;
                    } 
                    int i11 = i9 + 11 + this.c + 2;
                    if (this.p)
                      i11 = i9 - this.c; 
                    int i12 = this.k;
                    if (!a(this.D, i11, false)) {
                      this.i = i11;
                      i2 = 0;
                      j = 30;
                      bool4 = true;
                      break;
                    } 
                    if (this.F) {
                      if (!arrayOfBoolean[i12]) {
                        if (this.E.s[i12][1] != 0) {
                          this.E.s[i12][1] = (byte)-this.E.s[i12][1];
                        } else {
                          this.E.s[i12][0] = (byte)-this.E.s[i12][0];
                        } 
                        arrayOfBoolean[i12] = true;
                      } 
                      break;
                    } 
                    k();
                    return;
                  } 
                  if (b8 != 0) {
                    int i11 = this.D;
                    int i12 = this.i;
                    boolean bool5 = false;
                    if (this.D > i10 + b7) {
                      i11 = i10 + b7 + this.J;
                    } else if (this.D < i10) {
                      i11 = i10 - this.J;
                    } else {
                      bool5 = true;
                    } 
                    int i13 = this.k;
                    if (!a(i11, i12, true)) {
                      this.D = i11;
                      this.i = i12;
                      if (bool5) {
                        i2 = 0;
                        j = 30;
                      } 
                      bool4 = true;
                      break;
                    } 
                    if (this.F) {
                      if (!arrayOfBoolean[i13]) {
                        this.E.s[i13][0] = (byte)-this.E.s[i13][0];
                        arrayOfBoolean[i13] = true;
                      } 
                      break;
                    } 
                    if (this.k == -1) {
                      k();
                      return;
                    } 
                  } 
                  break;
              } 
              j = 0;
              if ((i5 == 7 || i5 == 8) && this.F)
                a(i7, i8); 
              break;
          } 
          if (bool4)
            break; 
        } 
      } 
    } 
    this.z = k;
    this.h = j;
    if ((this.m & 0x2) != 0 && (this.m & 0x1) == 0) {
      i += this.j ? 22 : 18;
      if (this.j && i > 100) {
        i = 100;
      } else if (!this.j && i > 60) {
        i = 60;
      } 
    } else if ((this.m & 0x2) == 0 && (this.m & 0x1) != 0) {
      i -= this.j ? 22 : 18;
      if (this.j && i < -100) {
        i = -100;
      } else if (!this.j && i < -60) {
        i = -60;
      } 
    } 
    if (i1 == 44 || i1 == 50 || i1 == 76) {
      if (i > 0) {
        i -= this.x ? 8 : 3;
      } else if (i < 0) {
        i += this.x ? 8 : 3;
      } 
      i -= (i1 == 76) ? 125 : 250;
    } else if (i1 == 43 || i1 == 49 || i1 == 52 || i1 == 54 || i1 == 66 || i1 == 69 || i1 == 73) {
      i = 250;
      if (i > 0) {
        i -= this.x ? 8 : 3;
      } else if (i < 0) {
        i += this.x ? 8 : 3;
      } 
    } else if (i > 0) {
      i -= this.x ? 8 : 3;
      if (i < 0)
        i = 0; 
    } else if (i < 0) {
      i += this.x ? 8 : 3;
      if (i > 0)
        i = 0; 
    } 
    int i3 = Math.abs(i);
    i3 = (i3 < 10 && i3 != 0) ? 1 : (i3 / 10);
    byte b3 = (i3 == 0) ? 0 : ((i < 0) ? -1 : 1);
    if (i3 > 8)
      i3 = 8; 
    for (byte b4 = 0; b4 < i3; b4++) {
      boolean bool = a(this.D + b3, this.i, true);
      m = (this.D + b3) / 16;
      i1 = this.v.R[n][m] & Byte.MAX_VALUE;
      if (i1 == 97 || i1 == 98 || i1 == 99 || i1 == 100)
        for (byte b5 = 0; b5 < this.v.Y.length; b5++) {
          if (this.v.Y[b5] == -1) {
            this.v.Y[b5] = m;
            this.v.P[b5] = n;
            bool = true;
            break;
          } 
        }  
      if (!bool) {
        this.D += b3;
        if (this.x && (!this.l || !this.I)) {
          byte b5 = this.p ? -1 : 1;
          if (!a(this.D, this.i + b5, false)) {
            m = (this.D + b3) / 16;
            n = (this.i + b5) / 16;
            i1 = this.v.R[n][m] & Byte.MAX_VALUE;
            if (i1 != 101 && i1 != 102 && i1 != 94)
              this.i += b5; 
          } 
        } 
      } else {
        boolean bool4 = false;
        for (byte b5 = 0; b5 < this.v.Y.length; b5++) {
          byte b7;
          byte b6 = 0;
          int i4 = b6;
          int i5 = 0;
          int i6 = this.v.P[b5];
          int i7 = this.v.Y[b5];
          if (i6 != -1 && i7 != -1) {
            b6 = this.v.R[i6][i7];
            i4 = b6 & Byte.MAX_VALUE;
            i5 = b6 & 0x80;
          } else if (this.k != -1) {
            switch (this.E.ao[this.k]) {
              case 0:
                i4 = 201;
                break;
              case 1:
                i4 = 202;
                break;
              case 2:
                i4 = 200;
                break;
            } 
          } 
          if (b3 > 0) {
            if (i4 == 114)
              i4 = 110; 
            if (i4 == 5)
              i4 = 2; 
          } else if (b3 < 0) {
            if (i4 == 113)
              i4 = 110; 
            if (i4 == 6)
              i4 = 2; 
          } 
          switch (i4) {
            case 0:
              break;
            case 3:
            case 4:
            case 113:
            case 114:
              if (this.p)
                break; 
              b7 = -1;
              if (!a(this.D + b3, this.i + b7, true)) {
                this.D += b3;
                this.i += b7;
                break;
              } 
              if (!this.l || !this.I) {
                this.i += b7;
                b5 = 0;
              } 
              break;
            case 5:
            case 6:
            case 115:
            case 116:
              if (!this.p && (!this.l || !this.I))
                break; 
              b7 = 1;
              if (!a(this.D + b3, this.i + b7, true)) {
                this.D += b3;
                this.i += b7;
                break;
              } 
              this.i += b7;
              b5 = 0;
              break;
            case 52:
            case 53:
            case 54:
            case 55:
              b7 = this.p ? 1 : -1;
              if (!a(this.D + b3, this.i + b7, true)) {
                this.D += b3;
                this.i += b7;
                break;
              } 
              if (!a(this.D + b3, this.i - b7, true)) {
                this.D += b3;
                this.i -= b7;
              } 
              break;
            case 93:
            case 95:
              if (!this.I && this.i - this.c == i6 * 16) {
                this.D += b3;
                if (i4 == 93 && this.D == i7 * 16 + 8) {
                  b(i6, i7);
                  this.v.R[i6][i7] = (byte)(0x5F | i5);
                } 
              } 
              break;
            case 97:
            case 98:
            case 99:
            case 100:
              if (!a(this.D + b3, this.i, false)) {
                this.D += b3;
                if ((i4 == 97 || i4 == 98) && this.D == i7 * 16 + 8) {
                  b(i6, i7);
                  if (i4 == 97) {
                    this.v.R[i6][i7] = (byte)(0x63 | i5);
                    this.v.R[i6 + 1][i7] = (byte)(this.v.R[i6 + 1][i7] & 0x80);
                    this.v.R[i6 + 1][i7] = (byte)(this.v.R[i6 + 1][i7] | 0x64);
                    break;
                  } 
                  if (i4 == 98) {
                    this.v.R[i6 - 1][i7] = (byte)(this.v.R[i6 - 1][i7] & 0x80);
                    this.v.R[i6 - 1][i7] = (byte)(this.v.R[i6 - 1][i7] | 0x63);
                    this.v.R[i6][i7] = (byte)(0x64 | i5);
                  } 
                } 
                break;
              } 
              b7 = this.p ? 1 : -1;
              if (!a(this.D + b3, this.i + b7, false)) {
                this.D += b3;
                this.i += b7;
              } 
              break;
            case 1:
            case 62:
            case 63:
            case 64:
            case 65:
            case 85:
            case 86:
            case 87:
            case 88:
            case 89:
            case 90:
            case 91:
            case 92:
            case 201:
            case 202:
              if (!this.F) {
                k();
                return;
              } 
              break;
            case 12:
            case 30:
            case 34:
              b(i6, i7);
              break;
            case 11:
            case 15:
            case 18:
            case 22:
            case 26:
            case 39:
              c(i6, i7);
            default:
              if ((i > 90 || i < -90) && !this.q) {
                d(this.g + 3);
              } else if (this.x && (i > 90 || i < -90)) {
                i = -i;
              } else {
                i = 0;
              } 
              if ((i4 == 7 || i4 == 8) && this.F)
                a(i6, i7); 
              bool4 = true;
              break;
          } 
        } 
        if (bool4)
          break; 
      } 
    } 
    if (this.x)
      this.G = 0; 
    this.s = i;
  }
  
  public void a(Graphics paramGraphics) {
    if (this.e && this.A > 13) {
      int k = paramGraphics.getClipX();
      int m = paramGraphics.getClipY();
      int n = paramGraphics.getClipWidth();
      int i1 = paramGraphics.getClipHeight();
      byte b1 = this.I ? 32 : 24;
      int i2 = b1 >> 1;
      byte b2 = this.I ? 24 : 23;
      int i = this.v.a(this.D) - i2;
      int j = this.v.b(this.i) - i2;
      paramGraphics.setClip(i, j, b1, b1);
      byte b3 = 0;
      if (this.A <= 21 && this.A > 17) {
        b3 = 1;
      } else if (this.A <= 17 && this.A > 13) {
        b3 = 2;
      } 
      paramGraphics.drawImage(this.w[b2], i, j - b3 * b1, 20);
      paramGraphics.setClip(k, m, n, i1);
    } else if (!this.e) {
      int i = this.v.a(this.D) - this.J;
      int j = this.v.b(this.i) - this.c;
      paramGraphics.drawImage(this.w[this.g], i, j, 20);
    } 
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\a.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */