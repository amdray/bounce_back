import com.nokia.mid.sound.Sound;
import com.nokia.mid.ui.DeviceControl;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.midlet.MIDlet;
import javax.microedition.rms.RecordStore;
import javax.microedition.rms.RecordStoreException;

public class CrystalMidlet extends MIDlet implements CommandListener {
  public static final String[] r = new String[] { "Continue", "New Game", "Options", "Records", "Help", "Exit" };
  
  public static String[] z = new String[] { "Game Help", "Controls" };
  
  public static String[] E = new String[] { "Move Left: 4 \nMove Right: 6 \nJump: 2 or 5" };
  
  public static String[] l = new String[] { "Points: {0}", "Bonus: {0}", "Total: {0}" };
  
  public static String[] w = new String[] { "Levels Completed: {0}", "Score: {0}", "New Record!" };
  
  public static String[] d = new String[] { "Congratulations! You have completed the last level of Bounce Back!" };
  
  public static String[] b = new String[] { "Congratulations! You have completed all the levels of Bounce Back in one go!" };
  
  public int[] c = new int[] { 0, 0, 0, 0, 0 };
  
  public boolean C = false;
  
  public i D;
  
  public h B;
  
  public Display a;
  
  public int k = 2;
  
  public boolean i;
  
  public boolean v;
  
  public byte q = 0;
  
  public int s;
  
  public int t;
  
  public int x;
  
  public boolean o;
  
  public boolean y = true;
  
  public boolean A = true;
  
  public Sound[] p;
  
  public Sound e;
  
  public boolean g = false;
  
  public boolean f = true;
  
  public boolean m = true;
  
  public boolean u;
  
  public boolean j;
  
  public int h;
  
  public int n;
  
  public void startApp() {
    DeviceControl.setLights(0, 100);
    this.a = Display.getDisplay(this);
    this.x = 0;
    this.t = 0;
    this.s = 1;
    b();
    b(true);
  }
  
  public void pauseApp() {
    if (this.B != null)
      this.B.h(); 
    if (this.D != null)
      this.D.d(); 
    notifyPaused();
  }
  
  public void destroyApp(boolean paramBoolean) {
    d(2);
    if (this.k != 6 && this.B != null && this.B.e != null)
      d(3); 
    notifyDestroyed();
  }
  
  public void a(int paramInt) {
    for (byte b = 0; b < this.c.length; b++) {
      if (paramInt > this.c[b]) {
        int j = this.c[b];
        this.c[b] = paramInt;
        paramInt = j;
      } 
    } 
    d(2);
  }
  
  public void b(boolean paramBoolean) {
    if (this.B != null)
      this.B.h(); 
    this.C = true;
    if (this.v) {
      this.q = 1;
      this.v = false;
    } 
    this.i = (this.k == 1 || this.q == 1 || this.q == 2);
    if (this.D == null) {
      this.D = new i(this, r, (byte)0, "Select", "Exit", "Bounce Back", (byte)0, paramBoolean);
      this.D.setCommandListener(this);
      this.D.a();
    } else {
      this.D.a(r, (byte)0, "Select", "Exit", "Bounce Back", (byte)0);
    } 
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void a() {
    this.C = true;
    int j = Math.min(21, this.s);
    if (j <= 0)
      j = 21; 
    String[] arrayOfString = new String[j];
    for (byte b = 0; b < j; b++)
      arrayOfString[b] = a("Level {0}", b + 1); 
    this.D.a(arrayOfString, (byte)2, "Select", "Back", "New Game", (byte)0);
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void i() {
    this.C = true;
    this.f = this.y;
    this.m = this.A;
    String[] arrayOfString = new String[2];
    arrayOfString[0] = this.y ? "Sounds On" : "Sounds Off";
    arrayOfString[1] = "Done";
    this.D.a(arrayOfString, (byte)3, "Select", "Back", "Options", (byte)3);
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void j() {
    this.C = true;
    String[] arrayOfString = new String[this.c.length];
    for (byte b = 0; b < this.c.length; b++)
      arrayOfString[b] = (b + 1) + ".   " + this.c[b]; 
    this.D.a(arrayOfString, (byte)4, (String)null, "Back", "Records", (byte)1);
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void e() {
    if (this.B != null)
      this.B.h(); 
    if (this.k == 6)
      this.B = null; 
    this.C = true;
    if (this.D == null) {
      this.D = new i(this, z, (byte)5, "Select", "Back", "Help");
      this.D.setCommandListener(this);
      this.D.a();
    } else {
      this.D.a(z, (byte)5, "Select", "Back", "Help", (byte)0);
    } 
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void g() {
    this.C = true;
    this.D.a(E, (String[])null, (byte)6, (String)null, "Back", "Controls");
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void c() {
    this.k = 5;
    this.B.h();
    this.C = true;
    String[] arrayOfString = new String[l.length];
    int j = (1200 - (int)(this.B.g / 1000L)) * this.t;
    if (j < 0)
      j = 0; 
    int k = this.B.C / 10;
    byte b = 0;
    arrayOfString[b++] = a(l[0], this.n);
    arrayOfString[b++] = a(l[1], j + k);
    this.n += j + k;
    arrayOfString[b++] = a(l[2], this.n);
    if (this.D == null) {
      this.D = new i(this, arrayOfString, (byte)9, "OK", null, "Level complete!", (byte)1, false);
      this.D.setCommandListener(this);
      this.D.a();
    } else {
      this.D.a(arrayOfString, (byte)9, "OK", (String)null, "Level complete!", (byte)1);
    } 
    this.a.setCurrent((Displayable)this.D);
    d(1);
    d(2);
  }
  
  public void a(boolean paramBoolean) {
    String[] arrayOfString;
    byte b;
    this.C = true;
    if (paramBoolean) {
      b = 11;
      arrayOfString = b;
    } else {
      b = 12;
      arrayOfString = d;
    } 
    this.D.a(arrayOfString, (String[])null, b, "OK", (String)null, (String)null);
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void f() {
    this.k = 3;
    this.q = 0;
    d(3);
    this.B.h();
    this.C = true;
    String[] arrayOfString = new String[w.length];
    byte b = 0;
    arrayOfString[b++] = a(w[0], this.x);
    arrayOfString[b++] = a(w[1], this.n);
    arrayOfString[b++] = w[2];
    if (this.n < this.c[this.c.length - 1]) {
      arrayOfString[arrayOfString.length - 1] = "";
    } else {
      a(this.n);
    } 
    if (this.D == null) {
      this.D = new i(this, arrayOfString, (byte)10, "OK", null, "Game Over!", (byte)1, false);
      this.D.setCommandListener(this);
      this.D.a();
    } else {
      this.D.a(arrayOfString, (byte)10, "OK", (String)null, "Game Over!", (byte)1);
    } 
    this.a.setCurrent((Displayable)this.D);
  }
  
  public void a(boolean paramBoolean1, boolean paramBoolean2) {
    this.D.c();
    Thread.yield();
    if (!paramBoolean1)
      this.B = null; 
    if (paramBoolean2) {
      this.h = 3;
      this.n = 0;
    } 
    this.C = false;
    if (this.B == null)
      this.B = new h(this); 
    if (!paramBoolean1) {
      this.B.a(this.t - 1);
    } else {
      a(0, 7);
    } 
    this.a.setCurrent((Displayable)this.B);
    this.B.a();
    this.k = 1;
  }
  
  public void d() {
    if (this.B == null)
      this.B = new h(this); 
    this.B.f();
    this.C = false;
    this.D.c();
    this.a.setCurrent((Displayable)this.B);
    this.B.a();
    this.k = 1;
  }
  
  public void h() {
    this.D.c();
    Thread.yield();
    if (this.k == 1) {
      d(1);
      d(2);
      d(3);
      this.v = true;
    } 
    this.k = 6;
    this.B = null;
    this.B = new h(this);
    this.h = 3;
    this.C = false;
    this.B.a(21);
    this.B.au = new d();
    if (this.B.au.a()) {
      this.a.setCurrent((Displayable)this.B);
      this.B.a();
    } else {
      this.B.au = null;
    } 
  }
  
  public void commandAction(Command paramCommand, Displayable paramDisplayable) {
    if (paramDisplayable != this.D)
      return; 
    try {
      if (paramCommand.getCommandType() == 2) {
        switch (this.D.g) {
          case 0:
            destroyApp(true);
            break;
          case 3:
            this.y = this.f;
            this.A = this.m;
          case 2:
          case 4:
          case 5:
            b(false);
            break;
          case 6:
            e();
            break;
        } 
      } else if (paramCommand.getCommandType() == 4) {
        byte b;
        switch (this.D.g) {
          case 0:
            switch (this.D.X) {
              case 0:
                if (this.k == 1) {
                  this.B = null;
                  d();
                  break;
                } 
                if (this.q != 0 && this.k != 4) {
                  this.k = 4;
                  if (this.q == 2) {
                    this.t++;
                    a(false, false);
                    break;
                  } 
                  d();
                } 
                break;
              case 1:
                a();
                break;
              case 2:
                i();
                break;
              case 3:
                j();
                break;
              case 4:
                e();
                break;
              case 5:
                destroyApp(true);
                break;
            } 
            break;
          case 2:
            if (this.k != 4) {
              this.k = 4;
              this.t = this.D.X + 1;
              this.x = 0;
              if (this.t == 1) {
                this.o = true;
              } else {
                this.o = false;
              } 
              a(false, true);
            } 
            break;
          case 3:
            b = 3;
            if (!this.g && this.D.X > 0) {
              this.D.X++;
              b--;
            } 
            switch (this.D.X) {
              case 0:
                this.y = !this.y;
                this.D.z[0] = this.y ? "Sounds On" : "Sounds Off";
                this.D.U = "Cancel";
                break;
              case 1:
                this.A = !this.A;
                this.D.z[1] = this.A ? "Vibra On" : "Vibra Off";
                this.D.U = "Cancel";
                c(1);
                break;
              case 2:
                if (this.g) {
                  b(false);
                } else {
                  this.D.a(this.D.d);
                } 
                this.g = !this.g;
                break;
              case 3:
                this.u = !this.u;
                this.D.z[b] = "Record " + (this.u ? "On" : "Off");
                break;
            } 
            break;
          case 5:
            switch (this.D.X) {
              case 0:
                h();
                break;
              case 1:
                g();
                break;
            } 
            break;
          case 9:
            if (this.t + 1 > this.s && this.s != 0) {
              this.s = this.t + 1;
              d(1);
            } 
            this.x++;
            if (this.t == 20) {
              this.t++;
              a(this.o);
              break;
            } 
            if (this.t == 21) {
              f();
              break;
            } 
            this.t++;
            a(false, false);
            break;
          case 11:
          case 12:
            if (this.o) {
              a(false, false);
              break;
            } 
            f();
            break;
          case 10:
            b(false);
            break;
        } 
      } 
    } catch (Exception exception) {
      exception.printStackTrace();
    } 
  }
  
  public void b() {
    try {
      RecordStore recordStore = RecordStore.openRecordStore("CrystalRMS", true);
      if (recordStore.getNumRecords() != 3) {
        recordStore.addRecord(null, 0, 0);
        recordStore.addRecord(null, 0, 0);
        recordStore.addRecord(null, 0, 0);
      } else {
        byte[] arrayOfByte1 = new byte[recordStore.getRecordSize(1)];
        byte[] arrayOfByte2 = new byte[recordStore.getRecordSize(2)];
        byte[] arrayOfByte3 = new byte[recordStore.getRecordSize(3)];
        arrayOfByte1 = recordStore.getRecord(1);
        arrayOfByte2 = recordStore.getRecord(2);
        arrayOfByte3 = recordStore.getRecord(3);
        if (arrayOfByte1 != null) {
          ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(arrayOfByte1);
          DataInputStream dataInputStream = new DataInputStream(byteArrayInputStream);
          this.s = dataInputStream.readInt();
        } 
        if (arrayOfByte2 != null) {
          ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(arrayOfByte2);
          DataInputStream dataInputStream = new DataInputStream(byteArrayInputStream);
          this.y = dataInputStream.readBoolean();
          this.A = dataInputStream.readBoolean();
          for (byte b = 0; b < this.c.length; b++)
            this.c[b] = dataInputStream.readInt(); 
        } 
        if (arrayOfByte3 != null) {
          ByteArrayInputStream byteArrayInputStream = new ByteArrayInputStream(arrayOfByte3);
          DataInputStream dataInputStream = new DataInputStream(byteArrayInputStream);
          this.q = dataInputStream.readByte();
          if (this.q != 0) {
            this.t = dataInputStream.readInt() + 1;
            this.h = dataInputStream.readInt();
            this.n = dataInputStream.readInt();
          } 
        } 
      } 
      recordStore.closeRecordStore();
    } catch (Exception exception) {}
  }
  
  public void d(int paramInt) {
    ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
    DataOutputStream dataOutputStream = new DataOutputStream(byteArrayOutputStream);
    RecordStore recordStore = null;
    try {
      byte b1;
      byte b2;
      switch (paramInt) {
        case 1:
          dataOutputStream.writeInt(this.s);
          break;
        case 2:
          dataOutputStream.writeBoolean(this.y);
          dataOutputStream.writeBoolean(this.A);
          for (b1 = 0; b1 < this.c.length; b1++)
            dataOutputStream.writeInt(this.c[b1]); 
          break;
        case 3:
          b2 = 0;
          if (this.k == 1) {
            b2 = 1;
            if (this.B.e.e)
              this.B.e.e(); 
          } else if (this.k == 5) {
            b2 = 2;
          } 
          dataOutputStream.writeByte(b2);
          if (this.B != null)
            this.B.a(dataOutputStream); 
          break;
      } 
      recordStore = RecordStore.openRecordStore("CrystalRMS", true);
      recordStore.setRecord(paramInt, byteArrayOutputStream.toByteArray(), 0, byteArrayOutputStream.size());
    } catch (Exception exception) {
      try {
        recordStore.closeRecordStore();
      } catch (RecordStoreException recordStoreException) {}
    } finally {
      try {
        recordStore.closeRecordStore();
      } catch (RecordStoreException recordStoreException) {}
    } 
  }
  
  public static String a(String paramString, int paramInt) {
    int j = paramString.indexOf("{0}");
    return (j == -1) ? paramString : (paramString.substring(0, j) + paramInt + paramString.substring(j + "{0}".length()));
  }
  
  public void a(int paramInt1, int paramInt2) {
    c c = new c("/res/s");
    if (this.p == null)
      this.p = new Sound[11]; 
    for (byte b = 0; b <= paramInt2; b++) {
      try {
        byte[] arrayOfByte = c.a();
        if (b >= paramInt1 && b <= paramInt2) {
          this.p[b] = new Sound(arrayOfByte, 1);
          this.p[b].init(arrayOfByte, 1);
        } 
      } catch (IllegalArgumentException illegalArgumentException) {}
    } 
  }
  
  public void b(int paramInt) {
    boolean bool = this.y;
    if (this.D != null && this.D.g == 3 && this.D.X == 0)
      bool = !bool; 
    if (bool && this.p != null && this.p[paramInt] != null)
      try {
        if (this.e != null)
          this.e.stop(); 
        this.p[paramInt].play(1);
        this.e = this.p[paramInt];
      } catch (Exception exception) {
        this.y = false;
      }  
  }
  
  public void c(int paramInt) {}
  
  static {
    E[0] = E[0] + " \nor use the joystick.";
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\CrystalMidlet.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */