import com.dashoptimization.*;

public class Callback {
    public void run(String model)
    {
        try (XPRSprob problem = new XPRSprob(null)) {
            problem.setIntControl (XPRS.LPLOG, 1);
            problem.addMessageListener(DefaultMessageListener::console);

            problem.readProb (model);
            problem.optimize ();
        }
    }

    public static void main(String [] args)
    {
        String model = args.length == 0 ? "../data/callback" : args[0];
        Callback c = new Callback ();
        c.run (model);
    }
}
