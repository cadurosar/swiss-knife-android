package cidre.distancebounding.swissknifetageightbits;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

public class Protocol {

    protected static final int THRESHOLD = 2;
    private static final String HMAC_SHA256_ALGORITHM = "HmacSHA256";

    public static byte[] hmacSha256(byte[] value, byte[] key) {
        try {
            SecretKeySpec signingKey = new SecretKeySpec(key, HMAC_SHA256_ALGORITHM);
            Mac mac = Mac.getInstance(HMAC_SHA256_ALGORITHM);
            mac.init(signingKey);
            byte[] bytes = mac.doFinal(value);
            return bytes;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public static Word generateNonce() {
        try {
            byte[] result = hmacSha256(new String("" + System.nanoTime()).getBytes(), Device.x[0].getBytes());
            Word ret = new Word(result);
   //         System.out.println("Generated nounce: " + ret);
            return ret;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;

    }

    public static Word generateA(Word cB, Word nB) {
        try {
            Word seed = Word.concat(nB, cB);
   //         System.out.println("Seed is: "+ seed);
            byte[] result = hmacSha256(seed.getBytes(), Device.x[0].getBytes());
            Word ret = new Word(result);
     //       System.out.println("Generated A: " + ret);
            return ret;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;

    }

    public static Word[] generateZ(Word a, Word x) {
        Word[] ret = {a, a.xor(x)};
        //System.out.println("Using x = : " + x);
        //System.out.println("Generated Z[0]: " + ret[0]);
        //System.out.println("Generated Z[1]: " + ret[1]);
        return ret;
    }

    public static byte[][] generatePreCalc(Word[] z) {
        byte[][] PreCalc = new byte[32][256];
        byte round = 0;
        int challenge;
        for (round = 0; round < 32; round++) {
            for (challenge = 0; challenge < 256; challenge++) {
                byte c[] = new byte[8];
                byte r[] = new byte[8];
                int number;
                byte mask[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, (byte) 0x80};
                for (number = 0; number < 8; number++) {
                    c[number] = (byte) (challenge & mask[number]);
                    if (c[number] == 0)
                        r[number] = (byte) (z[0].isolateByte(round).getBytes()[0] & mask[number]);
                    else
                        r[number] = (byte) (z[1].isolateByte(round).getBytes()[0] & mask[number]);
                }
                PreCalc[round][challenge] = 0;
                for (number = 0; number < 8; number++) {
                    PreCalc[round][challenge] |= r[number];
                }
            }
        }
        return PreCalc;
    }

    public static Word generateC() {
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            DataOutputStream dos = new DataOutputStream(baos);
            dos.writeLong(System.nanoTime());
            dos.close();
            byte[] seed = baos.toByteArray();
            Word seedWord = new Word(seed);
            byte[] result = hmacSha256(seed, Device.x[0].getBytes());
            Word ret = new Word(result);
//            System.out.println("Generated C: " + ret);
            return ret;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public static Word generateTB(Word x, Word received, Word id, Word nA,
                                  Word nB) {
        try {
            Word seed = Word.concat(nA, nB);
            seed = Word.concat(seed, id);
            seed = Word.concat(seed, received);
            byte[] result = hmacSha256(seed.getBytes(), x.getBytes());
            Word ret = new Word(result);
            return ret;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

}
