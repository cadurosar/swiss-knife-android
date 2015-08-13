package cidre.distancebounding.swissknifereaderonebit;

import java.util.Arrays;

public class Reader extends Device {

    private Word c;
    private byte[][] cByte;
    public int bitsTransmitted = 0;
    public long transmissionTime[] = new long[Word.NBITS];
    private Word tagReceived = null;
    private int errc, errr, errt;
    private static final long MAXTIME = 70500000;
    private boolean preLastPhase = true;
    public static long timeNV1 = 0;

    public byte[] tagNearby() {
        return sendFirstNounce();
    }

    private byte[] sendFirstNounce() {

        nA = Protocol.generateNonce();
        timeNV1 = System.nanoTime();
        return nA.getBytes();
    }

    public byte[] dealWithMessage(byte[] message) {
        if (nB == null)
            firstPhase(message);
        if (!transmissionEnded && message.length < 8)
            return rapidBitExchange(message);
        else
            return lastPhase(message);

    }

    private byte[] lastPhase(byte[] message) {
        if(preLastPhase) {
            preLastPhase = false;
            return new byte[]{0x00};
        } else {
//            System.out.println(message.getBytes().length);
            receivedCApostrophe(new Word(Arrays.copyOfRange(message, 0,
                    Word.NBYTES)));
            receivedTb(new Word(Arrays.copyOfRange(message, Word.NBYTES,
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
        z = Protocol.generateZ(this.a, Device.xXor[code]);
//        System.out.println(received);
        errc = 0;
        errt = 0;
        errr = 0;
        byte[] receivedBytes = tagReceived.getBytes();
        Word receivedCheckErrors = new Word(new byte[]{receivedBytes[3],receivedBytes[2],receivedBytes[1],receivedBytes[0]});

        for (int j = 0; j < Word.NBITS; j++) {
            Word cApostropheJ = receivedCheckErrors.isolateBit(j);
            if (!c.isolateBit(j).equals(cApostropheJ))
                errc++;
            else {
                if (!z[cApostropheJ.toInt()].isolateBit(j).equals(
                        received.isolateBit(j)))
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

    private byte[] rapidBitExchange(byte[] message) {
        if (bitsTransmitted == 0) {
            c = Protocol.generateC();
            cByte = c.generateCByte();
        } else {
            if(bitsTransmitted < 33)
                storeBit(message, 32-bitsTransmitted);
        }
        if (bitsTransmitted < Word.NBITS) {
            if (bitsTransmitted == Word.NBITS) {
                transmissionEnded = true;
            }
            return cByte[bitsTransmitted];
        }
        return new byte[] {0x00};
    }

    private void firstPhase(byte[] message) {
        nB = new Word(message);
    }

}
