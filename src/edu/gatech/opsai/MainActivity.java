package edu.gatech.opsai;

import edu.gatech.opsai.ServerManager;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.MediaRouter;
import android.media.MediaRouter.RouteInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder;
import android.text.format.Formatter;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity
{
	private Button rButton, kButton;
	private TextView ipTV;
	private ServerManager s;

	@Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        MediaRouter mr = (MediaRouter)getSystemService(Context.MEDIA_ROUTER_SERVICE);
        RouteInfo ri = mr.getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_AUDIO);
        D(ri.getName().toString());
        rButton = (Button) findViewById(R.id.runserver);
        kButton = (Button) findViewById(R.id.killserver);
        ipTV = (TextView) findViewById(R.id.iptextview);
        doBindService();

        rButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
            	String result = "Running the server :)";
            	Toast.makeText(getApplicationContext(), result, Toast.LENGTH_LONG).show();
            	if (s.isServerRunning())  {
            		Toast.makeText(getApplicationContext(), "Server is currently running.\nKilling the server.", Toast.LENGTH_LONG).show();
            		s.killServer();
            	}
            	s.startServer();
            }
        });
        
        kButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
            	if (s.isServerRunning())  {
            		Toast.makeText(getApplicationContext(), "Server is currently running.\nKilling the server.", Toast.LENGTH_LONG).show();
            		s.killServer();
            	} else {
            		Toast.makeText(getApplicationContext(), "Server is not currently running.\nNothing to kill", Toast.LENGTH_LONG).show();
            	}
            }
        });
        WifiManager wifiMgr = (WifiManager) getSystemService(WIFI_SERVICE);
        WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
        int ip = wifiInfo.getIpAddress();
        @SuppressWarnings("deprecation")
		String ipAddress = Formatter.formatIpAddress(ip);
        ipTV.setText("Address: " + ipAddress + "\nType \"vncviewer " + ipAddress + ":5901 -compresslevel 9 -viewonly\"");
    }
    

    void doBindService() {
		bindService(new Intent(this, ServerManager.class), mConnection, Context.BIND_AUTO_CREATE);
	}
    
    private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder binder) {
			s = ((ServerManager.MyBinder) binder).getService();
		}
		public void onServiceDisconnected(ComponentName className) {
			s = null;
		}
	};
    
    public void D(String x) {
    	final String LOG_TAG = "OPSAI - MainActivity";
    	Log.d(LOG_TAG, x);
    }
}
