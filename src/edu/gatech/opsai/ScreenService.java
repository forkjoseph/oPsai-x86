package edu.gatech.opsai;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import android.app.IntentService;
import android.content.Intent;
import android.location.Location;
import android.location.LocationManager;
import android.os.IBinder;
import android.util.Log;

public class ScreenService extends IntentService {
	private LocationManager mLocationManager;

	public ScreenService() {
		super("ScreenService");
	}

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onStart(Intent intent, int startId) { 
         super.onStart(intent, startId);
    }

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	@Override
	protected void onHandleIntent(Intent intent) {
        boolean screenOn = false;
        try{
            screenOn = intent.getBooleanExtra("screen_state", false);
        }catch(Exception e){}

        if (screenOn) {
     		D("Buddy! time to wake up XD");
        } else 
            D("Buddy! screen is off :(");
        
        Location mL = getLastKnownLocation();
        D("Location(Latitude, Longitude): (" + mL.getLatitude() + ", " + mL.getLongitude() + ")" +
        		(atGeorgiaTech(mL) ? " at Georgia Tech!" : " not at Georgia Tech :("));
        if (atGeorgiaTech(mL) && screenOn) {
        	CommandLineUtil c = new CommandLineUtil();
			String files_dir = getFilesDir().getAbsolutePath();
			Process sh;
			OutputStream os;
			String server_string = "/system/lib/opsaitcpserver.so";
			String server_running_string = "/system/lib/./opsaitcpserver.so";
			String permission_string="chmod 777 " + server_string;
			
			String result = c.runCommand(server_running_string);
			D("Run? " + (result.split("\n")[0].equalsIgnoreCase("running") 
					? " yes, It's running XD" : " no, It's not running ;__;"));
			
			if (!result.split("\n")[0].equalsIgnoreCase("running")){
//				try {
//					sh = Runtime.getRuntime().exec("su",null,new File(files_dir));
//					os = sh.getOutputStream();
//					writeCommand(os, permission_string);
//					writeCommand(os, server_string);
//				} catch (IOException e) {
//					e.printStackTrace();
//				} catch (Exception e) {
//					e.printStackTrace();
//				}
			} else {
				D(result);
				HashMap<Integer, String> display_resources = new HashMap<Integer, String>();
				Map<Integer, String> t = new TreeMap<Integer, String>(); // too lazy to sort ;_;
				int lines = result.split("\n").length;
				int num_resource;
				for(int i = 0; i < lines; i++) {
					String query = result.split("\n")[i].trim();
					if (query.startsWith("NUM"))
						num_resource = Integer.parseInt(query.substring(query.length()-1));
					else if (query.contains("~") && query.split("~")[1].startsWith("DISPLAY")){
						query = query.split("~")[1];
						query = query.split("DISPLAY")[1];
						int display_number = Integer.parseInt(query.split("=")[0]);
						String display_descp = query.split("=")[1];
						t.put(display_number, display_descp);
					}
				}
				
				for(Map.Entry<Integer, String> e : t.entrySet()) {
					D(e.getKey() + " " + e.getValue());
					display_resources.put(e.getKey(), e.getValue());
				}
				Intent i = new Intent(this, ResListActivity.class);
				i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
				i.putExtra("DISPLAY", display_resources);
				startActivity(i);
			}
        }
        sendBroadcast(intent);
	}
	
	static void writeCommand(OutputStream os, String command) throws Exception {
		os.write((command + "\n").getBytes("ASCII"));
	}
	
	@SuppressWarnings("unused")
	private boolean atGeorgiaTech(final Location loc) {
//		top left most: 33.781465, -84.407284    (y, x) ughh.... 
//		top right most: 33.781465, -84.386510
//		bottom left most: 33.766265, -84.407284
//		bottom right most: 33.766265, -84.386510
		if (loc == null) // should not happen but making sure...
			return false;
		final double x = loc.getLongitude();
		final double y = loc.getLatitude();
		
		double tlLat, tlLong, trLat, trLong, blLat, blLong, brLat, brLong;
		tlLat = trLat = 33.781465; 
		tlLong = blLong = -84.407284;
		blLat = brLat = 33.766265;
		trLong = brLong = -84.386510;
		
		return (y > blLat && y < tlLat && x > blLong && x < brLong);		
	}
	
	private Location getLastKnownLocation() {
		mLocationManager = (LocationManager) getApplicationContext().getSystemService(LOCATION_SERVICE);
		List<String> providers = mLocationManager.getProviders(true);
		Location bestLocation = null;
		for (String provider : providers) {
			Location l = mLocationManager.getLastKnownLocation(provider);
			if (l == null) 
				continue;
			if (bestLocation == null || l.getAccuracy() < bestLocation.getAccuracy())
				bestLocation = l;
		}
		
		if (bestLocation == null)  {
//			Klaus -> Lat: 33.7771489 Long: -84.3960913
			Location mL = new Location("default");
			mL.setLatitude(33.7771489);
			mL.setLongitude(-84.3960913);
			return mL;
		}
		return bestLocation;
	}
	
	private void D(String x){
        Log.d("OPSAI - SCREENSERVICE", x);
	}
}
