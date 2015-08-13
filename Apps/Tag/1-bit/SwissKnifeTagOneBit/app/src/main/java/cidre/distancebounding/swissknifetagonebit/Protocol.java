package cidre.distancebounding.swissknifetagonebit;

import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

public class Protocol {

    protected static final int THRESHOLD = 2;
    private static final String HMAC_SHA1_ALGORITHM = "HmacSHA1";


    private static byte[] hmacSha1(byte[] value, byte[] key)
            throws UnsupportedEncodingException, NoSuchAlgorithmException,
            InvalidKeyException {
        SecretKeySpec signingKey = new SecretKeySpec(key, HMAC_SHA1_ALGORITHM);
        Mac mac = Mac.getInstance(HMAC_SHA1_ALGORITHM);
        mac.init(signingKey);
        byte[] bytes = mac.doFinal(value);
        return bytes;
    }

    public static Word generateNonce()  {
        try {
            byte[] result = hmacSha1(new String(""+ System.nanoTime()).getBytes(), Device.x[0].getBytes());
            Word ret = new Word(new byte[]{result[0],result[1],result[2],result[3]});
//            System.out.println("Generated nounce: " + ret);
            return ret;
        } catch(Exception e){
            e.printStackTrace();
        }
        return null;

    }

    public static Word generateA(Word cB, Word nB) {
        try {
            byte[] CB = cB.getBytes();
            byte[] NB = nB.getBytes();
            byte[] seed = {CB[3],CB[2], CB[1], CB[0], NB[3],NB[2],NB[1],NB[0]};
            Word seedWord = new Word(seed);
//            System.out.println(seedWord);
            byte[] result = hmacSha1(seed, Device.x[0].getBytes());
            Word resultWord = new Word(result);
  //          System.out.println(resultWord);
            Word ret = new Word(new byte[]{result[0],result[1],result[2],result[3]});
    //        System.out.println("Generated A: " + ret);
            return ret;
        } catch(Exception e){
            e.printStackTrace();
        }
        return null;

    }

    public static Word[] generateZ(Word a, Word x) {
        Word[] ret = {a, a.xor(x)};
  //      System.out.println("Using x = : " + x);
    //    System.out.println("Generated Z[0]: " + ret[0]);
      //  System.out.println("Generated Z[1]: " + ret[1]);
        return ret;
    }

    public static Word generateC() {
        byte[] bytes = new byte[Word.NBYTES];
        Word ret = new Word(bytes);
//        System.out.println("Generated C: " + ret);
        return ret;
    }

    public static Word generateTB(Word x, Word received, Word id, Word nA,
                                  Word nB) {
        try {
            byte[] tb = new byte[Word.NBYTES];
            byte[] bytesNa = nA.getBytes();
            byte[] bytesNb = nB.getBytes();
            byte[] bytesC = received.getBytes();
            byte[] bytesX = x.getBytes();

            byte[] seed = {bytesNa[0], bytesNa[1], bytesNa[2], bytesNa[3],
                    bytesNb[0], bytesNb[1], bytesNb[2], bytesNb[3],
                    0x00, 0x00, 0x00, id.getBytes()[0],
                    bytesC[0], bytesC[1], bytesC[2], bytesC[3],
                    bytesX[0], bytesX[1], bytesX[2], bytesX[3]};
            byte[] result = hmacSha1(seed, Device.x[0].getBytes());
            Word ret = new Word(new byte[]{result[0], result[1], result[2], result[3]});
            return ret;
        } catch(Exception e){
            e.printStackTrace();
        }
        return null;
    }

    public static byte[][][] generateZByte(Word z[]){
        byte[][][] result = new byte[2][32][1];
        for(int i=0;i<2;i++){
            for(int j=0;j<32;j++){
                result[i][j] = z[i].isolateBitToSend(j);
            }
        }
        return result;
    }

}
