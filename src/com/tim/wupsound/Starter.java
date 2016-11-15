package com.tim.wupsound;

import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;

public class Starter {
    public static void main(String[] args) {        
        System.out.println("WAV to btsnd 0.1 alpha");
        final byte[] btsnd_header = new byte[]{0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00};
        final byte[] wav_header1 = new byte[]{0x52,0x49,0x46,0x46};
        final byte[] wav_header2 = new byte[]{0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,0x10,0x00,0x00,0x00,0x01,0x00,0x02,0x00,(byte) 0x80,(byte) 0xBB,0x00,0x00,0x00,(byte) 0xEE,0x02,0x00,0x04,0x00,0x10,0x00,0x64,0x61,0x74,0x61};

        if(args.length == 0){
            exitWithError();
        }
        
        Path path = Paths.get(args[0]);
        try {
            byte[] data = Files.readAllBytes(path);
            ByteBuffer buffer = ByteBuffer.allocate(data.length);
            buffer.put(data);
            
            byte[] compare_buffer1 = new byte[wav_header1.length];            
            buffer.position(0);
            buffer.get(compare_buffer1);
            
            byte[] compare_buffer2 = new byte[wav_header2.length];            
            buffer.position(0x08);
            buffer.get(compare_buffer2);
            
            if(!( Arrays.equals(compare_buffer1,wav_header1) &&
                  Arrays.equals(compare_buffer2,wav_header2) ) ){
                exitWithError();
            }
            
            ByteBuffer output = ByteBuffer.allocate(data.length - 0x2C + btsnd_header.length);
            output.put(btsnd_header);
            for(int i = 0x2C;i<data.length;){
                short cur = buffer.getShort(i);
                output.putShort(swap(cur));
                i +=2;
            }
            
            FileOutputStream fos = new FileOutputStream("bootSound.btsnd");
            fos.write(output.array());
            if(args.length == 2){
            	System.out.println(args[1]);
                if(args[1].equals("-noloop")) {
                	byte[] silenceloop = new byte[]{0x00,0x00,0x00,0x00};
                	int loopStart = (((int)fos.getChannel().position() -8) / 4);
                	fos.write(silenceloop);
                	fos.getChannel().position(4);
                	fos.write(ByteBuffer.allocate(4).order(ByteOrder.BIG_ENDIAN).putInt(loopStart).array());
                }
            }
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();
            exitWithError();
        }
        System.out.println("Saved to bootSound.btsnd");
    }
    
    public static void exitWithError(){
        System.out.println("You need to provide a 48000khz 16bit stereo .wav as input");
        System.exit(0);
    }
    
    public static short swap (short value){
      int b1 = value & 0xff;
      int b2 = (value >> 8) & 0xff;

      return (short) (b1 << 8 | b2 << 0);
    }

}
