package edu.gatech.opsai;


import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Scanner;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;

public class ServerManager extends Service {
	SharedPreferences preferences;
	private static PowerManager.WakeLock wakeLock = null;

	boolean serverOn = false;
	SocketListener serverConnection = null;

	private String rHost = null;
	private final IBinder mBinder = new MyBinder();
	private Handler handler;
	
    private final String LOG_TAG = "OPSAI - ServerManager";
    public void log(String x) {
    	Log.d(LOG_TAG, x);
    }
    
 // We return the binder class upon a call of bindService
 	@Override
 	public IBinder onBind(Intent arg0) {
 		return mBinder;
 	}

 	public class MyBinder extends Binder {
 		ServerManager getService() {
 			return ServerManager.this;
 		}
 	}

	@Override
	public void onCreate() {
		super.onCreate();

		handler = new Handler(Looper.getMainLooper());
		preferences = PreferenceManager.getDefaultSharedPreferences(this);

		if (serverConnection != null) {
			log("ServerConnection was already active!");
		} else {
			log("ServerConnection started");
			serverConnection = new SocketListener();
			serverConnection.start();
		}

	}
 

	//for pre-2.0 devices
	@Override
	public void onStart(Intent intent, int startId) {
		handleStart();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		handleStart();
		return START_NOT_STICKY;
	}
	
	private void handleStart()
	{
		startServer();		
	}
	
	private CommandLineUtil c;

	public void startServer() {
		try {
			Process sh;
			String files_dir = getFilesDir().getAbsolutePath();
			
			//our exec file is disguised as a library so it will get packed to lib folder according to cpu_abi
			String vncserver_path = "/system/lib/opsaivncserver.so";
//			String droidvncserver_exec="/data/data/edu.gatech.opsai/lib/libopsaivncserver.so"; // for final release

			File f=new File (vncserver_path);
			if (!f.exists())
			{
				String e="Error! Could not find daemon file, " + vncserver_path;
				showTextOnScreen(e);
				log(e);
				return;
			}
			
			
			Runtime.getRuntime().exec("chmod 777 " + vncserver_path);
 
			String permission_string="chmod 777 " + vncserver_path;
			String server_string= vncserver_path;
 
			boolean root=preferences.getBoolean("asroot",true);
 
			if (root)     
			{ 
				log("Running as root...");
				sh = Runtime.getRuntime().exec("su",null,new File(files_dir));
				OutputStream os = sh.getOutputStream();
				writeCommand(os, permission_string);
				writeCommand(os, server_string);
			}
			else
			{
				log("Not running as root...");
				Runtime.getRuntime().exec(permission_string);
				Runtime.getRuntime().exec(server_string,null,new File(files_dir));
			}
			// dont show password on logcat
			log("Starting " + vncserver_path);
			showRunning();
		} catch (IOException e) {
			log("startServer():" + e.getMessage());
		} catch (Exception e) {
			log("startServer():" + e.getMessage());
		}

	}

	void startReverseConnection(String host) {
		try {
			rHost = host;

			if (isServerRunning()) {
				killServer();
				Thread.sleep(2000);
			}

			startServer();
			rHost = null;

		} catch (InterruptedException e) {
			log(e.getMessage());
		}
	}
	
	void killServer() {
		dismissRunning();
		c = new CommandLineUtil();
		String result = c.runCommand("pgrep -o /system/lib/opsaivncserver.so");
//		String result = c.runCommand("pgrep -o /data/data/edu.gatech.opsai/lib/libopsaivncserver.so"); // for final release

		log("Trying to kill current process");
		String PID = result;
		log("Killing " + PID);
		DataOutputStream os = null;
		try {
			Process rootProcess = Runtime.getRuntime().exec("su");
			os = new DataOutputStream(rootProcess.getOutputStream());
			os.writeBytes("kill " + PID + "\n");
			os.writeBytes("exit\n");
			os.flush();
		} catch (IOException e) {
			e.printStackTrace();
		}finally {
			try {
				os.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			dismissRunning();
		}
	}
	
	public boolean isServerRunning() {
		c = new CommandLineUtil();
		String result = c.runCommand("pgrep -o /system/lib/opsaivncserver.so"); // for DEBUG
//		String result = c.runCommand("pgrep -o /data/data/edu.gatech.opsai/lib/libopsaivncserver.so"); // for final release

		if (result.equals("")) {
			log("Process is not running currently");
			return false;
		} else {
			log("Process is running currently");
			return true;
		}
	}

	class SocketListener extends Thread {
		DatagramSocket server = null;
		boolean finished = false;

		public void finishThread() {
			finished = true;
		}

		@Override
		public void run() {
		}
	}



	@Override
	public void onDestroy() {
		super.onDestroy();
	}

	static void writeCommand(OutputStream os, String command) throws Exception {
		os.write((command + "\n").getBytes("ASCII"));
	}

	public void showTextOnScreen(final String t) {
		handler.post(new Runnable() {
			public void run() {
				Toast.makeText(getApplicationContext(), t, Toast.LENGTH_LONG).show();
			}
		});
	}
	
	@SuppressWarnings("deprecation")
	public void showRunning() {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);

		int icon = R.drawable.opsai;
		CharSequence tickerText = "oPsai VNC Server is currently running";
		long when = System.currentTimeMillis();

		Notification notification = new Notification(icon, tickerText, when);

		Context context = getApplicationContext();
		CharSequence contentTitle = "oPsai VNC Server";
		CharSequence contentText = "oPsai VNC Server daemon is currently running";
		Intent notificationIntent = new Intent(this, MainActivity.class);
		PendingIntent contentIntent = PendingIntent.getActivity(getApplicationContext(), 0, notificationIntent, 0);

		notification.setLatestEventInfo(context, contentTitle, contentText,	contentIntent);
		notification.flags = Notification.FLAG_ONGOING_EVENT;
		mNotificationManager.notify(5901, notification);
	}
	
	void dismissRunning() {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
		mNotificationManager.cancel(5901);
	}
	
	
	// not sure yet .... 
	public void showClientConnected(String c) {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);

		int icon = R.drawable.opsai;
		CharSequence tickerText = c + " connected to VNC server";
		long when = System.currentTimeMillis();

		Notification notification = new Notification(icon, tickerText, when);

		Context context = getApplicationContext();
		CharSequence contentTitle = "oPsai VNC Server";
		CharSequence contentText = "Client Connected from " + c;
		Intent notificationIntent = new Intent();
		PendingIntent contentIntent = PendingIntent.getActivity(
				getApplicationContext(), 0, notificationIntent, 0);

		notification.setLatestEventInfo(context, contentTitle, contentText,
				contentIntent);

		mNotificationManager.notify(5901, notification);

		// lets see if we should keep screen on
		if (preferences.getBoolean("screenturnoff", false)) {
			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
			wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "VNC");
			wakeLock.acquire();
		}
	}

	void showClientDisconnected() {
		String ns = Context.NOTIFICATION_SERVICE;
		NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
		mNotificationManager.cancel(5901);

//		if (wakeLock != null && wakeLock.isHeld())
//			wakeLock.release();
	}
	
}