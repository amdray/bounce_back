import java.util.Timer;
import java.util.TimerTask;

public class f extends TimerTask {
  private Runnable b;
  
  private Timer a;
  
  f(Runnable paramRunnable, int paramInt) {
    this.b = paramRunnable;
    this.a = new Timer();
    this.a.schedule(this, 0L, paramInt);
  }
  
  public void run() {
    this.b.run();
  }
  
  void a() {
    if (this.a == null)
      return; 
    cancel();
    this.a.cancel();
    this.a = null;
  }
}


/* Location:              C:\Users\a.shmonin\Downloads\bounce_back_s60.jar!\f.class
 * Java compiler version: 1 (45.3)
 * JD-Core Version:       1.1.3
 */