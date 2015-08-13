package cidre.distancebounding.swissknifereaderonebit;

import java.io.Serializable;import java.lang.Byte;import java.lang.Math;import java.lang.Object;import java.lang.Override;import java.lang.String;import java.lang.StringBuilder;

public class Word implements Serializable {

    public static final int NBITS = 32;
    public static final int NBYTES = (int) Math.ceil(NBITS / 8.0);
    /**
     *
     */
    private static final long serialVersionUID = -8756608360612978424L;

    private final Byte[] value;

    public Word(Byte[] value) {
        this.value = value;
    }

    public byte[] getBytes() {
        byte[] ret = new byte[value.length];
        for (int i = 0; i < value.length; i++)
            ret[i] = value[i];
        return ret;
    }

    public Word(byte[] bytes) {
        Byte[] ret = new Byte[bytes.length];
        for (int i = 0; i < bytes.length; i++)
            ret[i] = bytes[i];
        this.value = ret;
    }

    public void storeBit(byte[] b, int position) {
        int index = 3 - position/8;
        if (value[index] == null)
            value[index] = (byte) (b[0] << (position % 8));
        else value[index] = (byte) (value[index] | (b[0] << (position % 8)));
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder(value.length * 2);
        for (byte b : value)
            sb.append(String.format("%02x", b & 0xff));
        return sb.toString();
    }

    public Word xor(Word other) {
        Byte[] ret = new Byte[this.value.length];
        for (int i = 0; i < this.value.length; i++) {
            ret[i] = byteXOR(this.value[i], other.value[i]);
        }
        return new Word(ret);
    }

    public long toLong() {
        long value = 0;
        for (int i = 0; i < this.value.length; i++) {
            value += (this.value[i] & 0xffL) << (8 * i);
        }
        return value;
    }

    private static byte byteXOR(byte a, byte b) {
        return (byte) (a ^ b);
    }

    public byte[] isolateBitToSend(int position) {
        int index = (position / 8);
//        System.out.println(index);
        return new byte[]{(byte) ((value[index] >> (31-position) % 8) & 0x01)};
    }


    public Word isolateBit(int position) {
        int index = (position / 8);
        return new Word(
                new Byte[]{(byte) ((value[index] >> (position) % 8) & 0x01)});
    }

    public int toInt() {
        int value = 0;
        for (int i = 0; i < this.value.length; i++) {
            value += (this.value[i] & 0xffL) << (8 * i);
        }
        return value;
    }

    @Override
    public boolean equals(Object comparison) {
        Word obj = (Word) comparison;
        if (obj.value.length != this.value.length) return false;
        for (int i = 0; i < obj.value.length; i++) {
            if (!value[i].equals(obj.value[i])) return false;
        }
        return true;
    }

    public byte[][] generateCByte(){
        byte[][] result = new byte[32][1];
        for(int position = 0; position < 32;position++)
            result[position][0] = this.isolateBitToSend(position)[0];
        return result;
    }
}
