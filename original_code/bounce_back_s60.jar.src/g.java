import com.nokia.mid.ui.DirectGraphics;
import com.nokia.mid.ui.DirectUtils;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

public final class g {
  public String ad;
  
  public boolean X;
  
  public boolean d;
  
  public boolean C;
  
  public boolean ab;
  
  public boolean t;
  
  public boolean J;
  
  public int z = 127;
  
  public int q = 128;
  
  public int i = 2130706559;
  
  public Image N;
  
  public Image[] V;
  
  public int f;
  
  public int A;
  
  public int h;
  
  public byte[] v;
  
  public int[] T;
  
  public byte[] b;
  
  public Image[] E;
  
  public byte[] l;
  
  public boolean[][][] s;
  
  public int[] af;
  
  public int[] Y = new int[5];
  
  public int[] P = new int[5];
  
  public int U;
  
  public byte[] O;
  
  public byte[][] m;
  
  public byte[] aa;
  
  public byte[] ai;
  
  public int n;
  
  public int a;
  
  public int ac;
  
  public int ae;
  
  int r;
  
  int y;
  
  public byte[][] R;
  
  public int u;
  
  public int o;
  
  public int j;
  
  public int Q;
  
  public Image F;
  
  private Graphics e;
  
  private DirectGraphics w;
  
  public int L;
  
  public int M;
  
  private int g;
  
  private int K;
  
  private int I;
  
  private int W;
  
  private int D;
  
  private int G;
  
  private int x;
  
  private int aj;
  
  public int H;
  
  public int k;
  
  private boolean[][] Z;
  
  public int B;
  
  public int ah;
  
  public int ag;
  
  public int c;
  
  public int S;
  
  public int ak;
  
  public int[] p = new int[] { 
      0, 270, 180, 90, 16384, 16654, 16564, 16474, 8192, 8462, 
      8372, 8282 };
  
  public g(byte[] paramArrayOfbyte1, int paramInt1, byte[] paramArrayOfbyte2, int paramInt2, byte[] paramArrayOfbyte3, int paramInt3, String paramString1, String paramString2, int paramInt4, int paramInt5, int paramInt6, int paramInt7, boolean paramBoolean1, boolean paramBoolean2) {
    this.B = paramInt5;
    this.ah = paramInt4;
    this.ag = paramInt6;
    this.c = paramInt7;
    this.C = paramBoolean1;
    this.ab = paramBoolean2;
    ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(paramArrayOfbyte1, paramInt1, paramArrayOfbyte1.length - paramInt1);
    DataInputStream dataInputStream = new DataInputStream(byteArrayInputStream);
    byte b1 = 0;
    byte b2 = 0;
    byte b3 = 0;
    try {
      b1 = dataInputStream.readByte();
      b2 = dataInputStream.readByte();
      this.d = (dataInputStream.readByte() != 0);
      this.X = (dataInputStream.readByte() != 0);
      this.f = dataInputStream.readByte();
      this.A = dataInputStream.readByte();
      if (this.f == 12) {
        this.f = 16;
        this.A = 16;
      } 
      this.h = dataInputStream.readByte();
      b3 = dataInputStream.readByte();
      this.z = dataInputStream.readByte() & 0xFF;
      this.q = dataInputStream.readByte() & 0xFF;
      this.i = dataInputStream.readInt();
    } catch (IOException iOException) {}
    this.V = new Image[b1];
    c c = null;
    if (paramString1 != null) {
      c = new c(paramString1);
      for (byte b4 = 0; b4 < b2; b4++) {
        byte[] arrayOfByte = c.a();
        this.V[b4] = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
      } 
    } 
    if (paramString2 != null && b2 < b1) {
      c = new c(paramString2);
      for (byte b4 = b2; b4 < b1; b4++) {
        byte[] arrayOfByte = c.a();
        this.V[b4] = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
      } 
    } 
    Object object = null;
    c = null;
    try {
      this.U = dataInputStream.readByte();
      if (this.U > 0) {
        this.m = new byte[this.U][];
        this.O = new byte[this.U];
        this.ai = new byte[this.U];
        this.aa = new byte[this.U];
        for (byte b4 = 0; b4 < this.U; b4++) {
          dataInputStream.readByte();
          this.O[b4] = dataInputStream.readByte();
          byte b5 = dataInputStream.readByte();
          this.m[b4] = new byte[b5];
          this.aa[b4] = this.O[b4];
          for (byte b6 = 0; b6 < b5; b6++)
            this.m[b4][b6] = dataInputStream.readByte(); 
        } 
      } 
    } catch (IOException iOException) {}
    this.v = new byte[this.h];
    this.l = new byte[this.h];
    this.s = new boolean[this.h][][];
    this.af = new int[this.h];
    this.b = new byte[this.h];
    this.T = new int[this.h];
    if (this.ab)
      this.E = new Image[this.h]; 
    try {
      for (byte b4 = 0; b4 < this.h; b4++) {
        if (b4 == b3) {
          byteArrayInputStream.close();
          dataInputStream.close();
          byteArrayInputStream = new ByteArrayInputStream(paramArrayOfbyte2, paramInt2, paramArrayOfbyte2.length - paramInt2);
          dataInputStream = new DataInputStream(byteArrayInputStream);
        } 
        byte b5 = dataInputStream.readByte();
        this.v[b4] = dataInputStream.readByte();
        this.T[b4] = dataInputStream.readByte();
        this.b[b4] = dataInputStream.readByte();
        this.l[b4] = dataInputStream.readByte();
        if (this.l[b4] == 1) {
          this.s[b4] = new boolean[this.f][this.A];
          for (byte b6 = 0; b6 < this.A; b6++) {
            for (byte b7 = 0; b7 < this.f; b7++)
              this.s[b4][b7][b6] = dataInputStream.readBoolean(); 
          } 
        } 
        this.af[b4] = dataInputStream.readInt();
        if (this.l[b4] == 3)
          this.s[b4] = this.s[this.af[b4]]; 
      } 
      byteArrayInputStream.close();
      dataInputStream.close();
    } catch (IOException iOException) {}
    if (this.ab) {
      for (byte b4 = 0; b4 < this.h; b4++) {
        if (this.v[b4] == 1) {
          Image image1 = this.V[this.T[b4]];
          if (this.b[b4] == 0) {
            this.E[b4] = image1;
          } else {
            Image image2 = this.E[b4] = DirectUtils.createImage(image1.getWidth(), image1.getHeight(), -1);
            DirectGraphics directGraphics = DirectUtils.getDirectGraphics(image2.getGraphics());
            directGraphics.drawImage(image1, 0, 0, 20, this.p[this.b[b4]]);
          } 
        } 
      } 
      for (byte b5 = 0; b5 < this.V.length; b5++)
        this.V[b5] = null; 
      this.V = null;
    } else {
      boolean[] arrayOfBoolean = new boolean[this.V.length];
      for (byte b4 = 0; b4 < this.h; b4++) {
        if (this.v[b4] == 1)
          arrayOfBoolean[this.T[b4]] = true; 
      } 
      for (byte b5 = 0; b5 < this.V.length; b5++) {
        if (!arrayOfBoolean[b5])
          this.V[b5] = null; 
      } 
    } 
    int i = paramInt3;
    this.n = paramArrayOfbyte3[i++];
    this.ac = paramArrayOfbyte3[i++];
    this.ae = this.ac * this.f;
    this.a = this.n * this.A;
    this.R = new byte[this.n][this.ac];
    for (byte b = 0; b < this.n; b++) {
      for (byte b4 = 0; b4 < this.ac; b4++)
        this.R[b][b4] = paramArrayOfbyte3[i++]; 
    } 
    Image image = Image.createImage(16, 16);
    Graphics graphics = image.getGraphics();
    graphics.setColor(this.i);
    graphics.fillRect(0, 0, 16, 16);
    this.N = Image.createImage(image);
    c();
  }
  
  public void c(int paramInt1, int paramInt2) {
    if (this.d) {
      this.u = a(paramInt1, this.ae);
    } else {
      this.u = b(paramInt1, this.ae);
    } 
    if (this.X) {
      this.o = a(paramInt2, this.a);
    } else {
      this.o = b(paramInt2, this.a);
    } 
  }
  
  public void d() {
    if (this.U != 0)
      for (byte b = 0; b < this.U; b++) {
        this.aa[b] = (byte)(this.aa[b] - 1);
        if (this.aa[b] == 0) {
          this.aa[b] = this.O[b];
          this.ai[b] = (byte)(this.ai[b] + 1);
          if (this.ai[b] == (this.m[b]).length)
            this.ai[b] = 0; 
        } 
      }  
    if (!this.C) {
      a();
      return;
    } 
    boolean bool = b();
    b(bool);
  }
  
  public boolean a(int paramInt1, int paramInt2, int paramInt3, int paramInt4, boolean[][] paramArrayOfboolean, boolean paramBoolean) {
    boolean bool = false;
    byte b1 = 0;
    for (byte b2 = 0; b2 < this.Y.length; b2++) {
      this.Y[b2] = -1;
      this.P[b2] = -1;
    } 
    int i = paramInt1 / this.f;
    int j = (paramInt1 + paramInt3) / this.f;
    int k = paramInt2 / this.A;
    int m = (paramInt2 + paramInt4) / this.A;
    try {
      for (int n = i; n <= j; n++) {
        for (int i1 = k; i1 <= m; i1++) {
          int i2 = this.R[i1][n] & this.z;
          if (this.l[i2] != 0 && a(i1, n, paramInt1, paramInt2, paramInt3, paramInt4, paramArrayOfboolean))
            if (paramBoolean) {
              bool = true;
              this.Y[b1] = n;
              this.P[b1] = i1;
              b1++;
            } else {
              return true;
            }  
        } 
      } 
    } catch (ArrayIndexOutOfBoundsException arrayIndexOutOfBoundsException) {
      return true;
    } 
    return bool;
  }
  
  private boolean a(int paramInt1, int paramInt2, int paramInt3, int paramInt4, int paramInt5, int paramInt6, boolean[][] paramArrayOfboolean) {
    int i = paramInt2 * this.f;
    int j = paramInt1 * this.A;
    int k = this.f - 1;
    int m = this.f - 1;
    int n = this.R[paramInt1][paramInt2] & 0xFF & this.z;
    byte b1 = this.l[n];
    byte b2 = this.b[n];
    int i1 = b2 & 0x3;
    boolean[][] arrayOfBoolean = this.s[n];
    int i2 = 0;
    int i3 = 0;
    int i4 = 0;
    int i5 = 0;
    int i6 = 0;
    int i7 = 0;
    if (paramInt3 < i) {
      i4 = i - paramInt3;
      i6 = paramInt5 - i4;
      if (i6 > this.f)
        i6 = this.f; 
    } else if (paramInt3 > i) {
      i2 = paramInt3 - i;
      i6 = this.f - i2;
      if (i6 > paramInt5)
        i6 = paramInt5; 
    } else if (paramInt3 == i) {
      i6 = this.f;
      if (i6 > paramInt5)
        i6 = paramInt5; 
    } 
    if (paramInt4 < j) {
      i5 = j - paramInt4;
      i7 = paramInt6 - i5;
      if (i7 > this.A)
        i7 = this.A; 
    } else if (paramInt4 > j) {
      i3 = paramInt4 - j;
      i7 = this.A - i3;
      if (i7 > paramInt6)
        i7 = paramInt6; 
    } else {
      i7 = this.A;
      if (i7 > paramInt6)
        i7 = paramInt6; 
    } 
    for (int i8 = i7 - 1; i8 >= 0; i8--) {
      for (int i9 = i6 - 1; i9 >= 0; i9--) {
        if (paramArrayOfboolean[i9 + i4][i8 + i5])
          if (b1 == 3) {
            int i10 = i9 + i2;
            int i11 = i8 + i3;
            int i12 = i10;
            int i13 = i11;
            byte b = b2;
            if ((b2 & 0x8) != 0)
              i10 = k - i10; 
            if ((b2 & 0x4) != 0)
              i11 = m - i11; 
            if (i1 == 3) {
              int i14 = i11;
              i11 = i10;
              i10 = m - i14;
            } 
            if (i1 == 1) {
              int i14 = i10;
              i10 = i11;
              i11 = m - i14;
            } 
            if (i1 == 2) {
              i10 = k - i10;
              i11 = m - i11;
            } 
            if (arrayOfBoolean[i11][i10])
              return true; 
          } else if (b1 == 2 || arrayOfBoolean[i8 + i3][i9 + i2]) {
            return true;
          }  
      } 
    } 
    return false;
  }
  
  public void a(Graphics paramGraphics) {
    paramGraphics.setClip(this.B, this.ah, this.ag, this.c);
    if (this.C) {
      if (this.F == null)
        return; 
      paramGraphics.drawImage(this.F, -this.S, -this.ak, 20);
      if (this.S > this.I)
        paramGraphics.drawImage(this.F, this.g - this.S, -this.ak, 20); 
      if (this.ak > this.W) {
        paramGraphics.drawImage(this.F, -this.S, this.K - this.ak, 20);
        if (this.S > this.I)
          paramGraphics.drawImage(this.F, this.g - this.S, this.K - this.ak, 20); 
      } 
    } else {
      if (this.e == null) {
        this.e = paramGraphics;
        this.w = DirectUtils.getDirectGraphics(paramGraphics);
      } 
      b(true);
    } 
  }
  
  public int a(int paramInt) {
    return paramInt + this.B - (this.k + this.G) * this.f - this.S;
  }
  
  public int b(int paramInt) {
    return paramInt + this.ah - (this.H + this.D) * this.A - this.ak;
  }
  
  public void c() {
    if (this.F != null)
      return; 
    this.M = (this.ag - 1) / this.f + 2;
    this.L = (this.c - 1) / this.A + 2;
    this.g = this.M * this.f;
    this.K = this.L * this.A;
    this.I = this.g - this.ag;
    this.W = this.K - this.c;
    if (!this.C)
      return; 
    this.F = Image.createImage(this.g, this.K);
    this.e = this.F.getGraphics();
    this.w = DirectUtils.getDirectGraphics(this.e);
    this.D = 0;
    this.G = 0;
    a();
    this.Z = new boolean[this.L][this.M];
    for (byte b = 0; b < this.L; b++) {
      for (byte b1 = 0; b1 < this.M; b1++)
        this.Z[b][b1] = true; 
    } 
  }
  
  private boolean b() {
    boolean bool = false;
    this.x = this.D;
    this.aj = this.G;
    int i = this.H;
    int j = this.k;
    a();
    int k = this.H - i;
    int m = this.k - j;
    if (!this.X) {
      int n = this.n >> 1;
      if (k >= n) {
        k -= this.n;
      } else if (k <= -n) {
        k += this.n;
      } 
    } 
    if (!this.d) {
      int n = this.ac >> 1;
      if (m >= n) {
        m -= this.ac;
      } else if (m <= -n) {
        m += this.ac;
      } 
    } 
    if (k == 0 && m == 0)
      return false; 
    if (!this.d || !this.X || (Math.abs(k) < this.L && Math.abs(m) < this.M)) {
      this.D = b(this.D + k, this.L);
      this.G = b(this.G + m, this.M);
      int n = this.x;
      if (k < 0)
        n = b(n - 1, this.L); 
      int i1 = b(n + k, this.L);
      byte b = (k < 0) ? -1 : 1;
      for (int i2 = n; i2 != i1; i2 = b(i2 + b, this.L)) {
        for (byte b1 = 0; b1 < this.M; b1++)
          this.Z[i2][b1] = true; 
      } 
      n = this.aj;
      if (m < 0)
        n = b(n - 1, this.M); 
      i1 = b(n + m, this.M);
      b = (m < 0) ? -1 : 1;
      for (int i3 = n; i3 != i1; i3 = b(i3 + b, this.M)) {
        for (byte b1 = 0; b1 < this.L; b1++)
          this.Z[b1][i3] = true; 
      } 
    } else {
      if (this.X)
        this.D = 0; 
      if (this.d)
        this.G = 0; 
      bool = true;
    } 
    a();
    return bool;
  }
  
  private void b(boolean paramBoolean) {
    if (this.C) {
      a(paramBoolean);
    } else {
      e();
    } 
  }
  
  private void a(boolean paramBoolean) {
    for (int i = this.L - 1; i >= 0; i--) {
      for (int j = this.M - 1; j >= 0; j--) {
        if (paramBoolean || this.Z[i][j]) {
          int k = this.H + i - this.D;
          if (k < this.H)
            k += this.L; 
          k = b(k, this.n);
          int m = this.k + j - this.G;
          if (m < this.k)
            m += this.M; 
          m = b(m, this.ac);
          a(this.R[k][m], j * this.f, i * this.A, true, 0);
          if (!paramBoolean)
            this.Z[i][j] = false; 
        } 
      } 
    } 
  }
  
  private void e() {
    int i = -this.S;
    int j = -this.ak;
    int k = 0;
    int m = this.H;
    while (j < this.c) {
      if (!this.X)
        m = b(m, this.n); 
      k = i;
      int n = this.k;
      while (k < this.ag) {
        if (!this.d)
          n = b(n, this.ac); 
        byte b = this.R[m][n];
        int i1 = b & this.z;
        if (this.v[i1] != 0 || b != i1)
          a(b, k, j, false, 0); 
        n++;
        k += this.f;
      } 
      m++;
      j += this.A;
    } 
  }
  
  private void a(int paramInt1, int paramInt2, int paramInt3, boolean paramBoolean, int paramInt4) {
    int j;
    int k;
    int i = paramInt1 & this.z;
    byte b = this.v[i];
    boolean bool = ((paramInt1 & 0xFF & this.q) != 0) ? true : false;
    if (bool) {
      this.e.setColor(this.i);
      this.e.fillRect(paramInt2, paramInt3, 16, 16);
    } 
    if (b == 0)
      return; 
    if (b == 5 || b == 2) {
      this.e.setColor(this.af[i]);
      this.e.fillRect(paramInt2, paramInt3, this.f, this.A);
    } 
    switch (b) {
      case 1:
      case 5:
        if (this.ab) {
          if (paramInt4 == 0) {
            this.e.drawImage(this.E[i], paramInt2, paramInt3, 20);
            break;
          } 
          this.w.drawImage(this.E[i], paramInt2, paramInt3, 20, this.p[paramInt4]);
          break;
        } 
        j = this.T[i] & 0xFF;
        k = (paramInt4 == 0) ? this.b[i] : paramInt4;
        k = 0;
        if (paramInt4 < 0)
          j = -paramInt4; 
        if (!k) {
          this.e.drawImage(this.V[j], paramInt2, paramInt3, 20);
          break;
        } 
        this.w.drawImage(this.V[j], paramInt2, paramInt3, 20, this.p[k]);
        break;
      case 3:
        j = this.af[i];
        k = this.b[i];
        k = -((this.T[this.m[j][0] & this.z] & 0xFF) + this.ai[j]);
        a(this.m[j][this.ai[j]], paramInt2, paramInt3, paramBoolean, k);
        break;
    } 
  }
  
  private void a() {
    this.J = this.t = true;
    this.r = this.u - this.j - (this.ag >> 1);
    if (!this.d)
      this.r = b(this.r, this.ae); 
    this.y = this.o - this.Q - (this.c >> 1);
    if (!this.X)
      this.y = b(this.y, this.a); 
    this.H = this.y / this.A;
    this.k = this.r / this.f;
    this.S = this.r - this.k * this.f + this.G * this.f;
    this.ak = this.y - this.H * this.A + this.D * this.A;
    if (this.d) {
      if (this.r < 0) {
        this.k = 0;
        this.r = 0;
        this.S = this.G * this.f;
        this.J = false;
      } else if (this.r >= this.ae - this.ag) {
        this.k = this.ac - this.M;
        this.r = this.k * this.f;
        this.S = this.ae - this.ag - this.k * this.A + this.G * this.A;
        this.J = false;
      } 
    } else {
      if (this.S < 0)
        this.S += this.g; 
      if (this.k < 0) {
        this.k += this.ac;
      } else if (this.k > this.ac) {
        this.k -= this.ac;
      } 
    } 
    if (this.X) {
      if (this.y < 0) {
        this.H = 0;
        this.y = 0;
        this.ak = this.D * this.A;
        this.t = false;
      } else if (this.y >= this.a - this.c) {
        this.H = this.n - this.L;
        this.y = this.H * this.A;
        this.ak = this.a - this.c - this.H * this.A + this.D * this.A;
        this.t = false;
      } 
    } else {
      if (this.ak < 0)
        this.ak += this.K; 
      if (this.H < 0) {
        this.H += this.n;
      } else if (this.H > this.n) {
        this.H -= this.n;
      } 
    } 
  }
  
  public static int b(int paramInt1, int paramInt2) {
    int i;
    for (i = paramInt1; i >= paramInt2; i -= paramInt2);
    while (i < 0)
      i += paramInt2; 
    return i;
  }
  
  public static int a(int paramInt1, int paramInt2) {
    int i = paramInt1;
    if (paramInt1 < 0) {
      i = 0;
    } else if (paramInt1 >= paramInt2) {
      i = paramInt2 - 1;
    } 
    return i;
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\g.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */