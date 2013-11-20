package com;
import java.io.*;

public final class MemoryStream 
{  
		//////////////////////////////////////////Serialize
		public static boolean Serialize(ByteArrayOutputStream output, byte val)
		{
			output.write(val);
			return true;
		}
		
		public static byte Deserialize(ByteArrayInputStream input, byte val)
		{
			val = (byte)input.read();
			return val;
		}
		
		public static boolean Serialize(ByteArrayOutputStream output, boolean val)
		{
			output.write((byte)(val ? 1 : 0));
			return true;
		}
		
		public static boolean Deserialize(ByteArrayInputStream input, boolean val)
		{
			val = (input.read() == 1) ? true : false;
			return val;
		}
		
		public static boolean Serialize(ByteArrayOutputStream output, short val)
		{
			output.write((byte)(val>>8));
			output.write((byte)(val));
			return true;
		}

		public static short Deserialize(ByteArrayInputStream input, short val)
		{
			int b1 = input.read();
			int b2 = input.read();
			val = (short)((b1<<8) | b2);
			return val;
		}
		
		public static boolean Serialize(ByteArrayOutputStream output, char val)
		{
			return Serialize(output, (short)val);
		}
		
		public static char Deserialize(ByteArrayInputStream input, char val)
		{
			short v = 0;
			v = Deserialize(input, v);
			val = (char)v;
			return val;
		}


		public static boolean Serialize(ByteArrayOutputStream output, int val)
		{
			output.write((byte)(val>>24));
			output.write((byte)(val>>16));
			output.write((byte)(val>>8));
			output.write((byte)(val));
			return true;
		}

		public static int Deserialize(ByteArrayInputStream input, int val)
		{
			int b1 = (int)input.read();
			int b2 = (int)input.read();
			int b3 = (int)input.read();
			int b4 = (int)input.read();
			val = (int)((b1<<24)|(b2<<16)|(b3<<8)|b4);
			return val;
		}
		
	
		public static boolean Serialize(ByteArrayOutputStream output, long val)
		{
			output.write((byte) (val >> 56));
			output.write((byte) (val >> 48));
			output.write((byte) (val >> 40));
			output.write((byte) (val >> 32));
			output.write((byte) (val >> 24));
			output.write((byte) (val >> 16));
			output.write((byte) (val >> 8));
			output.write((byte) (val));
			return true;
		}

		public static long Deserialize(ByteArrayInputStream input, long val)
		{
			long b1 = (long)input.read();
			long b2 = (long)input.read();
			long b3 = (long)input.read();
			long b4 = (long)input.read();
			long b5 = (long)input.read();
			long b6 = (long)input.read();
			long b7 = (long)input.read();
			long b8 = (long)input.read();
			val = ((b1 << 56) | (b2 << 48) | (b3 << 40) | (b4 << 32) 
				| (b5 << 24) | (b6 << 16) | (b7 << 8) | b8);
			
			return val;
		}

		public static boolean Serialize(ByteArrayOutputStream output, float val)
		{
			return Serialize(output, Float.floatToRawIntBits(val));
		}

		public static float Deserialize(ByteArrayInputStream input, float val)
		{
			int v = 0;
			v = Deserialize(input, v);
			val = Float.intBitsToFloat(v);
			return val;
		}
		
		public static boolean Serialize(ByteArrayOutputStream output, double val)
		{
			return Serialize(output, Double.doubleToRawLongBits(val));
		}

		public static double Deserialize(ByteArrayInputStream input, double val)
		{
			long v = 0;
			v = Deserialize(input, v);
			val = Double.longBitsToDouble(v);	
			
			return val;
		}
		
		public static boolean Serialize(ByteArrayOutputStream output, byte[] val)
		{
			Serialize(output, val.length);
			output.write(val, 0, val.length);

			return true;
		}
		
		public static byte[] Deserialize(ByteArrayInputStream input, byte[] val)
		{
			int leng = 0;
			leng = Deserialize(input, leng);
			val = new byte[leng];
			input.read(val, 0, leng);
	
			return val;
		}
		
		public static boolean Serialize(ByteArrayOutputStream output, String val)
		{
			byte[] rawBytes = val.getBytes();
			
			Serialize(output, val.length() + 1);
			output.write(rawBytes, 0, val.length());
			output.write(0);

			return true;
		}

		public static String Deserialize(ByteArrayInputStream input, String val)
		{
			int leng = 0;
			leng = Deserialize(input, leng);
			byte[] rawBytes = new byte[leng - 1];
			input.read(rawBytes, 0, leng - 1);
			//skip one byte
			input.read();

			val = new String(rawBytes);
	
			return val;
		}
		
}

