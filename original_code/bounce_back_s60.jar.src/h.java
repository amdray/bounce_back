import com.nokia.mid.ui.DirectGraphics;
import com.nokia.mid.ui.DirectUtils;
import com.nokia.mid.ui.FullCanvas;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import javax.microedition.lcdui.Font;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;
import javax.microedition.rms.RecordStore;
import javax.microedition.rms.RecordStoreException;

public class h extends FullCanvas implements Runnable {
  public CrystalMidlet H;
  
  public a e;
  
  public Runtime at;
  
  public int C;
  
  public int[][] h;
  
  public int X;
  
  public int[][] c;
  
  public int z;
  
  public byte y = Byte.MIN_VALUE;
  
  public long t;
  
  public long g;
  
  public g Z;
  
  public g A;
  
  public int a;
  
  public int as;
  
  public int ai;
  
  public int aq;
  
  public boolean n;
  
  public f aj;
  
  public int U;
  
  public int w;
  
  public boolean p;
  
  public int af;
  
  public boolean M;
  
  public int ae;
  
  public int d;
  
  public int W;
  
  public boolean ab;
  
  public boolean V;
  
  public boolean ad;
  
  public int L;
  
  public boolean ap;
  
  public byte[] b;
  
  private DirectGraphics K;
  
  public Image[] S;
  
  public Image[] x;
  
  public Image[] ac;
  
  public int ar;
  
  public int D;
  
  public Image Q;
  
  public int I;
  
  public boolean o;
  
  public int P;
  
  public int R;
  
  public d au;
  
  public e aa;
  
  public boolean q;
  
  public boolean E;
  
  public Font F;
  
  public int i;
  
  public Image av;
  
  public int T;
  
  public String[] r;
  
  public int[][] Y = new int[][] { { 6, 6, 160, 50 }, { 6, 6, 160, 40 }, { 6, 100, 160, 40 }, { 6, 6, 160, 40 }, { 6, 6, 160, 50 } };
  
  public int G;
  
  public int j;
  
  public byte[] ao;
  
  public int[][] ag;
  
  public byte[][] s;
  
  public int[][] f;
  
  public int[][] k;
  
  public byte[] B;
  
  public int[] v;
  
  public int[] J;
  
  private Image[] m = new Image[100];
  
  public long N = System.currentTimeMillis();
  
  public char[] ah = new char[3];
  
  public static int O = 0;
  
  public static Image[][] al = new Image[3][];
  
  public static Image[] ak;
  
  public static Image[] u;
  
  public static Image l;
  
  public static Image[] am;
  
  public static Image[] an;
  
  public h(CrystalMidlet paramCrystalMidlet) {
    this.H = paramCrystalMidlet;
    this.at = Runtime.getRuntime();
    this.p = false;
    this.af = 0;
    this.H.a(0, 7);
    this.b = new byte[6];
    for (byte b = 0; b < 6; b++)
      this.b[b] = 0; 
    this.E = false;
    this.q = false;
    this.F = Font.getFont(64, 0, 8);
    this.i = this.F.getHeight();
  }
  
  public void a(int paramInt) {
    this.ab = true;
    this.d = paramInt;
  }
  
  public void b(int paramInt) {
    byte b5;
    this.d = paramInt;
    this.Z = this.A = null;
    c c1 = new c("/res/lf");
    byte[] arrayOfByte1 = c1.c(paramInt << 1);
    DataInputStream dataInputStream = new DataInputStream(new ByteArrayInputStream(arrayOfByte1));
    byte b1 = 0;
    byte b2 = 0;
    byte b3 = 0;
    byte b4 = 0;
    try {
      b1 = dataInputStream.readByte();
      b2 = dataInputStream.readByte();
      b3 = dataInputStream.readByte();
      b4 = dataInputStream.readByte();
      this.ar = dataInputStream.readByte();
      this.D = dataInputStream.readByte();
      this.j = dataInputStream.readByte();
      if (this.j > 0) {
        this.ao = new byte[this.j];
        this.f = new int[this.j][2];
        this.k = new int[this.j][2];
        this.ag = new int[this.j][2];
        this.s = new byte[this.j][2];
        for (byte b = 0; b < this.j; b++) {
          byte b7 = this.ao[b] = dataInputStream.readByte();
          byte b8 = dataInputStream.readByte();
          byte b9 = dataInputStream.readByte();
          byte b10 = dataInputStream.readByte();
          byte b11 = dataInputStream.readByte();
          int m = dataInputStream.readByte() * 16;
          int n = dataInputStream.readByte() * 16;
          byte b12 = dataInputStream.readByte();
          b5 = dataInputStream.readByte();
          if (b8 > b10 || b9 > b11) {
            byte b13 = b8;
            b8 = b10;
            b10 = b13;
            b13 = b9;
            b9 = b11;
            b11 = b13;
            m = (b10 - b8) * 16;
            n = (b11 - b9) * 16;
            if (b5 > 0 || b12 > 0) {
              b5 = -b5;
              b12 = -b12;
            } 
          } 
          this.f[b][0] = b8;
          this.f[b][1] = b9;
          this.k[b][0] = b10;
          this.k[b][1] = b11;
          this.ag[b][1] = m;
          this.ag[b][0] = n;
          this.s[b][1] = (byte)b12;
          this.s[b][0] = (byte)b5;
        } 
      } 
    } catch (IOException iOException) {
      try {
        dataInputStream.close();
      } catch (IOException iOException1) {}
    } finally {
      try {
        dataInputStream.close();
      } catch (IOException iOException) {}
    } 
    arrayOfByte1 = null;
    byte[] arrayOfByte2 = c1.a();
    c1 = null;
    c c2 = new c("/res/tf");
    byte[] arrayOfByte3 = c2.a();
    byte[] arrayOfByte4 = c2.a();
    c2 = null;
    if (al[b1] == null) {
      this.Z = new g(arrayOfByte3, 0, arrayOfByte4, 0, arrayOfByte2, 0, "/res/if0", "/res/if" + b1, 0, 0, 176, 179, false, false);
      al[b1] = this.Z.V;
    } else {
      this.Z = new g(arrayOfByte3, 0, arrayOfByte4, 0, arrayOfByte2, 0, null, null, 0, 0, 176, 179, false, false);
      this.Z.V = al[b1];
    } 
    this.Z.ad = "fg";
    arrayOfByte3 = arrayOfByte4 = arrayOfByte2 = null;
    c c3 = new c("/res/bg");
    String str = "/res/ib" + b1;
    if (getClass().getResourceAsStream(str) == null)
      str = null; 
    arrayOfByte3 = c3.a();
    arrayOfByte4 = c3.a();
    arrayOfByte2 = c3.a();
    c3 = null;
    if (ak == null) {
      this.A = new g(arrayOfByte3, 4, arrayOfByte4, 4, arrayOfByte2, 4, "/res/ib0", str, 0, 0, 176, 179, false, false);
      ak = this.A.V;
    } else {
      this.A = new g(arrayOfByte3, 4, arrayOfByte4, 4, arrayOfByte2, 4, null, null, 0, 0, 176, 179, false, false);
      this.A.V = ak;
    } 
    this.A.ad = "bg";
    c3 = null;
    arrayOfByte3 = arrayOfByte4 = arrayOfByte2 = null;
    i();
    this.C = 0;
    this.g = 0L;
    this.I = 0;
    this.o = false;
    this.P = -1;
    this.R = -1;
    int i = b3 * 16;
    int j = b2 * 16;
    if (b4 == 0) {
      i += 8;
      j += 8;
      b5 = 0;
    } else {
      i += 12;
      j += 12;
      b5 = 11;
    } 
    this.M = true;
    if (this.e == null) {
      this.e = new a(i, j, b5, this, this.H);
    } else {
      this.e.a(i, j, b5);
    } 
    a(i, j, true, true);
    this.a = i;
    this.as = j;
    if (this.B == null) {
      this.B = new byte[5];
      this.v = new int[5];
      this.J = new int[5];
    } 
    this.h = new int[30][2];
    this.c = new int[36][2];
    this.X = 0;
    this.z = 0;
    byte b6 = 0;
    for (int k = this.Z.n - 1; k != -1; k--) {
      for (int m = this.Z.ac - 1; m != -1; m--) {
        byte b = this.Z.R[k][m];
        int n = b & Byte.MAX_VALUE;
        int i1 = b & 0x80;
        if (n == 93 || n == 94 || n == 97 || n == 101) {
          this.h[this.X][0] = k;
          this.h[this.X++][1] = m;
          b6++;
        } 
        if ((n >= 52 && n <= 61) || (n >= 66 && n <= 72)) {
          this.c[this.z][0] = k;
          this.c[this.z++][1] = m;
        } 
      } 
    } 
    this.W = b6;
    this.Z.c(this.e.D, this.e.i);
    this.A.c(this.e.D, this.e.i);
    if (paramInt > 20) {
      this.r = new String[5];
      this.r[0] = "4 to move left; 6 to move right; 2 or 5 to jump";
      this.r[1] = "Beware the Spikeys!";
      this.r[2] = "Collect gems and powerups,";
      this.r[3] = "Jump through all the hoops!";
      this.r[4] = "You can now exit the level!";
    } 
  }
  
  public void i() {
    c c = new c("/res/ic");
    if (this.S == null)
      if (an != null) {
        this.S = an;
      } else {
        this.S = new Image[3];
        for (byte b = 0; b < 3; b++) {
          byte[] arrayOfByte = c.a();
          this.S[b] = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
        } 
        an = this.S;
      }  
    if (this.x == null)
      if (am != null) {
        this.x = am;
      } else {
        this.x = new Image[4];
        for (byte b = 0; b < 4; b++) {
          byte[] arrayOfByte = c.a();
          this.x[b] = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
        } 
        am = this.x;
      }  
    if (this.Q == null)
      if (l != null) {
        this.Q = l;
      } else {
        byte[] arrayOfByte = c.a();
        this.Q = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
        l = this.Q;
      }  
    if (this.ac == null)
      if (u != null) {
        this.ac = u;
      } else {
        this.ac = new Image[4];
        for (byte b = 0; b < 4; b++) {
          byte[] arrayOfByte = c.a();
          this.ac[b] = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
        } 
        u = this.ac;
      }  
    c = null;
    Object object = null;
  }
  
  public void f() {
    this.V = true;
  }
  
  public void b() {
    try {
      RecordStore recordStore = RecordStore.openRecordStore("CrystalRMS", true);
      byte[] arrayOfByte = new byte[recordStore.getRecordSize(3)];
      arrayOfByte = recordStore.getRecord(3);
      ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(arrayOfByte);
      DataInputStream dataInputStream = new DataInputStream(byteArrayInputStream);
      dataInputStream.readByte();
      int i = dataInputStream.readInt();
      b(i);
      this.H.h = dataInputStream.readInt();
      this.H.x = dataInputStream.readInt();
      this.H.o = dataInputStream.readBoolean();
      this.H.n = dataInputStream.readInt();
      this.C = dataInputStream.readInt();
      this.g = dataInputStream.readLong();
      this.I = dataInputStream.readInt();
      this.o = dataInputStream.readBoolean();
      this.W = dataInputStream.readInt();
      this.e.D = dataInputStream.readInt();
      this.e.i = dataInputStream.readInt();
      this.e.n = dataInputStream.readInt();
      this.e.H = dataInputStream.readInt();
      this.e.s = dataInputStream.readInt();
      this.e.h = dataInputStream.readInt();
      this.e.G = dataInputStream.readInt();
      this.e.z = dataInputStream.readInt();
      this.e.b = dataInputStream.readInt();
      this.e.B = dataInputStream.readInt();
      this.e.x = dataInputStream.readBoolean();
      this.e.F = dataInputStream.readBoolean();
      this.e.I = dataInputStream.readBoolean();
      this.e.p = dataInputStream.readBoolean();
      this.e.q = dataInputStream.readBoolean();
      this.e.a = dataInputStream.readByte();
      this.e.r = dataInputStream.readByte();
      this.e.A = dataInputStream.readInt();
      this.e.g = dataInputStream.readInt();
      this.e.d = this.e.w[this.e.g].getWidth();
      this.e.u = this.e.w[this.e.g].getHeight();
      this.e.J = this.e.d >> 1;
      this.e.c = this.e.u >> 1;
      this.e.a(this.e.g);
      byte b = dataInputStream.readByte();
      int[][] arrayOfInt = new int[b][3];
      for (byte b1 = 0; b1 < b; b1++) {
        arrayOfInt[b1][0] = dataInputStream.readShort();
        arrayOfInt[b1][1] = dataInputStream.readShort();
        arrayOfInt[b1][2] = dataInputStream.readByte();
      } 
      for (short s = 0; s < this.Z.n; s = (short)(s + 1)) {
        for (short s1 = 0; s1 < this.Z.ac; s1 = (short)(s1 + 1)) {
          byte b3;
          int j = this.Z.R[s][s1] & Byte.MAX_VALUE;
          int k = this.Z.R[s][s1] & 0x80;
          switch (j) {
            case 7:
            case 8:
            case 12:
            case 30:
            case 34:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97:
            case 98:
            case 99:
            case 100:
            case 101:
            case 102:
            case 103:
            case 104:
              b3 = a(s, s1, arrayOfInt);
              if (b3 == -1)
                b3 = (byte)(0x0 | k); 
              this.Z.R[s][s1] = b3;
              break;
          } 
        } 
      } 
      arrayOfInt = null;
      for (byte b2 = 0; b2 < this.j; b2++) {
        this.ag[b2][0] = dataInputStream.readInt();
        this.ag[b2][1] = dataInputStream.readInt();
        this.s[b2][0] = dataInputStream.readByte();
        this.s[b2][1] = dataInputStream.readByte();
      } 
      recordStore.closeRecordStore();
    } catch (RecordStoreException recordStoreException) {
    
    } catch (IOException iOException) {}
  }
  
  public byte a(short paramShort1, short paramShort2, int[][] paramArrayOfint) {
    for (byte b = 0; b < paramArrayOfint.length; b++) {
      if (paramArrayOfint[b][0] == paramShort1 && paramArrayOfint[b][1] == paramShort2)
        return (byte)paramArrayOfint[b][2]; 
    } 
    return -1;
  }
  
  public void a(DataOutputStream paramDataOutputStream) {
    try {
      paramDataOutputStream.writeInt(this.d);
      paramDataOutputStream.writeInt(this.H.h);
      paramDataOutputStream.writeInt(this.H.x);
      paramDataOutputStream.writeBoolean(this.H.o);
      paramDataOutputStream.writeInt(this.H.n);
      paramDataOutputStream.writeInt(this.C);
      paramDataOutputStream.writeLong(this.g);
      paramDataOutputStream.writeInt(this.I);
      paramDataOutputStream.writeBoolean(this.o);
      paramDataOutputStream.writeInt(this.W);
      paramDataOutputStream.writeInt(this.e.D);
      paramDataOutputStream.writeInt(this.e.i);
      paramDataOutputStream.writeInt(this.e.n);
      paramDataOutputStream.writeInt(this.e.H);
      paramDataOutputStream.writeInt(this.e.s);
      paramDataOutputStream.writeInt(this.e.h);
      paramDataOutputStream.writeInt(this.e.G);
      paramDataOutputStream.writeInt(this.e.z);
      paramDataOutputStream.writeInt(this.e.b);
      paramDataOutputStream.writeInt(this.e.B);
      paramDataOutputStream.writeBoolean(this.e.x);
      paramDataOutputStream.writeBoolean(this.e.F);
      paramDataOutputStream.writeBoolean(this.e.I);
      paramDataOutputStream.writeBoolean(this.e.p);
      paramDataOutputStream.writeBoolean(this.e.q);
      paramDataOutputStream.writeByte(this.e.a);
      paramDataOutputStream.writeByte(this.e.r);
      paramDataOutputStream.writeInt(this.e.A);
      paramDataOutputStream.writeInt(this.e.g);
      int[][] arrayOfInt = new int[50][3];
      byte b1 = 0;
      for (byte b2 = 0; b2 < this.Z.n; b2++) {
        for (byte b = 0; b < this.Z.ac; b++) {
          byte b5 = this.Z.R[b2][b];
          int i = b5 & Byte.MAX_VALUE;
          switch (i) {
            case 7:
            case 8:
            case 12:
            case 30:
            case 34:
            case 35:
            case 93:
            case 94:
            case 95:
            case 96:
            case 97:
            case 98:
            case 99:
            case 100:
            case 101:
            case 102:
            case 103:
            case 104:
              arrayOfInt[b1][0] = b2;
              arrayOfInt[b1][1] = b;
              arrayOfInt[b1][2] = b5;
              b1++;
              break;
          } 
        } 
      } 
      paramDataOutputStream.writeByte(b1);
      for (byte b3 = 0; b3 < b1; b3++) {
        paramDataOutputStream.writeShort(arrayOfInt[b3][0]);
        paramDataOutputStream.writeShort(arrayOfInt[b3][1]);
        paramDataOutputStream.writeByte(arrayOfInt[b3][2]);
      } 
      arrayOfInt = null;
      for (byte b4 = 0; b4 < this.j; b4++) {
        paramDataOutputStream.writeInt(this.ag[b4][0]);
        paramDataOutputStream.writeInt(this.ag[b4][1]);
        paramDataOutputStream.writeByte(this.s[b4][0]);
        paramDataOutputStream.writeByte(this.s[b4][1]);
      } 
    } catch (IOException iOException) {}
  }
  
  public void paint(Graphics paramGraphics) {
    if (this.ab || this.V) {
      paramGraphics.setColor(30702);
      paramGraphics.fillRect(0, 0, 176, 208);
      paramGraphics.setColor(16768256);
      paramGraphics.drawImage(a("Loading…", 100, 20), 88, 104, 3);
    } 
    if (!this.n)
      return; 
    this.A.a(paramGraphics);
    this.Z.a(paramGraphics);
    c(paramGraphics);
    b(paramGraphics);
    this.e.a(paramGraphics);
    if (this.K == null)
      this.K = DirectUtils.getDirectGraphics(paramGraphics); 
    a(paramGraphics, this.K);
    paramGraphics.drawImage(this.av, this.Y[this.G][0], this.Y[this.G][1], 20);
    if (this.av != null && --this.T < 0) {
      this.av = null;
      this.G++;
    } 
    if (this.M)
      a(paramGraphics); 
    if (this.ad)
      paramGraphics.drawImage(a(CrystalMidlet.a("Level {0}", this.d + 1), 100, 20), 88, 104, 3); 
  }
  
  public void a(Graphics paramGraphics) {
    int i = paramGraphics.getClipX();
    int j = paramGraphics.getClipY();
    int k = paramGraphics.getClipWidth();
    int m = paramGraphics.getClipHeight();
    paramGraphics.setClip(0, 179, 176, 29);
    paramGraphics.drawImage(this.S[0], 0, 179, 20);
    if (this.W != 0)
      for (int n = this.W; n > 0; n--)
        paramGraphics.drawImage(this.S[2], 12 + n * 6, 187, 20);  
    if (this.ae > 0) {
      int n = this.ae;
      if (n > 25)
        n = 25; 
      paramGraphics.setColor(8388352);
      paramGraphics.fillRect(2, 181 + 25 - n, 4, n + 1);
    } 
    paramGraphics.setClip(157, 185, 12, 16);
    paramGraphics.drawImage(this.S[1], 157, 185 - 16 * this.H.h, 20);
    paramGraphics.setClip(i, j, k, m);
    this.M = false;
  }
  
  public void c(Graphics paramGraphics) {
    int i = this.Z.k - 2;
    int j = this.Z.H - 3;
    int k = i + this.Z.M + 2;
    int m = j + this.Z.L + 3;
    int n = this.ar;
    int i1 = this.D;
    if (n >= j && n <= m && i1 >= i && i1 <= k) {
      if (this.P == -1) {
        this.P = i1 * 16;
        this.R = n * 16;
      } 
      int i2 = this.Z.a(this.P);
      int i3 = this.Z.b(this.R);
      int i4 = Math.min(32 + i2, 176) - i2;
      int i5 = Math.min(24 + i3, 179) - i3;
      if (i4 > 0 && i5 > 0) {
        int i6 = this.I >> 1;
        paramGraphics.setClip(i2, i3, i4, i5);
        paramGraphics.drawImage(this.Q, i2, i3 - i6, 20);
        i3 += 24;
        i5 = Math.min(24 + i3, 179) - i3;
        if (i5 > 0) {
          i6 = 72 - i6;
          paramGraphics.setClip(i2, i3, i4, i5);
          paramGraphics.drawImage(this.Q, i2, i3 - i6, 20);
        } 
        paramGraphics.setClip(0, 0, 176, 179);
      } 
      if (this.o) {
        this.I++;
        if (this.I == 72)
          this.I = 48; 
      } else if (!this.o && this.W == 0) {
        this.I++;
        if (this.I == 48)
          this.o = true; 
      } 
      return;
    } 
  }
  
  public void a(Graphics paramGraphics, DirectGraphics paramDirectGraphics) {
    byte b1 = 16;
    byte b2 = 16;
    int i = this.Z.k - 1;
    int j = this.Z.H;
    int k = i + this.Z.M;
    int m = j + this.Z.L;
    for (int n = this.X - 1; n != -1; n--) {
      int i2 = this.h[n][0];
      int i3 = this.h[n][1];
      if (i2 >= j - 1 && i2 <= m && i3 >= i - 1 && i3 <= k) {
        int i4 = this.Z.a(i3 * 16);
        byte b3 = b1;
        byte b4 = b2;
        byte b = this.Z.R[i2][i3];
        int i5 = b & Byte.MAX_VALUE;
        int i6 = b & 0x80;
        if (i5 >= 93 && i5 <= 104) {
          byte b5;
          int i7;
          switch (i5) {
            case 93:
            case 95:
              b5 = (i5 == 93) ? 0 : 1;
              i7 = this.Z.b(i2 * 16) - 1;
              b4 = 14;
              paramGraphics.drawImage(this.x[b5], i4, i7, 20);
              break;
            case 94:
            case 96:
              b5 = (i5 == 94) ? 0 : 1;
              i4--;
              i7 = this.Z.b(i2 * 16);
              b3 = 14;
              paramDirectGraphics.drawImage(this.x[b5], i4, i7, 20, 270);
              break;
            case 97:
            case 99:
              b5 = (i5 == 97) ? 2 : 3;
              i7 = this.Z.b(i2 * 16);
              paramDirectGraphics.drawImage(this.x[b5], i4, i7, 20, 8192);
              paramDirectGraphics.drawImage(this.x[b5], i4, i7 + b2, 20, 180);
              break;
            case 101:
            case 103:
              b5 = (i5 == 101) ? 2 : 3;
              i7 = this.Z.b(i2 * 16);
              paramDirectGraphics.drawImage(this.x[b5], i4, i7, 20, 8462);
              paramDirectGraphics.drawImage(this.x[b5], i4 + b1, i7, 20, 270);
              break;
          } 
        } 
      } 
    } 
    for (int i1 = this.z - 1; i1 != -1; i1--) {
      int i2 = this.c[i1][0];
      int i3 = this.c[i1][1];
      if (i2 >= j && i2 <= m && i3 >= i && i3 <= k) {
        int i4 = this.Z.a(i3 * 16);
        int i5 = this.Z.b(i2 * 16);
        int i6 = this.Z.R[i2][i3] & Byte.MAX_VALUE;
        if (this.Z.ab) {
          paramGraphics.drawImage(this.Z.E[i6], i4, i5, 20);
        } else {
          byte b = this.Z.b[i6];
          if (b == 0) {
            paramGraphics.drawImage(this.Z.V[this.Z.T[i6]], i4, i5, 20);
          } else {
            paramDirectGraphics.drawImage(this.Z.V[this.Z.T[i6]], i4, i5, 20, this.Z.p[b]);
          } 
        } 
      } 
    } 
  }
  
  public void run() {
    O++;
    if (this.ap) {
      this.n = false;
      this.ap = false;
      this.o = false;
      if (this.au != null) {
        this.au = null;
        this.H.e();
      } else if (this.aa != null) {
        this.b[4] = 1;
      } else {
        this.H.c();
      } 
    } 
    if (this.ab) {
      this.n = false;
      repaint();
      serviceRepaints();
      b(this.d);
    } 
    if (this.V) {
      this.n = false;
      repaint();
      serviceRepaints();
      b();
    } 
    if (this.ab || this.V || this.ad) {
      this.n = true;
      this.ab = this.V = false;
      this.ad = (this.d < 21);
    } else {
      if (this.e == null)
        return; 
      if (this.au != null) {
        this.au.a(this.b);
        if (this.au.d) {
          this.au = null;
          this.H.e();
        } 
      } 
      c();
      if (this.j != 0)
        d(); 
      this.e.d();
      g();
    } 
    this.Z.c(this.e.D, this.e.i);
    this.Z.d();
    a(this.e.D, this.e.i, this.Z.J, this.Z.t);
    this.A.c(this.ai, this.aq);
    this.A.d();
    this.n = true;
    repaint();
    serviceRepaints();
  }
  
  public void a() {
    if (this.aj == null)
      this.aj = new f(this, 50); 
    this.t = System.currentTimeMillis();
    this.M = true;
    if (this.e != null) {
      this.Z.c(this.e.D, this.e.i);
      this.Z.d();
      a(this.e.D, this.e.i, this.Z.J, this.Z.t);
      this.A.c(this.ai, this.aq);
      if (this.A.C && this.A.F == null)
        this.A.c(); 
    } 
  }
  
  public void hideNotify() {
    for (byte b = 0; b < this.b.length; b++)
      this.b[b] = 2; 
    if (this.H.C) {
      this.H.C = false;
    } else if (this.au != null) {
      this.au = null;
      this.H.e();
    } else {
      h();
      this.H.i = true;
      this.H.b(false);
    } 
  }
  
  public synchronized void h() {
    if (this.aj != null) {
      this.g += System.currentTimeMillis() - this.t;
      this.aj.a();
      Thread.yield();
      this.aj = null;
    } 
    this.U = this.w = 0;
    Thread.yield();
  }
  
  public void keyPressed(int paramInt) {
    if (this.e == null)
      return; 
    if (this.ad) {
      this.ad = false;
      this.H.i = true;
      this.t = System.currentTimeMillis();
      return;
    } 
    if (this.au != null) {
      if (paramInt == -7)
        this.ap = true; 
      return;
    } 
    switch (paramInt) {
      case 49:
        if (this.L == 4) {
          this.L++;
          break;
        } 
        if (this.L == 5) {
          this.H.j = !this.H.j;
          this.L = 0;
          break;
        } 
        this.L = 0;
        if (this.H.j) {
          h();
          this.Z = this.A = null;
          this.e = null;
          this.ab = true;
          if (--this.d < 0)
            this.d = 21; 
          this.H.t = this.d + 1;
          Thread.yield();
          a();
        } 
        break;
      case 51:
        if (this.H.j) {
          h();
          this.Z = this.A = null;
          this.e = null;
          this.ab = true;
          if (++this.d > 21)
            this.d = 0; 
          this.H.t = this.d + 1;
          Thread.yield();
          a();
        } 
        break;
      case 55:
        if (this.L == 0 || this.L == 2) {
          this.L++;
          break;
        } 
        this.L = 0;
        break;
      case 52:
        if (this.L == 1) {
          this.L++;
        } else {
          this.L = 0;
        } 
        this.b[2] = 1;
        return;
      case 48:
        if (this.H.j)
          this.ap = true; 
        break;
      case 35:
        if (this.H.j)
          this.e.l(); 
        break;
      case 42:
        if (this.H.j)
          this.e.b(); 
        break;
      case -7:
      case -6:
        this.H.C = true;
        h();
        this.H.i = true;
        this.H.d(3);
        this.H.b(false);
        return;
      case 50:
        if (this.L == 3) {
          this.L++;
        } else {
          this.L = 0;
        } 
      case 53:
        this.b[0] = 1;
        return;
      case 54:
        this.b[3] = 1;
        return;
    } 
    switch (getGameAction(paramInt)) {
      case 1:
        this.b[0] = 1;
        break;
      case 6:
        this.b[1] = 1;
        break;
      case 2:
        this.b[2] = 1;
        break;
      case 5:
        this.b[3] = 1;
        break;
    } 
  }
  
  public void keyReleased(int paramInt) {
    if (this.e == null)
      return; 
    if (this.au != null)
      return; 
    switch (paramInt) {
      case 50:
      case 53:
        this.b[0] = 2;
        return;
      case 52:
        this.b[2] = 2;
        return;
      case 54:
        this.b[3] = 2;
        return;
    } 
    switch (getGameAction(paramInt)) {
      case 1:
        this.b[0] = 2;
        break;
      case 6:
        this.b[1] = 2;
        break;
      case 2:
        this.b[2] = 2;
        break;
      case 5:
        this.b[3] = 2;
        break;
    } 
  }
  
  protected void keyRepeated(int paramInt) {}
  
  public void a(int paramInt1, int paramInt2) {
    if (this.b[paramInt1] == 1) {
      this.e.c(paramInt2);
    } else if (this.b[paramInt1] == 2) {
      this.e.b(paramInt2);
    } 
  }
  
  public void c() {
    a(0, 8);
    a(1, 4);
    a(2, 1);
    a(3, 2);
    if (this.au != null) {
      if (this.b[5] != 0) {
        e();
        this.b[5] = 0;
      } 
      if (this.b[4] == 1) {
        this.au = null;
        this.H.b(false);
        this.b[4] = 0;
      } 
    } 
  }
  
  private void a(int paramInt1, int paramInt2, boolean paramBoolean1, boolean paramBoolean2) {
    if (paramBoolean1) {
      this.ai = paramInt1 >> 1;
    } else if (paramInt1 < 88) {
      this.ai = 44;
    } else {
      this.ai = this.Z.ae - 88 >> 1;
    } 
    if (paramBoolean2) {
      this.aq = paramInt2 >> 1;
    } else if (paramInt2 < 89) {
      this.aq = 44;
    } else {
      this.aq = this.Z.a - 89 >> 1;
    } 
  }
  
  private void e() {
    this.av = a(this.r[this.G], 0, 0);
    this.T = 80;
  }
  
  protected Image a(String paramString, int paramInt1, int paramInt2) {
    if (paramInt1 == 0 || paramInt2 == 0) {
      paramInt1 = this.Y[this.G][2];
      paramInt2 = this.Y[this.G][3];
    } 
    Image image = null;
    for (byte b = 0; b < this.m.length; b++) {
      if (this.m[b] == null) {
        image = this.m[b] = DirectUtils.createImage(paramInt1, paramInt2, -1);
        break;
      } 
      if (this.m[b].getWidth() == paramInt1 && this.m[b].getHeight() == paramInt2) {
        image = this.m[b];
        break;
      } 
    } 
    if (image == null)
      image = DirectUtils.createImage(paramInt1, paramInt2, 16777215); 
    Graphics graphics = image.getGraphics();
    graphics.setFont(this.F);
    graphics.setColor(16777215);
    graphics.fillRoundRect(0, 0, paramInt1 - 1, paramInt2 - 1, 4, 4);
    graphics.setColor(0);
    graphics.drawRoundRect(0, 0, paramInt1 - 1, paramInt2 - 1, 4, 4);
    i.a(graphics, paramString, 4, 4, paramInt1 - 8, paramInt2 - 8, this.F, 1, true, true);
    return image;
  }
  
  public void g() {
    for (int i = this.B.length - 1; i >= 0; i--) {
      if (this.B[i] != 0) {
        byte b = this.B[i] = (byte)(this.B[i] - 1);
        if (b == 10)
          this.Z.R[this.J[i]][this.v[i]] = 105; 
        if (this.B[i] == 5)
          this.Z.R[this.J[i]][this.v[i]] = 106; 
        if (this.B[i] == 0)
          this.Z.R[this.J[i]][this.v[i]] = 0; 
      } 
    } 
  }
  
  public int a(int paramInt, boolean paramBoolean) {
    switch (this.ao[paramInt]) {
      case 0:
        return paramBoolean ? 2 : 32;
      case 1:
        return paramBoolean ? 1 : 16;
      case 2:
        return paramBoolean ? 2 : 24;
    } 
    return 0;
  }
  
  public int b(int paramInt, boolean paramBoolean) {
    switch (this.ao[paramInt]) {
      case 0:
        return paramBoolean ? 2 : 32;
      case 1:
        return paramBoolean ? 1 : 16;
      case 2:
        return paramBoolean ? 1 : 11;
    } 
    return 0;
  }
  
  public void d() {
    int i = this.e.D - this.e.J - 1;
    int j = this.e.i - this.e.J - 1;
    int k = i + this.e.d + 1;
    int m = j + this.e.u + 1;
    for (byte b = 0; b < this.j; b++) {
      byte b1 = this.ao[b];
      int n = this.f[b][0];
      int i1 = this.f[b][1];
      int i2 = this.k[b][0];
      int i3 = this.k[b][1];
      int i4 = this.ag[b][0];
      int i5 = this.ag[b][1];
      byte b2 = this.s[b][0];
      byte b3 = this.s[b][1];
      if (b1 == 0 || b1 == 2) {
        int i6 = a(b, false);
        int i7 = b(b, false);
        byte b4 = (b2 > 0) ? 1 : -1;
        int i8 = Math.abs(b2);
        int i9 = i1 * 16 + i4;
        int i10 = n * 16 + i5;
        int i11 = i9 + i6;
        int i12 = i10 + i7;
        int i13 = (i3 - i1) * 16;
        int i14 = (i2 - n) * 16;
        for (byte b5 = 0; b5 < i8; b5++) {
          if (this.e.F && b1 == 0) {
            if (!a(i, j, k, m, i9, i10, i11, i12))
              i4 += b4; 
          } else {
            i4 += b4;
          } 
          if (i4 == 0 || i4 == i13) {
            b2 = -b2;
            b4 = -b4;
          } 
        } 
        i4 = g.a(i4, i13 + 1);
        b4 = (b3 > 0) ? 1 : -1;
        i8 = Math.abs(b3);
        for (byte b6 = 0; b6 < i8; b6++) {
          if (this.e.F && b1 == 0) {
            if (!a(i, j, k, m, i9, i10, i11, i12))
              i5 += b4; 
          } else {
            i5 += b4;
          } 
          if (i5 == 0 || i5 == i14)
            b3 = -b3; 
        } 
        i5 = g.a(i5, i14 + 1);
      } else {
        int i6 = (i2 - n) * 16;
        if (i5 == 0)
          b3 = 30; 
        if (i5 == i6)
          b3 = -40; 
        if (++b3 > 80)
          b3 = 80; 
        int i7 = a(b, false);
        int i8 = b(b, false);
        byte b4 = (b3 > 0) ? 1 : -1;
        int i9 = Math.abs(b3 / 10);
        if (i9 > 3)
          i9 = 3; 
        for (byte b5 = 0; b5 < i9; b5++) {
          if (this.e.F) {
            int i10 = i1 * 16;
            int i11 = n * 16 + i5;
            int i12 = i10 + i7;
            int i13 = i11 + i8;
            if (!a(i, j, k, m, i10, i11, i12, i13))
              i5 += b4; 
          } else {
            i5 += b4;
          } 
        } 
        i5 = g.a(i5, i6 + 1);
      } 
      this.ag[b][0] = i4;
      this.ag[b][1] = i5;
      this.s[b][0] = (byte)b2;
      this.s[b][1] = (byte)b3;
    } 
  }
  
  public void b(Graphics paramGraphics) {
    int i = this.Z.k;
    int j = this.Z.H;
    int k = i + this.Z.M;
    int m = j + this.Z.L;
    for (byte b = 0; b < this.j; b++) {
      int n = this.f[b][1];
      int i1 = this.f[b][0];
      int i2 = this.k[b][1] + a(b, true);
      int i3 = this.k[b][0] + b(b, false);
      if (a(i, j, k, m, n, i1, i2, i3)) {
        int i4 = (n - i) * 16 - this.Z.S;
        int i5 = (i1 - j) * 16 - this.Z.ak;
        byte b1 = 1;
        byte b2 = this.ao[b];
        if (b2 == 2) {
          b1 = 0;
        } else if (b2 == 1) {
          b1 = 2;
          if (this.s[b][1] > 0)
            b1 = 3; 
        } 
        paramGraphics.drawImage(this.ac[b1], i4 + this.ag[b][0], i5 + this.ag[b][1], 20);
      } 
    } 
  }
  
  public boolean a(int paramInt1, int paramInt2, int paramInt3, int paramInt4, int paramInt5, int paramInt6, int paramInt7, int paramInt8) {
    return (paramInt3 <= paramInt5) ? false : ((paramInt4 <= paramInt6) ? false : ((paramInt1 >= paramInt7) ? false : (!(paramInt2 >= paramInt8))));
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\h.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */