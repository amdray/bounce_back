import java.io.IOException;
import java.io.InputStream;

public class c {
  private String b;
  
  private int f;
  
  private int[] a;
  
  private short[] i;
  
  private int d;
  
  private int c;
  
  private int g;
  
  private InputStream e;
  
  public byte[] h;
  
  public c(String paramString) {
    this.b = paramString;
    try {
      this.e = getClass().getResourceAsStream(this.b);
      this.f = (this.e.read() << 8) + this.e.read();
      this.a = new int[this.f];
      this.i = new short[this.f];
      int i = (this.f + 1) * 2;
      this.g = i;
      for (byte b = 0; b < this.f; b++) {
        this.a[b] = i;
        this.i[b] = (short)((this.e.read() << 8) + this.e.read());
        i += this.i[b];
      } 
      this.c = this.g;
    } catch (Exception exception) {}
  }
  
  public byte[] a() {
    try {
      short s = this.i[this.d];
      this.h = new byte[s];
      for (int i = 0; i < s; i += this.e.read(this.h, i, s - i));
      this.c += s;
      this.d++;
      return this.h;
    } catch (IOException iOException) {
      return null;
    } 
  }
  
  public final byte[] c(int paramInt) {
    a(paramInt);
    return a();
  }
  
  public void a(int paramInt) {
    try {
      if (paramInt > this.d) {
        int i = this.a[paramInt] - this.c;
        b(i);
        this.d = paramInt;
      } else if (paramInt < this.d) {
        this.e.close();
        this.e = getClass().getResourceAsStream(this.b);
        int i = this.a[paramInt];
        this.c = 0;
        b(i);
        this.d = paramInt;
      } 
    } catch (IOException iOException) {}
  }
  
  private final void b(int paramInt) throws IOException {
    for (long l = this.e.skip(paramInt); l < paramInt; l += this.e.skip(paramInt - l));
    this.c += paramInt;
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\c.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */