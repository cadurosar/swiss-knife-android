package cidre.distancebounding.swissknifereadereightbits;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.nfc.NfcAdapter;
import android.nfc.Tag;
import android.nfc.tech.IsoDep;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.Vibrator;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;


public class MainActivity extends ListActivity {

    private static final boolean ISSHORTCIRCUIT = true;
    private static final int TOTALEXECUTIONS = 20;

    private static final byte[] APDU = new byte[]{0x00, (byte) 0xA4, 0x04, 0x00,
            0x07, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00};
    private static final byte[] END = new byte[]{(byte) 0xf0, (byte) 0xde, 0x00};
    private static final Word HELLO = new Word("Hello PC".getBytes());

    public static ArrayAdapter<String> adapter;
    public static ArrayList<String> listItems = new ArrayList<String>();
    public static boolean running = false;

    private long timeTagNearby = 0;
    private long[] timesBeforeSend = new long[32];
    private long[] timesAfterSend = new long[32];
    private static int executions = 0;
    NfcAdapter mAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mAdapter = NfcAdapter.getDefaultAdapter(this);
        if (mAdapter == null) {
            finish();
            return;
        }
        ListView listview = (ListView) findViewById(android.R.id.list);
        adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1,
                listItems);
        setListAdapter(adapter);
        resolveIntent(getIntent());

    }

    public static void toScreen(String a) {
        //listItems.add(a);
        //adapter.notifyDataSetChanged();
    }


    public static void toScreen2(String a) {
        listItems.add(a);
        adapter.notifyDataSetChanged();
    }


    private void resolveIntent(Intent intent) {
        toScreen("Intent arrived, probably found tag");
        timeTagNearby = System.nanoTime();
        Parcelable tag = intent.getParcelableExtra(NfcAdapter.EXTRA_TAG);
        if (tag == null)
            return;
        IsoDep tagA = IsoDep.get((Tag) tag);
        byte[] received;
        try {
            tagA.connect();
            tagA.setTimeout(90000);
            System.out.println("Start");
            Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
            // Vibrate for 100 milliseconds
            v.vibrate(100);
            Word result = null;

            for (int i = 0; (i < TOTALEXECUTIONS && !ISSHORTCIRCUIT) || (i == 0 && ISSHORTCIRCUIT); i++) {
                result = new Word(tagA.transceive(APDU));

                if (!ISSHORTCIRCUIT) {
                    if (!result.equals(HELLO)) {
                        toScreen("Incorrect response to APDU");
                        break;
                    }
                    Reader r = new Reader();
                    running = true;
                    long timeBeforeSend = System.nanoTime();
                    Word nA = r.tagNearby();
                    result = new Word(tagA.transceive(nA.getBytes()));
                    long timeAfterSend = System.nanoTime();
                    byte[] toSend;

                    Word wordToSend = r.dealWithMessage(result);
                    while (running) {
                        toSend = wordToSend.getBytes();
                        if (toSend.length == 1) {
                            if (r.bitsTransmitted > 0 && r.bitsTransmitted < 33) {
                                r.transmissionTime[r.bitsTransmitted - 1] = timeAfterSend - timeBeforeSend;
                                timesBeforeSend[r.bitsTransmitted - 1] = timeBeforeSend;
                                timesAfterSend[r.bitsTransmitted - 1] = timeAfterSend;
                            }
                            r.bitsTransmitted++;
                        }
                        timeBeforeSend = System.nanoTime();
                        byte[] bytesReceived = tagA.transceive(toSend);
                        timeAfterSend = System.nanoTime();
                        result = new Word(bytesReceived);
                        wordToSend = r.dealWithMessage(result);
                    }
                    try {
                        PrintWriter writer = new PrintWriter("/storage/sdcard0/SwissKnife/Test-" + executions + ".txt", "UTF-8");
                        for (int j = 0; j < 32; j++) {
                            writer.println(timesBeforeSend[j] + ","
                                    + timesBeforeSend[j] + ","
                                    + timesBeforeSend[j] + ","
                                    + timesAfterSend[j] + ","
                                    + timesAfterSend[j] + ","
                                    + timesAfterSend[j]);
                        }
                        writer.println(Reader.timeNV1 + "," + timesBeforeSend[0]);
                        writer.close();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    executions++;

                }

            }
            if (!ISSHORTCIRCUIT)
                tagA.transceive(END);

            if (ISSHORTCIRCUIT && TOTALEXECUTIONS == 1) {
                byte[] resultBytes = result.getBytes();
                boolean passed = resultBytes[0] > 0;
                toScreen("Passou = " + passed + ", Id = " + resultBytes[1] + ",Errc = " + resultBytes[2] + ", Err = " + resultBytes[3] + ", ErrT = " + resultBytes[4]);
            }
            long executionTime = System.nanoTime() - timeTagNearby;
            toScreen2("Execution time in milisseconds = " + executionTime
                    / 1000000.0);
            System.out.println("Execution time in milisseconds = " + executionTime
                    / 1000000.0);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onNewIntent(Intent intent) {
        setIntent(intent);
        resolveIntent(intent);
    }

}
