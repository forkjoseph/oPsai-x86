package edu.gatech.opsai;

import edu.gatech.opsai.ServerManager;
import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
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
import android.media.*;

public class MainActivity extends Activity
{
	private Button rButton, kButton;
	private TextView ipTV;
	private ServerManager s;
	private NotificationManager notificationManager;


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
            	statusNotify(true);
            }
        });
        
        kButton.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
            	if (s.isServerRunning())  {
            		Toast.makeText(getApplicationContext(), "Server is currently running.\nKilling the server.", Toast.LENGTH_LONG).show();
            		s.killServer();
                	statusNotify(false);
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
    
	void statusNotify(boolean isrunning) {
        notificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        Intent intent = new Intent(this, MainActivity.class);
        PendingIntent pIntent = PendingIntent.getActivity(this, 0, intent, 0);

        if (isrunning){
	        Notification noti = new Notification.Builder(this).setContentTitle("oPsai VNC server is running")
	        		.setContentIntent(pIntent).setAutoCancel(false).build();
	//            .setContentText("Subject")
	        noti.flags |= Notification.FLAG_ONGOING_EVENT;
	        notificationManager.notify(123, noti);
        } else {
	        notificationManager.cancel(123);
        }
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
