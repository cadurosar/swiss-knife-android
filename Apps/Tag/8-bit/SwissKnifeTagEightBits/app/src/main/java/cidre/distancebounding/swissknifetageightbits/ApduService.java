package cidre.distancebounding.swissknifetageightbits;

import android.nfc.cardemulation.HostApduService;
import android.os.Bundle;
import android.util.Log;

import java.io.PrintWriter;
import java.util.ArrayList;

public class ApduService extends HostApduService {


    private static int executions = 0;
    private long time;
    private Tag t;
    private ArrayList<Long> processingTimesList;
    private long timesReceived[];
    private long timesSent[];

    @Override
    public byte[] processCommandApdu(byte[] apdu, Bundle extras) {
        time = System.nanoTime();
        long timereceived = time;
        if (selectAidApdu(apdu)) {
            System.out.println("Hello" +
                    "");
            processingTimesList = new ArrayList<Long>(32);
            if (t != null) {
                t = null;
                try {
                    PrintWriter writer = new PrintWriter("/sdcard/SwissKnife/Test-" + executions + ".txt", "UTF-8");
                    for (int i = 0; i < 32; i++) {
                        writer.println(timesReceived[i] + ","
                                + timesReceived[i] + ","
                                + timesReceived[i] + ","
                                + timesSent[i] + ","
                                + timesSent[i]);
                    }
                    writer.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                executions++;
            }

            return getWelcomeMessage();
        } else if (apdu.length == 3 && apdu[0] == (byte) 0xf0 && apdu[1] == (byte) 0xde && apdu[2] == 0x00) {
            try {
                PrintWriter writer = new PrintWriter("/sdcard/SwissKnife/Test-" + executions + ".txt", "UTF-8");
                for (int i = 0; i < 32; i++) {
                    writer.println(timesReceived[i] + ","
                            + timesReceived[i] + ","
                            + timesReceived[i] + ","
                            + timesSent[i] + ","
                            + timesSent[i]);
                }
                writer.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
            return getByeMessage();
        } else {
            try {
                Word received = new Word(apdu);
//                System.out.println(received);
                if (t == null) {
                    t = new Tag();
                    timesReceived = new long[32];
                    timesSent = new long[32];

                }
                Word word = t.dealWithMessage(received);
                byte[] toSend = word.getBytes();
                sendResponseApdu(toSend);

                long postSent = System.nanoTime();
                time = postSent - time;
                if (apdu.length == 1 && toSend.length == 1) {
//                processingTimesList.add(time);
                    timesReceived[t.getBitsReceived() - 1] = timereceived;
                    timesSent[t.getBitsReceived() - 1] = postSent;
//                MainActivity.listItems.add("I recieved " + received+ " and responded with "+word);
//                MainActivity.adapter.notifyDataSetChanged();
                }
                return null;
            } catch (Exception e) {
                e.printStackTrace();
            }
            return null;
        }

    }

    private byte[] getWelcomeMessage() {
        return "Hello PC".getBytes();
    }

    private byte[] getByeMessage() {
        int a = 0;
        return "Hello PD".getBytes();
    }


    @Override
    public void onDeactivated(int reason) {
        Log.i("HCEDEMO", "Deactivated: " + reason);
        long[] array = new long[processingTimesList.size()];
        for (int i = 0; i < processingTimesList.size(); i++) {
            array[i] = processingTimesList.get(i); // Watch out for NullPointerExceptions!
        }
        t = null;
    }

    private boolean selectAidApdu(byte[] apdu) {
        return apdu.length >= 2 && apdu[0] == (byte) 0 && apdu[1] == (byte) 0xa4;
    }

}
