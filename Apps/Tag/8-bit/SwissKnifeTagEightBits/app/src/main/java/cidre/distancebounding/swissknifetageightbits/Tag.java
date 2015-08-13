package cidre.distancebounding.swissknifetageightbits;

import java.io.IOException;

public class Tag extends Device {

    private int bitsReceived;
    private int code;

    public int getBitsReceived(){
        return bitsReceived;
    }

    public Tag() {
//        code = new Random().nextInt(5);
        code = 0;
//        System.out.println("My code is " + code);
//        initialize();
    }

    public Word dealWithMessage(Word message) throws IOException {
        if (nA == null) return firstPhase(message);
        else return rapidBitExchange(message);
    }

    private Word lastPhase() throws IOException {
        tb = Protocol.generateTB(x[code], received, id[code], nA, nB);
//        System.out.print(x[code]);
//        System.out.print(" " + id[code]);
//        System.out.print(" " + received);
//        System.out.print(" " + nA);
//        System.out.print(" " + nB);
//        System.out.println(" " + tb);
        return Word.concat(received,tb);
    }

    private Word rapidBitExchange(Word message) throws IOException {
        Word lastReceivedBit = message.isolateByte(0);
        if(bitsReceived != 32) {
            storeByte(lastReceivedBit, bitsReceived);
            return new Word(new byte[] {preCalc[bitsReceived++][lastReceivedBit.toInt()]});
        }
        else
            return lastPhase();
        }

    private Word firstPhase(Word message) {
        nA = message;
        nB = Protocol.generateNonce();
        a = Protocol.generateA(CB, nB);
        z = Protocol.generateZ(a, x[code]);
        preCalc = Protocol.generatePreCalc(z);
        return nB;
    }
}
