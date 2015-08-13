package cidre.distancebounding.swissknifetagonebit;

public class Tag extends Device {

    private int bitsReceived;
    private int code;
    private byte[][][] zByte;

    public int getBitsReceived(){
        return bitsReceived;
    }

    public Tag() {
//        code = new Random().nextInt(5);
        code = 0;
//        System.out.println("My code is " + code);
        initialize();
    }

    public byte[] dealWithMessage(byte[] message) {
        if (nA == null) return firstPhase(message);
        else return rapidBitExchange(message);
    }

    private byte[] lastPhase() {
        for(int i=0;i<32;i++) {
            storeBit(byteReceived[i], 31 - i);
        }
        tb = Protocol.generateTB(x[code], received, id[code], nA, nB);
//        System.out.print(x[code]);
//        System.out.print(" " + id[code]);
//        System.out.print(" " + received);
//        System.out.print(" " + nA);
//        System.out.print(" " + nB);
//        System.out.println(" " + tb);
        return concat(received.getBytes(), tb.getBytes()) ;
    }

    private byte[] rapidBitExchange(byte[] message) {
        if(bitsReceived != Word.NBITS) {
            byteReceived[bitsReceived] = (byte) (message[0] & 0x01);
            return zByte[message[0] & 0x01][bitsReceived++];
        }
        else
            return lastPhase();
        }

    private byte[] firstPhase(byte[] message) {
        nA = new Word(message);
        nB = Protocol.generateNonce();
        a = Protocol.generateA(CB, nB);
        z = Protocol.generateZ(a, xXor[code]);
        zByte = Protocol.generateZByte(z);
        return nB.getBytes();
    }

    public byte[] concat(byte[] a, byte[] b) {
        int aLen = a.length;
        int bLen = b.length;
        byte[] c= new byte[aLen+bLen];
        System.arraycopy(a, 0, c, 0, aLen);
        System.arraycopy(b, 0, c, aLen, bLen);
        return c;
    }
}
