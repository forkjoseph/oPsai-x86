package edu.gatech.opsai;

import edu.gatech.opsai.ServerManager;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.MediaRouter;
import android.media.MediaRouter.RouteInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends Activity
{
	private Button button;
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        MediaRouter mr = (MediaRouter)getSystemService(Context.MEDIA_ROUTER_SERVICE);
        RouteInfo ri = mr.getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_AUDIO);
        D(ri.getName().toString());
        button = (Button) findViewById(R.id.runserver);
        doBindService();

        button.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
            	String result = "Running the server :)";
            	Toast.makeText(getApplicationContext(), result, Toast.LENGTH_LONG).show();
            	if (s.isServerRunning()) 
            		s.killServer();
            	s.startServer();

            }
        });
    }
    
	private ServerManager s = null;

    void doBindService() {
		bindService(new Intent(this, ServerManager.class), mConnection,
				Context.BIND_AUTO_CREATE);
	}
    private ServiceConnection mConnection = new ServiceConnection() {

		public void onServiceConnected(ComponentName className, IBinder binder) {

			s = ((ServerManager.MyBinder) binder).getService();
		}

		public void onServiceDisconnected(ComponentName className) {
			s = null;
		}
	};
    
    private final String LOG_TAG = "OPSAI - MainActivity";
    public void D(String x) {
    	Log.d(LOG_TAG, x);
    }
}
