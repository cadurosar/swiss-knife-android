package cidre.distancebounding.swissknifereadereightbits;

import java.util.Arrays;

public class Reader extends Device {

    private Word c;
    public int bitsTransmitted = 0;
    public long transmissionTime[] = new long[32];
    private Word tagReceived = null;
    private int errc, errr, errt;
    private static final long MAXTIME = 70500000;
    private boolean preLastPhase = true;
    public static long timeNV1 = 0;

    public Word tagNearby() {
        return sendFirstNounce();
    }

    private Word sendFirstNounce() {

        nA = Protocol.generateNonce();
        timeNV1 = System.nanoTime();
        return nA;
    }

    public Word dealWithMessage(Word message) {
        if (nB == null)
            firstPhase(message);
        if (!transmissionEnded && message.getBytes().length <= 32)
            return rapidBitExchange(message);
        else
            return lastPhase(message);

    }

    private Word lastPhase(Word message) {
        if (preLastPhase) {
            preLastPhase = false;
            return new Word(new byte[]{0x00});
        } else {
//            System.out.println(message.getBytes().length);
            receivedCApostrophe(new Word(Arrays.copyOfRange(message.getBytes(), 0,
                    Word.NBYTES)));
            receivedTb(new Word(Arrays.copyOfRange(message.getBytes(), Word.NBYTES,
                    Word.NBYTES * 2)));
            return null;
        }
    }

    private void receivedTb(Word message) {
        tb = message;
        closeConnection();
        searchForXAndId();
//	MainActivity.toScreen(">>>>>> STATISTICS ABOUT TRANSMISSION");
//	new Statistics(transmissionTime).printStatistics();
    }

    private void closeConnection() {
        MainActivity.running = false;
    }

    private void searchForXAndId() {
        for (int i = 0; i < id.length; i++) {
            MainActivity.toScreen("Testing number" + i);
            Word generatedTb = Protocol.generateTB(x[i], tagReceived, id[i],
                    nA, nB);
            String out = " " + x[i];
            out += " " + id[i];
            out += " " + tagReceived;
            out += " " + nA;
            out += " " + nB;
            out += " " + tb;
            //System.out.println(out + " " + generatedTb);
            MainActivity.toScreen(out + " " + generatedTb);
            if (tb.equals(generatedTb)) {
                verifyErrors(i);
                break;
            } else
                MainActivity.toScreen("NO!");
        }

    }

    private void verifyErrors(int code) {
        a = Protocol.generateA(CB, nB);
        z = Protocol.generateZ(this.a, Device.x[code]);
        preCalc = Protocol.generatePreCalc(z);
//        System.out.println(received);
        errc = 0;
        errt = 0;
        errr = 0;
        for (int j = 0; j < 32; j++) {
            Word cApostropheJ = tagReceived.isolateByte(j);
            if (!c.isolateByte(j).equals(cApostropheJ))
                errc++;
            else {
                Word preCalcj = new Word(preCalc[j]);
                if (!preCalcj.isolateByte(cApostropheJ.toInt()).equals(
                        received.isolateByte(j)))
                    errr++;
                else if (transmissionTime[j] > Reader.MAXTIME)
                    errt++;
            }
        }
        MainActivity.toScreen("Connection errors: " + errc);
        MainActivity.toScreen("Response errors: " + errr);
        MainActivity.toScreen("Time errors: " + errt);
        MainActivity.toScreen("Did we authenticate it? "
                + ((errc + errt + errr) < Protocol.THRESHOLD));
    }

    private void receivedCApostrophe(Word message) {
        tagReceived = message;
    }

    private Word rapidBitExchange(Word message) {
        if (c == null) {
            c = Protocol.generateC();
        } else {
            if (bitsTransmitted < 33)
                storeByte(message, bitsTransmitted-1);
        }
        if (bitsTransmitted < 32)
            return sendByte(c, bitsTransmitted);
        return new Word(new byte[]{0x00});
    }

    private void firstPhase(Word message) {
        nB = message;
    }
}
