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
			System.out.println("This program only supports RIFF wav files.");
        	System.out.println("Usage:");
        	System.out.println("java -jar wav2btsnd.jar -in <infile> <optional args>");
        	System.out.println("The program will automatically detect the type of input file and choose which type of file to output (E.g.: .wav input will automatically output .btsnd file");
			System.out.println();
        	System.out.println("Optional Args:");
        	System.out.println("  -out <outfile>: File path to create the converted file in. Default is name of input file + appropriate file extension in the same directory as the program.");
			System.out.println();
			System.out.println("  -makeWav:       Convert .btsnd to .wav. (Can be used to Force Convert)");
			System.out.println("      Incompatible with: -makeBtsnd");
			System.out.println();
			System.out.println("  -makeBtsnd:     Convert .wav to .btsnd. (Can be used to Force Convert)");
			System.out.println("      Incompatible with: -makeWav");
			System.out.println();
        	System.out.println("These args are only for making btsnd's:");
        	System.out.println("  -loopPoint <loop_sample>: (int) Specifies a specific sample to loop from there to the end, once play-through of the btsnd has finished once.");
			System.out.println("      Incompatible with: -noLoop, -makeWav");
			System.out.println();
			System.out.println("  -noLoop:        Disables the looping for btsnd.");
			System.out.println("      Incompatible with: -loopPoint, -makeWav");
			System.out.println();
			System.out.println("  -gamepadOnly:   Makes sound only hearable on gamepad.");
			System.out.println("      Incompatible with: -tvOnly");
			System.out.println();
			System.out.println("  -tvOnly:        Makes sound only hearable on TV (main console).");
			System.out.println("      Incompatible with: -gamepadOnly");
        	System.exit(0);
        }

        String inPath = "";
        String outPath = "";
        boolean makeBtsnd = true;
		boolean usedMake = false; // tells if args -makeWav or -makeBtsnd were used by the user
        int hearableWhere = 2;
        int loopPoint = 0;
        boolean silentLoop = false;

        for(int i=0;i<args.length;i++) {
        	String currentArg = args[i];

			switch (currentArg) {
				case "-in" -> {
					i++;
					inPath = args[i];

					if (inPath.endsWith(".btsnd"))
						makeBtsnd = false;

					if (outPath.isEmpty())
						outPath = file_no_ext(inPath);
				}
				case "-out" -> {
					i++;
					outPath = args[i];
				}
				case "-makeWav" -> {
					if (usedMake)
						exitWithError(errorCode.multipleMakeArgs);

					makeBtsnd = false;
					usedMake = true;
				}
				case "-makeBtsnd" -> {
					if (usedMake)
						exitWithError(errorCode.multipleMakeArgs);

					makeBtsnd = true;
					usedMake = true;
				}
				case "-loopPoint" -> {
					i++;
					loopPoint = Integer.parseInt(args[i]);
				}
				case "-noLoop" ->
					silentLoop = true;
				case "-gamepadOnly" ->
					hearableWhere = 1;
				case "-tvOnly" ->
					hearableWhere = 0;
			}
        }
        //error checking for the args
        if(inPath.isEmpty()) {
        	exitWithError(errorCode.noInFile);
        }
        if(silentLoop && loopPoint > 0) {
        	exitWithError(errorCode.multipleLoopArgs);
        }
        if((silentLoop || loopPoint > 0) && !makeBtsnd) {
        	exitWithError(errorCode.invalidArgsForMakeWav);
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
	                exitWithError(errorCode.badInFile);
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
	            exitWithError(errorCode.noneExistInFile);
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
	            exitWithError(errorCode.noneExistInFile);
	        }
	        System.out.println("Saved to " + outPath);
	    }
    }

	public enum errorCode {
		noInFile, badInFile,
		multipleLoopArgs,
		multipleMakeArgs,
		noneExistInFile, invalidArgsForMakeWav
	}
    
    public static void exitWithError(errorCode reason){
		switch (reason) {
			case noInFile -> {
				System.out.println("Must use argument -in to provide input file");
				System.exit(0);
			}
			case badInFile -> {
				System.out.println("Input .WAV file must be 48000khz (DAT) 16bit stereo");
				System.exit(0);
			}
			case multipleLoopArgs -> {
				System.out.println("You cant provide 2 loop arguments!");
				System.exit(0);
			}
			case multipleMakeArgs -> {
				System.out.println("Can't use -makeWav and -makeBtsnd together");
				System.exit(0);
			}
			case noneExistInFile -> {
				System.out.println("Cannot open input file. Check if it exists or if the relative path is correct.");
				System.exit(0);
			}
			case invalidArgsForMakeWav -> {
				System.out.println("Can't use -loopPoint and -noLoop when the output is .wav");
				System.exit(0);
			}
		}
    }

	/** Get the full name of a file (may include path depending on input) and removes the file extension.
	    E.G.: C:/dir/file.txt -> C:/dir/file
	  * @param path The path to the file (absolute or relative)
	  * @return File path without extension */
	public static String file_no_ext(String path) {
		int end = path.length();

		// find the dot of the file extension
		for (int i = path.length() - 1; i >= 0; i--)
		{
			if (path.charAt(i) == '.')
			{
				end = i;
				break;
			}
		}

		return path.substring(0, end);
	}
    
    public static short swap (short value){
		  int b1 = value & 0xff;
		  int b2 = (value >> 8) & 0xff;

		  return (short)(b1 << 8 | b2);
    }

}
