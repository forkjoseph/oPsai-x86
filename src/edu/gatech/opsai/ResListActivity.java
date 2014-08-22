package edu.gatech.opsai;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.ListActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

public class ResListActivity extends Activity {
	private ServerManager s;

	@Override
	  protected void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	    setContentView(R.layout.listview);
        doBindService();

	    final ListView listview = (ListView) findViewById(R.id.list);
	    Intent intent = getIntent();    
		HashMap<Integer, String> display = (HashMap<Integer, String>) intent.getSerializableExtra("DISPLAY");

	    final ArrayList<String> list = new ArrayList<String>();
	    
		for(Map.Entry<Integer, String> e : display.entrySet()) {
			list.add("Display " + e.getKey() + " at " + e.getValue());
		}
	    final StableArrayAdapter adapter = new StableArrayAdapter(this,
	        android.R.layout.simple_list_item_1, list);
	    listview.setAdapter(adapter);

	    listview.setOnItemClickListener(new AdapterView.OnItemClickListener() {

	      @Override
	      public void onItemClick(AdapterView<?> parent, final View view,final int position, long id) {
	        final String item = (String) parent.getItemAtPosition(position);
	        view.animate().setDuration(2000).alpha(0).withEndAction(new Runnable() {
	              @Override
	              public void run() {      
	            	CommandLineUtil c = new CommandLineUtil();
	    			String server_running_string = "/system/lib/./opsaitcpserver.so";
	            	String result = c.runCommand(server_running_string + " " + (position));
	            	D("Result: " + result);

	            	
	            	server_running_string = "/system/lib/./opsaivncserver.so &";
	            	s.startServer();
//	                adapter.notifyDataSetChanged();
	                view.setAlpha(1);
	              }
	        });
	      }

	    });
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

	private class StableArrayAdapter extends ArrayAdapter<String> {

		HashMap<String, Integer> mIdMap = new HashMap<String, Integer>();

		public StableArrayAdapter(Context context, int textViewResourceId,
				List<String> objects) {
			super(context, textViewResourceId, objects);
			for (int i = 0; i < objects.size(); ++i) {
				mIdMap.put(objects.get(i), i);
			}
		}

		@Override
		public long getItemId(int position) {
			String item = getItem(position);
			return mIdMap.get(item);
		}

		@Override
		public boolean hasStableIds() {
			return true;
		}

	}
	
	private void D(String x){
        Log.d("OPSAI - RESLISTACTIVITY", x);
	}

}