package edu.gatech.opsai;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;
import android.util.Log;

public class WakeUpReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		//beauty of one line code XD
        context.startService((new Intent(context, ScreenService.class)).putExtra("screen_state",
        		((PowerManager) context.getSystemService(Context.POWER_SERVICE)).isScreenOn()));
	}
	
	public void D(String x) {
    	final String LOG_TAG = "OPSAI - WakeUpReceiver";
    	Log.d(LOG_TAG, x);
    }
}
