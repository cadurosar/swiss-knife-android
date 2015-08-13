package cidre.distancebounding.swissknifetageightbits;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.Serializable;

public class Word implements Serializable {

    public static final int NBYTES = 32;
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

    public int toInt() {
        int value = 0;
        for (int i = 0; i < this.value.length; i++) {
            value += (this.value[i] & 0xffL) << (8 * i);
        }
        return value & 0xFF;
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

    public static Word concat(Word a, Word b) throws IOException {
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        outputStream.write(a.getBytes());
        outputStream.write(b.getBytes());

        byte c[] = outputStream.toByteArray();
        return new Word(c);
    }

    public Word isolateByte(int position) {
        return new Word(new Byte[]{value[position]});
    }

    public void storeByte(Word b, int position) {
        value[position] = b.getBytes()[0];
    }
}
