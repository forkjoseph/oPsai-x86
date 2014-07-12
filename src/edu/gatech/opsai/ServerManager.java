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
//	public static String SOCKET_ADDRESS = "org.onaips.vnc.gui";
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
			String droidvncserver_exec="/system/lib/opsaivncserver.so";
			File f=new File (droidvncserver_exec);
			if (!f.exists())
			{
				String e="Error! Could not find daemon file, " + droidvncserver_exec;
				showTextOnScreen(e);
				log(e);
				return;
			}
			
			
			Runtime.getRuntime().exec("chmod 777 " + droidvncserver_exec);
 
			String permission_string="chmod 777 " + droidvncserver_exec;
			String server_string= droidvncserver_exec;
 
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
			log("Starting " + droidvncserver_exec);

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
		c = new CommandLineUtil();
		String result = c.runCommand("pgrep -o /system/lib/opsaivncserver.so");
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
		}
	}
	
	public boolean isServerRunning() {
		c = new CommandLineUtil();
		String result = c.runCommand("pgrep -o /system/lib/opsaivncserver.so");
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
		/*	try {
				server = new DatagramSocket(13131);
				log("Listening...");

				while (!finished) {
					DatagramPacket answer = new DatagramPacket(new byte[1024],
							1024);
					server.receive(answer);

					String resp = new String(answer.getData());
					resp = resp.substring(0, answer.getLength());

					log("RECEIVED " + resp);  

					if (resp.length() > 5
							&& resp.substring(0, 6).equals("~CLIP|")) {
						resp = resp.substring(7, resp.length() - 1);
						ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);

						clipboard.setText(resp.toString());
					} else if (resp.length() > 6
							&& resp.substring(0, 6).equals("~SHOW|")) {
						resp = resp.substring(6, resp.length() - 1);
						showTextOnScreen(resp);
					} else if (resp.length() > 15
							&& (resp.substring(0, 15).equals("~SERVERSTARTED|") || resp
									.substring(0, 15).equals("~SERVERSTOPPED|"))) {
						Intent intent = new Intent("org.onaips.vnc.ACTIVITY_UPDATE");
						sendBroadcast(intent);
					} 
					else if (preferences.getBoolean("notifyclient", true)) {
						if (resp.length() > 10
								&& resp.substring(0, 11).equals("~CONNECTED|")) {
							resp = resp.substring(11, resp.length() - 1);
							showClientConnected(resp);
						} else if (resp.length() > 13
								&& resp.substring(0, 14).equals(
								"~DISCONNECTED|")) {
							showClientDisconnected();
						}
					} else {
						log("Received: " + resp);
					}
				}
			} catch (IOException e) {
				log("ERROR em SOCKETLISTEN " + e.getMessage());
			}*/
		}
	}



	@Override
	public void onDestroy() {
		super.onDestroy();
		//showTextOnScreen("Droid VNC server service killed...");
	}

	static void writeCommand(OutputStream os, String command) throws Exception {
		os.write((command + "\n").getBytes("ASCII"));
	}

	public void showTextOnScreen(final String t) {
		handler.post(new Runnable() {

			public void run() {
				// TODO Auto-generated method stub
				Toast.makeText(getApplicationContext(), t, Toast.LENGTH_LONG)
				.show();
			}
		});
	}



	
}