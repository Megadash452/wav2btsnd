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
    	//btsnd is BIG_ENDIAN wav is LITTLE_ENDIAN     
        System.out.println("WAV to btsnd 0.1 alpha");
        final byte[] wav_header1 = new byte[]{0x52,0x49,0x46,0x46};
        final byte[] wav_header2 = new byte[]{0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20
        ,0x10,0x00,0x00,0x00,0x01,0x00,0x02,0x00,(byte) 0x80,(byte) 0xBB,0x00,0x00,0x00
        ,(byte) 0xEE,0x02,0x00,0x04,0x00,0x10,0x00,0x64,0x61,0x74,0x61};
        
        if(args.length == 0){
        	System.out.println("Usage:");
        	System.out.println("java -jar wav2btsnd.jar -in <infile> <optional args>");
        	System.out.println("Optional Args:");
        	System.out.println("-out <outfile> specifies a location to create the converted file, defualt is a bootSound.btsnd created in the same directory as this jar file.");
        	System.out.println("-makeWav converts a btsnd to a wav, instead of a wav to a btsnd.");
        	System.out.println("the following commands are only for making btsnds");
        	System.out.println("-loopPoint <sampleforlooping> specifies a specific sample to loop from there to the end, once playthrough of the btsnd has finished once. (cant be used with -noLoop.)");
        	System.out.println("-noLoop makes it where the btsnd doesnt loop its sound. (cant be used with -loopPoint.)");
        	System.out.println("-gamepadOnly makes sound only hearable on gamepad(BUGGY?). (cant be used with -tvOnly.)");
        	System.out.println("-tvOnly makes sound only hearable on tv(sound plays twice and then stops no matter what?). (cant be used with -gamepadOnly.)");
        	System.exit(0);
        	
        }
        String inPath = null;
        String outPath = "bootSound";
        boolean makeBtsnd = true;
        int hearableWhere = 2;
        int loopPoint = 0;
        boolean silentLoop = false;
        for(int i=0;i<args.length;i++) {
        	String currentArg = args[i];
    		if(currentArg.equals("-in")) {
    			i++;
    			inPath = args[i];
    		} else if(currentArg.equals("-out")) {
    			i++;
    			outPath = args[i];
    		} else if(currentArg.equals("-makeWav")) {
    			makeBtsnd = false;
    		} else if(currentArg.equals("-loopPoint")) {
    			i++;
    			loopPoint = Integer.parseInt(args[i]);
    		} else if(currentArg.equals("-noLoop")) {
    			silentLoop = true;
    		} else if(currentArg.equals("-gamepadOnly")) {
    			hearableWhere = 1;
    		} else if(currentArg.equals("-tvOnly")) {
    			hearableWhere = 0;
    		}
        }
        //error checking for the args
        if(inPath == null) {
        	exitWithError("noinfile");
        }
        if(silentLoop == true && loopPoint > 0) {
        	exitWithError("multipleloopargs");
        }
        if((silentLoop == true || loopPoint > 0) && makeBtsnd == false) {
        	exitWithError("invalidargsformakewav");
        }
        if(makeBtsnd) {
        	if(!outPath.endsWith(".btsnd")) {
        		outPath += ".btsnd";
        	}
	        Path path = Paths.get(inPath);
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
	                  Arrays.equals(compare_buffer2,wav_header2) )){
	                exitWithError("badinfile");
	            }
	            
	            ByteBuffer output = ByteBuffer.allocate(data.length - 0x2C + 8);
	            output.put(ByteBuffer.allocate(4).order(ByteOrder.BIG_ENDIAN).putInt(hearableWhere).array());
	            if(silentLoop) {
	            	loopPoint = (((data.length - 0x2C) / 4) + 1);
	            }
	            output.put(ByteBuffer.allocate(4).order(ByteOrder.BIG_ENDIAN).putInt(loopPoint).array());
	            for(int i = 0x2C;i<data.length;i+=2){
	                short cur = buffer.order(ByteOrder.LITTLE_ENDIAN).getShort(i);
	                output.putShort(cur);
	            }
	            
	            FileOutputStream fos = new FileOutputStream(outPath);
	            fos.write(output.array());
	            if(silentLoop){
                	byte[] silenceloop = new byte[]{0x00,0x00,0x00,0x00};
                	fos.write(silenceloop);
	            }
	            fos.close();
	        } catch (IOException e) {
	            e.printStackTrace();
	            exitWithError("nonexistinfile");
	        }
	        System.out.println("Saved to " + outPath);
	    } else {
        	if(!outPath.endsWith(".wav")) {
        		outPath += ".wav";
        	}
		    Path path = Paths.get(inPath);
	        try {
	            byte[] data = Files.readAllBytes(path);
	            ByteBuffer buffer = ByteBuffer.allocate(data.length);
	            buffer.put(data);
	            
	            ByteBuffer output = ByteBuffer.allocate(data.length - 8 + 0x2C);
	            output.put(wav_header1);
	            output.put(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(data.length - 8 + 0x2C).array());
	            output.put(wav_header2);
	            output.put(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(data.length - 8).array());
	            for(int i = 8;i<data.length;i+=2){
	                short cur = buffer.order(ByteOrder.BIG_ENDIAN).getShort(i);
	                output.putShort(swap(cur));
	            }
	            FileOutputStream fos = new FileOutputStream(outPath);
	            fos.write(output.array());
	            fos.close();
	        } catch (IOException e) {
	            e.printStackTrace();
	            exitWithError("nonexistinfile");
	        }
	        System.out.println("Saved to " + outPath);
	    }
    }
    
    public static void exitWithError(String reason){
    	if(reason.equals("noinfile")) {
     		System.out.println("you must use -in and provide a file to convert!");
     		System.exit(0);
     	} else if(reason.equals("badinfile")) {
    		System.out.println("You need to provide a 48000khz 16bit stereo .wav as input.");
    		System.exit(0);
    	} else if(reason.equals("multipleloopargs")) {
    		System.out.println("You cant provide 2 loop arguments!");
    		System.exit(0);
    	} else if(reason.equals("nonexistinfile")) {
    		System.out.println("In-file you provided doesnt exist!");
    		System.exit(0);
    	} else if(reason.equals("invalidargsformakewav")) {
    		System.out.println("you cant use -loopPoint or -noLoop with -makeWav!");
    		System.exit(0);
    	}
    }
    
    public static short swap (short value){
      int b1 = value & 0xff;
      int b2 = (value >> 8) & 0xff;

      return (short) (b1 << 8 | b2 << 0);
    }

}
