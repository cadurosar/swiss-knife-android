package cidre.distancebounding.swissknifetageightbits;

public abstract class Device {
    protected Word nA;
    protected Word nB;
    protected Word a;
    protected Word[] z;
    protected byte[][] preCalc;
    protected static final Word CB = new Word(Protocol.hmacSha256("This is our shared secret please don't tell anyone".getBytes(), "This is my secret powerful master key".getBytes()));
    protected Word tb;
    protected static Word[] x;
    protected static Word[] xXor;
    protected static Word[] id;
    protected Word received = new Word(new Byte[Word.NBYTES]);
    protected boolean transmissionEnded = false;

    protected void storeByte(Word b, int position) {
        received.storeByte(b, position);
    }

    protected Word sendByte(Word b, int position) {
        Word bitToSend = b.isolateByte(position-1);
        if (position == 32) {
            transmissionEnded = true;
        }
        return bitToSend;
    }


    {
        Device.x = new Word[]{new Word(Protocol.hmacSha256("Carlos Eduardo Rosar Kos Lassance".getBytes(), "This is my secret powerful master key".getBytes()))};
        Device.id = new Word[]{new Word(new byte[]{0x1})};
    }
}
