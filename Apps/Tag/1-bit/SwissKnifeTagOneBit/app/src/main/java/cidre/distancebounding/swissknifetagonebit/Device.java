package cidre.distancebounding.swissknifetagonebit;

public abstract class Device {
    protected Word nA;
    protected Word nB;
    protected Word a;
    protected Word[] z;
    protected static final Word CB = new Word(new Byte[]{(byte) 0x10, 0x20, 0x30, (byte) 0x40});
    protected Word tb;
    protected static Word[] x,xXor;
    protected static Word[] id;
    protected final Word received = new Word(new Byte[Word.NBYTES]);
    protected byte[] byteReceived = new byte[32];

    protected void storeBit(byte b, int position) {
        received.storeBit(b, position);
    }

    protected byte[] sendBit(Word b, int position) {
        byte[] bitToSend = b.isolateBitToSend(position);
        //Log.i("HCEEMO", "Sending bit " + position + "/" + Word.NBITS);
        return bitToSend;
    }

    protected static void initialize() {
        int z = Word.NBITS;
        Device.x = new Word[] {new Word (new byte[] {0x10,0x20,0x30,0x40})};
        Device.xXor = new Word[] {new Word (new byte[] {0x40,0x30,0x20,0x10})};
        Device.id = new Word[] {new Word (new byte[] {0x1})};
    }
}
