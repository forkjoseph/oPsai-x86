package edu.gatech.opsai;


import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import edu.gatech.opsai.ServerManager;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder;
import android.text.format.Formatter;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity
{
	private Button rButton, kButton, cButton; 
	private TextView ipTV;
	private EditText alsaET;
	private ServerManager s;
	private BroadcastReceiver mBroadcastReceiver;

	@Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        rButton = (Button) findViewById(R.id.runserver);
        kButton = (Button) findViewById(R.id.killserver);
        ipTV = (TextView) findViewById(R.id.iptextview);
        
        cButton = (Button) findViewById(R.id.changealsa);
        alsaET = (EditText) findViewById(R.id.alsa);
        
        doBindService();

        // manual debug mode
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
        
        cButton.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View arg0) {
				if (alsaET.getText() == null) {
					try {
						throw new Exception();
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
				
				CommandLineUtil c = new CommandLineUtil();
				String result = c.runCommand("pgrep -o mediaserver");
				File f = new File("/storage/emulated/legacy/tinyalsa");
				String s;
				BufferedReader reader;
				BufferedWriter writer = null;
				
				try {
					reader = new BufferedReader(new FileReader(f));
					D("Current IP address:");
					while((s = reader.readLine()) != null) {
						D(s); 
					}
					// modifying 
					writer = new BufferedWriter(new FileWriter(f, false));
					writer.write(alsaET.getText().toString());
				} catch (FileNotFoundException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				} finally {
					if (writer != null) {
						try {
							writer.close();
						} catch (IOException e) {
							e.printStackTrace();
						}
					}
					D("Done with reading");
				}
				
				DataOutputStream os = null;
				try {
					D("Killing the processor");
					Process rootProcess = Runtime.getRuntime().exec("su");
					os = new DataOutputStream(rootProcess.getOutputStream());
					os.writeBytes("kill -9 " + result + "\n");
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
        });
        
        WifiManager wifiMgr = (WifiManager) getSystemService(WIFI_SERVICE);
        WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
        int ip = wifiInfo.getIpAddress();
        @SuppressWarnings("deprecation")
		String ipAddress = Formatter.formatIpAddress(ip);
        ipTV.setText("Address: " + ipAddress + "\nType \"vncviewer " + ipAddress + ":5901 -compresslevel 9 \"");
        sensorDebug(); 
    }
	
	public void sensorDebug() {
		SensorManager sm = (SensorManager)getSystemService(SENSOR_SERVICE);
		List<Sensor> l = sm.getSensorList(Sensor.TYPE_ALL);
		
		for(Sensor s : l) 
			D(s.toString());
		
	}
	
	protected void onResume() {
		super.onResume();
		IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_ON);
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		mBroadcastReceiver = new WakeUpReceiver();
        registerReceiver(mBroadcastReceiver, filter);
	}
	
	@Override
	protected void onStop() {
		super.onStop();
	}
	
	protected void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mBroadcastReceiver);
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
