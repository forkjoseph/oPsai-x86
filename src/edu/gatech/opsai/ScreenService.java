package edu.gatech.opsai;

import java.util.List;

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
        
        sendBroadcast(intent);
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
