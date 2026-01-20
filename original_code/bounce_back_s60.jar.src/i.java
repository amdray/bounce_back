import com.nokia.mid.ui.DirectGraphics;
import com.nokia.mid.ui.DirectUtils;
import com.nokia.mid.ui.FullCanvas;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Font;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

public class i extends FullCanvas implements Runnable {
  protected f u;
  
  protected int D = 250;
  
  protected int W = 0;
  
  protected int K = 0;
  
  protected int v = 0;
  
  protected int L = 0;
  
  public String[] z;
  
  protected int[] o;
  
  protected int[] k;
  
  protected boolean y = false;
  
  public boolean p = false;
  
  public boolean A = false;
  
  public int X;
  
  protected CommandListener w;
  
  public Command R;
  
  public Command d;
  
  public Command m;
  
  public Command B;
  
  public Command l;
  
  public boolean F;
  
  public int e;
  
  private CrystalMidlet H;
  
  public byte g;
  
  public byte V;
  
  private String J;
  
  public boolean aa;
  
  public String I;
  
  public String U;
  
  public int j;
  
  public int x;
  
  public int ab;
  
  public String[] q;
  
  public boolean G;
  
  private int O = 88;
  
  private int Y = 30;
  
  private int S;
  
  private int n;
  
  private int h;
  
  private int t;
  
  public int T;
  
  public boolean a;
  
  private boolean P;
  
  public Image E;
  
  public Image f;
  
  public Image C;
  
  public Image M;
  
  public Image Q;
  
  public Image b;
  
  public int r;
  
  public int i;
  
  public int N;
  
  public Font Z;
  
  public Font c;
  
  public int s;
  
  public i(int paramInt1, int paramInt2, int paramInt3) {
    a(paramInt1, paramInt2, paramInt3);
  }
  
  protected void a(int paramInt1, int paramInt2, int paramInt3) {
    if (paramInt1 > 0) {
      this.y = true;
      this.D = paramInt1;
    } 
    this.p = (paramInt2 > 0);
    this.A = false;
    this.W = paramInt2;
    this.v = paramInt3;
    this.K = this.W;
    this.L = this.v;
    this.F = false;
    this.e = 0;
  }
  
  public void a(int paramInt) {
    if (this.p || this.A) {
      if (this.p) {
        this.p = false;
        this.W = 0;
        repaint();
        serviceRepaints();
      } 
    } else {
      int j = getGameAction(paramInt);
      if (paramInt == -6)
        j = 8; 
      if (paramInt == -7 && this.m != null) {
        this.H.b(10);
        this.w.commandAction(this.m, (Displayable)this);
      } 
      switch (j) {
        case 2:
        case 5:
          if (this.l != null)
            this.w.commandAction(this.l, (Displayable)this); 
          break;
        case 1:
          if (this.B != null)
            this.w.commandAction(this.B, (Displayable)this); 
          if (this.z != null)
            this.X = (this.X > 0) ? (this.X - 1) : (this.z.length - 1); 
          a(true);
          b(true);
          repaint();
          serviceRepaints();
          break;
        case 6:
          if (this.B != null)
            this.w.commandAction(this.B, (Displayable)this); 
          if (this.z != null)
            this.X = (this.X < this.z.length - 1) ? (this.X + 1) : 0; 
          a(false);
          b(false);
          repaint();
          serviceRepaints();
          break;
        case 8:
          this.H.b(9);
          if (this.L > 0) {
            this.R = this.d;
            this.A = true;
            this.v = this.L;
            a();
            break;
          } 
          if (this.d != null)
            this.w.commandAction(this.d, (Displayable)this); 
          break;
      } 
    } 
  }
  
  public void setCommandListener(CommandListener paramCommandListener) {
    this.w = paramCommandListener;
  }
  
  protected final void paint(Graphics paramGraphics) {
    if (this.p || this.A) {
      if (this.p) {
        d(paramGraphics, this.W);
        if (this.W <= 0)
          this.p = false; 
      } else {
        b(paramGraphics, this.v);
      } 
    } else {
      b(paramGraphics);
      if (this.y)
        a(paramGraphics); 
      if (this.z != null)
        for (int j = this.z.length - 1; j >= 0; j--) {
          if (j != this.X) {
            c(paramGraphics, j);
          } else {
            a(paramGraphics, this.X);
          } 
        }  
    } 
  }
  
  protected void showNotify() {
    if (this.r != 0) {
      a();
    } else {
      this.v = this.L;
      this.A = false;
      a();
      if (this.W > 0) {
        this.p = true;
      } else {
        repaint();
        serviceRepaints();
      } 
    } 
  }
  
  public void run() {
    if (!this.p && !this.A) {
      b();
    } else if (this.p) {
      c(this.W--);
    } else {
      d(this.v--);
    } 
    repaint();
    serviceRepaints();
    if (this.v <= 0 && this.A) {
      c();
      if (this.R != null)
        this.w.commandAction(this.R, (Displayable)this); 
    } 
  }
  
  public void a(String[] paramArrayOfString) {
    this.z = paramArrayOfString;
    this.o = new int[this.z.length];
    this.k = new int[this.z.length];
    for (int j = this.z.length - 1; j >= 0; j--) {
      this.o[j] = e(j);
      this.k[j] = b(j);
    } 
    this.X = 0;
  }
  
  public void addCommand(Command paramCommand) {
    switch (paramCommand.getCommandType()) {
      case 4:
        this.d = paramCommand;
        break;
      case 2:
      case 7:
        this.m = paramCommand;
        break;
      case 8:
        this.B = paramCommand;
        break;
      case 1:
        this.l = paramCommand;
        break;
    } 
  }
  
  public static final boolean a(Graphics paramGraphics, String paramString, int paramInt1, int paramInt2, int paramInt3, int paramInt4, Font paramFont, int paramInt5, boolean paramBoolean) {
    return a(paramGraphics, paramString, paramInt1, paramInt2, paramInt3, paramInt4, paramFont, paramInt5, paramBoolean, false);
  }
  
  public static final boolean a(Graphics paramGraphics, String paramString, int paramInt1, int paramInt2, int paramInt3, int paramInt4, Font paramFont, int paramInt5, boolean paramBoolean1, boolean paramBoolean2) {
    int j = 0;
    int k = paramFont.getHeight();
    int m = 0;
    while (j < paramInt4 && m < paramString.length()) {
      int i1 = 0;
      int n = i1;
      StringBuffer stringBuffer = new StringBuffer("");
      while (i1 < paramInt3 && m < paramString.length() && paramString.charAt(m) != '\n') {
        stringBuffer.append(paramString.charAt(m));
        i1 += paramFont.charWidth(paramString.charAt(m));
        if (paramString.charAt(m) == ' ')
          n = m; 
        m++;
      } 
      if (m >= paramString.length() || paramString.charAt(m) == '\n') {
        paramGraphics.drawString(stringBuffer.toString(), paramInt1 + (paramBoolean2 ? (paramInt3 >> 1) : 0), paramInt2 + j, paramBoolean2 ? (0x10 | 0x1) : (0x10 | 0x4));
        if (m < paramString.length() && paramString.charAt(m) == '\n')
          m++; 
      } else if (n > 0 && paramBoolean1) {
        paramGraphics.drawString(stringBuffer.toString().substring(0, stringBuffer.toString().length() - m - n), paramInt1 + (paramBoolean2 ? (paramInt3 >> 1) : 0), paramInt2 + j, paramBoolean2 ? (0x10 | 0x1) : (0x10 | 0x4));
        m = n + 1;
      } else {
        paramGraphics.drawString(stringBuffer.toString().substring(0, stringBuffer.toString().length() - 1), paramInt1 + (paramBoolean2 ? (paramInt3 >> 1) : 0), paramInt2 + j, paramBoolean2 ? (0x10 | 0x1) : (0x10 | 0x4));
        m--;
      } 
      j += k + paramInt5;
      if (m >= paramString.length())
        return false; 
    } 
    return true;
  }
  
  public i(CrystalMidlet paramCrystalMidlet, String[] paramArrayOfString, byte paramByte1, String paramString1, String paramString2, String paramString3, byte paramByte2, boolean paramBoolean) {
    this(50, 22, 22);
    this.H = paramCrystalMidlet;
    c(paramBoolean);
    this.H.a(paramBoolean ? 8 : 9, 10);
    if (paramString2 != null)
      this.m = new Command("Back", 2, 1); 
    this.d = new Command("", 4, 1);
    if (paramBoolean)
      this.r = 150; 
    this.P = false;
    this.Z = Font.getFont(64, 1, 0);
    this.c = Font.getFont(64, 0, 8);
    a(paramArrayOfString, paramByte1, paramString1, paramString2, paramString3, paramByte2);
    if (paramBoolean)
      this.W += 150; 
  }
  
  public i(CrystalMidlet paramCrystalMidlet, String[] paramArrayOfString, byte paramByte, String paramString1, String paramString2, String paramString3) {
    this(paramCrystalMidlet, paramArrayOfString, paramByte, paramString1, paramString2, paramString3, (byte)0, false);
  }
  
  public void a(String[] paramArrayOfString, byte paramByte1, String paramString1, String paramString2, String paramString3, byte paramByte2) {
    this.J = paramString3;
    this.g = paramByte1;
    this.I = a(paramString1);
    this.U = a(paramString2);
    if (paramString2 == null) {
      this.m = null;
    } else if (this.m == null) {
      this.m = new Command("Back", 2, 1);
    } 
    this.T = 0;
    this.h = 0;
    this.j = 0;
    this.x = 0;
    this.S = 0;
    this.n = -150;
    if (paramArrayOfString != null) {
      if (paramArrayOfString.length < 9) {
        int j = 9 - paramArrayOfString.length >> 1;
        this.O = 88 + j * 0;
        this.Y = 30 + j * 18;
      } else {
        this.O = 88;
        this.Y = 30;
      } 
      a(paramArrayOfString);
    } else {
      this.z = null;
    } 
    if (this.g == 0 && !this.H.i)
      this.X++; 
    this.aa = false;
    if (this.z == null) {
      this.p = false;
      this.A = false;
    } else {
      this.W = 22;
      this.v = 22;
      this.K = this.W;
      this.L = this.v;
      this.p = true;
      this.A = false;
    } 
    this.V = paramByte2;
    if (this.V == 1) {
      this.a = false;
    } else {
      this.a = true;
    } 
    a();
  }
  
  public void a(String[] paramArrayOfString1, String[] paramArrayOfString2, byte paramByte, String paramString1, String paramString2, String paramString3) {
    this.ab = 0;
    this.J = paramString3;
    this.g = paramByte;
    this.I = a(paramString1);
    this.U = a(paramString2);
    if (paramString2 == null) {
      this.m = null;
    } else if (this.m == null) {
      this.m = new Command("Back", 2, 1);
    } 
    this.O = 88;
    this.Y = 30;
    this.j = 0;
    this.x = 0;
    this.q = paramArrayOfString1;
    this.z = paramArrayOfString2;
    this.aa = true;
    this.p = false;
    this.A = false;
    this.G = false;
    this.V = 2;
    this.a = false;
    a();
  }
  
  protected void b() {
    if (this.t < 18) {
      this.t++;
    } else {
      this.t = 0;
    } 
  }
  
  protected void c(int paramInt) {
    if (this.r > 0)
      return; 
    if (paramInt == 12 || paramInt == 22)
      this.h = 0; 
    if (paramInt > 12) {
      this.n += 11 + this.h;
      if ((22 - paramInt) % 2 == 1)
        this.h += 2; 
    } else if (paramInt <= 12 && paramInt > 6) {
      this.n -= 3 - this.h;
      if (12 - paramInt == 0 || 12 - paramInt == 2)
        this.h++; 
    } else if (paramInt <= 6) {
      this.n += 3 - this.h;
      if (6 - paramInt == 2 || 6 - paramInt == 4)
        this.h--; 
    } 
  }
  
  protected void d(int paramInt) {
    if (paramInt == 22) {
      this.h = 0;
      this.n = 0;
    } 
    this.n += 1 + this.h;
    if ((22 - paramInt) % 2 == 1)
      this.h += 2; 
  }
  
  public void keyPressed(int paramInt) {
    switch (paramInt) {
      case 48:
        if (this.s == 2) {
          this.s++;
          break;
        } 
        this.s = 0;
        break;
      case 49:
        if (this.s == 1) {
          this.s++;
          break;
        } 
        this.s = 0;
        break;
      case 51:
        if (this.s == 3) {
          this.s++;
          break;
        } 
        if (this.s == 6) {
          this.H.s = 0;
          if (this.g == 2)
            this.H.a(); 
          this.H.d(1);
          break;
        } 
        this.s = 0;
        break;
      case 52:
        if (this.s == 5) {
          this.s++;
          break;
        } 
        this.s = 0;
        break;
      case 54:
        if (this.s == 4) {
          this.s++;
          break;
        } 
        this.s = 0;
        break;
      case 57:
        if (this.s == 0) {
          this.s++;
          break;
        } 
        this.s = 0;
        break;
      default:
        this.s = 0;
        break;
    } 
    if (this.V == 0 || this.p || this.A) {
      if (this.r == 0) {
        a(paramInt);
      } else if (this.H.y) {
        this.H.p[8].stop();
      } 
    } else {
      int j = getGameAction(paramInt);
      if (paramInt == -6)
        j = 8; 
      if (paramInt == -7 && this.m != null) {
        this.X = -1;
        this.H.b(10);
        this.w.commandAction(this.m, (Displayable)this);
      } 
      switch (this.V) {
        case 1:
          switch (j) {
            case 8:
              if (this.d != null)
                this.w.commandAction(this.d, (Displayable)this); 
              break;
            case 1:
              if (this.X > 0) {
                this.x += 18;
                this.X--;
                this.T--;
              } 
              break;
            case 6:
              if (this.X < this.z.length - 9) {
                this.X++;
                this.x -= 18;
                this.T++;
              } 
              break;
          } 
          break;
        case 3:
          switch (j) {
            case 1:
              if (this.z != null)
                this.X = (this.X > 0) ? (this.X - 1) : (this.z.length - 1); 
              a(true);
              break;
            case 6:
              if (this.z != null)
                this.X = (this.X < this.z.length - 1) ? (this.X + 1) : 0; 
              a(false);
              break;
            case 8:
              this.H.b(9);
              if (this.d != null)
                this.w.commandAction(this.d, (Displayable)this); 
              break;
          } 
          break;
        case 2:
          switch (j) {
            case 6:
              if (this.G)
                this.x -= 20; 
              break;
            case 1:
              if (this.x != 0)
                this.x += 20; 
              break;
            case 8:
              if (this.d != null)
                this.w.commandAction(this.d, (Displayable)this); 
            case 5:
              if (this.ab < this.q.length - 1) {
                this.ab++;
                this.x = 0;
                this.G = false;
                break;
              } 
              if (this.q.length != 1) {
                this.ab = 0;
                this.x = 0;
                this.G = false;
              } 
              break;
            case 2:
              if (this.ab != 0) {
                this.ab--;
                this.x = 0;
                this.G = false;
                break;
              } 
              if (this.q.length != 1) {
                this.ab = this.q.length - 1;
                this.x = 0;
                this.G = false;
              } 
              break;
          } 
          break;
      } 
    } 
  }
  
  protected int e(int paramInt) {
    return 88;
  }
  
  protected int b(int paramInt) {
    return this.Y + paramInt * 18;
  }
  
  protected void a(Graphics paramGraphics) {}
  
  protected void b(Graphics paramGraphics) {
    paramGraphics.setFont(this.c);
    DirectGraphics directGraphics = DirectUtils.getDirectGraphics(paramGraphics);
    paramGraphics.setColor(30702);
    paramGraphics.fillRect(0, 0, 176, 208);
    paramGraphics.drawImage(this.C, 0, 0, 20);
    if (this.V != 2 && this.g != 9 && this.g != 10) {
      paramGraphics.drawImage(this.M, 0, this.i, 20);
      directGraphics.drawImage(this.M, 176 - this.N, this.i, 20, 8192);
    } 
    if (this.V == 2) {
      int j = paramGraphics.getClipY();
      paramGraphics.setClip(paramGraphics.getClipX(), 24, paramGraphics.getClipWidth(), paramGraphics.getClipHeight());
      paramGraphics.setColor(16777215);
      this.G = a(paramGraphics, this.q[this.ab], 2, 24 + this.x, 172, 70 - this.x, paramGraphics.getFont(), 2, true);
      paramGraphics.setClip(0, 0, 176, 208);
    } 
    if (this.J != null) {
      paramGraphics.setColor(16777215);
      paramGraphics.setFont(this.Z);
      paramGraphics.drawString(this.J, 88, 2, 17);
      paramGraphics.setFont(this.c);
    } 
    if (this.I != null) {
      paramGraphics.setColor(16777215);
      paramGraphics.drawString(this.I, 5, 190, 20);
    } 
    if (this.U != null) {
      paramGraphics.setColor(16777215);
      paramGraphics.drawString(this.U, 171, 190, 24);
    } 
    if (this.z != null && this.z.length > 9) {
      paramGraphics.drawImage(this.E, 160, 21, 20);
      directGraphics.drawImage(this.E, 160, 172, 20, 16384);
    } 
  }
  
  protected void d(Graphics paramGraphics, int paramInt) {
    paramGraphics.setFont(this.c);
    if (this.r > 0) {
      if (this.r > 75) {
        paramGraphics.setColor(0);
        paramGraphics.fillRect(0, 0, 176, 208);
        paramGraphics.drawImage(this.b, 88, 104, 0x2 | 0x1);
      } else {
        if (this.r == 75)
          this.H.b(8); 
        paramGraphics.drawImage(this.Q, 0, 0, 0x10 | 0x4);
      } 
      this.r--;
      if (this.r <= 0) {
        this.b = this.Q = null;
        this.r = 0;
        this.W = 22;
      } 
      return;
    } 
    b(paramGraphics);
    paramGraphics.setColor(16777215);
    paramGraphics.setClip(0, this.i, 176, 208);
    for (byte b = 0; b < this.z.length; b++) {
      if (this.g == 0 && b == 0 && !this.H.i) {
        paramGraphics.setColor(12303291);
      } else {
        paramGraphics.setColor(16777215);
      } 
      if (a(b, this.T, this.T + 9, this.z.length))
        if (this.g == 4) {
          paramGraphics.drawSubstring(this.z[b], 0, 2, this.o[b] + this.S + this.j - 56, this.k[b] + this.n + this.x, 20);
          paramGraphics.drawSubstring(this.z[b], 2, this.z[b].length() - 2, this.o[b] + this.S + this.j + 56, this.k[b] + this.n + this.x, 24);
        } else {
          paramGraphics.drawString(this.z[b], this.o[b] + this.S + this.j, this.k[b] + this.n + this.x, 17);
        }  
    } 
    paramGraphics.setClip(0, 0, 176, 208);
  }
  
  protected void c(Graphics paramGraphics, int paramInt) {
    if (!a(paramInt, this.T, this.T + 9, this.z.length))
      return; 
    if (this.g == 0 && paramInt == 0 && !this.H.i) {
      paramGraphics.setColor(12303291);
    } else {
      paramGraphics.setColor(16777215);
    } 
    if (this.g == 4) {
      paramGraphics.drawSubstring(this.z[paramInt], 0, 2, this.o[paramInt] + this.j - 56, this.k[paramInt] + this.x, 20);
      paramGraphics.drawSubstring(this.z[paramInt], 2, this.z[paramInt].length() - 2, this.o[paramInt] + this.j + 56, this.k[paramInt] + this.x, 24);
    } else {
      paramGraphics.drawString(this.z[paramInt], this.o[paramInt] + this.j, this.k[paramInt] + this.x, 17);
    } 
  }
  
  protected void b(Graphics paramGraphics, int paramInt) {
    d(paramGraphics, paramInt);
  }
  
  protected void a(Graphics paramGraphics, int paramInt) {
    if (this.a) {
      paramGraphics.setColor(187);
      paramGraphics.fillRect(18, this.k[paramInt] + this.x + -3, 139, 18);
      paramGraphics.setColor(6710886);
      paramGraphics.drawRect(18, this.k[paramInt] + this.x + -3, 139, 18);
      switch (this.t) {
        case 0:
        case 1:
        case 17:
        case 18:
          paramGraphics.setColor(16768256);
          break;
        case 2:
        case 3:
        case 15:
        case 16:
          paramGraphics.setColor(16763904);
          break;
        case 4:
        case 5:
        case 13:
        case 14:
          paramGraphics.setColor(16759552);
          break;
        case 6:
        case 7:
        case 11:
        case 12:
          paramGraphics.setColor(16750848);
          break;
        case 8:
        case 9:
        case 10:
          paramGraphics.setColor(16746496);
          break;
      } 
    } else {
      paramGraphics.setColor(16777215);
    } 
    if (this.g == 4) {
      paramGraphics.drawSubstring(this.z[paramInt], 0, 2, this.o[paramInt] + this.j - 56, this.k[paramInt] + this.x, 20);
      paramGraphics.drawSubstring(this.z[paramInt], 2, this.z[paramInt].length() - 2, this.o[paramInt] + this.j + 56, this.k[paramInt] + this.x, 24);
    } else {
      paramGraphics.drawString(this.z[paramInt], this.o[paramInt] + this.j, this.k[paramInt] + this.x, 17);
    } 
  }
  
  public void a(Command paramCommand) {
    this.R = paramCommand;
    this.A = true;
    this.v = this.L;
  }
  
  public void a(boolean paramBoolean) {
    if (this.g == 0 && this.X == 0 && !this.H.i)
      if (paramBoolean) {
        this.X = this.z.length - 1;
        if (this.g == 0)
          return; 
        this.T = this.z.length - 9;
        this.j = -this.T * 0;
        this.x = -this.T * 18;
      } else {
        this.X++;
        this.T = 0;
        this.j = 0;
        this.x = 0;
      }  
  }
  
  public void b(boolean paramBoolean) {
    if (!a(this.X, this.T, this.T + 9, this.z.length))
      if (paramBoolean) {
        if (this.g == 0)
          return; 
        if (this.X == this.z.length - 1) {
          this.T = this.z.length - 9;
          this.j = -this.T * 0;
          this.x = -this.T * 18;
        } else {
          this.T--;
          this.j += 0;
          this.x += 18;
        } 
      } else if (this.X == 0) {
        this.T = 0;
        this.j = 0;
        this.x = 0;
      } else {
        this.T++;
        if (this.T > this.z.length - 9) {
          this.T = this.z.length - 9;
          return;
        } 
        this.j -= 0;
        this.x -= 18;
      }  
  }
  
  private boolean a(int paramInt1, int paramInt2, int paramInt3, int paramInt4) {
    if (paramInt3 > paramInt4) {
      int j = paramInt3 - paramInt4;
      if (paramInt1 >= paramInt2 && paramInt1 < paramInt4)
        return true; 
      if (paramInt1 < j)
        return true; 
    } else {
      return (paramInt1 >= paramInt2 && paramInt1 < paramInt3);
    } 
    return false;
  }
  
  private void c(boolean paramBoolean) {
    c c = new c("/res/im");
    byte[] arrayOfByte = c.a();
    if (paramBoolean)
      this.Q = Image.createImage(arrayOfByte, 0, arrayOfByte.length); 
    arrayOfByte = c.a();
    if (paramBoolean)
      this.b = Image.createImage(arrayOfByte, 0, arrayOfByte.length); 
    arrayOfByte = c.a();
    this.C = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
    arrayOfByte = c.a();
    this.M = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
    arrayOfByte = c.a();
    this.E = Image.createImage(arrayOfByte, 0, arrayOfByte.length);
    this.f = DirectUtils.createImage(this.E.getWidth(), this.E.getHeight(), 0);
    DirectGraphics directGraphics = DirectUtils.getDirectGraphics(this.f.getGraphics());
    directGraphics.drawImage(this.E, 0, 0, 20, 16384);
    this.i = this.C.getHeight();
    this.N = this.M.getWidth();
  }
  
  public void hideNotify() {
    c();
    if (this.r == 0) {
      this.p = false;
      this.A = false;
      this.W = 0;
      this.L = this.v = 0;
    } else if (this.H.y && this.H.p[8] != null) {
      this.H.p[8].stop();
    } 
  }
  
  public void a() {
    if (!this.P) {
      if (this.y && this.u == null)
        this.u = new f(this, this.D); 
      this.P = true;
    } 
  }
  
  public void d() {
    if (this.r < 0) {
      c();
    } else if (this.H.y && this.H.p[8] != null) {
      this.H.p[8].stop();
    } 
  }
  
  public void c() {
    if (this.P) {
      if (this.u != null) {
        this.u.cancel();
        this.u = null;
      } 
      this.P = false;
    } 
  }
  
  public String a(String paramString) {
    if (paramString == null)
      return null; 
    int j = 84;
    int k = this.c.stringWidth(paramString);
    if (k >= j) {
      j -= this.c.stringWidth("...");
      int m;
      for (m = paramString.length() - 1; m > 0 && this.c.substringWidth(paramString, 0, m) >= j; m--);
      paramString = paramString.substring(0, m) + "...";
    } 
    return paramString;
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\i.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */