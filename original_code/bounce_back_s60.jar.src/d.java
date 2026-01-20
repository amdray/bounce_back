import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;

public class d {
  public boolean d = false;
  
  public int b = 0;
  
  public b a = null;
  
  public b c = null;
  
  public boolean a() {
    DataInputStream dataInputStream = null;
    try {
      dataInputStream = new DataInputStream(getClass().getResourceAsStream("res/r"));
      while (true) {
        short s = dataInputStream.readShort();
        int i = dataInputStream.readInt();
        byte b1 = dataInputStream.readByte();
        a(new b(s, i, b1));
      } 
    } catch (EOFException eOFException) {
      try {
        dataInputStream.close();
      } catch (IOException iOException) {}
    } catch (IOException iOException) {}
    this.a = this.c;
    return true;
  }
  
  public void a(b paramb) {
    if (this.c == null) {
      this.c = paramb;
      this.a = paramb;
    } else {
      this.a.a = paramb;
      this.a = this.a.a;
    } 
  }
  
  public void a(byte[] paramArrayOfbyte) {
    while (this.a.d == this.b) {
      switch (this.a.b) {
        case 8:
          paramArrayOfbyte[0] = this.a.c;
          break;
        case 4:
          paramArrayOfbyte[1] = this.a.c;
          break;
        case 1:
          paramArrayOfbyte[2] = this.a.c;
          break;
        case 2:
          paramArrayOfbyte[3] = this.a.c;
          break;
        case 20:
          paramArrayOfbyte[4] = 1;
          break;
        case 21:
          paramArrayOfbyte[5] = this.a.c;
          break;
      } 
      this.a = this.a.a;
      if (this.a == null) {
        this.d = true;
        break;
      } 
    } 
    this.b++;
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\d.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */